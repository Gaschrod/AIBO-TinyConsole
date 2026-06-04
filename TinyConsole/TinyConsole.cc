//
// TinyConsole.cc
// Minimal OPEN-R TCP console with ChaCha20+Poly1305 AEAD and ERS-7 leg motion.
//
// Motion commands are handled asynchronously: the TCP response is sent
// immediately, and the motion itself runs via the Ready() / subject callback
// loop driven by the OPEN-R scheduler.
//
// Gait implementation notes (ERS-7 joint convention):
//   J1 hip abduction/adduction   (large value = leg rotated outward)
//   J2 hip flexion/extension     (higher = more forward, range 10  88°)
//   J3  knee flexion               (higher = more bent = foot lifted)
//
//   Walking direction depends on which end of the J2 range the "planted"
//   foot sits at.  All gait angles below are INITIAL ESTIMATES derived from
//   the ERS-7 Model Information document; empirical tuning on hardware is
//   expected and the architecture makes that straightforward.
//
//   Joint layout used throughout (index 0-11):
//     0-2  : RF  J1, J2, J3   (Right Front,  r4/c1â€¦)
//     3-5  : LF  J1, J2, J3   (Left  Front,  r2/c1â€¦)
//     6-8  : RR  J1, J2, J3   (Right Rear,   r5/c1â€¦)
//     9-11 : LR  J1, J2, J3   (Left  Rear,   r3/c1â€¦)
//

#include <math.h>
#include <string.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OUnits.h>
#include <OPENR/OSyslog.h>
#include <OPENR/ODebug.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "TinyConsole.h"

// ================================================================
//  Static member definitions
// ================================================================

uint32_t TinyConsole::sessionId_ = 0;

// ================================================================
//  Joint locators  (ERS-7, Appendix A.1 of Model Information)
// ================================================================

static const char* const JOINT_LOCATOR[TinyConsole::NUM_JOINTS] = {
    "PRM:/r4/c1-Joint2:41",         // RF J1
    "PRM:/r4/c1/c2-Joint2:42",      // RF J2
    "PRM:/r4/c1/c2/c3-Joint2:43",   // RF J3

    "PRM:/r2/c1-Joint2:21",         // LF J1
    "PRM:/r2/c1/c2-Joint2:22",      // LF J2
    "PRM:/r2/c1/c2/c3-Joint2:23",   // LF J3

    "PRM:/r5/c1-Joint2:51",         // RR J1
    "PRM:/r5/c1/c2-Joint2:52",      // RR J2
    "PRM:/r5/c1/c2/c3-Joint2:53",   // RR J3

    "PRM:/r3/c1-Joint2:31",         // LR J1
    "PRM:/r3/c1/c2-Joint2:32",      // LR J2
    "PRM:/r3/c1/c2/c3-Joint2:33",   // LR J3
};

// ================================================================
//  Pose tables  (all angles in degrees)
//
//  Layout: { RF_J1, RF_J2, RF_J3,
//             LF_J1, LF_J2, LF_J3,
//             RR_J1, RR_J2, RR_J3,
//             LR_J1, LR_J2, LR_J3 }
//
//  ERS-7 software limits (Model Information 2.1.1):
//    Left  J1: 115  130    Right J1: 130  115
//    All   J2:10  88     All   J3: 25  122
// ================================================================

// Broadbase upright, legs spread in stable stance.
static const double BROADBASE_POSE[12] = {
     0, 4, 30,   // RF
     0, 4, 30,   // LF
     0, 4, 30,   // RR
     0, 4, 30,   // LR
};

// Sleeping body low, rear knees tucked (J3=122 fully bent).
static const double SLEEPING_POSE[12] = {
      59,  0,  30,  // RF
      59,  0,  30,  // LF
    -110,  15, 122,  // RR
    -110,  15, 122,  // LR
};

