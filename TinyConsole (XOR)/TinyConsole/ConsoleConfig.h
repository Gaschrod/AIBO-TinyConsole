//
// ConsoleConfig.h
// Tunables for the TinyConsole OPEN-R object.
//

#ifndef _ConsoleConfig_h_DEFINED
#define _ConsoleConfig_h_DEFINED

// ---------------------------------------------------------------
//  Network
// ---------------------------------------------------------------
#define CONSOLE_PORT    7777        // any non-telnet port
#define CONSOLE_BUFSIZE 4096        // shared ANT buffer size

// ---------------------------------------------------------------
//  XOR key  — must match xor_console_client.py exactly
// ---------------------------------------------------------------
static const unsigned char XOR_KEY[]  = { 0xA5, 0x3C, 0x7F, 0x11, 0xDE };
static const int           XOR_KEYLEN = 5;

// ---------------------------------------------------------------
//  Protocol
// Plain-text banner sent BEFORE XOR begins so the client knows
// the session is live and when to start decoding.
// ---------------------------------------------------------------
#define CONSOLE_BANNER  "CONSOLE_READY\n"

#endif // _ConsoleConfig_h_DEFINED

