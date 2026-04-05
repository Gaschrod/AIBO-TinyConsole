//
// Copyright 2001,2002,2003 Sony Corporation 
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//

#include <iostream>
using std::cout;
using std::endl;
#include "MNetMsgQ.h"

MNetMsgQ::MNetMsgQ(void)
{
    first_ = static_cast<MNMsgData*>(NULL);
    last_  = static_cast<MNMsgData*>(NULL);
}


OStatus    
MNetMsgQ::AddMoNetMessage(OMoNetMessage* msg)
{
    MNMsgData* tmp = new MNMsgData;
    if (tmp != static_cast<MNMsgData*>(NULL)) {
        tmp->msg_ = *msg;
        if (last_ != static_cast<MNMsgData*>(NULL)) {
            last_->next_ = tmp;
            last_ = tmp;
        } else {
            first_ = tmp;
            last_  = tmp;
        }
        return oSUCCESS;
    } else {
        cout << " --- Fail MsgQ Add ---"    << endl
             << (void*)(msg->command_) << " "
             << (void*)(msg->network_) << " "
             << (char*)(msg->motion_)   << " "
             << (char*)(msg->startpos_) << " "
             << (char*)(msg->endpos_)   << endl;

        return oFAIL;
    }
}


OMoNetMessage*
MNetMsgQ::GetMoNetMessage(void)
{
    if (first_ != static_cast<MNMsgData*>(NULL)) {
        return &(first_->msg_);
    } else {
        return static_cast<OMoNetMessage*>(NULL);
    }
}


OStatus
MNetMsgQ::DeleteFirstOne(void)
{
    if (first_ != static_cast<MNMsgData*>(NULL)) {
        MNMsgData* tmp = first_;
        if (first_->next_ != static_cast<MNMsgData*>(NULL)) {
            first_ = first_->next_;
        } else {
            first_ = last_ = static_cast<MNMsgData*>(NULL);
        }
        delete tmp;	
    }

    return oSUCCESS;
}


void
MNetMsgQ::Print(void) const
{
    MNMsgData* tmp = first_;
    int i = 0;

    cout << " --- Info --- MoNetMessage Q" << endl;
    while (tmp != (MNMsgData*)0) {
        cout << " [" << i++ << "] " 
             << (void*)(tmp->msg_.command_) << " "
             << (void*)(tmp->msg_.network_) << " "
             << (char*)(tmp->msg_.motion_)   << " "
             << (char*)(tmp->msg_.startpos_) << " "
             << (char*)(tmp->msg_.endpos_)   << endl;

        tmp = tmp->next_;
    }
    cout << endl;

    return;
}
