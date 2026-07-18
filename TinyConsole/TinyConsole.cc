//
// TinyConsole.cc
// Minimal OPEN-R TCP console with ChaCha20+Poly1305 AEAD and ERS-7 leg motion.
//
// Motion commands are handled asynchronously: the TCP response is sent
// immediately, and the motion itself runs via the Ready() / subject callback
// loop driven by the OPEN-R scheduler.
//
// Notes about legs joints:
//   J1 hip abduction/adduction   (large value = leg rotated outward)
//   J2 hip flexion/extension     (higher = more forward)
//   J3 knee flexion               (higher = more bent = foot lifted)
//
//   Walking direction depends on which end of the J2 range the "planted"
//   foot sits at.

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

uint32_t TinyConsole::sessionId = 0;

// ================================================================
// Joint locators (taken from ERS-7 Model Information official Sony's documentation)
//
// This can be very confusing but the official documentation uses a view from the side
// as a point of reference when naming legs. It is recommended to use the side-view image 
// as a reference (the head of the robot is located on the left side of the image):
//
// - Left  = front legs (left of the side view)
// - Right = rear of the robot (right of the side view)
// - Front = left legs (visible in the side view)
// - Rear  = right legs (hidden in the side view)
// ================================================================


static const char* const JOINT_LOCATOR[TinyConsole::NUM_JOINTS] = {
	"PRM:/r4/c1-Joint2:41",         // Right Front J1
    "PRM:/r4/c1/c2-Joint2:42",      // Right Front J2
    "PRM:/r4/c1/c2/c3-Joint2:43",   // Right Front J3

    "PRM:/r2/c1-Joint2:21",         // Left Front J1
    "PRM:/r2/c1/c2-Joint2:22",      // Left Front J2
    "PRM:/r2/c1/c2/c3-Joint2:23",   // Left Front J3

    "PRM:/r3/c1-Joint2:31",         // Left Rear J1
    "PRM:/r3/c1/c2-Joint2:32",      // Left Rear J2
    "PRM:/r3/c1/c2/c3-Joint2:33",   // Left Rear J3

    "PRM:/r5/c1-Joint2:51",         // Right Rear J1
    "PRM:/r5/c1/c2-Joint2:52",      // Right Rear J2
    "PRM:/r5/c1/c2/c3-Joint2:53",   // Right Rear J3
};



// ================================================================
// Pose tables  (all angles in degrees)
//
// Here:
//  - Front = front legs 
//  - Rear = rear legs
//
// Note: use the ERS-7 Model Information official Sony's documentation 
// as a reference for the maximum and minimum values of each joint.
// Not following it could lead to damage of the robot's legs and/or motors.
// ================================================================

// Sleeping body low, rear knees tucked

static const double SLEEPING_POSE_0[12] = {
     10,  1,  30,  // Front
	 10,  1,  30,  // Front
	-35,  5,  60, // Rear
	-35,  5,  60, // Rear
};

static const double SLEEPING_POSE_1[12] = {
     10,  1,   45,  // Front
     10,  1,   45,  // Front
    -45,  5,   70,  // RR
    -45,  5,   70,  // RR
};

static const double SLEEPING_POSE_2[12] = {
     10,  1,   80,  // Front
	 10,  1,   80,  // Front
    -65,  5,   80,  // RR
    -65,  5,   80,  // RR
};

static const double SLEEPING_POSE_3[12] = {
      80,  1,   -25,  // Front -> watch out for unecessary tension on J1
	  80,  1,   -25,  // Front -> //
    -125,  5,   120,  // RR
    -125,  5,   120,  // RR
};

// Rising from sleeping to standing pose based on Motion Files (Sit_to_Stand.MTN available in Motion Files folder of the repository)
static const double RISING_POSE_0[12] = {
	  65,  4,   43, // Front
	  65,  4,   43, // Front
	 -95, 15,  120, // Back
	 -95,  15, 120, // Back
};

static const double RISING_POSE_1[12] = {
	  -15,  5, 107, // Front
	  -15,  5, 107, // Front
	  -78, 12, 122, // Rear
	  -78, 12, 122, // Rear
}; // Frame 114

static const double RISING_POSE_2[12] = {
	  -18,  2, 101, // Front
	  -18,  2, 101, // Front
	  -54,  3,  96, // Rear
	  -54,  3,  96, // Rear
}; // Frame 154

static const double RISING_POSE_3[12] = {
	  -21,  2, 74, // Front
	  -21,  2, 74, // Front
	  -44,  3, 95, // Rear
	  -44,  3, 95, // Rear
	 
}; // Frame 190


// Broadbase upright, legs spread in stable stance.
// To be modified: same as rising for debug purpose
static const double BROADBASE_POSE[12] = {
	   -5,  3, 30,  // Front
	   -5,  3, 30,  // Front
	   -5,  3, 30,  // Back
	   -5,  3, 30,  // Back
};

// ----------------------------------------------------------------
//  Forward walking motion based on Motion Files (Walk_forward.MTN available in Motion Files folder of the repository)
//
//	A = front right up
//  B = front right down
//  C = back left up
//  D = back left down
//  E = front left up
//  F = front left down
//  G = back right up
//  H = back right down
// ----------------------------------------------------------------

static const double WALK_FWD_A[12] = {
    -24, 22, 91, // Front right
     -5,  1, 43, // Front left
    -27,  1, 94, // Back left
    -48, 21, 94, // Back right
}; // Frame 50

static const double WALK_FWD_B[12] = {
      16,  7, 23, // Front right
     -11,  7, 47, // Front left
     -22, 10, 93, // Back left
     -45, 12, 99, // Back right
}; // Frame 100

