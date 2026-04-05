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
 * $Date: Mon Feb 25 16:34:09 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef MIPSRELOCATIONTYPE_H_
#define MIPSRELOCATIONTYPE_H_

/*
  We cannot use the standard System V ABI MIPS Supplement relocation entries,
  because some embedded compilers (like GHS) do not comply with this standard.
  some relocation entries therefore are meaningless.

  We stay as close as possible to the standard in the definition of these
  relocation entries.

  one important difference is that we specify everything in offsets from
  the beginning of the data or text segment.  this allows us to strip the
  symbol table at run time.
 */


enum MIPSRelocationSymbol {
/*
  We express all relocation entries using offsets from the start
  of either the text or data section.
 */
  R_TextStart = 0,
  R_DataStart = 1
};


enum MIPSRelocationType {
  R_MIPS_AP_NONE = 0,    // R_MIPS_NONE
  R_MIPS_AP_16 = 1,      // R_MIPS_16
  R_MIPS_AP_32 = 2,      // R_MIPS_32
  R_MIPS_AP_26 = 3,      // R_MIPS_26
  R_MIPS_AP_HI16 = 4,    // R_MIPS_HI16
  R_MIPS_AP_LO16 = 5     // R_MIPS_LO16
};


#endif // MIPSRELOCATIONTYPE_H_

