//
// OSyslog.h
//
// Copyright 1997,1998,1999,2000,2001,2002 Sony Corporation
//

#ifndef _OSyslog_h_DEFINED
#define _OSyslog_h_DEFINED

#include <Types.h>
#include <OPENR/OPENR.h>

//
//  OSYSLOGx() macros definitions
//  OSYSLOG_SHOW 0 : 
//  OSYSLOG_SHOW 1 : OSYSLOG1() 
//  OSYSLOG_SHOW 2 : OSYSLOG1() OSYSLOG2()
//  OSYSLOG_SHOW 3 : OSYSLOG1() OSYSLOG2() OSYSLOG3()
//  OSYSLOG_SHOW 4 : OSYSLOG1() OSYSLOG2() OSYSLOG3() OSYSLOG4()
//

#ifndef OSYSLOG_SHOW
#if SYSMSGSHOW > 0
#define OSYSLOG_SHOW 4
#else
#define OSYSLOG_SHOW 1
#endif
#endif

#if OSYSLOG_SHOW > 0
#define OSYSLOG1(x)       (osyslog x)
#if OSYSLOG_SHOW > 1
#define OSYSLOG2(x)       (osyslog x)
#if OSYSLOG_SHOW > 2
#define OSYSLOG3(x)       (osyslog x)
#if OSYSLOG_SHOW > 3
#define OSYSLOG4(x)       (osyslog x)
#endif	/* OSYSLOG_SHOW > 3 */
#endif	/* OSYSLOG_SHOW > 2 */
#endif	/* OSYSLOG_SHOW > 1 */
#endif	/* OSYSLOG_SHOW > 0 */

#ifndef OSYSLOG1
#define OSYSLOG1(x)
#endif /* OSYSLOG1 */

#ifndef OSYSLOG2
#define OSYSLOG2(x)
#endif /* OSYSLOG2 */

#ifndef OSYSLOG3
#define OSYSLOG3(x)
#endif /* OSYSLOG3 */

#ifndef OSYSLOG4
#define OSYSLOG4(x)
#endif /* OSYSLOG4 */


#define OSYSPRINT(x)      (osysprint x) 
#ifdef OPENR_DEBUG
#define OSYSDEBUG(x)      (osysprint x)
#else
#define OSYSDEBUG(x)
#endif /* OPENR_DEBUG */


//
// Max size of OSyslog message
//
const size_t osyslogMAX_SIZE = 243;

typedef longword OSyslogPriority;
const OSyslogPriority osyslogERROR            = 1;
const OSyslogPriority osyslogWARNING          = 2;
const OSyslogPriority osyslogINFO             = 3;
const OSyslogPriority osyslogDEBUG            = 0xffffffff;
const OSyslogPriority osyslogDEBUG_NO_NEWLINE = 0xfffffffe;

OStatus osyslog(OSyslogPriority priority, const char* format, ...);
OStatus osysprint(const char* format, ...);

#endif /* _OSyslog_h_DEFINED */