static const double WALK_FWD_C[12] = {
      8,  -1,  23, // Front right
    -12,  17,  31, // Front left
    -78,  35,  90, // Back left - was -78,48,122
    -36,  -2,  99, // Back right
}; // Frame 190

static const double WALK_FWD_D[12] = {
       7, -1, 23, // Front right
     -27, 20, 58, // Front left
     -51, 24, 89, // Back left
     -34, -2, 98, // Back right
};// Frame 210 of WWWFWD.MTN

static const double WALK_FWD_E[12] = {
       -8, 4, 46, // Front right
     -11, 17, 88, // Front left
     -47, 17, 97, // Back left
     -24, 5, 92,  // Back right
}; // Frame 290

static const double WALK_FWD_F[12] = {
    -11,  7, 47,   // Front right
     16,  7, 23,   // Front left
    -45, 12, 99,   // Back left
    -21,  9, 88,   // Back right
}; // Frame 308

static const double WALK_FWD_G[12] = {
    -12, 17,  30, // Front right
      8, -1,  23, // Front left
    -36, -2,  98, // Back left
    -50, 35,  80, // Back right -> was -79, 48, 122
}; // Frame 402

static const double WALK_FWD_H[12] = {
    -12, 17, 29, // Front right
      7, -1, 23, // Front left
    -34, -2, 98, // Back left
    -57, 27, 75, // Back right
}; // Frame 420



// ----------------------------------------------------------------
//  Backward trot roles reversed based on Motion Files (Walk_backward.MTN available in Motion Files folder of the repository)
//
//
//  A = back right up
//  B = back right down
//  C = front left up
//  D = front left down
//  E = back left up
//  F = back left down
//  G = front right up
//  H = front right down
// ----------------------------------------------------------------

static const double WALK_BACK_A[12] = {
     -26, 12,  74,  // Front right
       7,  5,  41,  // Front left
     -35,  7,  82,  // Back left
     -32, 21, 117,  // Back right
}; // Frame 88

static const double WALK_BACK_B[12] = {
     -24, 10, 75,  // Front right
      13,  7, 29,  // Front left
     -36, 10, 81,  // Back left
     -12,  8, 68,  // Back right
}; // Frame 100 for front legs ; 105 for back legs

static const double WALK_BACK_C[12] = {
     -13, -2,  68,  // Front right
     -57, 41, 112,  // Front left
     -39, 19,  70,  // Back left
     -24, -2,  79,  // Back right
}; // Frame 195

static const double WALK_BACK_D[12] = {
      -10, -2, 65,  // Front right
      -32, 22, 69,  // Front left
      -39, 19, 69,  // Back left
      -25, -2, 80,  // Back right
}; // Frame 212 for front legs ; 206 for back legs

static const double WALK_BACK_E[12] = {
       7,  5,  41,  // Front right
     -26, 12,  74,  // Front left
     -32, 21, 117,  // Back left
     -35,  7,  82,  // Back right
}; // Frame 300

static const double WALK_BACK_F[12] = {
      13,  7, 29,  // Front right
     -24, 10, 75,  // Front left
     -12,  8, 68,  // Back left
     -36, 10, 81,  // Back right
}; // Frame 318 for back ; 313 for front


static const double WALK_BACK_G[12] = {
      -57, 41, 112,  // Front right
      -13, -2,  68,  // Front left
      -24, -2,  79,  // Back left
      -39, 19,  70,  // Back right
}; // Frame 407


static const double WALK_BACK_H[12] = {
     -38, 24, 80,  // Front right
     -11, -2, 66,  // Front left
     -25, -2, 80,  // Back left
     -39, 19, 69,  // Back right
}; // Frame 420

// ================================================================
//  Constructor
// ================================================================

TinyConsole::TinyConsole()
    : txCounter(0),
      rxCounter(0),
      pendingClose(false),
      handshakeStage(HS_BANNER_SENT),
      recvPhase(RX_CLIENT_NONCE),
      pendingFrameLen(0),
      motionState(MSTATE_IDLE),
      pendingCmd(MCMD_NONE),
      motionCounter(0),
      walkForward(true)
{
    conn.state = CONNECTION_CLOSED;
    memset(sessionNonce,    0, sizeof(sessionNonce));
    memset(serverNonceSent, 0, sizeof(serverNonceSent));
    memset(motionStart,  0, sizeof(motionStart));
    memset(motionDelta,  0, sizeof(motionDelta));

    for (int i = 0; i < NUM_CMD_VECTORS; i++)
        cmdRegion[i] = 0;
}

// ================================================================
//  OPEN-R lifecycle
// ================================================================

OStatus
TinyConsole::DoInit(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyConsole::DoInit()\n"));

    NEW_ALL_SUBJECT_AND_OBSERVER; // Macros defined in <OPENR/core_macro.h>
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

    ipstackRef = antStackRef("IPStack");

    antEnvCreateSharedBufferMsg sendBufMsg(CONSOLE_BUFSIZE);
    sendBufMsg.Call(ipstackRef, sizeof(sendBufMsg));
    if (sendBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc send buffer %d", sendBufMsg.error));
        return oFAIL;
    }
    conn.sendBuffer = sendBufMsg.buffer;
    conn.sendBuffer.Map();
    conn.sendData = (byte*)conn.sendBuffer.GetAddress();

    antEnvCreateSharedBufferMsg recvBufMsg(CONSOLE_BUFSIZE);
    recvBufMsg.Call(ipstackRef, sizeof(recvBufMsg));
    if (recvBufMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole: can't alloc recv buffer %d", recvBufMsg.error));
        return oFAIL;
    }
    conn.recvBuffer = recvBufMsg.buffer;
    conn.recvBuffer.Map();
    conn.recvData = (byte*)conn.recvBuffer.GetAddress();
    
    if (subject[sbjMove]->IsReady() == true) {
        double cur[NUM_JOINTS];
        ReadCurrentPose(cur);
        SetupInterpolation(cur, SLEEPING_POSE_3, STARTUP_COUNTER);
        SetJointGain(); // Position of this function is important as if it isn't called at the "good" time, 
        // joints won't move at all when receiving commands
		motionState = MSTATE_STARTUP;
		AdvanceInterpolation(STARTUP_COUNTER);
    } else {
        motionState = MSTATE_IDLE;
    }

    ENABLE_ALL_SUBJECT;
    ASSERT_READY_TO_ALL_OBSERVER;

    return Listen();
}

