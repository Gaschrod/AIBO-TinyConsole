// ConsoleConfig.h

#ifndef ConsoleConfig_h_DEFINED
#define ConsoleConfig_h_DEFINED

#include <stdint.h>     // uint8_t, uint16_t, uint32_t, uint64_t, int64_t

// ---------------------------------------------------------------
//  Network
// ---------------------------------------------------------------
#define CONSOLE_PORT    7777
#define CONSOLE_BUFSIZE 4096

// ---------------------------------------------------------------
//  Nonce (12 bytes, RFC 7539 layout, built per-session)
//
//  bytes [0..3]  : session ID — static counter incremented on every
//                  ListenCont().  Resets to 0 on reboot
//  bytes [4..7]  : client IPv4 address (little-endian)
//  bytes [8..11] : per-message counter (little-endian), starts at 0
//                  for each session and is incremented before every
//                  call to rfc7539_init (both TX and RX directions)
// ---------------------------------------------------------------
#define NONCE_SIZE  12

// ---------------------------------------------------------------
//  AEAD wire frame:
//
//    uint16_t ciphertext_length (LE)
//    ciphertext <- same length as plaintext
//    Poly1305 tag <- always 16 bytes
//
//  CONSOLE_MAX_PLAINTEXT = largest command/response that
//  fits in one frame while keeping the total frame <= CONSOLE_BUFSIZE.
// ---------------------------------------------------------------
#define FRAME_HEADER_SIZE     2
#define FRAME_TAG_SIZE        16
#define CONSOLE_MAX_PLAINTEXT (CONSOLE_BUFSIZE - FRAME_HEADER_SIZE - FRAME_TAG_SIZE)

// ---------------------------------------------------------------
//  Plaintext banner sent BEFORE AEAD begins, immediately followed
//  by NONCE_SIZE raw nonce bytes (not text-encoded).
//  Client reads until '\n', then reads NONCE_SIZE more bytes.
// ---------------------------------------------------------------
#define CONSOLE_BANNER  "CONSOLE_READY\n"

// ---------------------------------------------------------------
//  ChaCha20+Poly1305 key  (32 bytes, must match the client)
// ---------------------------------------------------------------
static const uint8_t CHACHA_KEY[32] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};


// ---------------------------------------------------------------
//  Ed25519 robot identity (handshake authentication)
//
//  Generated offline
//
//  ROBOT_ED25519_SK uses TweetNaCl's "combined" secret-key format:
//  bytes [0..31]  = the 32-byte seed
//  bytes [32..63] = the corresponding 32-byte public key
//
//  ROBOT_ED25519_SK = !secret! key
//
//  ROBOT_ED25519_PK = public key (last 32 bytes of ROBOT_ED25519_SK)
// ---------------------------------------------------------------
static const uint8_t ROBOT_ED25519_SK[64] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

static const uint8_t ROBOT_ED25519_PK[32] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

// ---------------------------------------------------------------
//  Ed25519 client identity (handshake authentication)
//
//  The robot uses crypto_sign_open() with this public key to verify
//  that the computer connecting to it holds the matching private seed.
//
//  Use the keys_generator.py script to generate a new client keypair.
// ---------------------------------------------------------------
static const uint8_t CLIENT_ED25519_PK[32] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

// Domain-separation tag mixed into every signed handshake transcript.
// Must match HANDSHAKE_CONTEXT in aibo_link.py / chacha20_console_client.py
// byte-for-byte!
static const char HANDSHAKE_CONTEXT[] = "AIBO-TinyConsole-Handshake";
#define HANDSHAKE_CONTEXT_LEN (sizeof(HANDSHAKE_CONTEXT) - 1)

#define HANDSHAKE_SIG_SIZE 64

#endif // ConsoleConfig_h_DEFINED
