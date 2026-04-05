/*
 * The Aperios Software Platform
 *
 * This program is subject to copyright protection in accordance with the
 * applicable law. It must not, except where allowed by law, by any means or
 * in any form be reproduced, distributed or lent. Moreover, no part of the
 * program may be used, viewed, printed, disassembled or otherwise interfered
 * with in any form, except where allowed by law, without the express written
 * consent of the copyright holder.
 *
 * Copyright 1988-2002 Sony Corporation
 * All Rights Reserved.
 *
 */
/*
 * $Date: Mon Feb 25 16:33:25 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 * $Date: Mon Feb 25 16:33:25 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef _ObletSpec_h_
#define _ObletSpec_h_

#include <Types.h>
#include <string.h>
#include <assert.h>
#include <Params.h>

struct ObletVersion {
  /*
    specifies the version of an Oblet.  in version 3.5,
    3 is major number & 5 is minor number.
   */
  
  word major;
  word minor;
#ifndef	USE_INITIALIZER_LIST
  ObletVersion(word ma = 0, word mi = 0);
#endif	/* USE_INITIALIZER_LIST */
};

struct ObletSpec {
  /*
    specifies a oblet version.
    */
#define ObletSpec_MaxObletNameLength	NAMELEN
public:
  char name[ObletSpec_MaxObletNameLength];
  ObletVersion interface;
  ObletVersion implementation;
  
#ifndef	USE_INITIALIZER_LIST
public:
  ObletSpec();
  ObletSpec(char *nm, word int_ma, word int_mi, word imp_ma, word imp_mi);
  ObletSpec(const ObletSpec &src);
  
  const ObletVersion& GetInterfaceVersion() const {return interface;}
  ObletVersion& GetInterfaceVersion(){return interface;}
  const ObletVersion& GetImplementationVersion() const {return implementation;}
  ObletVersion& GetImplementationVersion(){return implementation;}
  
  char *GetName() {return name;}
  const char *GetName() const {return name;}
  void SetName(const char *str);
  
  int operator == (const ObletSpec&) const;

#ifdef OBLETMGR_USES_NEWREGION
  void* operator new (size_t size);
  void  operator delete (void* base);
#endif /* OBLETMGR_USES_NEWREGION */

private:
  bool CompareName(const ObletSpec &s) const;
#endif	/* USE_INITIALIZER_LIST */
  
};

#ifdef DEBUG
#include <stream.h>
ostream& operator << (ostream& s, const ObletSpec& spec);
#endif

struct ObletSpecRep {
  // should have identical layout as ObletSpec !!!
  // this seems to be the only way to avoid .text section in snapheader
  char name[NAMELEN];
  word inma, inmi, imma, immi;
};

#endif
