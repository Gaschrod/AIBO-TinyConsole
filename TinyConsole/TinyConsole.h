//
// TinyConsole.h
// Minimal OPEN-R TCP console with ChaCha20+Poly1305 AEAD and leg motion.
//
//   PC client                         AIBO (this object)
//   ---------                         ------------------
//   connect()            ------>      ListenCont() which build a 12-byte session nonce and send "CONSOLE_READY\n" + nonce (plaintext)
//   recv banner line + nonce
//   init ChaCha20+Poly1305 ctx
//   -- AEAD session starts here on both sides --
//
//   recv 64-byte signature, verify
//   against pinned robot_pubkey          -- see SignHandshake();
//   before trusting anything below          proves the peer holds
//                                            ROBOT_ED25519_SK
//
//   send AEAD frame      ------>      ReceiveCont() [phase 0]
//                                       recv 2-byte length header
//                                     ReceiveCont() [phase 1]
//                                       recv ciphertext + 16-byte tag
//                                       AeadDecrypt  dispatch
//                                       AeadEncrypt response Send()
//   recv AEAD frame      <------
//
//  Motion commands (async response is sent immediately, motion runs in background):
//   GET_UP     stand to broadbase position
//   REST       lie down to sleeping position
//   FORWARD    diagonal trot forward (loops until interrupted)
//   BACK       diagonal trot backward (loops until interrupted)
//   STOP       stop action, return to broadbase
//   QUIT       close connection (existing)
//

#ifndef TinyConsole_h_DEFINED
#define TinyConsole_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OUnits.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <stdint.h>
#include <OPENR/core_macro.h> // needed for using macros

#include "TCPConnection.h"
#include "ConsoleConfig.h"  // NONCE_SIZE, CHACHA_KEY, ROBOT_ED25519_*, etc.
#include "rfc7539.h"        // chacha20poly1305_ctx, rfc7539_init/finish
extern "C"{
	#include "tweetnacl.h"      // crypto_sign / crypto_sign_open (Ed25519)
}
#include "def.h"
#include "entry.h"

// ================================================================
//  Motion enumerations
// ================================================================

enum MovingResult {
	MOVING_CONT,
	MOVING_FINISH
};

// Tracks progress through the handshake so SendCont()
// Knows what to do once each send completes:
//   HS_BANNER_SENT -> just sent banner+nonce, wait for client's nonce
//   HS_SIG_SENT    -> just sent our Ed25519 signature, wait for first
//                     AEAD frame (the client verifies before sending one)
//   HS_ESTABLISHED -> handshake done, normal AEAD traffic from here on
enum HandshakeStage {
    HS_BANNER_SENT,
    HS_SIG_SENT,
    HS_ESTABLISHED
};

// What kind of bytes ReceiveCont() is expecting on its next invocation.
// These are LABELS and the runtime order is
//
//   RX_CLIENT_NONCE -> RX_CLIENT_SIG -> RX_FRAME_HEADER -> RX_FRAME_BODY
//                                          ^------------------------|
//                                          (header/body then loop for
//                                           every subsequent message)
//
// The nonce and client-signature phases run exactly once, during the
// handshake; header and body alternate for the lifetime of the session.
enum ReceivePhase {
    RX_CLIENT_NONCE,    // 12-byte client nonce contribution (handshake)
    RX_CLIENT_SIG,      // 64-byte client Ed25519 signature  (handshake, mutual auth)
    RX_FRAME_HEADER,    // 2-byte AEAD frame length header    (steady state)
    RX_FRAME_BODY       // ciphertext + 16-byte Poly1305 tag  (steady state)
};

// High-level command issued from the TCP console.
// Stored in pendingCmd_ and applied at the next Ready() phase boundary.
enum MotionCmd {
    MCMD_NONE    = 0,
    MCMD_GETUP,      // transition to broadbase (standing)
    MCMD_REST,       // transition to sleeping (lying down)
    MCMD_FORWARD,    // start continuous forward trot
    MCMD_BACK,       // start continuous backward trot
    MCMD_STOP        // stop walking, return to broadbase
};