OStatus
TinyConsole::DoStop(const OSystemEvent& event)
{
    OSYSDEBUG(("TinyConsole::DoStop()\n"));

    motionState = MSTATE_IDLE;
    pendingCmd  = MCMD_NONE;

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
//  A pending TCP command (pendingCmd) is applied at the start of
//  each callback so that new commands interrupt walking on the next
//  phase boundary clean and glitch-free.
// ================================================================

void
TinyConsole::Ready(const OReadyEvent& event)
{
    OSYSDEBUG(("TinyConsole::Ready() state=%d\n", (int)motionState));

    // ---- Apply any pending TCP command --------------------------------
    if (pendingCmd != MCMD_NONE) {
        MotionCmd cmd = pendingCmd;
        pendingCmd   = MCMD_NONE;
        BeginMotion(cmd);
        // Fall through so the switch below sends the very first frame.
    }

    // ---- Advance the current motion phase -----------------------------
    switch (motionState) {

		case MSTATE_STARTUP:
			if (AdvanceInterpolation(STARTUP_COUNTER) == MOVING_FINISH)
				motionState = MSTATE_IDLE;
			break;

        case MSTATE_IDLE:
            // Nothing to do; we won't get another Ready() until
            // TriggerReady() sends a frame for the next command.
            break;

        case MSTATE_GETUP_PREP:
        	if (AdvanceInterpolation(GETUP_COUNTER) == MOVING_FINISH) {
        		SetupInterpolation(RISING_POSE_0, RISING_POSE_1, GETUP_COUNTER);
        		motionState = MSTATE_GETUP_PREP_1;
			}
			break;
			
		case MSTATE_GETUP_PREP_1:
        	if (AdvanceInterpolation(GETUP_COUNTER) == MOVING_FINISH) {
        		SetupInterpolation(RISING_POSE_1, RISING_POSE_2, GETUP_COUNTER);
        		motionState = MSTATE_GETUP_PREP_2;
			}
			break;

		case MSTATE_GETUP_PREP_2:
        	if (AdvanceInterpolation(GETUP_COUNTER) == MOVING_FINISH) {
        		SetupInterpolation(RISING_POSE_2, RISING_POSE_3, GETUP_COUNTER);
        		motionState = MSTATE_GETUP_PREP_3;
			}
			break;

		case MSTATE_GETUP_PREP_3:
        	if (AdvanceInterpolation(GETUP_COUNTER) == MOVING_FINISH) {
        		SetupInterpolation(RISING_POSE_3, BROADBASE_POSE, GETUP_COUNTER);
        		motionState = MSTATE_GETUP;
			}
			break;
		

        case MSTATE_GETUP:
            if (AdvanceInterpolation(GETUP_COUNTER) == MOVING_FINISH)
                motionState = MSTATE_IDLE;
            break;
	
		case MSTATE_PREPA_REST_0:
			if (AdvanceInterpolation(REST_COUNTER) == MOVING_FINISH) {
        		SetupInterpolation(SLEEPING_POSE_0, SLEEPING_POSE_1, REST_COUNTER);
        		motionState = MSTATE_PREPA_REST_1;
			}
			break;

		case MSTATE_PREPA_REST_1:
			if (AdvanceInterpolation(REST_COUNTER) == MOVING_FINISH) {
        		SetupInterpolation(SLEEPING_POSE_1, SLEEPING_POSE_2, REST_COUNTER);
        		motionState = MSTATE_PREPA_REST_2;
			}
			break;
		
		case MSTATE_PREPA_REST_2:
			if (AdvanceInterpolation(REST_COUNTER) == MOVING_FINISH) {
        		SetupInterpolation(SLEEPING_POSE_2, SLEEPING_POSE_3, REST_COUNTER);
        		motionState = MSTATE_REST;
			}
			break;

		
        case MSTATE_REST:
            if (AdvanceInterpolation(REST_COUNTER) == MOVING_FINISH)
                motionState = MSTATE_IDLE;
            break;

        // Walk phases alternate: A B C D E F G H until interrupted.
        case MSTATE_WALK_A:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_A  : WALK_BACK_A;
                const double* to   = walkForward? WALK_FWD_B  : WALK_BACK_B;
                SetupInterpolation(from, to, WALK_COUNTER);
                motionState = MSTATE_WALK_B;
            }
            break;

        case MSTATE_WALK_B:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_B  : WALK_BACK_B;
                const double* to   = walkForward? WALK_FWD_C  : WALK_BACK_C;
                SetupInterpolation(from, to, WALK_COUNTER);
                motionState = MSTATE_WALK_C;
            }
            break;

        case MSTATE_WALK_C:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_C  : WALK_BACK_C;
                const double* to   = walkForward? WALK_FWD_D  : WALK_BACK_D;
                SetupInterpolation(from, to, WALK_COUNTER);
				motionState = MSTATE_WALK_D;
            }
            break;

        case MSTATE_WALK_D:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_D  : WALK_BACK_D;
                const double* to   = walkForward? WALK_FWD_E  : WALK_BACK_E;
                SetupInterpolation(from, to, WALK_COUNTER);
                motionState = MSTATE_WALK_E;
            }
            break;

		case MSTATE_WALK_E:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_E  : WALK_BACK_E;
                const double* to   = walkForward? WALK_FWD_F  : WALK_BACK_F;
                SetupInterpolation(from, to, WALK_COUNTER);
                motionState = MSTATE_WALK_F;
            }
            break;
         
		case MSTATE_WALK_F:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_F  : WALK_BACK_F;
                const double* to   = walkForward? WALK_FWD_G  : WALK_BACK_G;
                SetupInterpolation(from, to, WALK_COUNTER);
                motionState = MSTATE_WALK_G;
            }
            break;

		case MSTATE_WALK_G:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_G  : WALK_BACK_G;
                const double* to   = walkForward? WALK_FWD_H  : WALK_BACK_H;
                SetupInterpolation(from, to, WALK_COUNTER);
                motionState = MSTATE_WALK_H;
            }
            break;


		case MSTATE_WALK_H:
            if (AdvanceInterpolation(WALK_COUNTER) == MOVING_FINISH) {
                const double* from = walkForward? WALK_FWD_H  : WALK_BACK_H;
                const double* to   = walkForward? WALK_FWD_A  : WALK_BACK_A;
                SetupInterpolation(from, to, WALK_COUNTER);
                motionState = MSTATE_WALK_A;
            }
            break;


        case MSTATE_TO_BASE:
            if (AdvanceInterpolation(STOP_COUNTER) == MOVING_FINISH)
                motionState = MSTATE_IDLE;
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

    conn.state   = CONNECTION_CONNECTED;
    pendingClose= false;
    handshakeStage= HS_BANNER_SENT;
    recvPhase   = RX_CLIENT_NONCE;

    // Build 12-byte session nonce:
    //   [0..3]  session ID (LE) monotone per boot, resets on reboot
    //   [4..7]  client IPv4   (LE)
    //   [8..11] per-message counter, written by AdvanceNonce()
    uint32_t sid  = (uint32_t)sessionId++;
    uint32_t addr = listenMsg->fAddress.Address();

    sessionNonce[0] = (uint8_t)( sid         & 0xFF);
    sessionNonce[1] = (uint8_t)((sid >>  8)  & 0xFF);
    sessionNonce[2] = (uint8_t)((sid >> 16)  & 0xFF);
    sessionNonce[3] = (uint8_t)((sid >> 24)  & 0xFF);
    sessionNonce[4] = (uint8_t)( addr        & 0xFF);
    sessionNonce[5] = (uint8_t)((addr >>  8) & 0xFF);
    sessionNonce[6] = (uint8_t)((addr >> 16) & 0xFF);
    sessionNonce[7] = (uint8_t)((addr >> 24) & 0xFF);
    sessionNonce[8] = sessionNonce[9] = sessionNonce[10] = sessionNonce[11] = 0;

    memcpy(serverNonceSent, sessionNonce, NONCE_SIZE);

    txCounter= 0;
    rxCounter= 0;

    // Send plaintext banner + raw nonce
    const char* banner    = CONSOLE_BANNER;
    int         bannerLen = (int)strlen(banner);

    memcpy(conn.sendData,             banner,        bannerLen);
    memcpy(conn.sendData + bannerLen, sessionNonce, NONCE_SIZE);
    conn.sendSize = bannerLen + NONCE_SIZE;

    TCPEndpointSendMsg sendMsg(conn.endpoint,
                               conn.sendData, conn.sendSize);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendCont], sizeof(sendMsg));
    conn.state    = CONNECTION_SENDING;
    conn.sendSize = 0;
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

    conn.state = CONNECTION_CONNECTED;

    if (pendingClose) {
        pendingClose= false;
        Close();
        return;
    }

    switch (handshakeStage) {

        case HS_BANNER_SENT:
            // Just sent banner+nonce; wait for the client's nonce.
            recvPhase= RX_CLIENT_NONCE;
            Receive(NONCE_SIZE, NONCE_SIZE);
            break;

        case HS_SIG_SENT:
            // Just sent our handshake signature. The client verifies it,
            // then sends its own 64-byte signature over the same transcript
            // (mutual auth). Wait before accepting AEAD traffic.
            handshakeStage= HS_ESTABLISHED;
            recvPhase= RX_CLIENT_SIG;
            Receive(HANDSHAKE_SIG_SIZE, HANDSHAKE_SIG_SIZE);
            break;

        case HS_ESTABLISHED:
            // Just sent an ordinary encrypted response.
            recvPhase= RX_FRAME_HEADER;
            Receive(FRAME_HEADER_SIZE, FRAME_HEADER_SIZE);
            break;
    }
}

