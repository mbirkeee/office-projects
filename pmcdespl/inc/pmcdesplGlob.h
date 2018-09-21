/******************************************************************************
 * File: lgGlob.h
 *-----------------------------------------------------------------------------
 * Copyright (c) 2000, QCC Communications Corporation
 *-----------------------------------------------------------------------------
 * Date: May 11, 2000
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Contains all of the global variables for makevers
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/pmcdespl/inc/pmcdesplGlob.h,v 1.2 2006/12/16 19:58:25 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: pmcdesplGlob.h,v $
 * Revision 1.2  2006/12/16 19:58:25  mikeb
 * Initial Latex to PDF conversion support.
 *
 * Revision 1.1.1.1  2006/05/23 01:37:06  mikeb
 * Initial Import
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#ifndef  H_lgGlob
#define  H_lgGlob

#include "mbTypes.h"

#ifdef  External
#undef  External
#endif

#ifdef  INIT_GLOBALS
#define External
#else
#define External extern
#endif  /* INIT_GLOBALS */

External Boolean_t      gStopDetected;
External Int32u_t       gMyPid;
External Int32u_t       gWatchdogCounter;
External Int32s_t       gSystemPid;
External Boolean_t      gSkipPrintFlag;
External Boolean_t      gSkipConvertFlag;
External Boolean_t      gArchiveFlag;
External Char_p         gConvertCommand_p;
External Char_p         gPrintCommand_p;
External Char_p         gArchiveCommand_p;
External Char_p         gBaseDir_p;
External Int32s_t       gMallocCount;
External pthread_t      gWorkerThread;

 #endif /* H_lgGlob */
