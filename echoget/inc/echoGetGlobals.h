/******************************************************************************
 * File: echosendGlob.h
 *-----------------------------------------------------------------------------
 * Copyright (c) 2003, Michael Bree
 *-----------------------------------------------------------------------------
 * Date: Jan 27, 2003
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/echoget/inc/echoGetGlobals.h,v 1.11 2007/03/17 15:26:57 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: echoGetGlobals.h,v $
 * Revision 1.11  2007/03/17 15:26:57  mikeb
 * Add gSetOnlineFlag
 *
 * Revision 1.10  2007/02/26 00:18:46  mikeb
 * Move mysql connect into worker thread.
 *
 * Revision 1.9  2007/02/25 15:50:29  mikeb
 * Update document get to handle sub-dirs in doc directory
 *
 * Revision 1.8  2007/01/21 13:23:04  mikeb
 * Add read_date != 3 to SQL command that gets the echo list
 *
 * Revision 1.7  2007/01/06 21:52:01  mikeb
 * Add ability to specify remote SCP machine; default is drbree.com
 *
 * Revision 1.6  2006/12/13 04:50:31  mikeb
 * Add watchdog thread
 *
 * Revision 1.5  2006/12/10 17:29:29  mikeb
 * Remove echoput files
 *
 * Revision 1.4  2006/12/10 04:58:07  mikeb
 * Cleanups
 *
 * Revision 1.3  2006/12/09 01:58:01  mikeb
 * Fix memory leak
 *
 * Revision 1.2  2006/12/07 06:05:56  mikeb
 * Dec. 6 updates.
 *
 * Revision 1.1.1.1  2006/05/23 01:50:08  mikeb
 * Initial Import
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#ifndef  H_echosendGlob
#define  H_echosendGlob

#include "mbTypes.h"

#ifdef  External
#undef  External
#endif

#ifdef  INIT_GLOBALS
#define External
#else
#define External extern
#endif  /* INIT_GLOBALS */

External Int32u_t       gPort;
External Char_p         gHost_p;
External Char_p         gBaseDir_p;
External MYSQL         *gMySQL_p;
External Int32u_t       gSilentNew;
External Int32u_t       gMyId;
External Int32u_t       gMyPid;
External Int32u_t       gMaxAge;
External Char_p         gBuf_p;
External Boolean_t      gMoveBadFlag;
External Boolean_t      gMoveEmptyFlag;
External Boolean_t      gSetBackupFlag;
External Boolean_t      gSetOnlineFlag;
External Boolean_t      gStopDetected;
External Boolean_t      gCheckLocalEchos;
External Boolean_t      gGetDocs;
External Boolean_t      gCheckDocs;
External Int32u_t       gEchoID;
External Int32u_t       gWatchdogCounter;
External Int32s_t       gSystemPid;
External Char_t         gRemoteMachine[256];

External Char_p         gLocalDir_p;
External Char_t         gLocalDir[256];
External Int32s_t       gMaxEchosToGet = 0x100;

External Lock_t         gLogLock;
External Lock_p         gLogLock_p;

External Int32u_t       gCrcTable[256];

#endif /* H_echosendGlob */
