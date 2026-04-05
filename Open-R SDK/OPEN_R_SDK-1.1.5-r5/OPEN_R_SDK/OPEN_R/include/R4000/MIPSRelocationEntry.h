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
 * $Date: Mon Feb 25 16:34:11 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef MIPSRELOCATIONENTRY_H_
#define MIPSRELOCATIONENTRY_H_

#include "MIPSRelocationType.h"
#include <Types.h>

struct MIPSRelocationEntryNoAddend {
  // Plain Old Data
  MIPSRelocationSymbol symbol; // symbol used as base of offset
  MIPSRelocationType   rtype;  // relocation type
	size_t	offset;		// offset from base address of segment 
};

struct MIPSRelocationEntry {
  // Plain Old Data
  MIPSRelocationSymbol symbol; // symbol used as base of offset
  MIPSRelocationType   rtype;  // relocation type
	size_t	offset;		// offset from base address of segment
  size_t addend;    // relocation addend.  (value to be used during relocation)
};

/*
  Note :
   We treat these entries as plain old C data.
   we cannot use inheritance relationship because this inhibits
   the use of the initializer list.  MkHeader uses this syntax to
   generate snapshot.cc.  constructors cannot be used, since
   the snapshot should not contain a .text section.
 */


#endif //MIPSRELOCATIONENTRY_H_