// Name for a ReceivePhase (for debug only)
static const char*
ReceivePhaseName(ReceivePhase p)
{
    switch (p) {
        case RX_CLIENT_NONCE:  return "RX_CLIENT_NONCE";
        case RX_CLIENT_SIG:    return "RX_CLIENT_SIG";
        case RX_FRAME_HEADER:  return "RX_FRAME_HEADER";
        case RX_FRAME_BODY:    return "RX_FRAME_BODY";
    }
    return "RX_UNKNOWN";
}

void
TinyConsole::ReceiveCont(ANTENVMSG msg)
{
    TCPEndpointReceiveMsg* recvMsg = (TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::ReceiveCont() phase=%s n=%d\n",
               ReceivePhaseName(recvPhase), recvMsg->sizeMin));

    if (recvMsg->error == TCP_CONNECTION_CLOSED) {
        OSYSPRINT(("TinyConsole: client disconnected\n"));
        Close();
        return;
    }
    if (recvMsg->error != TCP_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::ReceiveCont FAILED %d", recvMsg->error));
        Close();
        return;
    }
	
    conn.state = CONNECTION_CONNECTED;
    int n = recvMsg->sizeMin;

    // ------------------------------------------------------------------
    //  RX_CLIENT_NONCE -> client nonce (12 bytes, plaintext, handshake only)
    // ------------------------------------------------------------------
    if (recvPhase == RX_CLIENT_NONCE) {
        if (n < NONCE_SIZE) {
            OSYSLOG1((osyslogERROR, "TinyConsole: short client nonce (%d bytes)", n));
            Close();
            return;
        }

        uint8_t clientNonce[NONCE_SIZE];
        memcpy(clientNonce, conn.recvData, NONCE_SIZE);

        // Keep a copy: RX_CLIENT_SIG needs it to rebuild the client's signed
        // transcript, and sessionNonce[0..7] is about to be overwritten.
        memcpy(clientNonceRecv, clientNonce, NONCE_SIZE);

        for (int i = 0; i < 8; i++)
        	sessionNonce[i] ^= clientNonce[i];
        	
        uint8_t sig[HANDSHAKE_SIG_SIZE];
        if (!SignHandshake(clientNonce, sig)) {
            OSYSLOG1((osyslogERROR,"TinyConsole: handshake signing failed"));
            Close();
            return;
        }

        memcpy(conn.sendData, sig, HANDSHAKE_SIG_SIZE);
        handshakeStage = HS_SIG_SENT;
        Send(conn.sendData, HANDSHAKE_SIG_SIZE);
        return;
    }
    
    // ------------------------------------------------------------------
    //  RX_CLIENT_SIG -> client's Ed25519 handshake signature (64 bytes, plaintext)
    //
    //  Mutual authentication: the client signs the same transcript the
    //  robot signed in RX_CLIENT_NONCE (HANDSHAKE_CONTEXT || serverNonceSent
    //  || clientNonceRecv). Verifying it against the pinned CLIENT_ED25519_PK
    //  proves its identity. Fail closed on any mismatch.
    // ------------------------------------------------------------------
    if (recvPhase == RX_CLIENT_SIG) {
        if (n < HANDSHAKE_SIG_SIZE) {
            OSYSLOG1((osyslogERROR,
                      "TinyConsole: short client signature (%d bytes)", n));
            Close();
            return;
        }

        if (!VerifyClientHandshake(conn.recvData)) {
            OSYSLOG1((osyslogERROR,
                      "TinyConsole: client identity verification FAILED "
                      "- rejecting connection"));
            Close();
            return;
        }

        OSYSPRINT(("TinyConsole: client identity verified\n"));

        // Client is now authenticated; proceed to normal AEAD traffic.
        recvPhase = RX_FRAME_HEADER;
        Receive(FRAME_HEADER_SIZE, FRAME_HEADER_SIZE);
        return;
    }

    // ------------------------------------------------------------------
    //  RX_FRAME_HEADER -> 2-byte frame header
    // ------------------------------------------------------------------
    if (recvPhase == RX_FRAME_HEADER) {
        if (n < FRAME_HEADER_SIZE) {
            OSYSLOG1((osyslogERROR, "TinyConsole: short frame header (%d bytes)", n));
            Close();
            return;
        }

        uint16_t ctLen = (uint16_t)( (unsigned char)conn.recvData[0]        )
                       | (uint16_t)(((unsigned char)conn.recvData[1]) << 8  );

        if (ctLen == 0 || ctLen > (uint16_t)CONSOLE_MAX_PLAINTEXT) {
            OSYSLOG1((osyslogERROR,
                      "TinyConsole: bad frame ciphertext length %u", ctLen));
            Close();
            return;
        }

        pendingFrameLen= ctLen;
        recvPhase      = RX_FRAME_BODY;
        Receive((int)ctLen + FRAME_TAG_SIZE, (int)ctLen + FRAME_TAG_SIZE);
        return;
    }

    // ------------------------------------------------------------------
	// RX_FRAME_BODY -> AEAD body (ciphertext + 16-byte tag) & Dispatch
	// ------------------------------------------------------------------
	if (recvPhase == RX_FRAME_BODY) {
		int bodyLen = (int)pendingFrameLen+ FRAME_TAG_SIZE;
    	if (n < bodyLen) {
        	OSYSLOG1((osyslogERROR,
                  "TinyConsole: short frame body (got %d, expected %d)",
                  n, bodyLen));
        Close();
        return;
    	}

    	int ptLen = 0;
    	if (!AeadDecrypt(conn.recvData, bodyLen, conn.recvData, &ptLen)) {
	    	Close();
        	return;
    	}

    	if (ptLen < CONSOLE_BUFSIZE) conn.recvData[ptLen] = '\0';

    	// Strip trailing CR/LF in-place.
    	char* cmd    = (char*)conn.recvData;
    	int   cmdLen = ptLen;
    	while (cmdLen > 0 &&
           (cmd[cmdLen-1] == '\r' || cmd[cmdLen-1] == '\n'))
        	cmd[--cmdLen] = '\0';

    // ------------------------------------------------------------------
    //  Command dispatch
    //
    //  Motion commands are ASYNCHRONOUS: responses are sent immediately and
    //  set pendingCmd.  The motion state machine picks it up at the
    //  next Ready() call (triggered by TriggerReady() if currently idle).
    // ------------------------------------------------------------------
    	const char* response = "OK\n";

    	if (cmdLen == 0) {
        	response = "> ";

    	} else if (!strncmp(cmd, "PING", 4)) {
        	response = "PONG\n";

    	} else if (!strncmp(cmd, "GET_UP", 6)) {
        	pendingCmd= MCMD_GETUP;
        	TriggerReady();
        	response = "STANDING_UP\n";

    	} else if (!strncmp(cmd, "REST", 4)) {
        	pendingCmd= MCMD_REST;
        	TriggerReady();
        	response = "RESTING\n";

    	} else if (!strncmp(cmd, "FORWARD", 7)) {
        	pendingCmd= MCMD_FORWARD;
        	TriggerReady();
        	response = "MOVING_FORWARD\n";

    	} else if (!strncmp(cmd, "BACK", 4)) {
        	pendingCmd= MCMD_BACK;
        	TriggerReady();
        	response = "MOVING_BACK\n";

    	} else if (!strncmp(cmd, "STOP", 4)) {
        	pendingCmd= MCMD_STOP;
	        TriggerReady();
        	response = "STOPPING\n";

    	} else if (!strncmp(cmd, "HELP", 4)) {
        	response = "Available commands: PING, GET_UP, REST, FORWARD, BACK, STOP, HELP, INFO, QUIT\n";

    	} else if (!strncmp(cmd, "INFO", 4)) {        
        	response = "\n I am an AIBO robot from Sony, model ERS-7M3/T.\n I was born in Japan for the European market.\n";

    	} else if (!strncmp(cmd, "QUIT", 4)) {
        	response      = "BYE\n";
        	pendingClose= true;
    	}

    	int frameLen = 0;
    	if (!AeadEncrypt((const byte*)response, (int)strlen(response),
                     conn.sendData, &frameLen)) {
        	OSYSLOG1((osyslogERROR, "TinyConsole: AeadEncrypt failed"));
        	Close();
        	return;
    }

    	Send(conn.sendData, frameLen);
	}
}