// Internal motion state machine state.
enum MotionState {
    MSTATE_IDLE,        // no motion; Ready() is not being called
    MSTATE_STARTUP,
    MSTATE_GETUP_PREP,   // -> RISING_POSE_0
    MSTATE_GETUP_PREP_1, // RISING_POSE_0 -> RISING_POSE_1
    MSTATE_GETUP_PREP_2, // RISING_POSE_1 -> RISING_POSE_2
    MSTATE_GETUP_PREP_3, // RISING_POSE_2 -> RISING_POSE_3
    MSTATE_GETUP,        // RISING_POSE_3 -> BROADBASE_POSE
    MSTATE_PREPA_REST_0,   // Intermediary state before SLEEPING_POSE
    MSTATE_PREPA_REST_1,
    MSTATE_PREPA_REST_2, 
    MSTATE_REST,         // SLEEPING_POSE_1 (final)
    MSTATE_WALK_A,       // 
    MSTATE_WALK_B,       // 
    MSTATE_WALK_C,       // 
    MSTATE_WALK_D,       // 
    MSTATE_WALK_E,       // 
    MSTATE_WALK_F,       // 
    MSTATE_WALK_G,       // 
    MSTATE_WALK_H,       // 
    MSTATE_TO_BASE       // returning to broadbase after stop
};

// ================================================================
//  TinyConsole
// ================================================================

class TinyConsole : public OObject
{
public:
    TinyConsole();
    virtual ~TinyConsole() {}
	
	static const int NUM_JOINTS         = 12; // 4 legs x 3 joints
    static const int NUM_CMD_VECTORS    = 2;  // double-buffered regions
	
    OSubject*  subject[numOfSubject];
    OObserver* observer[numOfObserver];

    // OPEN-R lifecycle
    virtual OStatus DoInit   (const OSystemEvent& event);
    virtual OStatus DoStart  (const OSystemEvent& event);
    virtual OStatus DoStop   (const OSystemEvent& event);
    virtual OStatus DoDestroy(const OSystemEvent& event);

    // ANT extra-entry callbacks
    void ListenCont (ANTENVMSG msg);
    void SendCont   (ANTENVMSG msg);
    void ReceiveCont(ANTENVMSG msg);
    void CloseCont  (ANTENVMSG msg);

    // OPEN-R subject Ready callback (called when joint-command data
    // is consumed by the actuator observer after each NotifyObservers()).
    void Ready(const OReadyEvent& event);

private:
    // ================================================================
    //  TCP functions
    // ================================================================

    OStatus Listen  ();
    OStatus Receive (int sizeMin, int sizeMax);
    OStatus Send    (const byte* data, int size);
    OStatus Close   ();

    // ================================================================
    //  AEAD functions
    // ================================================================

    void AdvanceNonce  (uint32_t& counter, bool isTx);
    bool AeadEncrypt   (const byte* plaintext, int ptLen,
                        byte* frameOut, int* frameLen);
    bool AeadDecrypt   (const byte* frame, int frameLen,
                        byte* plaintext, int* ptLen);

    // Signs (HANDSHAKE_CONTEXT || serverNonceSent_ || clientNonce) with
    // ROBOT_ED25519_SK and writes the 64-byte detached signature to
    // sigOut. Returns false only if the underlying crypto_sign() call
    // itself fails (it shouldn't, given a well-formed key).
    bool SignHandshake (const uint8_t* clientNonce,
                        uint8_t sigOut[HANDSHAKE_SIG_SIZE]);

    // Verifies the client's 64-byte handshake signature over the same
    // transcript the robot just signed (HANDSHAKE_CONTEXT ||
    // serverNonceSent_ || clientNonce), using the CLIENT_ED25519_PK.
    // Returns true only on a valid signature over the expected transcript.
    bool VerifyClientHandshake (const uint8_t* clientSig);

    // ================================================================
    //  Motion constants
    // ===============================================================