// ----------------------------------------------------------------
//  Forward trot diagonal pairs (RF+LR) and (LF+RR).
//
//  Phase A: RF and LR are the SWING legs (lifted, J2 retracted).
//           LF and RR are the STANCE legs (grounded, J2 near max).
//
//  Phase B: LF and RR become the swing legs; RF and LR become stance.
//
//  Each phase interpolates from the previous pose to the new target;
//  the robot body net advances forward because the swing legs plant
//  further forward (higher J2) when they land at the broadbase
//  position between phases.
//
//  Tuning knobs:
//    Swing  J2 (72 / 52): decrease to lengthen stride, min ~60 / 40
//    Lift   J3 (55 / 50): increase for higher foot clearance
//    Stance J2 (86 / 68): keep close to broadbase max (88 / 70)
// ----------------------------------------------------------------

static const double WALK_FWD_A[12] = {
     0, 4, 55,   // RF  swing : J2 retracted + J3 lifted
     0, 4, 24,   // LF  stance: J2 extended,  J3 low
     0, 4, 50,   // RR  swing : J2 retracted + J3 lifted
     0, 4, 24,   // LR  stance: J2 extended,  J3 low
};

static const double WALK_FWD_B[12] = {
     0, 4, 24,   // RF  stance
     0, 4, 55,   // LF  swing
     0, 4, 24,   // RR  stance
     0, 4, 50,   // LR  swing
};

// ----------------------------------------------------------------
//  Backward trot roles reversed: swing legs start extended (J2
//  near max) and plant further back, pulling the body rearward.
// ----------------------------------------------------------------

static const double WALK_BACK_A[12] = {
     0, 4, 55,   // RF  swing : J2 extended  + J3 lifted
     0, 4, 24,   // LF  stance: J2 retracted, J3 low
     0, 4, 50,   // RR  swing : J2 extended  + J3 lifted
     0, 4, 24,   // LR  stance: J2 retracted, J3 low
};

static const double WALK_BACK_B[12] = {
     110, 4, 24,   // RF  stance
     110, 4, 55,   // LF  swing
    -110, 4, 24,   // RR  stance
    -110, 4, 50,   // LR  swing
};

// ================================================================
//  Constructor
// ================================================================

TinyConsole::TinyConsole()
    : txCounter_(0),
      rxCounter_(0),
      pendingClose_(false),
      recvPhase_(0),
      pendingFrameLen_(0),
      motionState_(MSTATE_IDLE),
      pendingCmd_(MCMD_NONE),
      motionCounter_(0),
      walkForward_(true)
{
    conn_.state = CONNECTION_CLOSED;
    memset(sessionNonce_, 0, sizeof(sessionNonce_));
    memset(motionStart_,  0, sizeof(motionStart_));
    memset(motionDelta_,  0, sizeof(motionDelta_));

    for (int i = 0; i < NUM_CMD_VECTORS; i++)
        cmdRegion_[i] = 0;
}

// ================================================================
//  OPEN-R lifecycle
// ================================================================

OStatus
TinyConsole::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyConsole::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER;
    REGISTER_ALL_ENTRY;
    SET_ALL_READY_AND_NOTIFY_ENTRY;

	OpenPrimitives();
    NewCommandVectorData();


	OPENR::SetMotorPower(opowerON);
    
    return oSUCCESS;
}

OStatus
TinyConsole::DoStart(const OSystemEvent& event)
{
	OSYSDEBUG(("TinyConsole::DoStart()\n"));

    ipstackRef_ = antStackRef("IPStack");

    antEnvCreateSharedBufferMsg sendBufMsg(CONSOLE_BUFSIZE);
    sendBufMsg.Call(ipstackRef_, sizeof(sendBufMsg));
    if (sendBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc send buffer %d", sendBufMsg.error));
        return oFAIL;
    }
    conn_.sendBuffer = sendBufMsg.buffer;
    conn_.sendBuffer.Map();
    conn_.sendData = (byte*)conn_.sendBuffer.GetAddress();

    antEnvCreateSharedBufferMsg recvBufMsg(CONSOLE_BUFSIZE);
    recvBufMsg.Call(ipstackRef_, sizeof(recvBufMsg));
    if (recvBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc recv buffer %d", recvBufMsg.error));
        return oFAIL;
    }
    conn_.recvBuffer = recvBufMsg.buffer;
    conn_.recvBuffer.Map();
    conn_.recvData = (byte*)conn_.recvBuffer.GetAddress();
    


    //if (subject[sbjMove]->IsReady() == true) {
    //    double cur[NUM_JOINTS];
    //    ReadCurrentPose(cur);
    //    SetupInterpolation(cur, cur, 1);

    //    RCRegion* rgn = FindFreeRegion();
    //    if (rgn) { // Added null check for safety
    //        for (int i = 0; i < NUM_JOINTS; i++)
    //            SetJointValue(rgn, i, cur[i], cur[i]);
    //        subject[sbjMove]->SetData(rgn);
    //        subject[sbjMove]->NotifyObservers();
    //    }
    //    motionState_ = MSTATE_ADJUSTING;
    //} else {
    //    motionState_ = MSTATE_IDLE;
    //}
    if (subject[sbjMove]->IsReady() == true) {
		SetJointGain();
	}
    
    motionState_ = MSTATE_IDLE;

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return Listen();
}

