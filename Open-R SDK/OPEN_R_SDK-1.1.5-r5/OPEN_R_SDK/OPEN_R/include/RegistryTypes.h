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
 * $Date: Mon Feb 25 16:33:51 JST 2002 $
 * $Revision: /main/tokyo/AIBO9/1 $
 */


#ifndef _RegistryTypes_H_
#define _RegistryTypes_H_

enum {
        NAME_MAX = 80, /* include the strings's end of NULL */
        DATA_MAX = 1024,
        DISCRIM = '/',
	MAXENTRIES_PER_REGISTRY = 10000
};

    /* The types of values handled by the Registry */
typedef enum DataType {
        REG_DATATYPE_NONE       = 1,
        REG_DATATYPE_INT64      = 2,
        REG_DATATYPE_INT32      = 3,
        REG_DATATYPE_OPAQUE     = 4,
        REG_DATATYPE_UNISTRING  = 5,
        REG_DATATYPE_FLOAT      = 6,
        REG_DATATYPE_CHARSTRING = 7,
        REG_DATATYPE_OID    =  8,
        REG_DATATYPE_RID    =  9,
        REG_DATATYPE_UNDEF  = 10 
} RegValueType;

typedef enum EntryType {
        NODE = 1,
        LEAF = 2
} RegEntryType;


#endif
