/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _antEnvBaseMsg_H_
#define _antEnvBaseMsg_H_

#include <antTypes.h>
#include <antModuleRef.h>
#include <TID.h>

//
// link to Aperios message passing
//

enum
{
  Entry_antEnv_Initialize = 0,
  Entry_antEnv_HartBeat,
  Entry_antEnv_External,

  antEnv_numEntries
};
  
//
// a small class to hide the stack identifier (OID, MAV_Dispatcher*, name of the stack)
//

// forward declaration
class MAV_Dispatcher;


class antStackRef
{
public:
  antStackRef();
  antStackRef(const OID& oid_);
  antStackRef(const MAV_Dispatcher* disp_);
  antStackRef(char* name_);
  antStackRef& operator=(const antStackRef& ref_);
  ~antStackRef();

  OID& get_oid();
  MAV_Dispatcher* get_disp();
  
  bool IsValid();
  
private:
  OID oid;
  MAV_Dispatcher* disp;
};

inline
antStackRef::antStackRef()
{
  oid.MakeInvalid();
  disp = 0;
}


inline
antStackRef::antStackRef(const OID& oid_)
{
  disp = 0;
  oid  = oid_;
}

inline
antStackRef::antStackRef(const MAV_Dispatcher* disp_)
{
  disp = (MAV_Dispatcher*) disp_;
  oid.MakeInvalid();
}


inline
antStackRef&
antStackRef::operator=(const antStackRef& ref_)
{
  if (&ref_ != this)
  {
    oid  = ref_.oid;
    disp = ref_.disp;
  }
  return *this;
}

  
inline
OID&
antStackRef::get_oid()
{
  return oid;
}

inline
MAV_Dispatcher*
antStackRef::get_disp()
{
  return disp;
}


inline
bool
antStackRef::IsValid()
{
  return (oid.IsValid() || (0 != disp));
}

//
// constructor with name is dependent upon metaspace
// and is defined in the corresponding libantXXX
// libraries
//

  
//
// the basic external message
//

enum antEnvMsgInvocation
{
  ANTENVMSG_SEND = 0,
  ANTENVMSG_CALL
};

class antEnvMsgContInfo: public TID
{
public:
  antEnvMsgContInfo(): TID() {};
  antEnvMsgContInfo(TID& _tid): TID(_tid) {};
  antEnvMsgContInfo(OID _target, uint32 _selector): TID(_target, _selector) {};

  OID GetOID() {return TID::GetMailer();};
  uint32 GetSelector() {return TID::GetValue();};
};

struct antEnvMsg
{
public:
  // destination info
  Selector dstSelector;
  antModuleRef dstModule;   // if 0 we send to the antEnv directly
  int32 dstOperation;

  // sender of message
  OID sender;

  // message size
  int32 msgSize;

  // where does the result need to go
  antEnvMsgInvocation invocationType;
  antEnvMsgContInfo contInfo;

  // continuation pointer
  void* continuation;

public:
  // constructors
  antEnvMsg() {};
  antEnvMsg(antEnvMsg& _msg);
  antEnvMsg(int32 _operation, Selector selector = Entry_antEnv_External);
  antEnvMsg(antModuleRef _module,
            int32 _operation,
            Selector selector = Entry_antEnv_External);

  // assignment
  antEnvMsg& operator=(antEnvMsg& _msg);

  // message passing

  antError Reply();

#ifndef ANT_INTERNAL

  antError Send(antStackRef _target, OID& _sender, int32 _selector, int _msgSize);
  antError Call(antStackRef _dest, int _msgSize);
  static antEnvMsg* Receive(ANTENVMSG _msg);

  int32 GetOperation();
  OID& GetSender();

#endif // ANT_INTERNAL

private:
  // so we know who the sender is
  static boolean myOIDValid;
  static OID myOID;
};

//
// inline definitions
//

inline
antEnvMsg::antEnvMsg(antEnvMsg& _msg)
{
  dstSelector = _msg.dstSelector;
  dstModule = _msg.dstModule;
  dstOperation = _msg.dstOperation;
  msgSize = _msg.msgSize;
  invocationType = _msg.invocationType;
  contInfo = _msg.contInfo;
  sender = _msg.sender;
  continuation = _msg.continuation;
}

inline
antEnvMsg::antEnvMsg(
                        antModuleRef _module,
                        int32 _operation,
                        Selector _selector
                      )
{
  // destination info
  dstSelector = _selector;
  dstModule = _module;
  dstOperation = _operation;
}

inline
antEnvMsg::antEnvMsg(int32 _operation, Selector _selector)
{
  // destination info
  dstSelector = _selector;
  dstOperation = _operation;
}


inline
antEnvMsg&
antEnvMsg::operator=(antEnvMsg& _msg)
{
  dstSelector = _msg.dstSelector;
  dstModule = _msg.dstModule;
  dstOperation = _msg.dstOperation;
  msgSize = _msg.msgSize;
  invocationType = _msg.invocationType;
  contInfo = _msg.contInfo;
  sender = _msg.sender;
  continuation = _msg.continuation;

  return *this;
}

#endif  // _antEnvBaseMsg_H_