OStatus
TinyConsole::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyConsole::DoStop()\n"));

    motionState_ = MSTATE_IDLE;
    pendingCmd_  = MCMD_NONE;

    DISABLE_ALL_SUBJECT;
    DEASSERT_READY_TO_ALL_OBSERVER;

    return Close();
}

OStatus
TinyConsole::DoDestroy(const OSystemEvent& event)
{
    DELETE_ALL_SUBJECT_AND_OBSERVER;
    return oSUCCESS;
}

// ================================================================
//  OPEN-R subject Ready callback
//
//  Called by the OPEN-R scheduler each time the actuator observer
//  has consumed the last command-vector frame we sent.  This drives
//  the motion state machine one step forward.
//
//  A pending TCP command (pendingCmd_) is applied at the start of
//  each callback so that new commands interrupt walking on the next
//  phase boundary clean and glitch-free.
// ================================================================

void
TinyConsole::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("TinyConsole::Ready() state=%d\n", (int)motionState_));

    // ---- Apply any pending TCP command --------------------------------
    if (pendingCmd_ != MCMD_NONE) {
        MotionCmd cmd = pendingCmd_;
        pendingCmd_   = MCMD_NONE;
        BeginMotion(cmd);
        // Fall through so the switch below sends the very first frame.
    }

    // ---- Advance the current motion phase -----------------------------
    switch (motionState_) {

        case MSTATE_IDLE:
            // Nothing to do; we won't get another Ready() until
            // TriggerReady() sends a frame for the next command.
            break;

        case MSTATE_ADJUSTING:
            // The startup "stay put" frame was consumed; go idle.
            motionState_ = MSTATE_IDLE;
            break;

        case MSTATE_GETUP:
            if (AdvanceInterpolation(GET_UP_COUNTER) == MOVING_FINISH)
                motionState_ = MSTATE_IDLE;
            break;

        case MSTATE_REST:
            if (AdvanceInterpolation(REST_COUNTER) == MOVING_FINISH)
                motionState_ = MSTATE_IDLE;
            break;

        // Walk phases alternate: A  B A B  until interrupted.
        case MSTATE_WALK_A:
            if (AdvanceInterpolation(WALK_PHASE_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward_ ? WALK_FWD_A  : WALK_BACK_A;
                const double* to   = walkForward_ ? WALK_FWD_B  : WALK_BACK_B;
                SetupInterpolation(from, to, WALK_PHASE_COUNTER);
                motionState_ = MSTATE_WALK_B;
            }
            break;

        case MSTATE_WALK_B:
            if (AdvanceInterpolation(WALK_PHASE_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward_ ? WALK_FWD_B  : WALK_BACK_B;
                const double* to   = walkForward_ ? WALK_FWD_A  : WALK_BACK_A;
                SetupInterpolation(from, to, WALK_PHASE_COUNTER);
                motionState_ = MSTATE_WALK_A;
            }
            break;

        case MSTATE_TO_BASE:
            if (AdvanceInterpolation(STOP_COUNTER) == MOVING_FINISH)
                motionState_ = MSTATE_IDLE;
            break;
    }
}

// ================================================================
//  ANT extra-entry callbacks
// ================================================================

void
TinyConsole::ListenCont(ANTENVMSG msg)
{
    TCPEndpointListenMsg* listenMsg =
        (TCPEndpointListenMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::ListenCont() lAddr=%x lPort=%d fAddr=%x fPort=%d\n",
               listenMsg->lAddress.Address(), listenMsg->lPort,
               listenMsg->fAddress.Address(), listenMsg->fPort));

    if (listenMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::ListenCont FAILED %d",
                  listenMsg->error));
        Close();
        return;
    }

    conn_.state   = CONNECTION_CONNECTED;
    pendingClose_ = false;
    recvPhase_    = 0;

    // Build 12-byte session nonce:
    //   [0..3]  session ID (LE) monotone per boot, resets on reboot
    //   [4..7]  client IPv4   (LE)
    //   [8..11] per-message counter, written by AdvanceNonce()
    uint32_t sid  = (uint32_t)sessionId_++;
    uint32_t addr = listenMsg->fAddress.Address();

    sessionNonce_[0] = (uint8_t)( sid         & 0xFF);
    sessionNonce_[1] = (uint8_t)((sid >>  8)  & 0xFF);
    sessionNonce_[2] = (uint8_t)((sid >> 16)  & 0xFF);
    sessionNonce_[3] = (uint8_t)((sid >> 24)  & 0xFF);
    sessionNonce_[4] = (uint8_t)( addr        & 0xFF);
    sessionNonce_[5] = (uint8_t)((addr >>  8) & 0xFF);
    sessionNonce_[6] = (uint8_t)((addr >> 16) & 0xFF);
    sessionNonce_[7] = (uint8_t)((addr >> 24) & 0xFF);
    sessionNonce_[8] = sessionNonce_[9] = sessionNonce_[10] = sessionNonce_[11] = 0;

    txCounter_ = 0;
    rxCounter_ = 0;

    // Send plaintext banner + raw nonce (26 bytes total).
    const char* banner    = CONSOLE_BANNER;
    int         bannerLen = (int)strlen(banner);

    memcpy(conn_.sendData,             banner,        bannerLen);
    memcpy(conn_.sendData + bannerLen, sessionNonce_, NONCE_SIZE);
    conn_.sendSize = bannerLen + NONCE_SIZE;

    TCPEndpointSendMsg sendMsg(conn_.endpoint,
                               conn_.sendData, conn_.sendSize);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef_, myOID_,
                 Extra_Entry[entrySendCont], sizeof(sendMsg));
    conn_.state    = CONNECTION_SENDING;
    conn_.sendSize = 0;
}

