// ============================================================================
//  replay_counter.cc
//  Two-slot, CRC-validated persistent anti-replay counter on the Memory Stick.
// ============================================================================

#include "replay_counter.h"

#include <stdio.h>
#include <string.h>

// ---- Configuration ---------------------------------------------------------
// Directory must already exist on the stick
#define RC_SLOT0_PATH "/MS/OPEN-R/MW/CONF/RPLYCTR0.BIN"
#define RC_SLOT1_PATH "/MS/OPEN-R/MW/CONF/RPLYCTR1.BIN"

// Record: 4-byte magic 'R''C''0''1' | 8-byte counter (big-endian) | 4-byte CRC-32
#define RC_MAGIC0 0x52u  // 'R'
#define RC_MAGIC1 0x43u  // 'C'
#define RC_MAGIC2 0x30u  // '0'
#define RC_MAGIC3 0x31u  // '1'
#define RC_RECORD_LEN 16

// ---- Module state ----------------------------------------------------------
static int      g_current_slot = -1;   // -1 = none authoritative yet (first boot)
static uint64_t g_counter      = 0;

// ---- CRC-32 bitwise ------------------
// Only 12 bytes are hashed per call, so a table is unnecessary.
static uint32_t rc_crc32(const uint8_t* data, unsigned int len)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (unsigned int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            if (crc & 1u) crc = (crc >> 1) ^ 0xEDB88320u;
            else          crc =  crc >> 1;
        }
    }
    return ~crc;
}

// ---- Big-endian serialisation of the 64-bit counter --------------------
static void rc_put_be64(uint8_t* p, uint64_t v)
{
    for (int i = 7; i >= 0; --i) { p[i] = (uint8_t)(v & 0xFFu); v >>= 8; }
}

static uint64_t rc_get_be64(const uint8_t* p)
{
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) { v = (v << 8) | (uint64_t)p[i]; }
    return v;
}

// ---- Per-slot load ---------------------------------------------------------
typedef enum { RC_OK, RC_ABSENT, RC_CORRUPT } rc_status;

static rc_status rc_load_slot(const char* path, uint64_t* out)
{
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) return RC_ABSENT;            // not present / unreadable

    uint8_t buf[RC_RECORD_LEN];
    size_t n = fread(buf, 1, RC_RECORD_LEN, fp);
    fclose(fp);

    if (n != RC_RECORD_LEN) return RC_CORRUPT;
    if (buf[0] != RC_MAGIC0 || buf[1] != RC_MAGIC1 ||
        buf[2] != RC_MAGIC2 || buf[3] != RC_MAGIC3) return RC_CORRUPT;

    uint32_t stored = ((uint32_t)buf[12] << 24) | ((uint32_t)buf[13] << 16) |
                      ((uint32_t)buf[14] <<  8) |  (uint32_t)buf[15];
    if (stored != rc_crc32(buf, 12)) return RC_CORRUPT;

    *out = rc_get_be64(buf + 4);
    return RC_OK;
}

// ---- Per-slot write (whole record, fresh file) -----------------------------
static bool rc_write_slot(int slot, uint64_t value)
{
    const char* path = (slot == 0) ? RC_SLOT0_PATH : RC_SLOT1_PATH;

    uint8_t buf[RC_RECORD_LEN];
    buf[0] = RC_MAGIC0; buf[1] = RC_MAGIC1; buf[2] = RC_MAGIC2; buf[3] = RC_MAGIC3;
    rc_put_be64(buf + 4, value);
    uint32_t crc = rc_crc32(buf, 12);
    buf[12] = (uint8_t)(crc >> 24); buf[13] = (uint8_t)(crc >> 16);
    buf[14] = (uint8_t)(crc >>  8); buf[15] = (uint8_t)(crc);

    FILE* fp = fopen(path, "wb");
    if (fp == NULL) return false;                // if write-protect switch on (example)

    size_t n = fwrite(buf, 1, RC_RECORD_LEN, fp);
    fflush(fp);                                  // push out of stdio buffers
    if (fclose(fp) != 0) return false;           // commit to the Memory Stick
    return (n == RC_RECORD_LEN);
}

// ---- Functions ------------------------------------------------------------
bool ReplayCounter_Load(uint64_t* out)
{
    uint64_t c0 = 0, c1 = 0;
    rc_status s0 = rc_load_slot(RC_SLOT0_PATH, &c0);
    rc_status s1 = rc_load_slot(RC_SLOT1_PATH, &c1);

    bool v0 = (s0 == RC_OK);
    bool v1 = (s1 == RC_OK);

    if (v0 && v1) {
        // Newest = larger counter (always persist prev+1 to the other slot)
        if (c0 >= c1) { g_counter = c0; g_current_slot = 0; }
        else          { g_counter = c1; g_current_slot = 1; }
    } else if (v0) {
        g_counter = c0; g_current_slot = 0;
    } else if (v1) {
        g_counter = c1; g_current_slot = 1;
    } else {
        // No valid slot. If both were absent -> first-ever boot: equals 0
        // Otherwise a file exists but is corrupt -> tampering and refuse to serve (fail closed)
        if (s0 == RC_ABSENT && s1 == RC_ABSENT) {
            g_counter = 0; g_current_slot = -1;
        } else {
            return false;
        }
    }

    *out = g_counter;
    return true;
}

bool ReplayCounter_Persist(uint64_t value)
{
    // Write to the slot that is not authoritative. The current one stays intact
    // until the new write has fully committed. (-1 -> first write lands in slot 0)
    int target = (g_current_slot == 0) ? 1 : 0;
    if (!rc_write_slot(target, value)) return false;

    g_current_slot = target;
    g_counter = value;
    return true;
}