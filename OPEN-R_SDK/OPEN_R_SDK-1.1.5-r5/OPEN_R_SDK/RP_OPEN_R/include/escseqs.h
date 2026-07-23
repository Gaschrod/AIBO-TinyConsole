/*
 * The Aperios Operating System
 *
 * This program is subject to copyright protection in accordance with the
 * applicable law. It must not, except where allowed by law, by any means or
 * in any form be reproduced, distributed or lent. Moreover, no part of the
 * program may be used, viewed, printed, disassembled or otherwise interfered
 * with in any form, except where allowed by law, without the express written
 * consent of the copyright holder.
 *
 * Copyright (C) 1988-1998 Sony Corporation
 * All Rights Reserved.
 *
 */
/*
 * $Date: 2002/01/28 02:16:00 $
 * $Revision: 1.2 $
 */

#ifndef _escseqs_h_DEFINED
#define _escseqs_h_DEFINED

/* ESCSEQS_BF: boldface                 */
/* ESCSEQS_RM: normal (roman)           */
/* ESCSEQS_TCx: text colour x           */
/* ESCSEQS_BCx: background colour x     */

#ifndef WITH_DEBUGGER

#define ESCSEQS_BF  "[1m"
#define ESCSEQS_RM  "[0m"
#define ESCSEQS_TC0 "[30m"
#define ESCSEQS_TC1 "[31m"
#define ESCSEQS_TC2 "[32m"
#define ESCSEQS_TC3 "[33m"
#define ESCSEQS_TC4 "[34m"
#define ESCSEQS_TC5 "[35m"
#define ESCSEQS_TC6 "[36m"
#define ESCSEQS_TC7 "[37m"
#define ESCSEQS_TC8 "[38m"
#define ESCSEQS_TC9 "[39m"
#define ESCSEQS_BC0 "[40m"
#define ESCSEQS_BC1 "[41m"
#define ESCSEQS_BC2 "[42m"
#define ESCSEQS_BC3 "[43m"
#define ESCSEQS_BC4 "[44m"
#define ESCSEQS_BC5 "[45m"
#define ESCSEQS_BC6 "[46m"
#define ESCSEQS_BC7 "[47m"
#define ESCSEQS_BC8 "[48m"
#define ESCSEQS_BC9 "[49m"

#else /* !WITH_DEBUGGER */

#define ESCSEQS_BF		""
#define ESCSEQS_RM  	""
#define ESCSEQS_TC0 	""
#define ESCSEQS_TC1 	""
#define ESCSEQS_TC2 	""
#define ESCSEQS_TC3 	""
#define ESCSEQS_TC4 	""
#define ESCSEQS_TC5 	""
#define ESCSEQS_TC6 	""
#define ESCSEQS_TC7 	""
#define ESCSEQS_TC8 	""
#define ESCSEQS_TC9 	""
#define ESCSEQS_BC0 	""
#define ESCSEQS_BC1 	""
#define ESCSEQS_BC2 	""
#define ESCSEQS_BC3 	""
#define ESCSEQS_BC4 	""
#define ESCSEQS_BC5 	""
#define ESCSEQS_BC6 	""
#define ESCSEQS_BC7 	""
#define ESCSEQS_BC8 	""
#define ESCSEQS_BC9 	""

#endif /* !WITH_DEBUGGER */

#define ESCSEQS_ERROR   ESCSEQS_TC7 ESCSEQS_BC1
#define ESCSEQS_WARNING ESCSEQS_TC1
#define ESCSEQS_METHOD  ESCSEQS_TC2

#define ESCSEQS_ALLOC   ESCSEQS_TC0 ESCSEQS_BC3

#define ESCSEQS_RED     ESCSEQS_TC0 ESCSEQS_BC1
#define ESCSEQS_GREEN   ESCSEQS_TC0 ESCSEQS_BC2
#define ESCSEQS_YELLOW  ESCSEQS_TC0 ESCSEQS_BC3
#define ESCSEQS_BLUE    ESCSEQS_TC0 ESCSEQS_BC4
#define ESCSEQS_BLACK   ESCSEQS_TC0 ESCSEQS_BC0

/* for compatibility with StdIo.h which is now obsolete */
#define WT_WHITE    ESCSEQS_RM
#define WT_BLACK    ESCSEQS_TC0
#define WT_RED      ESCSEQS_TC1
#define WT_GREEN    ESCSEQS_TC2
#define WT_YELLOW   ESCSEQS_TC3
#define WT_BLUE     ESCSEQS_TC4
#define WT_PINK     ESCSEQS_TC5
#define WT_LBLUE    ESCSEQS_TC6

#define WT_BKBLACK  ESCSEQS_BC0
#define WT_BKRED    ESCSEQS_BC1
#define WT_BKGREEN  ESCSEQS_BC2
#define WT_BKYELLOW ESCSEQS_BC3
#define WT_BKBLUE   ESCSEQS_BC4
#define WT_BKPINK   ESCSEQS_BC5
#define WT_BKLBLUE  ESCSEQS_BC6
#define WT_BKWHITE  ESCSEQS_BC7

#ifndef WITH_DEBUGGER

#define WT_REVERSE      "^[[7m"
#define WT_UNREVERSE    "^[[27m"
#define WT_ULINE        "^[[4m"
#define WT_DLULINE      "^[[24m"

#else /* !WITH_DEBUGGER */

#define WT_REVERSE		""
#define WT_UNREVERSE	""
#define WT_ULINE		""
#define WT_DLULINE		""

#endif /* !WITH_DEBUGGER */

#define WT_RESET    WT_WHITE

#endif  /* _escseqs_h_DEFINED */
