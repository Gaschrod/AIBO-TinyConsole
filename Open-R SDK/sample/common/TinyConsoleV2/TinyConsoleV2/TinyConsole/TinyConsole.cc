#include <OPENR/OSyslog.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include "TinyConsole.h"

OStatus TinyConsole::DoInit(const OSystemEvent& event) {
    return oSUCCESS;
}

OStatus TinyConsole::DoStart(const OSystemEvent& event) {
    ipstackRef = antStackRef("IPStack");
    console.Initialize(myOID_, ipstackRef);
    return oSUCCESS;
}

OStatus TinyConsole::DoStop(const OSystemEvent& event) {
    console.Close();
    return oSUCCESS;
}

OStatus TinyConsole::DoDestroy(const OSystemEvent& event) {
    return oSUCCESS;
}

void TinyConsole::ListenCont(ANTENVMSG msg) {
    console.ListenCont((TCPEndpointListenMsg*)antEnvMsg::Receive(msg));
}
void TinyConsole::SendCont(ANTENVMSG msg) {
    console.SendCont((TCPEndpointSendMsg*)antEnvMsg::Receive(msg));
}
void TinyConsole::ReceiveCont(ANTENVMSG msg) {
    console.ReceiveCont((TCPEndpointReceiveMsg*)antEnvMsg::Receive(msg));
}
void TinyConsole::CloseCont(ANTENVMSG msg) {
    console.CloseCont((TCPEndpointCloseMsg*)antEnvMsg::Receive(msg));
}

