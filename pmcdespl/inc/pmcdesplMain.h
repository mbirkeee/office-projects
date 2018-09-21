/******************************************************************************
 * File:   pmcdesplMain.h
 *-----------------------------------------------------------------------------
 * Copyright (c) 2001, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Author: Michael Bree
 * Date:   March 6, 2001
 *-----------------------------------------------------------------------------
 * Description:
 *
  *-----------------------------------------------------------------------------
 */

#ifndef H_pmcdesplMain
#define H_pmcdesplMain

#include "mbTypes.h"
#include "mbQueue.h"

#ifdef  External
#undef  External
#endif

#ifdef INIT_GLOBALS
#define External
#else
#ifdef __cplusplus
#define External extern "C"
#else
#define External extern
#endif
#endif

#define MAX_PATH            512

#define DS_LOCKFILE_NAME    ".lockfile"
#define DS_PROG_NAME        "pmcdespl"

#define DS_CONVERT_COMMAND  "/usr/bin/html2ps"
/* #define DS_PRINT_COMMAND    "/usr/bin/lpr -P hp2100-ps" */
#define DS_PRINT_COMMAND    "/usr/bin/lpr -P HP_LaserJet_2100_Series"
#define DS_ARCHIVE_COMMAND  "/bin/gzip -c"
#define DS_LATEX_COMMAND    "/usr/bin/latex "
#define DS_DVIPDF_COMMAND   "/usr/bin/dvipdfm -p letter "

#define DS_FILE_MODE_ALL    1
#define DS_FILE_MODE_REG    2

#define RAND( _parm ) (Int32u_t)( (Float_t)(_parm) * ((Float_t)random()/(Float_t)RAND_MAX) )

typedef struct  DsFile_s
{
    qLinkage_t  linkage;
    Char_t      name[MAX_PATH];
    Char_t      path[MAX_PATH];
    Char_t      pathName[MAX_PATH];
} DsFile_t, *DsFile_p;

Int32s_t strPos
(
    Char_p      s1,
    Char_p      s2
);

Char_p dsCharTime( time_t *tm );

Char_p dsCharDate( time_t *tm );

Void_t dsProcessTempFiles
(
    Char_p    tempDir_p,
    Char_p    debugDir_p,
    Char_p    printCommand,
    Int32u_t  skipPrint
);

Void_t          dsHandleSignals
(
    Ints_t      sig
);

Ints_t          dsLog
(
    Char_p      string_p,
    ...
);

Int32s_t        dsGetFileList
(
    Char_p      dir_p,
    qHead_p     q_p,
    Ints_t      mode
);

Int32s_t        dsFileCompare
(
    Char_p      fileName1_p,
    Char_p      fileName2_p
);

Int32s_t dsProcessHtml
(
    DsFile_p    fileSpool_p,
    DsFile_p    fileFlag_p,
    Char_p      tempDir_p
);

Int32s_t dsProcessLatex
(
    DsFile_p    fileSpool_p,
    DsFile_p    fileFlag_p,
    Char_p      psDir_p
);


Int32s_t    dsFileListFree( qHead_p file_q );
Int32s_t    dsFileListFreeEntry( DsFile_p file_p );
Void_p      dsWorkerThreadEntry( Void_p args_p );
int         dsSystem( char *cmd_p, int pidFlag );

Int32s_t    dsLockFileGet( void );
Char_p      mbMallocStr( Char_p string_p );
Void_p      mbMalloc( int size );
Void_p      mbCalloc( int size );
Void_t      mbFreeFunc( Void_p ptr_p );
#define     mbFree( _parm ) mbFreeFunc( (Void_p)(_parm) )

#endif /* H_pmcdesplMain */
