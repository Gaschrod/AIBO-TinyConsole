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

#ifndef _LE2CmdStruct_h_DEFINED
#define _LE2CmdStruct_h_DEFINED

#define PASSWORD_OF_LE2CMDSTRUCT   "OMLE2"
#define VERSION_ID_OF_LE2CMDSTRUCT 0x02000001

struct LE2Command {
    unsigned int  OrgCommand;    // + 4 =  4 
    unsigned int  CusCommand;    // + 4 =  8
    int           TransVector;   // + 4 = 12
    int           RotatVector;   // + 4 = 16
    int           times;         // + 4 = 20
    unsigned int  VersionID;     // + 4 = 24
    char          Password[8];   // + 8 = 32
    LE2Command() {
        OrgCommand  = LE2COM_BACK_TO_STANDING;
        CusCommand  = 0xFFFFFFFF;
        TransVector = 0;
        RotatVector = 0;
        times       = 1;
        strcpy(Password, PASSWORD_OF_LE2CMDSTRUCT);
        VersionID   = VERSION_ID_OF_LE2CMDSTRUCT;
    }
};

#endif // _LE2CmdStruct_h_DEFINED
