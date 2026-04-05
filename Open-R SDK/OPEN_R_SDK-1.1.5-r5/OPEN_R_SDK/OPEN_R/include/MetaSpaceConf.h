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
 * $Date: Mon Feb 25 16:33:28 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */

#ifndef	_MetaSpaceConf_h_DEFINED
#define	_MetaSpaceConf_h_DEFINED

#include	<Types.h>
#include	<Params.h>


class	MetaDescriptor;

/*
 *		object list in meta space
 * NOTE: Ordering describes dependency between objects.
 *	Initialization must be done according to this order.
 */

struct ObjectSymbol {
	sbyte		name [NAMELEN];		/* object symbol */
};

struct MetaOperation {
	sbyte		name [NAMELEN];		/* object symbol */
	longword	metacall;		/* meta call number */
	longword	selector;		/* selector in object */
};

struct MetaSpaceInfo {
	sbyte		name [NAMELEN];		/* metaspace symbol */
	size_t		nObjects;		/* number of members */
	ObjectSymbol*	pObject;		/* object symbols */
	size_t		nOperations;		/* number of operations */
	MetaOperation*	pOperation;		/* mapping to object selector */
	MetaDescriptor*	descriptor;		/* meta descriptor */
};

struct MetaSpaceConf {
	size_t		nMetaSpaces;		/* number of meta spaces */
	MetaSpaceInfo*	pMetaSpace;		/* meta spaces */
	size_t		nPrimitives;		/* number of primivies */
	ObjectSymbol*	pPrimitive;		/* primitive objects */
};

#endif	/* _MetaSpaceConf_h_DEFINED */