void
TinyConsole::CloseCont(ANTENVMSG msg)
{
    TCPEndpointCloseMsg* closeMsg =
        (TCPEndpointCloseMsg*)antEnvMsg::Receive(msg);

    OSYSDEBUG(("TinyConsole::CloseCont()\n"));

    conn.state = CONNECTION_CLOSED;
    Listen();
}

// ================================================================
//  Private TCP helpers
// ================================================================

OStatus
TinyConsole::Listen()
{
    OSYSDEBUG(("TinyConsole::Listen()\n"));

    if (conn.state != CONNECTION_CLOSED) return oFAIL;

    antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP, CONSOLE_BUFSIZE * 2);
    tcpCreateMsg.Call(ipstackRef, sizeof(tcpCreateMsg));
    if (tcpCreateMsg.error != ANT_SUCCESS) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Listen endpoint FAIL %d",
                  tcpCreateMsg.error));
        return oFAIL;
    }
    conn.endpoint = tcpCreateMsg.moduleRef;

    TCPEndpointListenMsg listenMsg(conn.endpoint, IP_ADDR_ANY, CONSOLE_PORT);
    listenMsg.continuation = (void*)0;
    listenMsg.Send(ipstackRef, myOID_,
                   Extra_Entry[entryListenCont], sizeof(listenMsg));

    conn.state = CONNECTION_LISTENING;
    OSYSPRINT(("TinyConsole: listening on port %d\n", CONSOLE_PORT));
    return oSUCCESS;
}

