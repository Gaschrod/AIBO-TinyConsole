//
//  rmcoop.h
//
//   Remote Message Passing of
//          APERIOS MCOOP on Linux
//

#ifndef rmcoop_h_DEFINED
#define rmcoop_h_DEFINED

#include <OID.h>
#include <MCOOPError.h>


sError RFind(const char* name, OID* object);
sError RSend(OID dest, Selector meth, void* msg, size_t sizeOfMsg);
sError RCall(OID dest, Selector meth, void* msg, size_t sizeOfMsg,
             void* replyMsg = 0, size_t sizeOfReplyMsg = 0);

/*
 * functions below are not implemented yet.
 */
/*
sError SendWithRBox(OID dest, Selector meth, void* msg, size_t sizeOfMsg,
                    RID* rBoxID);
sError Receive(RID rBoxID, void* msg = 0, size_t sizeOfMsg = 0,
               size_t* sizeOfReceivedMsg = 0);
*/


// OPEN-R API 
//  SelectProxyPort() will select which robot to apply OPEN-R API(OPENR::function).
//  Connection to robot is not established until OPEN-R API is called for the 
//  first time. Once connetction is established, socket descriptor is cached and 
//  can be used repeatedly.
//  CloseProxyPort() will close the connection to selected robot and also remove 
//  from cache.
namespace OPENR {
    void SelectProxyPort(const char* host_name, unsigned short port = 11111,
                         int numSeconds = 10);
    int  CloseProxyPort(void);
}


#endif
