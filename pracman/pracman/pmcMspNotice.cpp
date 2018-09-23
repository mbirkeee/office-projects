//---------------------------------------------------------------------------
// File:    pmcMspNotice.cpp
//---------------------------------------------------------------------------
// Date:    March 26, 2001
// Author:  Michael A. Bree
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcMsp.h"
#include "pmcMspNotice.h"
#include "pmcMspReturns.h"
#include "pmcUtils.h"
#include "pmcClaimListForm.h"

//---------------------------------------------------------------------------
// Function:  pmcMspNoticeFileHandle
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspNoticeFileHandle( void )
{
    bool                foundClaimsin = FALSE;
    bool                foundValidrpt = FALSE;
    bool                foundNotice = FALSE;
    bool                foundReturns = FALSE;
    bool                foundInfo = FALSE;
    Int32s_t            returnCode = FALSE;
    Char_p              buf_p;
    Char_p              archive_p;

    mbMalloc( buf_p, 256 );
    mbMalloc( archive_p, 256 );

    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, FALSE );

    if( foundNotice )
    {
        sprintf( archive_p, "%s", pmcMakeFileName( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p, buf_p ) );
        sprintf( buf_p, "%s.%s", archive_p, PMC_MSP_FILENAME_NOTICE );

        // Archive the notice file
        if( mbFileCopy( pmcNotice_p, buf_p ) != MB_RET_OK )
        {
            mbDlgExclaim( "Error: Could not create MSP archive file %s.\nContact system administrator.", archive_p );
        }
        else
        {
            if( !pmcMspFileDatabaseAdd( buf_p, PMC_MSP_FILE_TYPE_NOTICE ) )
            {
                mbDlgExclaim( "Error adding file %s to database.\nContact system administrator.", buf_p );
            }

            // Attempt to print the archived file
            if( pmcMspNoticeFilePrint( buf_p ) == TRUE )
            {
                // Archive printed OK, delete original
                unlink( pmcNotice_p );
                returnCode = TRUE;
            }
            else
            {
                // Trouble printing archive, delete it.
                mbDlgExclaim( "Error printing MSP notice file %s.\n", pmcNotice_p );
                unlink( buf_p );
            }
        }
    }
    else
    {
        mbDlgExclaim( "Could not find MSP notice file %s.\n", pmcNotice_p );
    }

    mbFree( buf_p );
    mbFree( archive_p );

    return returnCode;

}

//---------------------------------------------------------------------------
// Function:  pmcMspNoticeFilePrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspNoticeFilePrint
(
    Char_p                  fileName_p
)
{
    Char_p                          tempName_p;
    Char_p                          spoolFileName_p;
    Char_p                          flagFileName_p;
    Char_p                          buf1_p;
    Char_p                          buf2_p;
    Int32s_t                        returnCode = FALSE;
    FILE                           *fp = NIL;
    FILE                           *in_p = NIL;
    MbDateTime                      dateTime;

    mbMalloc( tempName_p, 128 );
    mbMalloc( spoolFileName_p, 128 );
    mbMalloc( flagFileName_p, 128 );
    mbMalloc( buf1_p, 256);
    mbMalloc( buf2_p, 256);

    sprintf( tempName_p, "%s.html", pmcMakeFileName( NIL, spoolFileName_p ) );
    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgExclaim( "Error opening notice print file %s", spoolFileName_p );
        goto exit;
    }

    in_p = fopen( fileName_p, "r" );
    if( in_p == NIL )
    {
        mbDlgExclaim( "Error opening notice file %s", fileName_p );
        goto exit;
    }

    fprintf( fp, "<HTML><HEAD><Title>MSP Notice</TITLE></HEAD><BODY>\n" );
    fprintf( fp, "<H2><CENTER>MSP 'NOTICE' File Report</CENTER></H1><HR>\n" );

    fprintf( fp, "<PRE WIDTH = 80>\n" );

    fprintf( fp, "File name:       %s\n", fileName_p );

    dateTime.SetDateTime( mbToday(), mbTime() );
    fprintf( fp, "Printed:         %s ", dateTime.HM_TimeString( ) );
    fprintf( fp, "%s\n",  dateTime.MDY_DateString( ) );

    fprintf( fp, "\n" );

    while( fgets( buf1_p, 256, in_p ) != 0 )
    {
        // Null terminate buffer (in case some kind of binary data was read)
        *(buf1_p + 255) = 0;

        mbStrClean( buf1_p, buf2_p, TRUE );
        fprintf( fp, "%s\n", buf2_p );
    }

    fprintf( fp, "</PRE><HR>\n\n" );
    PMC_REPORT_FOOTER( fp );
    // End of document
    fprintf( fp, "</BODY></HTML>\n" );

    returnCode = TRUE;

    mbLog( "Printed processed notice report '%s'\n", fileName_p );

    mbDlgInfo( "MSP notice file sent to printer." );

exit:

    if( in_p ) fclose( in_p );

    if( fp )
    {
        fclose( fp );
        if( returnCode == FALSE )
        {
            unlink( spoolFileName_p );
        }
        else
        {
            // Must trigger the print by writing to the flags directory
            fp  = fopen( flagFileName_p, "w" );
            if( fp == NIL )
            {
                mbDlgExclaim( "Could not open flag file '%s'", flagFileName_p );
            }
            else
            {
                fprintf( fp, "%s", flagFileName_p );
                fclose( fp );
            }
        }
    }

    mbFree( tempName_p );
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;

}