OStatus
TinyConsole::Send(const byte* /*data*/, int size)
{
    if (conn.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Send called in wrong state %d",
                  conn.state));
        return oFAIL;
    }

    TCPEndpointSendMsg sendMsg(conn.endpoint, conn.sendData, size);
    sendMsg.continuation = (void*)0;
    sendMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entrySendCont],
                 sizeof(TCPEndpointSendMsg));

    conn.state    = CONNECTION_SENDING;
    conn.sendSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Receive(int sizeMin, int sizeMax)
{
    if (conn.state != CONNECTION_CONNECTED) {
        OSYSLOG1((osyslogERROR, "TinyConsole::Receive called in wrong state %d",
                  conn.state));
        return oFAIL;
    }

    TCPEndpointReceiveMsg recvMsg(conn.endpoint,
                                  conn.recvData, sizeMin, sizeMax);
    recvMsg.continuation = (void*)0;
    recvMsg.Send(ipstackRef, myOID_,
                 Extra_Entry[entryReceiveCont], sizeof(recvMsg));

    conn.state    = CONNECTION_RECEIVING;
    conn.recvSize = 0;
    return oSUCCESS;
}

OStatus
TinyConsole::Close()
{
    if (conn.state == CONNECTION_CLOSED ||
        conn.state == CONNECTION_CLOSING) return oFAIL;

    TCPEndpointCloseMsg closeMsg(conn.endpoint);
    closeMsg.continuation = (void*)0;
    closeMsg.Send(ipstackRef, myOID_,
                  Extra_Entry[entryCloseCont], sizeof(closeMsg));

    conn.state = CONNECTION_CLOSING;
    return oSUCCESS;
}

// ================================================================
//  AEAD helpers
// ================================================================

