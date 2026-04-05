//
// Copyright (C) 2001 Sony Corporation
// All Rights Reserved.
//

#ifndef stub_macro_h_DEFINED
#define stub_macro_h_DEFINED

#include <OPENR/OSubject.h>
#include <OPENR/OObserver.h>
#include <OPENR/ObjcommMessages.h>
#include <OPENR/OPENREvent.h>

//
// Connecting token
//
#define _Hook(name)    _##name
#define _Control(name) _Control##name
#define _Ready(name)   _Ready##name
#define _Connect(name) _Connect##name
#define _Notify(name)  _Notify##name

//
//  Function Declaration
//
typedef void (*_Hook)    (OReasonMessage*);
typedef void (*_Control) (OControlMessage*);
typedef void (*_Ready)   (OReadyMessage*);
typedef void (*_Connect) (OConnectMessage*);
typedef void (*_Notify)  (ONotifyMessage*);


//
//  Macro for Stub Function Declaration
//
#define StubFuncDeclare_Basic(name) \
extern void _Hook(name)    (OReasonMessage*);

#define StubFuncDeclare_Subject(name) \
extern void _Control(name) (OControlMessage*); \
extern void _Ready(name)   (OReadyMessage*);

#define StubFuncDeclare_Observer(name) \
extern void _Connect(name) (OConnectMessage*); \
extern void _Notify(name)  (ONotifyMessage*);

//
//  Macro for Stub Function Definitions
//
#define StubFuncDefine_Basic(name) \
extern "C" void \
_Hook(name)(OReasonMessage* reasonMsg) \
{ \
    Self.name( OSystemEvent(reasonMsg->reason, \
			    reasonMsg->param, reasonMsg->paramSize) ); \
    Exit(); \
}

#define StubFuncDefine_Control_nofunc(id) \
extern "C" void \
_Control(id)(OControlMessage* msg) \
{ \
    Self.subject[id]->ControlHandler(*msg); \
    /* Support for Self connection  */ \
    if (msg->observerID.oid != Self.subject[id]->GetID().oid){ \
       Exit(); \
    } \
} \

#define StubFuncDefine_Control(id, funcname) \
extern "C" void \
_Control(id)(OControlMessage* msg) \
{ \
    OStatus status \
	= Self.funcname( OControlEvent(id, msg->observerID, \
				       msg->subName, msg->command) ); \
    Self.subject[id]->ControlHandler(*msg, status); \
    /* Support for Self connection  */ \
    if (msg->observerID.oid != Self.subject[id]->GetID().oid){ \
       Exit(); \
    } \
} \


#define StubFuncDefine_Ready_nofunc(id) \
extern "C" void \
_Ready(id)(OReadyMessage* msg) \
{ \
    Self.subject[id]->ReadyHandler(*msg); \
    Exit(); \
}

#define StubFuncDefine_Ready(id, funcname) \
extern "C" void \
_Ready(id)(OReadyMessage* msg) \
{ \
    Self.subject[id]->ReadyHandler(*msg); \
    Self.funcname(OReadyEvent(id, msg->observerID, msg->command)); \
    Exit(); \
}

#define StubFuncDefine_Connect_nofunc(id) \
extern "C" void \
_Connect(id)(OConnectMessage* msg) \
{ \
    Self.observer[id]->ConnectHandler(*msg); \
    Exit(); \
} \


#define StubFuncDefine_Connect(id, funcname) \
extern "C" void \
_Connect(id)(OConnectMessage* msg) \
{ \
    OStatus status \
	= Self.funcname(OConnectEvent(id, msg->subName, msg->command)); \
    Self.observer[id]->ConnectHandler(*msg, status); \
    Exit(); \
} \

#define StubFuncDefine_Notify(id, funcname) \
extern "C" void \
_Notify(id)(ONotifyMessage* msg) \
{ \
    ONotifyEvent event;  \
    event.SetIndex(id);  \
    Self.observer[id]->NotifyHandler(*msg, &event); \
    Self.funcname( event ); \
    Exit(); \
}

#define StubFuncDefine_Notify_nofunc(id) \
extern "C" void \
_Notify(id)(ONotifyMessage* msg) \
{ \
    ONotifyEvent event;  \
    event.SetIndex(id);  \
    Self.observer[id]->NotifyHandler(*msg, &event); \
    Exit(); \
}

#endif /* stub_macro_h_DEFINED */
