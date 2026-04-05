//
//   Messages used in Proxy communication
//   between OPEN-R Emulator & DRX-900
//
#ifndef ProxyMessages_h_DEFINED
#define ProxyMessages_h_DEFINED

#include <OID.h>
#include <SystemError.h>


struct RemoteCommandHeader {            // total 16bytes

  enum RemoteAperiosCommand {
    COMMAND_UNDEF,
    FIND,
    FIND_REPLY,
    SEND,
    SEND_REPLY,
    CALL,
    CALL_REPLY,
    SENDWITHRBOX,
    REPLY
  };

  int                   totalSize;      // 4bytes
  int                   nitem;          // 4bytes
  int                   sizeitem;       // 4bytes
  RemoteAperiosCommand  command;        // 4bytes

  RemoteCommandHeader(int size, RemoteAperiosCommand com = COMMAND_UNDEF):
    totalSize(sizeof(RemoteCommandHeader) - sizeof(totalSize) + size),
    nitem(1),
    sizeitem(sizeof(command) + size),
    command(com) {}
};


struct FindMessage {
  RemoteCommandHeader    header;
  char                   name[64];      // 64bytes

  FindMessage() : header(64, RemoteCommandHeader::FIND) { name[0] = '\0'; }
};

struct FindReplyMessage {
  RemoteCommandHeader  header;
  sError               error;           //  4bytes
  OID                  oid;             //  4bytes  total 8bytes

  FindReplyMessage() :
    header(8, RemoteCommandHeader::FIND_REPLY),
    error(sSUCCESS),
    oid(OID_UNDEF) {}
};

struct SendMessage {
  RemoteCommandHeader   header;
  OID                   oid;            //  4bytes
  Selector              sel;            //  4bytes
  size_t                size;           //  4bytes   total 12bytes

  SendMessage(size_t s)                 //  size of payload
    : header(12+s, RemoteCommandHeader::SEND), oid(OID_UNDEF), sel(UNDEF), size(s) {}
};

struct SendReplyMessage {
  RemoteCommandHeader   header;
  sError                error;          // 4bytes
  SendReplyMessage() : header(4, RemoteCommandHeader::SEND_REPLY) {}
};


struct CallMessage {
  // Should not change the order of the members.
  // There is an assumption about the order in TCPGateway.cc.
  RemoteCommandHeader   header;
  OID                   oid;            // 4bytes
  Selector              sel;            // 4bytes
  size_t                sendSize;       // 4bytes
  size_t                receiveSize;    // 4bytes   total 16bytes

  CallMessage(size_t size)              // size of payload
    : header(16+size, RemoteCommandHeader::CALL),
      oid(OID_UNDEF), sel(UNDEF),
      sendSize(size), receiveSize(0) {}
};

struct CallReplyMessage {
  RemoteCommandHeader   header;
  sError                error;           // 4bytes
  size_t                size;            // 4bytes  total  8bytes

  CallReplyMessage(size_t s)             // size of payload
    : header(8+s, RemoteCommandHeader::CALL_REPLY), error(sUNDEF), size(s) {}
};

#endif



