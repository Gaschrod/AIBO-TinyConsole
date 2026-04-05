//
// Copyright 1999,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#ifndef _SECmdStruct_h_DEFINED
#define _SECmdStruct_h_DEFINED

#include <string.h>
#include <OMWares/MMM_CommandList.h>

#define PASSWORD_OF_SECMDSTRUCT   "OMSE"
#define VERSION_ID_OF_SECMDSTRUCT 0x02000002

struct SECommand {
    unsigned int  Command_;       // + 4 =  4 
    unsigned int  time_;          // + 4 =  8
    int           tilt_;          // + 4 = 12
    int           pan_;           // + 4 = 16
    unsigned int  cycle_;         // + 4 = 20
    unsigned int  VersionID_;     // + 4 = 24
    char          Password_[8];   // + 8 = 32
    SECommand() {
        Command_     = mmmscTailHome;
        time_        = 0;
        tilt_        = 0;
        pan_         = 0;
        cycle_       = 0;
        strcpy(Password_, PASSWORD_OF_SECMDSTRUCT);
        VersionID_  = VERSION_ID_OF_SECMDSTRUCT;
    };
    void SetUp(void)
        {
            Command_     = mmmscTailHome;
            time_        = 0;
            tilt_        = 0;
            pan_         = 0;
            cycle_       = 0;
            strcpy(Password_, PASSWORD_OF_SECMDSTRUCT);
            VersionID_ = VERSION_ID_OF_SECMDSTRUCT;
        }
};

#endif // _SECmdStruct_h_DEFINED