    // Step counts (each step = one ocommandMAX_FRAMES block = 16*frames = 16*8ms = 128ms).
    static const int STARTUP_COUNTER    =  24; // 3s
    static const int GETUP_COUNTER      =  6;  // 0.756s
    static const int REST_COUNTER       = 12;  // 1.512s
    static const int WALK_COUNTER       =  3;  // 0.756s
    static const int STOP_COUNTER       =  8;  // 1.008s

    // Servo gain values from ERS-7 Model Information (standard gains).
    // Joint order within a leg: J1 (hip rotation), J2 (hip pitch), J3 (knee).
    static const word J1_PGAIN = 0x001C;
    static const word J1_IGAIN = 0x0008;
    static const word J1_DGAIN = 0x0001;
    static const word J2_PGAIN = 0x0014;
    static const word J2_IGAIN = 0x0004;
    static const word J2_DGAIN = 0x0001;
    static const word J3_PGAIN = 0x001C;
    static const word J3_IGAIN = 0x0008;
    static const word J3_DGAIN = 0x0001;
    static const word PID_PSHIFT = 0x000E;
    static const word PID_ISHIFT = 0x0002;
    static const word PID_DSHIFT = 0x000F;

    // ================================================================
    //  Motion functions
    // ================================================================

    void          OpenPrimitives();
    void          NewCommandVectorData();
    void          SetJointGain();
    RCRegion*     FindFreeRegion();
    void          SetJointValue(RCRegion* rgn, int idx,
                                double startDeg, double endDeg);

    // Configure the interpolation parameters for the next motion segment.
    // 'from' and 'to' are 12-element degree arrays
    // Resets motionCounter_ to 0.
    void          SetupInterpolation(const double* from,
                                     const double* to,
                                     int           steps);

    // Send one command-vector frame along the interpolation trajectory
    // and advance motionCounter_.  Returns MOVING_FINISH when the last
    // frame has been sent.
    MovingResult  AdvanceInterpolation(int maxSteps);

    // Read current joint positions from hardware into a 12-element array.
    void          ReadCurrentPose(double* outDeg);

    // Apply a pending motion command: set up interpolation parameters and
    // transition motionState_.  Called from the top of Ready().
    void          BeginMotion(MotionCmd cmd);

    // If the motion state machine is idle, send a no-op "stay put" frame
    // so the OPEN-R scheduler will fire Ready() and pick up pendingCmd_.
    void          TriggerReady();

    // ================================================================
    //  Members
    // ================================================================

    // --- TCP ---
    antStackRef   ipstackRef;
    TCPConnection conn;

    // Per-session AEAD state
    uint8_t  sessionNonce[NONCE_SIZE];
    uint32_t txCounter;
    uint32_t rxCounter;
    bool     pendingClose;
    HandshakeStage handshakeStage;

    // Exact bytes sent as our nonce contribution in ListenCont(), kept
    // around because sessionNonce_[0..7] gets overwritten in place by
    // the client-nonce XOR before the handshake signature is computed.
    uint8_t  serverNonceSent[NONCE_SIZE];

    // The client's raw nonce, captured in phase 0. Needed again in phase 3
    // to rebuild the exact transcript the client signed for mutual auth.
    uint8_t  clientNonceRecv[NONCE_SIZE];

    // Receive-phase state: which kind of bytes we expect next.
    // See enum ReceivePhase
    ReceivePhase recvPhase;
    uint16_t pendingFrameLen;

    static uint32_t sessionId;

    // --- Motion ---
    OPrimitiveID  jointID[NUM_JOINTS];
    RCRegion*     cmdRegion[NUM_CMD_VECTORS];

    MotionState   motionState;
    MotionCmd     pendingCmd;
    int           motionCounter;
    bool 		  walkForward;

    // Interpolation trajectory: target[i] = start[i] + delta[i] * step
    double        motionStart[NUM_JOINTS];
    double        motionDelta[NUM_JOINTS];
};

#endif // TinyConsole_h_DEFINED