void
TinyConsole::SendCont(ANTENVMSG msg)
{
    TCPEndpointSendMsg* sendMsg =
        (TCPEndpointSendMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::SendCont()\n"));

    if (sendMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::SendCont FAILED %d",
                  sendMsg->error));
        Close();
        return;
    }

    conn_.state = CONNECTION_CONNECTED;

    if (pendingClose_) {
        pendingClose_ = false;
        Close();
        return;
    }

    recvPhase_ = 0;
    Receive(FRAME_HEADER_SIZE, FRAME_HEADER_SIZE);
}

void
TinyConsole::ReceiveCont(ANTENVMSG msg)
{
    TCPEndpointReceiveMsg* recvMsg =
        (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::ReceiveCont() phase=%d n=%d\n",
               recvPhase_, recvMsg->sizeMin));

    if (recvMsg->error == TCP_CONNECTION_CLOSED) {
        OSYSPRINT(("TinyConsole: client disconnected\n"));
        Close();
        return;
    }
    if (recvMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::ReceiveCont FAILED %d",
                  recvMsg->error));
        Close();
        return;
    }

    conn_.state = CONNECTION_CONNECTED;
    int n = recvMsg->sizeMin;

    // ------------------------------------------------------------------
    //  Phase 0 2-byte frame header
    // ------------------------------------------------------------------
    if (recvPhase_ == 0) {
        if (n < FRAME_HEADER_SIZE) {
            OSYSLOG1((osyslogERROR,
                      "TinyConsole: short frame header (%d bytes)", n));
            Close();
            return;
        }

        uint16_t ctLen = (uint16_t)( (unsigned char)conn_.recvData[0]        )
                       | (uint16_t)(((unsigned char)conn_.recvData[1]) << 8  );

        if (ctLen == 0 || ctLen > (uint16_t)CONSOLE_MAX_PLAINTEXT) {
            OSYSLOG1((osyslogERROR,
                      "TinyConsole: bad frame ciphertext length %u", ctLen));
            Close();
            return;
        }

        pendingFrameLen_ = ctLen;
        recvPhase_       = 1;
        Receive((int)ctLen + FRAME_TAG_SIZE, (int)ctLen + FRAME_TAG_SIZE);
        return;
    }

    // ------------------------------------------------------------------
    //  Phase 1 AEAD body (ciphertext + 16-byte tag)
    // ------------------------------------------------------------------
    int bodyLen = (int)pendingFrameLen_ + FRAME_TAG_SIZE;
    if (n < bodyLen) {
        OSYSLOG1((osyslogERROR,
                  "TinyConsole: short frame body (got %d, expected %d)",
                  n, bodyLen));
        Close();
        return;
    }

    int ptLen = 0;
    if (!AeadDecrypt(conn_.recvData, bodyLen, conn_.recvData, &ptLen)) {
        Close();
        return;
    }

    if (ptLen < CONSOLE_BUFSIZE) conn_.recvData[ptLen] = '\0';

    // Strip trailing CR/LF in-place.
    char* cmd    = (char*)conn_.recvData;
    int   cmdLen = ptLen;
    while (cmdLen > 0 &&
           (cmd[cmdLen-1] == '\r' || cmd[cmdLen-1] == '\n'))
        cmd[--cmdLen] = '\0';

    // ------------------------------------------------------------------
    //  Command dispatch
    //
    //  Motion commands are ASYNCHRONOUS: we respond immediately and
    //  set pendingCmd_.  The motion state machine picks it up at the
    //  next Ready() call (triggered by TriggerReady() if currently idle).
    // ------------------------------------------------------------------
    const char* response = "OK\n";

    if (cmdLen == 0) {
        response = "> ";

    } else if (!strncmp(cmd, "PING", 4)) {
        response = "PONG\n";

    } else if (!strncmp(cmd, "GET_UP", 6)) {
        pendingCmd_ = MCMD_GETUP;
        TriggerReady();
        response = "STANDING_UP\n";

    } else if (!strncmp(cmd, "REST", 4)) {
        pendingCmd_ = MCMD_REST;
        TriggerReady();
        response = "RESTING\n";

    } else if (!strncmp(cmd, "FORWARD", 7)) {
        pendingCmd_ = MCMD_FORWARD;
        TriggerReady();
        response = "MOVING_FORWARD\n";

    } else if (!strncmp(cmd, "BACK", 4)) {
        pendingCmd_ = MCMD_BACK;
        TriggerReady();
        response = "MOVING_BACK\n";

    } else if (!strncmp(cmd, "STOP", 4)) {
        pendingCmd_ = MCMD_STOP;
        TriggerReady();
        response = "STOPPING\n";

    } else if (!strncmp(cmd, "QUIT", 4)) {
        response      = "BYE\n";
        pendingClose_ = true;
    }

    int frameLen = 0;
    if (!AeadEncrypt((const byte*)response, (int)strlen(response),
                     conn_.sendData, &frameLen)) {
        OSYSLOG1((osyslogERROR, "TinyConsole: AeadEncrypt failed"));
        Close();
        return;
    }

    Send(conn_.sendData, frameLen);
}

