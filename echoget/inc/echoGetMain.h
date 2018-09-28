/******************************************************************************
 * File:        echoGetMain.h
 *-----------------------------------------------------------------------------
 * Author:      Michael A. Bree Copyright (c) 2003
 *-----------------------------------------------------------------------------
 * Date:        January 27, 2003
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/echoget/inc/echoGetMain.h,v 1.11 2007/07/22 14:51:32 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: echoGetMain.h,v $
 * Revision 1.11  2007/07/22 14:51:32  mikeb
 * Change name of lockfile to .echoget-lockfile
 *
 * Revision 1.10  2007/07/14 14:20:11  mikeb
 * Add compression flag (-C) to scp command.
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
 * Revision 1.6  2007/01/05 01:42:57  mikeb
 * Fix nasty bug that deleted all backup echos
 *
 * Revision 1.5  2006/12/13 04:50:31  mikeb
 * Add watchdog thread
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
 *-----------------------------------------------------------------------------
 */

#ifndef H_echoGetMain
#define H_echoGetMain

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

#define EG_PROG_NAME        "echoget"
#define EG_LOCKFILE_NAME    ".echoget-lockfile"
#define EG_STOP_NAME        "stop"

#define EG_REMOTE_DOC_DIR   "/home/server/.pracman/documents"
#define EG_LOCAL_DOC_DIR    "/home/server/.pracman/documents"

#define EG_REMOTE_DIR       "/data/.pracman/echos/"
#define EG_LOCAL_DIR        "/data/.pracman/echos/"
#define EG_LOCAL_BAD_DIR    "../echos-bad/"

//#define EG_SCP_CMD          "/usr/local/bin/scp -B -q -c blowfish"
//#define EG_SCP_CMD          "/usr/bin/scp -p -B -C -q -c blowfish"
#define EG_SCP_CMD          "/usr/bin/scp -p -B -C -q"

#define EG_RM_CMD           "/bin/rm -rf "
#define EG_MV_CMD           "/bin/mv -f "
#define EG_MKDIR_CMD        "/bin/mkdir "
#define EG_CHMOD_CMD        "/bin/chmod -R 777 "

#define EG_REMOTE_MACHINE_DEFAULT   "drbree.com"

#define MB_CHAR_DASH            (0x2D)
#define MB_CHAR_SPACE           (0x20)
#define MB_CHAR_TAB             (0x09)
#define MB_CHAR_COLON           (0x3A)
#define MB_CHAR_PERIOD          (0x2E)
#define MB_CHAR_BACKSLASH       (0x5C)
#define MB_CHAR_SLASH           (0x2F)
#define MB_CHAR_LF              (0x0A)
#define MB_CHAR_CR              (0x0D)
#define MB_CHAR_FF              (0x0C)
#define MB_CHAR_QUOTE_DOUBLE    (0x22)
#define MB_CHAR_QUOTE_SINGLE    (0x27)

// CRC Polynomial
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
#define EG_CRC_POLYNOMIAL 0x04C11DB7L

#define EG_FILE_MODE_ALL    1
#define EG_FILE_MODE_REG    2

//#define RAND( _parm ) (Int32u_t)( (Float_t)(_parm) * ((Float_t)random()/(Float_t)RAND_MAX) )

typedef struct EgEcho_s
{
    qLinkage_t  linkage;
    Int32u_t    ID;
    Int32u_t    status;
    Int32u_t    date;
    Char_p      name_p;
    Char_p      path_p;
    qHead_t     localFileQueue;
    qHead_p     localFile_q;
    qHead_t     remoteFileQueue;
    qHead_p     remoteFile_q;
} EgEcho_t, *EgEcho_p;


typedef struct  EgFile_s
{
    qLinkage_t      linkage;
    Char_p          name_p;
    Char_p          path_p;
    Char_p          pathName_p;
    Char_p          origName_p;
    Int32u_t        crc;
    Boolean_t       found;
} EgFile_t, *EgFile_p;

Int32s_t strPos
(
    Char_p      s1,
    Char_p      s2
);

Char_p egCharTime( time_t *tm );

Char_p egCharDate( time_t *tm );

#if 0
Void_t egProcessTempFiles
(
    Char_p    tempDir_p,
    Char_p    debugDir_p,
    Char_p    printCommand,
    Int32u_t  skipPrint
);

Void_t          egHandleSignals
(
    Ints_t      sig
);
#endif

Ints_t          egLog
(
    Char_p      string_p,
    ...
);

Int32s_t        egFileListGet
(
    Char_p      dir_p,
    qHead_p     q_p,
    Ints_t      mode
);

Int32s_t        egFileListFree
(
    qHead_p     file_q
);

Int32s_t        egFileListFreeEntry
(
    EgFile_p    file_p
);

Int32s_t        egFileCompare
(
    Char_p      fileName1_p,
    Char_p      fileName2_p
);

Void_t      egEchoQueueFree( qHead_p echo_q );
Int32s_t    egEchoQueueFreeEntry( EgEcho_p echo_p );
Int32s_t    egEchoQueueRemoteGet( qHead_p echo_q );
Void_t      egEchoQueueDisplay( qHead_p echo_q );
Int32s_t    egEchoQueueLocalGet( qHead_p echo_q );
Int32s_t    egSqlExec( Char_p cmd_p );
Int32s_t    egEchoGet( EgEcho_p echo_p );
Int32s_t    egSqlExecInt( Char_p cmd_p, Int32u_p result_p );
Int32s_t    egEchoRemoteFileQueueGet( EgEcho_p echo_p );
Int32s_t    egEchoFileQueueDisplay( qHead_p file_q );
Char_p      egStrClean( Char_p in_p, Char_p out_p );
Int32s_t    egLockFileGet( void );
Char_p      egSqlExecString( Char_p cmd_p, Char_p string_p );
Int32u_t    egToday( void );
void        egCrcInit( void );
Int32u_t    egCrcFile( Char_p fileName_p, Int32u_p  size_p );
Int32u_t    egCrcUpdate( Int32u_t crc, Int8u_p data_p, Int32u_t dataLen );
void        egPurgeTemp( void );
Int32s_t    egEchoVerify( EgEcho_p echo_p, Boolean_t localFlag );
Int32s_t    egMoveBadEcho( EgEcho_p echo_p, Char_p badDir_p );

Char_p      mbStrTrailingSlashRemove( Char_p str_p );
Int32s_t    egStopCheck( void );
Void_p      egWorkerThreadEntry( Void_p args_p );

Char_p      mbMallocStr( Char_p string_p );
Void_p      mbMalloc( int size );
Void_p      mbCalloc( int size );
Void_t      mbFreeFunc( Void_p ptr_p );
#define     mbFree( _parm ) mbFreeFunc( (Void_p)(_parm) )

// Int32s_t    egEchoDelete( EgEcho_p echo_p );

#endif /* H_echoGetMain */

