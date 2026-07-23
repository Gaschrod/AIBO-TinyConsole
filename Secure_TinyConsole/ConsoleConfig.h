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
//                  ListenCont(). Low 32 bits of the persistent session counter
//                  that survives reboots (stored in Memory Stick)
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

// Fixed-size padding to mitigate encrypted traffic-analysis
// Every frame carries exactly PAD_BLOCK plaintext bytes -> all frames are identical in size
// Real length: carried in 2-byte LE prefix **INSIDE**  encrypted block
//      padded plaintext = [uint16_t LE real_len][payload][padding]
//
// If message size were to grow in size (for example encrypted video stream), 
// PAD_BLOCK should be increased (512, 1024, 2048, etc)

#define PAD_LEN_PREFIX 2
#define PAD_BLOCK 256 // /!\ must be smaller than CONSOLE_MAX_PLAINTEXT /!\ 

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
    0x40, 0x17, 0x90, 0xF9, 0x5E, 0x5F, 0x57, 0x20,
    0xE7, 0x61, 0x31, 0x73, 0xBF, 0x71, 0x4A, 0xD0,
    0x92, 0x01, 0xFC, 0x99, 0x5B, 0xAE, 0x18, 0x8F,
    0xFF, 0x3C, 0x28, 0x49, 0xCA, 0x2C, 0x8B, 0x57,
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
    0xCE, 0x19, 0x4B, 0xF6, 0xC6, 0xCF, 0xD8, 0x02,
    0x0F, 0xC1, 0xED, 0x6F, 0xA0, 0xEF, 0x74, 0xCF,
    0x5B, 0x29, 0x74, 0x58, 0xF6, 0x0B, 0xB1, 0xA8,
    0xDC, 0xFE, 0xC7, 0xAA, 0x80, 0x4D, 0xFB, 0x49,
    0xA7, 0x52, 0xAF, 0x21, 0x83, 0xBC, 0x58, 0xC2,
    0x2F, 0x0D, 0x3E, 0x53, 0x0C, 0x66, 0x9A, 0x95,
    0xF4, 0xE2, 0x08, 0xF7, 0x1C, 0x5C, 0x6E, 0xC9,
    0x01, 0xF6, 0xA2, 0x3B, 0x9A, 0x23, 0x39, 0x33,
};

static const uint8_t ROBOT_ED25519_PK[32] = {
    0xA7, 0x52, 0xAF, 0x21, 0x83, 0xBC, 0x58, 0xC2,
    0x2F, 0x0D, 0x3E, 0x53, 0x0C, 0x66, 0x9A, 0x95,
    0xF4, 0xE2, 0x08, 0xF7, 0x1C, 0x5C, 0x6E, 0xC9,
    0x01, 0xF6, 0xA2, 0x3B, 0x9A, 0x23, 0x39, 0x33,
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
    0x8E, 0x89, 0x44, 0x6B, 0x55, 0xFA, 0x27, 0x58,
    0xBC, 0xB4, 0x03, 0xF7, 0x17, 0xCC, 0x0C, 0xD1,
    0xB5, 0xD9, 0x05, 0x61, 0x9D, 0xED, 0x92, 0x90,
    0x77, 0xD1, 0x21, 0x96, 0xF9, 0xCD, 0xEB, 0xFD,
};

// Domain-separation tag mixed into every signed handshake transcript.
// Must match HANDSHAKE_CONTEXT in aibo_link.py / chacha20_console_client.py
// byte-for-byte!
static const char HANDSHAKE_CONTEXT[] = "AIBO-TinyConsole-Handshake";
#define HANDSHAKE_CONTEXT_LEN (sizeof(HANDSHAKE_CONTEXT) - 1)

#define HANDSHAKE_SIG_SIZE 64

#endif // ConsoleConfig_h_DEFINED