void
TinyConsole::CloseCont(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg =
        (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::CloseCont()\n"));

    conn_.state = CONNECTION_CLOSED;
    Listen();
}

// ================================================================
//  Private TCP helpers
// ================================================================

OStatus
TinyConsole::Listen()
{
    OSYSDEBUG(("TinyConsole::Listen()\n"));

    if (conn_.state != CONNECTION_CLOSED) return oFAIL;

    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP, CONSOLE_BUFSIZE * 2);
    tcpCreateMsg.Call(ipstackRef_, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Listen endpoint FAIL %d",
                  tcpCreateMsg.error));
        return oFAIL;
    }
    conn_.endpoint = tcpCreateMsg.moduleRef;

    TCPEndpointListenMsg listenMsg(conn_.endpoint, IP_ADDR_ANY, CONSOLE_PORT);
    listenMsg.continuation = (void*)0;
    listenMsg.Send(ipstackRef_, myOID_,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));

    conn_.state = CONNECTION_LISTENING;
    OSYSPRINT(("TinyConsole: listening on port %d\n", CONSOLE_PORT));
    return oSUCCESS;
}

OStatus
TinyConsole::Send(const byte* /*data*/, int size)
{
    if (conn_.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Send called in wrong state %d",
                  conn_.state));
        return oFAIL;
    }

    TCPEndpointSendMsg sendMsg(conn_.endpoint, conn_.sendData, size);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef_, myOID_,
                 Extra_Entry[entrySendCont],
                 sizeof(TCPEndpointSendMsg));

    conn_.state    = CONNECTION_SENDING;
    conn_.sendSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Receive(int sizeMin, int sizeMax)
{
    if (conn_.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Receive called in wrong state %d",
                  conn_.state));
        return oFAIL;
    }

    TCPEndpointReceiveMsg recvMsg(conn_.endpoint,
                                  conn_.recvData, sizeMin, sizeMax);
    recvMsg.continuation = (void*)0;
    recvMsg.Send(ipstackRef_, myOID_,
                 Extra_Entry[entryReceiveCont], sizeof(recvMsg));

    conn_.state    = CONNECTION_RECEIVING;
    conn_.recvSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Close()
{
    if (conn_.state == CONNECTION_CLOSED ||
        conn_.state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(conn_.endpoint);
    closeMsg.continuation = (void*)0;
    closeMsg.Send(ipstackRef_, myOID_,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    conn_.state = CONNECTION_CLOSING;
    return oSUCCESS;
}

// ================================================================
//  AEAD helpers  (unchanged from original TinyConsole)
// ================================================================

void
TinyConsole::AdvanceNonce(uint32_t& counter)
{
    uint32_t mc = counter++;
    sessionNonce_[8]  = (uint8_t)( mc        & 0xFF);
    sessionNonce_[9]  = (uint8_t)((mc >>  8) & 0xFF);
    sessionNonce_[10] = (uint8_t)((mc >> 16) & 0xFF);
    sessionNonce_[11] = (uint8_t)((mc >> 24) & 0xFF);
}

bool
TinyConsole::AeadEncrypt(const byte* plaintext, int ptLen,
                          byte* frameOut, int* frameLen)
{
    if (ptLen <= 0 || ptLen > CONSOLE_MAX_PLAINTEXT) {
        OSYSLOG1((osyslogERROR,
                  "TinyConsole::AeadEncrypt bad ptLen %d", ptLen));
        return false;
    }

    AdvanceNonce(txCounter_);

    chacha20poly1305_ctx ctx;
    rfc7539_init(&ctx, (uint8_t*)CHACHA_KEY, sessionNonce_);

    frameOut[0] = (uint8_t)( ptLen       & 0xFF);
    frameOut[1] = (uint8_t)((ptLen >> 8) & 0xFF);

    chacha20poly1305_encrypt(&ctx,
                              (uint8_t*)plaintext,
                              frameOut + FRAME_HEADER_SIZE,
                              (size_t)ptLen);

    rfc7539_finish(&ctx,
                   (int64_t)0,
                   (int64_t)ptLen,
                   frameOut + FRAME_HEADER_SIZE + ptLen);

    *frameLen = FRAME_HEADER_SIZE + ptLen + FRAME_TAG_SIZE;
    return true;
}

bool
TinyConsole::AeadDecrypt(const byte* frame, int frameLen,
                          byte* plaintext, int* ptLen)
{
    int ctLen = frameLen - FRAME_TAG_SIZE;
    if (ctLen <= 0) {
        OSYSLOG1((osyslogERROR,
                  "TinyConsole::AeadDecrypt bad frameLen %d", frameLen));
        return false;
    }

    AdvanceNonce(rxCounter_);

    chacha20poly1305_ctx ctx;
    rfc7539_init(&ctx, (uint8_t*)CHACHA_KEY, sessionNonce_);

    chacha20poly1305_decrypt(&ctx,
                              (uint8_t*)frame,
                              plaintext,
                              (size_t)ctLen);

    uint8_t expectedTag[FRAME_TAG_SIZE];
    rfc7539_finish(&ctx,
                   (int64_t)0,
                   (int64_t)ctLen,
                   expectedTag);

    if (!poly1305_verify(expectedTag,
                         (const unsigned char*)(frame + ctLen))) {
        OSYSLOG1((osyslogERROR,
                  "TinyConsole: Poly1305 tag mismatch â€” dropping frame"));
        memset(plaintext, 0, ctLen);
        *ptLen = 0;
        return false;
    }

    *ptLen = ctLen;
    return true;
}

// ================================================================
//  Motion helpers
// ================================================================

void
TinyConsole::OpenPrimitives()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OStatus result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID_[i]);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR,
                      "TinyConsole::OpenPrimitives() joint %d FAILED %d",
                      i, result));
        }
    }
}