void
TinyConsole::AdvanceNonce(uint32_t& counter, bool isTx)
{
    uint32_t mc = counter++;
    mc |= (isTx ? (1U << 31) : 0);
    sessionNonce[8]  = (uint8_t)( mc        & 0xFF);
    sessionNonce[9]  = (uint8_t)((mc >>  8) & 0xFF);
    sessionNonce[10] = (uint8_t)((mc >> 16) & 0xFF);
    sessionNonce[11] = (uint8_t)((mc >> 24) & 0xFF);
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

    AdvanceNonce(txCounter, true);

    chacha20poly1305_ctx ctx;
    rfc7539_init(&ctx, (uint8_t*)CHACHA_KEY, sessionNonce);

    frameOut[0] = (uint8_t)( ptLen       & 0xFF);
    frameOut[1] = (uint8_t)((ptLen >> 8) & 0xFF);

	rfc7539_auth(&ctx, frameOut, FRAME_HEADER_SIZE);

    chacha20poly1305_encrypt(&ctx,
                              (uint8_t*)plaintext,
                              frameOut + FRAME_HEADER_SIZE,
                              (size_t)ptLen);

    rfc7539_finish(&ctx,
                   (int64_t)FRAME_HEADER_SIZE,
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

    AdvanceNonce(rxCounter, false);

    chacha20poly1305_ctx ctx;
    rfc7539_init(&ctx, (uint8_t*)CHACHA_KEY, sessionNonce);
	
	uint8_t frame_header_bytes[FRAME_HEADER_SIZE];
	frame_header_bytes[0] = (uint8_t)( ctLen & 0xFF);
	frame_header_bytes[1] = (uint8_t)((ctLen >> 8) & 0xFF);
	
	rfc7539_auth(&ctx, frame_header_bytes, FRAME_HEADER_SIZE);
	
    chacha20poly1305_decrypt(&ctx,
                              (uint8_t*)frame,
                              plaintext,
                              (size_t)ctLen);

    uint8_t expectedTag[FRAME_TAG_SIZE];
    
    rfc7539_finish(&ctx,
                   (int64_t)FRAME_HEADER_SIZE,
                   (int64_t)ctLen,
                   expectedTag);

    if (!poly1305_verify(expectedTag,
                         (const unsigned char*)(frame + ctLen))) {
        OSYSLOG1((osyslogERROR,
                  "TinyConsole: Poly1305 tag mismatch - dropping frame"));
        memset(plaintext, 0, ctLen);
        *ptLen = 0;
        return false;
    }

    *ptLen = ctLen;
    return true;
}

// ================================================================
//  Handshake signing (Ed25519 / TweetNaCl)
//
//  Authenticates the robot to the client: proves this endpoint holds
//  ROBOT_ED25519_SK, which is never shared with clients (unlike CHACHA_KEY).
// ================================================================

bool
TinyConsole::SignHandshake(const uint8_t* clientNonce,
                            uint8_t sigOut[HANDSHAKE_SIG_SIZE])
{
    // Transcript = context || our raw nonce as sent || client's raw nonce
    // as received. Both nonces are already fully public (sent in the
    // clear during the handshake) thus signing them just proves *this*
    // robot produced/accepted *this* exchange.
    uint8_t msg[HANDSHAKE_CONTEXT_LEN + NONCE_SIZE + NONCE_SIZE];
    memcpy(msg, HANDSHAKE_CONTEXT, HANDSHAKE_CONTEXT_LEN);
    memcpy(msg + HANDSHAKE_CONTEXT_LEN, serverNonceSent, NONCE_SIZE);
    memcpy(msg + HANDSHAKE_CONTEXT_LEN + NONCE_SIZE, clientNonce, NONCE_SIZE);

    // crypto_sign() is TweetNaCl/NaCl's "combined" signing API: it
    // writes (64-byte signature || a copy of msg) into sm. We only 
    // send the first 64 bytes -> the client already has msg (it's the
    // two nonces it just exchanged)
    uint8_t sm[sizeof(msg) + HANDSHAKE_SIG_SIZE];
    unsigned long long smlen = 0;

    if (crypto_sign(sm, &smlen, msg, sizeof(msg),
                    (const unsigned char*)ROBOT_ED25519_SK) != 0) {
        return false;
    }

    memcpy(sigOut, sm, HANDSHAKE_SIG_SIZE);
    return true;
}

// ================================================================
//  Handshake verification (client -> robot, mutual auth)
//
//  Authenticates the CLIENT to the robot. Rebuilds the exact transcript
//  the client signed and checks the detached 64-byte signature against
//  the CLIENT_ED25519_PK. Uses TweetNaCl's combined-form
//  crypto_sign_open(), reconstructing sm = sig || msg locally (the client
//  transmits only the 64-byte detached signature; the robot already holds
//  msg, since it is the two nonces it exchanged).
// ================================================================

bool
TinyConsole::VerifyClientHandshake(const uint8_t* clientSig)
{
    // Same transcript layout and order as SignHandshake()
    const int MSG_LEN = HANDSHAKE_CONTEXT_LEN + NONCE_SIZE + NONCE_SIZE;

    uint8_t msg[HANDSHAKE_CONTEXT_LEN + NONCE_SIZE + NONCE_SIZE];
    memcpy(msg, HANDSHAKE_CONTEXT, HANDSHAKE_CONTEXT_LEN);
    memcpy(msg + HANDSHAKE_CONTEXT_LEN, serverNonceSent, NONCE_SIZE);
    memcpy(msg + HANDSHAKE_CONTEXT_LEN + NONCE_SIZE, clientNonceRecv, NONCE_SIZE);

    // crypto_sign_open() consumes the combined form sm = sig(64) || msg
    uint8_t sm[HANDSHAKE_SIG_SIZE + HANDSHAKE_CONTEXT_LEN + NONCE_SIZE + NONCE_SIZE];
    memcpy(sm, clientSig, HANDSHAKE_SIG_SIZE);
    memcpy(sm + HANDSHAKE_SIG_SIZE, msg, MSG_LEN);

    uint8_t m[HANDSHAKE_SIG_SIZE + HANDSHAKE_CONTEXT_LEN + NONCE_SIZE + NONCE_SIZE];
    unsigned long long mlen = 0;

    // Returns 0 iff the signature is valid for msg under CLIENT_ED25519_PK
    if (crypto_sign_open(m, &mlen, sm,
                         (unsigned long long)(HANDSHAKE_SIG_SIZE + MSG_LEN),
                         (const unsigned char*)CLIENT_ED25519_PK) != 0) {
        return false;
    }

    // Defensive: recovered message must match our transcript exactly
    if (mlen != (unsigned long long)MSG_LEN || memcmp(m, msg, MSG_LEN) != 0) {
        return false;
    }

    return true;
}

// TweetNaCl declares "randombytes" extern and calls it internally from
// crypto_sign_keypair() / crypto_box_keypair(). 
// This only exists so the linker has something to resolve -> if it ever
// actually runs, something upstream is badly wrong.
extern "C" void
randombytes(unsigned char* buf, unsigned long long n)
{
    OSYSLOG1((osyslogERROR,
              "TinyConsole: randombytes() called -- should be unreachable"));
    memset(buf, 0, n);
}

// ================================================================
//  Motion helpers
// ================================================================

void
TinyConsole::OpenPrimitives()
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OStatus result = OPENR::OpenPrimitive(JOINT_LOCATOR[i], &jointID[i]);
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

        cmdRegion[i] = new RCRegion(cmdVecData->vectorInfo.memRegionID,
                                     cmdVecData->vectorInfo.offset,
                                     (void*)cmdVecData,
                                     cmdVecData->vectorInfo.totalSize);

        cmdVecData->SetNumData(NUM_JOINTS);

        for (int j = 0; j < NUM_JOINTS; j++) {
            OCommandInfo* info = cmdVecData->GetInfo(j);
            info->Set(odataJOINT_COMMAND2, jointID[j], ocommandMAX_FRAMES);
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
            OPENR::EnableJointGain(jointID[idx]);
            OPENR::SetJointGain(jointID[idx],
                                pgain[j], igain[j], dgain[j],
                                PID_PSHIFT, PID_ISHIFT, PID_DSHIFT);
        }
    }
}

