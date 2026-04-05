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
 * $Revision: /main/tokyo/AIBO9/2 $
 */

#ifndef MIPSRELOCATIONENTRYTABLE_H_
#define MIPSRELOCATIONENTRYTABLE_H_

#include "MIPSRelocationEntry.h"
#include "Types.h"

struct MIPSRelocationEntryTable {
#ifndef USE_INITIALIZER_LIST
  // define only struct layout for snapheader.cc

public:
  MIPSRelocationEntryTable ();
	MIPSRelocationEntryTable (size_t);
	MIPSRelocationEntryTable(const MIPSRelocationEntryTable&);

  ~MIPSRelocationEntryTable ();
	MIPSRelocationEntryTable& operator = (const MIPSRelocationEntryTable&);

#if 1 /* avoiding memory leak */
     void CleanUp();
#endif /* avoiding memory leak */

  //
  // accessors
  //
	size_t GetSize() const;
  void SetSize(size_t newSize);

  void SetEntries(MIPSRelocationEntry *);
  // sets size & reallocates table.
  //does not copy old contents to new table.
  
	bool IsValid() const;

  const MIPSRelocationEntry *GetEntries() const;
  MIPSRelocationEntry *GetEntries();

  bool SetEntry(size_t index,
                MIPSRelocationSymbol symbol,
                MIPSRelocationType type,
                size_t offset,
                size_t addend);
  
private:
	void invalidate();
  
private:  
#endif // USE_INITIALIZER_LIST  
  size_t		size;
  MIPSRelocationEntry *pEntry;
  static MIPSRelocationEntry *allocateMIPSRelocationEntryArray (size_t count);
  static void deleteMIPSRelocationEntryArray (MIPSRelocationEntry* base);
};

#ifndef USE_INITIALIZER_LIST // for snapheader.cc
//
// inline function definitions
//

inline size_t
MIPSRelocationEntryTable::GetSize() const
{
  return size;
}

inline void
MIPSRelocationEntryTable::SetSize(size_t size_)
{
  size = size_;
}

inline bool
MIPSRelocationEntryTable::IsValid() const
{
  return (size_t)-1 != size;
}

inline void
MIPSRelocationEntryTable::invalidate()
{
  size = (size_t)-1;
}

inline const MIPSRelocationEntry *
MIPSRelocationEntryTable::GetEntries() const
{
  return pEntry;
}

inline MIPSRelocationEntry *
MIPSRelocationEntryTable::GetEntries()
{
  return pEntry;
}

#endif // USE_INITIALIZER_LIST

#endif // MIPSRELOCATIONENTRYTABLE_H_