void
TinyConsole::NewCommandVectorData()
{
    for (int i = 0; i < NUM_CMD_VECTORS; i++) {
        MemoryRegionID      cmdVecDataID;
        OCommandVectorData* cmdVecData;

        OStatus result = OPENR::NewCommandVectorData(NUM_JOINTS,
                                                      &cmdVecDataID,
                                                      &cmdVecData);
        if (result != oSUCCESS) {
            OSYSLOG1((osyslogERROR,
                      "TinyConsole::NewCommandVectorData() [%d] FAILED %d",
                      i, result));
            continue;
        }

        cmdRegion_[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                     cmdVecData->vectorInfo.offset,
                                     (void*)cmdVecData,
                                     cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_JOINTS);

        for (int j = 0; j < NUM_JOINTS; j++) {
            OCommandInfo* info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, jointID_[j], ocommandMAX_FRAMES);
        }
    }
}

// Set PID gains on all leg joints using ERS-7 standard values.
// Only call after subject[sbjMove]->IsReady() returns true.
void
TinyConsole::SetJointGain()
{
    // gain[j] = {PGAIN, IGAIN, DGAIN} for joint-within-leg index j (0=J1, 1=J2, 2=J3)
    static const word pgain[3] = { J1_PGAIN, J2_PGAIN, J3_PGAIN };
    static const word igain[3] = { J1_IGAIN, J2_IGAIN, J3_IGAIN };
    static const word dgain[3] = { J1_DGAIN, J2_DGAIN, J3_DGAIN };

    for (int leg = 0; leg < 4; leg++) {
        for (int j = 0; j < 3; j++) {
            int idx = leg * 3 + j;
            OPENR::EnableJointGain(jointID_[idx]);
            OPENR::SetJointGain(jointID_[idx],
                                pgain[j], igain[j], dgain[j],
                                PID_PSHIFT, PID_ISHIFT, PID_DSHIFT);
        }
    }
}

