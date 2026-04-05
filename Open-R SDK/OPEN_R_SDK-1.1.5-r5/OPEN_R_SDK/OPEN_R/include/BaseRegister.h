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

#ifndef	BASEREGISTER_H_
#define	BASEREGISTER_H_

#include <Types.h>
#include <CPU.h>


#define	REG_UNDEF	0xFFFFFFFF

struct BaseRegister {
  /*
    Describes how a register should point to a segment (data or text).
    If type is absolute, the absolute value will be written to the register.
    If type is relative, the base of the segment + an offset will be witten to
    the register.
   */

  enum BaseRegisterType {
    relative,
    absolute,
    undef
  };

#ifndef USE_INITIALIZER_LIST
  /*
    USE_INITIALIZER_LIST removes constructors & inline code
    this should only be used by snapheader.cc, to avoid generating
    .data & .bss sections.
    */
public:
	BaseRegister ();
  BaseRegister (size_t regNumber,
                size_t v,
                BaseRegisterType t = relative);
  
	bool Relocate (void* base, CPURegisters* regs) const;
  /*
    write the correct value to the appropriate register in ctxt.
    base is the base address of the segment.
   */
  void *Relocate(void *base) const;
  /*
    return the value to be written to the register,
    given the base address of the segment.
   */

  //
  // Accessors
  //
  size_t GetRegister() const;
  BaseRegisterType GetType() const;
  size_t GetValue()   const;
  bool IsValid() const;

  void Initialize(size_t regnr, BaseRegisterType type, size_t value);
  
private:
#endif
	size_t	reg;		/* register number described in CPURegisters,
                     not actual register number. */
  BaseRegisterType type;
  size_t  value;
  
};

#ifndef USE_INITIALIZER_LIST

inline size_t
BaseRegister::GetRegister() const
{
  return reg;
}

inline BaseRegister::BaseRegisterType
BaseRegister::GetType() const
{
  return type;
}

inline size_t
BaseRegister::GetValue() const
{
  return value;
}

inline bool
BaseRegister::IsValid() const
{
  return reg != REG_UNDEF && type != undef;
}

inline void
BaseRegister::Initialize(size_t reg_,
                         BaseRegisterType type_,
                         size_t value_)
{
  reg = reg_;
  type = type_;
  value = value_;
}

#ifdef DEBUG

#include <stream.h>

//
// debug stream operator
//
inline ostream& operator << (ostream& s,
                             const BaseRegister::BaseRegisterType& brt) {
  
  switch (brt) {
    case BaseRegister::relative: 
      s << "relative";
      break;
      
    case BaseRegister::absolute:
      s << "absolute";
      break;
      
    case BaseRegister::undef:
      s << "undef";
      break;
      
    default:
      s << "??";
      break;
  };
  return s;
}

inline ostream& operator << (ostream& s, const BaseRegister& br) {
  if (! br.IsValid()) {
    s << "invalid baseregister" << endl;
  } else {
    s << "relocate register " << br.GetRegister()
      << " " << br.GetType()
      << " by 0x" << (void *)br.GetValue();
  }
  return s;
}

#endif // DEBUG
#endif // USE_INITIALIZER_LIST
#endif // BASEREGISTER_H_ 
