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
 * $Date: Mon Feb 25 16:34:12 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef MIPSRELOCATIONINFO_H_
#define MIPSRELOCATIONINFO_H_

#include "BaseRegister.h"
#include "MIPSRelocationEntryTable.h"

#ifdef USE_INITIALIZER_LIST

// collapse class hierarchy for snapheader.cc

struct MIPSRelocationInfo {
	BaseRegister text; // baseregister pointing to text area
	BaseRegister data; // baseregister pointing to data area
	void* entryPoint;	// offset from text start
	MIPSRelocationEntryTable textRelocation; // relocation entries for text
  MIPSRelocationEntryTable dataRelocation; // relocation entries for data

};

#else

#include "RelocationInfo.h"

struct MIPSRelocationInfo : public RelocationInfo {
	MIPSRelocationEntryTable textRelocation; // relocation entries for text
  MIPSRelocationEntryTable dataRelocation; // relocation entries for data 

  MIPSRelocationInfo () {}
#if 1 /* avoiding memory leak */
  void CleanUp ();
#endif /* avoiding memory leak */

  bool IsValid() const;
protected:
  MIPSRelocationInfo(const MIPSRelocationInfo&);
  // use MIPSRelocator to clone.
  
  friend class MIPSRelocator; 
};


inline bool
MIPSRelocationInfo::IsValid() const
{
  return textRelocation.IsValid() && dataRelocation.IsValid();
}



#ifdef DEBUG
#include <stream.h>

//
// debug stream operators
//

inline ostream& operator << (ostream& s, const MIPSRelocationInfo& ri){
  s << "relocation info : " << endl
    << "text  : " << ri.text << endl
    << "data  : " << ri.data << endl
    << ri.textRelocation.GetSize() << " text relocation entries ("
    << ri.textRelocation.GetEntries() << ")"<< endl
    << ri.dataRelocation.GetSize() << " data relocation entries ("
    << ri.dataRelocation.GetEntries() << ")"<< endl
    << endl;
  return s;
}
#endif // DEBUG
#endif // USE_INITIALIZER_LIST
#endif // MIPSRelocationInfo
