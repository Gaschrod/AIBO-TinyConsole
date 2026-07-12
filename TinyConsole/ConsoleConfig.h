//
// ConsoleConfig.h
// Tunables for the TinyConsole OPEN-R object.
//

#ifndef _ConsoleConfig_h_DEFINED
#define _ConsoleConfig_h_DEFINED

#include <stdint.h>     // uint8_t, uint16_t, uint32_t, uint64_t, int64_t

// ---------------------------------------------------------------
//  Network
// ---------------------------------------------------------------
#define CONSOLE_PORT    7777
#define CONSOLE_BUFSIZE 4096

// ---------------------------------------------------------------
//  ChaCha20+Poly1305 key  (32 bytes, must match the client)
//
//  Replace these placeholder bytes with your own secret before
//  deployment.  Keep the key out of version control.
// ---------------------------------------------------------------
static const uint8_t CHACHA_KEY[32] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
};

// ---------------------------------------------------------------
//  Nonce (12 bytes, RFC 7539 layout, built per-session)
//
//  bytes [0..3]  : session ID — static counter incremented on every
//                  ListenCont().  Resets to 0 on reboot: document
//                  this limitation for your deployment.
//  bytes [4..7]  : client IPv4 address (little-endian).
//  bytes [8..11] : per-message counter (little-endian), starts at 0
//                  for each session and is incremented before every
//                  call to rfc7539_init (both TX and RX directions).
// ---------------------------------------------------------------
#define NONCE_SIZE  12

// ---------------------------------------------------------------
//  AEAD wire frame (everything after the plaintext banner):
//
//    [ uint16_t ciphertext_length (LE) ]
//    [ ciphertext                      ]   <- same length as plaintext
//    [ Poly1305 tag                    ]   <- always 16 bytes
//
//  CONSOLE_MAX_PLAINTEXT is the largest command or response that
//  fits in one frame while keeping the total frame <= CONSOLE_BUFSIZE.
// ---------------------------------------------------------------
#define FRAME_HEADER_SIZE     2
#define FRAME_TAG_SIZE        16
#define CONSOLE_MAX_PLAINTEXT (CONSOLE_BUFSIZE - FRAME_HEADER_SIZE - FRAME_TAG_SIZE)

// ---------------------------------------------------------------
//  Protocol
//  Plaintext banner sent BEFORE AEAD begins, immediately followed
//  by NONCE_SIZE raw nonce bytes (not text-encoded).
//  Client reads until '\n', then reads NONCE_SIZE more bytes.
// ---------------------------------------------------------------
#define CONSOLE_BANNER  "CONSOLE_READY\n"

#endif // _ConsoleConfig_h_DEFINED
