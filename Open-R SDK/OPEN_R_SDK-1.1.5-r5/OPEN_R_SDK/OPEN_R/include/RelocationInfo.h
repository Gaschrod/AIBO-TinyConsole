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
 * $Date: Mon Feb 25 16:33:56 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */


#ifndef	RELOCATIONINFO_H_
#define	RELOCATIONINFO_H_


#include "BaseRegister.h"

struct RelocationInfo {
  /*
    base class for all platforms.
    platform independent implementations should inherit.

    We cannot use virtual functions here - this would yield a layout
    mismatch between the snapshot version & the version used in coding.
   */
	BaseRegister	text; // baseregister pointing to text area
	BaseRegister	data; // baseregister pointing to data area
	void*		entryPoint;	// offset from text start

  RelocationInfo ();
protected:
  RelocationInfo(const RelocationInfo&);
  // copying is unsafe.  use Relocators clone interface..
};



#endif // RELOCATIONINFO_H_
