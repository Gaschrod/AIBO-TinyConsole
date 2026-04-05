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
 * $Date: Mon Feb 25 16:33:18 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef	_SnapShotHdr_h_DEFINED
#define	_SnapShotHdr_h_DEFINED

#include	<Types.h>
#include	<Params.h>

#define	IUNDEF		0


/* object type */
#define	IINSTANCE	1
#define	IINIT		2
#define	ICLASS		3
#define	IOBLET		4

/* machine execution mode */
#define	IKERNEL		1
#define	ISUPERVISOR	2
#define	IUSER		3

/* cache */
#define	INCACHE		0
#define	ICACHE		1

/* TLB */
#define	INTLB		0
#define	ITLB		1

/* relocation type */
#define IPIC		0x1
#define IPID		0x2

/* relocation method */
#define	IOFFSET		1
#define IGOT		2

struct InstanceInfo
{
	sbyte		metaspace [NAMELEN];	/* config : metaspace name */
	size_t		stack;			/* config : stack size */
	size_t		heap;			/* config : heap size */
	void*		aux;			/* ?????? : pointer to aux */
	longword	mode;			/* config : execution mode */
	longword	cache;			/* config : cache usage */
	longword	TLB;			/* config : TLB usage */
	//longword	nSelectors;		/* config : number of selectors */
	longword	dummy;		/* config : number of selectors */
	longword	cpuPriority;		/* config : CPU priority */
	longword	schedPriority;		/* config : scheduling priority */
	sbyte		vmspace [NAMELEN];	/* config : virtual memory */
};

struct ClassInfo
{
	InstanceInfo	cls;
	InstanceInfo	inst;
};

struct ObletInfo
{
	sbyte		metaspace [NAMELEN];	/* config : metaspace name */
	longword	dummy1;			/* config : dummy */
	longword	dummy2;			/* config : dummy */
	struct ObletInitInfo*		aux;	/* pointer to oblet aux */
	longword	mode;			/* config : execution mode */
	longword	cache;			/* config : cache usage */
	longword	TLB;			/* config : TLB usage */
};

struct Layout {
	void*	loadStart;		/* layout : segment start */
	void*	entry;			/* layout : entry point */
	void*	textStart;		/* layout : text start address */
#ifdef NEW_V131_SNAP
	void*	phyTextStart;		/* layout : physical text start */
#endif
	void*	dataStart;		/* layout : data start address */
#ifdef NEW_V131_SNAP
	void*	phyDataStart;		/* layout : physical data start */
#endif
	size_t	textSize;		/* layout : text size in file */
	size_t	dataSize;		/* layout : data size in file */
	size_t	bssSize;		/* layout : bss section size */
};

struct RelInfo {
	longword	type;		/* config : pic/pid */
	longword	method;		/* config : relocation method */
	//class RelocationInfo *		aux;		/* ????? : pointer to relocation aux */
	void * 		aux;		/* ????? : pointer to relocation aux */
};

struct SnapShotHdr {
	sbyte		object [NAMELEN];	/* config : object name */
	sbyte		module [NAMELEN];	/* config : module name */
	longword	objectType;		/* config : object type */

	union {
		ClassInfo	cls;		/* config : class */
		InstanceInfo	inst;		/* config : instance */
		ObletInfo	oblet;		/* config : oblet */
	};

	Layout		layout;			/* layout : memory layout */
	RelInfo		rel;			/* config : relocation info */
};

#define SNAPSHOT_HDRS    "lar.shdrs"

#endif	/* _SnapShotHdr_h_DEFINED */
