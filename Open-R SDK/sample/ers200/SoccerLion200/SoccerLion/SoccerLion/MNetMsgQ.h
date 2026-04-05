//
// Copyright 2001,2002 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <OPENR/OStatus.h>
#include <OMWares/OMoNetMessage.h>

#ifndef  _MNetMsgQ_h_DEFINED
#define  _MNetMsgQ_h_DEFINED

struct MNMsgData
{
    MNMsgData*     next_;
    OMoNetMessage  msg_;

    MNMsgData(void){
        next_ = static_cast<MNMsgData*>(NULL);
        return;
    }
};


class MNetMsgQ{
public:
    MNetMsgQ(void);
    ~MNetMsgQ() {}

    OStatus        AddMoNetMessage(OMoNetMessage* msg);
    OMoNetMessage* GetMoNetMessage(void);
    OStatus        DeleteFirstOne(void);
    void           Print(void) const;

private:
    MNMsgData* first_;
    MNMsgData* last_;
};

#endif // _MNetMsgQ_h_DEFINED
