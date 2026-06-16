//
// TinyConsole.h
// Minimal OPEN-R TCP console with ChaCha20+Poly1305 AEAD and leg motion.
//
//   PC client                         AIBO (this object)
//   ---------                         ------------------
//   connect()            ------>      ListenCont()
//                                       build 12-byte session nonce
//                                       send "CONSOLE_READY\n" + nonce (plaintext)
//   recv banner line + nonce
//   init ChaCha20+Poly1305 ctx
//   -- AEAD session starts here on both sides --
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
//   STOP       stop walking, return to broadbase
//   QUIT       close connection (existing)
//

#ifndef _TinyConsole_h_DEFINED
#define _TinyConsole_h_DEFINED

#include <OPENR/OObject.h>
#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/OPENRAPI.h>
#include <OPENR/OUnits.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <stdint.h>
#include <OPENR/core_macro.h>

#include "TCPConnection.h"
#include "ConsoleConfig.h"  // NONCE_SIZE, CHACHA_KEY, etc.
#include "rfc7539.h"        // chacha20poly1305_ctx, rfc7539_init/finish
#include "def.h"
#include "entry.h"

// ================================================================
//  Motion enumerations
// ================================================================

enum MovingResult {
	MOVING_CONT,
	MOVING_FINISH
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
    //  TCP helpers
    // ================================================================

    OStatus Listen  ();
    OStatus Receive (int sizeMin, int sizeMax);
    OStatus Send    (const byte* data, int size);
    OStatus Close   ();

    // ================================================================
    //  AEAD helpers
    // ================================================================

    void AdvanceNonce  (uint32_t& counter);
    bool AeadEncrypt   (const byte* plaintext, int ptLen,
                        byte* frameOut, int* frameLen);
    bool AeadDecrypt   (const byte* frame, int frameLen,
                        byte* plaintext, int* ptLen);

    // ================================================================
    //  Motion constants
    // ===============================================================

    // Step counts (each step = one ocommandMAX_FRAMES block = 128 ms).
    static const int STARTUP_COUNTER    = 24;
    static const int GETUP_COUNTER     = 6;  // 1 s to stand up
    static const int REST_COUNTER       = 12; // 2 s to lie down
    static const int WALK_SWING_COUNTER = 24; // 3 s to swing leg forward/backward
    static const int WALK_DROP_COUNTER  = 8;  // 1 s to drop foot to ground ("quickly")
    static const int STOP_COUNTER       = 8;  // 1 s to return to broadbase

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
    //  Motion helpers
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
    antStackRef   ipstackRef_;
    TCPConnection conn_;

    // Per-session AEAD state
    uint8_t  sessionNonce_[NONCE_SIZE];
    uint32_t txCounter_;
    uint32_t rxCounter_;
    bool     pendingClose_;

    // Two-phase receive state
    int      recvPhase_;
    uint16_t pendingFrameLen_;

    static uint32_t sessionId_;

    // --- Motion ---
    OPrimitiveID  jointID_[NUM_JOINTS];
    RCRegion*     cmdRegion_[NUM_CMD_VECTORS];

    MotionState   motionState_;
    MotionCmd     pendingCmd_;
    int           motionCounter_;

    // Interpolation trajectory: target[i] = start[i] + delta[i] * step
    double        motionStart_[NUM_JOINTS];
    double        motionDelta_[NUM_JOINTS];

    // Which direction the current walk is going.
    bool          walkForward_;
};

#endif // _TinyConsole_h_DEFINED
