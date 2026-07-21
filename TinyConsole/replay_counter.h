#ifndef REPLAY_COUNTER_H
#define REPLAY_COUNTER_H

// ============================================================================
//  replay_counter.h
//  Persistent, monotonic anti-replay counter for TinyConsole.
//
//  Storage: two-slot (ping-pong) -> each update is written WHOLE to the
//  slot that is not currently authoritative.
//  In case of a power loss/CPU crash/fatale error during the write -> leaves the previous value intact. 
//  The authoritative value is the larger of the two slots that pass a 4-byte magic + CRC-32 check.
// ============================================================================

#include <stdint.h>

// Load the last persisted value:
//   - Returns true and sets *out on success. 
//   - Returns false only when a slot file exists but neither slot is valid
//     -> corruption/tampering: the caller MUST fail closed
// Call once at start-up (DoInit / DoStart).
bool ReplayCounter_Load(uint64_t* out);

// Persist value to the alternate slot and make it authoritative.
//   - Returns false on any I/O failure (example: Memor Stick write-protect switch on)
// Call once per connection before the value is used in the handshake.
// A crash can only skip values forward and never reuse one. On false, the
// caller MUST abort the handshake.
bool ReplayCounter_Persist(uint64_t value);

#endif // REPLAY_COUNTER_H