RCRegion*
TinyConsole::FindFreeRegion()
{
    for (int i = 0; i < NUM_CMD_VECTORS; i++) {
        if (cmdRegion[i] && cmdRegion[i]->NumberOfReference() == 1)
            return cmdRegion[i];
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
    info->Set(odataJOINT_COMMAND2, jointID[idx], ocommandMAX_FRAMES);

    OCommandData*        data = cmdVecData->GetData(idx);
    OJointCommandValue2* jval = (OJointCommandValue2*)data->value;

    double delta = endDeg - startDeg;
    for (int k = 0; k < ocommandMAX_FRAMES; k++) {
        double d = startDeg + delta * k / (double)ocommandMAX_FRAMES;
        jval[k].value = oradians(d);
        jval[k].padding = 0;
    }
}

// Initialise a linear interpolation: 'from[]' -> 'to[]' over *steps*
// command-vector frames.  Resets motionCounterto 0.
void
TinyConsole::SetupInterpolation(const double* from,
                                 const double* to,
                                 int           steps)
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        motionStart[i] = from[i];
        motionDelta[i] = (steps > 0) ? (to[i] - from[i]) / steps : 0.0;
    }
    motionCounter= 0;
}

// Send one command-vector block at the current step of the interpolation,
// advance motionCounter, and notify observers.
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

    int step = motionCounter;

    for (int i = 0; i < NUM_JOINTS; i++) {
        double cur = motionStart[i] + motionDelta[i] *  step;
        double nxt = motionStart[i] + motionDelta[i] * (step + 1);
        SetJointValue(rgn, i, cur, nxt);
    }

    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();

    motionCounter++;
    return (motionCounter>= maxSteps) ? MOVING_FINISH : MOVING_CONT;
}

// Read the current joint positions from hardware into outDeg[12].
void
TinyConsole::ReadCurrentPose(double* outDeg)
{
    for (int i = 0; i < NUM_JOINTS; i++) {
        OJointValue jv;
        OPENR::GetJointValue(jointID[i], &jv);
        outDeg[i] = degrees(jv.value / 1000000.0);
    }
}

// Set up interpolation parameters and transition motionState for the
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
            SetupInterpolation(cur, RISING_POSE_0, GETUP_COUNTER);
            motionState = MSTATE_GETUP_PREP;
            break;

        case MCMD_REST:
            OSYSPRINT(("TinyConsole: REST\n"));
            SetupInterpolation(cur, SLEEPING_POSE_0, REST_COUNTER);
            motionState = MSTATE_PREPA_REST_1;
            break;

		case MCMD_FORWARD:
            OSYSPRINT(("TinyConsole: FORWARD walk\n"));
            walkForward = true;
            SetupInterpolation(cur, WALK_FWD_A, WALK_COUNTER); // UPDATED COUNTER
            motionState = MSTATE_WALK_A;
            break;

        case MCMD_BACK:
            OSYSPRINT(("TinyConsole: BACK walk\n"));
            walkForward = false;
            SetupInterpolation(cur, WALK_BACK_A, WALK_COUNTER); // UPDATED COUNTER
            motionState = MSTATE_WALK_A;
            break;
            
        case MCMD_STOP:
            OSYSPRINT(("TinyConsole: STOP\n"));
            if (motionState != MSTATE_IDLE &&
                motionState != MSTATE_TO_BASE) {
                SetupInterpolation(cur, BROADBASE_POSE, STOP_COUNTER);
                motionState = MSTATE_TO_BASE;
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
// pendingCmdat the next phase boundary.
void
TinyConsole::TriggerReady()
{
    if (motionState != MSTATE_IDLE) return; // already active

    double cur[NUM_JOINTS];
    ReadCurrentPose(cur);

    RCRegion* rgn = FindFreeRegion();
    if (!rgn) return;

    for (int i = 0; i < NUM_JOINTS; i++)
        SetJointValue(rgn, i, cur[i], cur[i]);

    subject[sbjMove]->SetData(rgn);
    subject[sbjMove]->NotifyObservers();
    // motionState stays IDLE; the pending command is applied in Ready().
}
