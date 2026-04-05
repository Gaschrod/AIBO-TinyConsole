//
// Copyright 2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <iostream>
using std::cin;
using std::cout;
#include <OPENR/OSyslog.h>
#include "Crash.h"

void access_null_data_pointer()
{
    int *a = 0;
    OSYSPRINT(("*a is %d\n", *a)); // !!! EXCEPTION !!!
}

void access_null_text_pointer()
{
    void (*null_func_pointer)(void) = 0;
    null_func_pointer(); // !!! EXCEPTION !!!
}

void destroy_stack()
{
    char array_in_stack[10];
    strcpy(array_in_stack, "Length of this string is longer than 10.");
    return; // !!! EXCEPTION !!!
}

struct sample_struct {
    short a;
    char b[4];
};

void cause_address_miss_alignment()
{
    sample_struct ss;
    char data[] = "123";
    ss.a = 1;
    *(unsigned int *)ss.b = *(unsigned int *)data; // !!! EXCEPTION !!!
    OSYSPRINT(("ss.a is %d, ss.b is %s\n", ss.a, ss.b));
}

void use_unusable_coprocessor()
{
    asm("mfc0 $2, $12");
}

static uint32 broken_text[] = {
    0x00000000, // nop
    0x0000003d,
    0x03e00008, // jr r31
    0x00000000, // nop
};

void jump_to_broken_text()
{
    void (*func_pointer)() = (void (*)())broken_text;
    func_pointer(); // !!! EXCEPTION !!!
}

void cause_tlb_modification_error()
{
    MemoryRegionID memid;
    unsigned char *addr;
    sError ans;
    ans = NewSharedMemoryRegion(4096, MEMPROTINFO_R, &memid, (void**)&addr);
    if (ans != sSUCCESS) {
        printf("test failed!\n");
        return;
    }
    *addr = 0x1; // !!! EXCEPTION !!!
    DeleteSharedMemoryRegion(memid);
}

void overflow_on_convert()
{
    float a = (float) 0xffffffff;
    for (int i = 0; i < 20; i++) {
        a *= 2.0;
        OSYSPRINT(("a is %ud\n", (unsigned int) a)); // !!! EXCEPTION !!!
    }
}

void not_a_number_source()
{
    float a = 0.0 / 0.0;
    for (int i = 0; i < 20; i++) {
        a *= 3.0; // !!! EXCEPTION !!!
        OSYSPRINT(("a is %f\n", a));
    }
}

float denormalized_number_source_body(float a)
{
    return a * 3.0;
}

void denormalized_number_source()
{
    float a = 1.0e-37 / 100.0;
    float b = denormalized_number_source_body(a); // !!! EXCEPTION !!!
    OSYSPRINT(("a * 3.0: %e\n", b));
}

struct crash_func_entry {
    void (*func)();
    const char *help;
};

crash_func_entry crash_func_table[] = {
    { access_null_data_pointer, "access null data pointer" },
    { access_null_text_pointer, "access null text pointer" },
    { destroy_stack, "destroy stack" },
    { cause_address_miss_alignment, "cause address miss alignment" },
    { use_unusable_coprocessor, "use unusable coprocessor" },
    { jump_to_broken_text, "jump to broken text" },
    { cause_tlb_modification_error,
      "cause TLB modification error (memprot only)" },
    { overflow_on_convert, "overflow on convert" },
    { not_a_number_source, "use not-a-number source" },
    { denormalized_number_source, "use denormalized number source" },
};

Crash::Crash ()
{
}

OStatus
Crash::DoInit(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
Crash::DoStart(const OSystemEvent& event)
{
    OSYSPRINT(("\n=== Crash Menu ===\n"));
    uint32 crash_func_table_max =
        sizeof(crash_func_table) / sizeof(crash_func_entry);
    uint32 i;
    for (;;) {
        for (int j = 0; j < crash_func_table_max; j++) {
            OSYSPRINT((" %d. %s\n", j, crash_func_table[j].help));
        }
        cout << "\nselect: ";
        cin >> i;
        if (i < crash_func_table_max) {
            break;
        }
    }
    OSYSPRINT(("Crash ...\n"));
    crash_func_table[i].func();

    return oSUCCESS;
}    

OStatus
Crash::DoStop(const OSystemEvent& event)
{
    return oSUCCESS;
}

OStatus
Crash::DoDestroy(const OSystemEvent& event)
{
    return oSUCCESS;
}
