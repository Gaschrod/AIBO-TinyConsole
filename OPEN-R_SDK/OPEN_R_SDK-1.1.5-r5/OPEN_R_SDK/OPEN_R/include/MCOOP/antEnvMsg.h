/*
 *  Copyright 2002 Sony Corporation
 */
#ifndef _antEnvMsg_H_
#define _antEnvMsg_H_

#include <MCOOP.h>
#ifndef __GNUC__
#include <InitInfo.h>
#include <EntryTable.h>
#endif /* __GNUC__ */

#define ANTENVMSG void*

#include <antEnvInternalMsg.h>

#ifndef __GNUC__
extern "C" void InitGlobals();
extern "C" void  _HeapInitialize(size_t);

#define EXIT() Exit()
#define EXITFROMPROLOGUE() ExitFromPrologue()

#define PROLOGUE() extern "C" void Prologue(InitInfo* pInfo)
#define PROLOGUEINIT() {_HeapInitialize(pInfo->heapSize);InitGlobals();};

#define CREATEENTRYTABLE(size) int eTP[SIZEOF_ENTRYTABLE(size)/sizeof(int)]; EntryTable* eT = new ((byte*)eTP)EntryTable(size);
#define SETENTRY(sel, entry) eT->SetEntry(sel, (Entry)entry)
#define SETENTRYTABLE() SetEntryTable(eT->GetSize(),eT)

#endif /* __GNUC__ */

#endif
