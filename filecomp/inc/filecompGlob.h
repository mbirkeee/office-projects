/******************************************************************************
 * File: filecompGlob.h
 *-----------------------------------------------------------------------------
 * Copyright (c) 2006 Michael A. Bree
 *-----------------------------------------------------------------------------
 * Date: May 23, 2006
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/filecomp/inc/filecompGlob.h,v 1.3 2007/06/23 22:30:35 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: filecompGlob.h,v $
 * Revision 1.3  2007/06/23 22:30:35  mikeb
 * Update banner
 *
 * Revision 1.2  2006/12/05 13:47:07  mikeb
 * Update queue macros
 * Add totals to end of display
 * dos2unix files
 *
 * Revision 1.1.1.1  2006/05/23 01:51:24  mikeb
 * Initial Import
 *
 *
 *
 *-----------------------------------------------------------------------------
 */

#ifndef  H_filecompGlob
#define  H_filecompGlob

#include "mabGeneral.h"

#ifdef  External
#undef  External
#endif

#ifdef  INIT_GLOBALS
#define External
#else
#define External extern
#endif  /* INIT_GLOBALS */

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

External Boolean_t      SkipSymlinks;
External Int32u_t       TempFileCount;
External Int32u_t       TotalFileCount;
External Int32u_t       mbCrcTable[256];

#endif /* H_filecompGlob */