RCRegion*
TinyConsole::FindFreeRegion()
{
    for (int i = 0; i < NUM_CMD_VECTORS; i++) {
        if (cmdRegion_[i] && cmdRegion_[i]->NumberOfReference() == 1)
            return cmdRegion_[i];
    }
    return 0; // all regions currently in use
}

// Populate joint idx in region rgn with a smooth interpolation from
// startDeg to endDeg spread across ocommandMAX_FRAMES sub-frames.
void
TinyConsole::SetJointValue(RCRegion* rgn, int idx,
                            double startDeg, double endDeg)
{
    OCommandVectorData* cmdVecData = (OCommandVectorData*)rgn->Base();

    OCommandInfo* info = cmdVecData->GetInfo(idx);
    info->Set(odataJOINT_COMMAND2, jointID_[idx], ocommandMAX_FRAMES);

    OCommandData*        data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = endDeg - startDeg;
    for (int k = 0; k < ocommandMAX_FRAMES; k++) {
        double d = startDeg + delta * k / (double)ocommandMAX_FRAMES;
        jval[k].value = oradians(d);
        jval[k].padding = 0;
    }
}

// Initialise a linear interpolation from 'from[]' to 'to[]' over 'steps'
// command-vector frames.  Resets motionCounter_ to 0.
void
TinyConsole::SetupInterpolation(const double* from,
                                 const double* to,
                                 int           steps)
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        motionStart_[i] = from[i];
        motionDelta_[i] = (steps > 0) ? (to[i] - from[i]) / steps : 0.0;
    }
    motionCounter_ = 0;
}

// Send one command-vector block at the current step of the interpolation,
// advance motionCounter_, and notify observers.
// Returns MOVING_FINISH when the last step has been sent.
MovingResult
TinyConsole::AdvanceInterpolation(int maxSteps)
{
    RCRegion* rgn = FindFreeRegion();
    if (!rgn) {
        // Both regions in use (shouldn't happen in steady-state).
        OSYSLOG1((osyslogERROR, "TinyConsole::AdvanceInterpolation no free region"));
        return MOVING_CONT;
    }

    int step = motionCounter_;

    for (int i = 0; i < NUM_JOINTS; i++) {
        double cur = motionStart_[i] + motionDelta_[i] *  step;
        double nxt = motionStart_[i] + motionDelta_[i] * (step + 1);
        SetJointValue(rgn, i, cur, nxt);
    }

    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    motionCounter_++;
    return (motionCounter_ >= maxSteps) ? MOVING_FINISH : MOVING_CONT;
}

// Read the current joint positions from hardware into outDeg[12].
void
TinyConsole::ReadCurrentPose(double* outDeg)
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OJointValue jv;
        OPENR::GetJointValue(jointID_[i], &jv);
        outDeg[i] = degrees(jv.value / 1000000.0);
    }
}

// Set up interpolation parameters and transition motionState_ for the
// given command.  Does NOT send a frame; the caller's switch statement
// will send the first frame on the same Ready() invocation.
void
TinyConsole::BeginMotion(MotionCmd cmd)
{    
    double cur[NUM_JOINTS] = {0.0};
    ReadCurrentPose(cur);

    switch (cmd) {

        case MCMD_GETUP:
            OSYSPRINT(("TinyConsole: GET_UP\n"));
            SetupInterpolation(cur, BROADBASE_POSE, GET_UP_COUNTER);
            motionState_ = MSTATE_GETUP;
            break;

        case MCMD_REST:
            OSYSPRINT(("TinyConsole: REST\n"));
            SetupInterpolation(cur, SLEEPING_POSE, REST_COUNTER);
            motionState_ = MSTATE_REST;
            break;

        case MCMD_FORWARD:
            OSYSPRINT(("TinyConsole: FORWARD walk\n"));
            walkForward_ = true;
            SetupInterpolation(cur, WALK_FWD_A, WALK_PHASE_COUNTER);
            motionState_ = MSTATE_WALK_A;
            break;

        case MCMD_BACK:
            OSYSPRINT(("TinyConsole: BACK walk\n"));
            walkForward_ = false;
            SetupInterpolation(cur, WALK_BACK_A, WALK_PHASE_COUNTER);
            motionState_ = MSTATE_WALK_A;
            break;

        case MCMD_STOP:
            OSYSPRINT(("TinyConsole: STOP\n"));
            if (motionState_ != MSTATE_IDLE &&
                motionState_ != MSTATE_TO_BASE) {
                SetupInterpolation(cur, BROADBASE_POSE, STOP_COUNTER);
                motionState_ = MSTATE_TO_BASE;
            }
            break;

        default:
            break;
    }
}

// If the state machine is currently idle (Ready() is not firing), send a
// no-op "stay put" frame so that the OPEN-R scheduler will invoke Ready()
// and pick up the newly set pendingCmd_.
// If motion is already in progress, this is a no-op Ready() will handle
// pendingCmd_ at the next phase boundary.
void
TinyConsole::TriggerReady()
{
    if (motionState_ != MSTATE_IDLE) return; // already active

    double cur[NUM_JOINTS];
    ReadCurrentPose(cur);

    RCRegion* rgn = FindFreeRegion();
    if (!rgn) return;

    for (int i = 0; i < NUM_JOINTS; i++)
        SetJointValue(rgn, i, cur[i], cur[i]);

    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();
    // motionState_ stays IDLE; the pending command is applied in Ready().
}
