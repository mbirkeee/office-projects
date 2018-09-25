//---------------------------------------------------------------------------
// File:    pmcMspValidrpt.cpp
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
#include "pmcClaimListForm.h"
#include "pmcMsp.h"
#include "pmcMspClaimsin.h"
#include "pmcMspValidrpt.h"
#include "pmcMspReturns.h"
#include "pmcUtils.h"
#include "pmcClaimListForm.h"
#include "pmcDateSelectForm.h"
#include "pmcTables.h"

//---------------------------------------------------------------------------
// Function:  pmcMspFileValidrptHandle
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspValidrptFileHandle( Boolean_t internetFlag )
{
    bool                foundClaimsin = FALSE;
    bool                foundValidrpt = FALSE;
    bool                foundNotice = FALSE;
    bool                foundReturns = FALSE;
    bool                foundInfo = FALSE;
    Int32s_t            returnCode = FALSE;
    Char_p              dir_p;
    Char_p              archive_p;

    mbMalloc( dir_p, 256 );
    mbMalloc( archive_p, 256 );

    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, FALSE );

    if( foundValidrpt || internetFlag == TRUE )
    {
        if( !foundClaimsin )
        {
            mbDlgExclaim( "Error: Found %s but not %s.\nContact system administrator.", pmcValidrpt_p, pmcClaimsin_p );
            goto exit;
        }

        if( !foundInfo )
        {
            mbDlgExclaim( "Error: Found %s but not %s.\nContact system administrator.", pmcClaimsin_p, pmcInfo_p );
            goto exit;
        }
        else if( internetFlag == FALSE )
        {
            // It looks like we have the CLAIMSIN, CLAIMSIN_INFO, and VALIDRPT files.  Move
            // VALIDRPT to the archive directory, then perform the compare from there.  That
            // allows us to use the same code to perform the same check in the future on the
            // archived files.

            FILE    *fp;

            fp = fopen( pmcInfo_p, "r" );
            if( fp == NIL )
            {
                mbDlgExclaim( "Error opening %s.\nContact system administrator.", pmcInfo_p );
                goto exit;
            }

            // Read line into archive_p - clean into dir_p
            while( fgets( archive_p, 256, fp ) != 0 )
            {
                mbStrClean( archive_p, dir_p, FALSE );
                if( strlen( dir_p ) ) break;
            }
            fclose( fp );

            sprintf( archive_p, "%s.%s", dir_p, PMC_MSP_FILENAME_VALIDRPT );

            // Next lets archive the validrpt file
            if( mbFileCopy( pmcValidrpt_p, archive_p ) != MB_RET_OK )
            {
                mbDlgExclaim( "Error creating archive file %s.\nContact system administrator.", archive_p );
                goto exit;
            }

            // Add an entry into the database for this file
            if( !pmcMspFileDatabaseAdd( archive_p, PMC_MSP_FILE_TYPE_VALIDRPT ) )
            {
                 mbDlgExclaim( "Error adding file %s to database.\nContact system administrator.", archive_p );
            }

            // Now lets read the information from the VALIDRPT file.  This function
            // is multipurpose... it compares the validrpt file to the claimsin file
            // and it prints (a version of) the validrpt file
            if( pmcMspValidrptFileProcess( archive_p ) == TRUE )
            {
                mbLog( " pmcMspFileValidrptProcess( %s ) = TRUE\n", archive_p );
                mbDlgInfo( "The MSP response file %s was sucessfully processed.\n", pmcValidrpt_p );
                returnCode = TRUE;
            }
            else
            {
                mbLog( " pmcValidrptProcess( %s ) = FALSE\n", archive_p );
                mbDlgInfo( "The MSP response file %s was NOT sucessfully processed.\nContact the system administrator.", pmcValidrpt_p );
            }

            // Get rid of the files.  They are archived.
            unlink( pmcClaimsin_p );
            unlink( pmcValidrpt_p );
            unlink( pmcInfo_p );
        }
        else
        {
            mbDlgInfo( "The CLAIMSIN file has been marked as successfully submitted.  A copy has been stored in the database." );
            unlink( pmcClaimsin_p );
            unlink( pmcInfo_p );
            returnCode = TRUE;
        }
    }
    else
    {
        mbDlgExclaim( "Could not find file %s\n", pmcValidrpt_p );
    }

exit:

    mbFree( dir_p );
    mbFree( archive_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspValidrptFileProcess
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspValidrptFileProcess
(
    Char_p                  fileName_p
)
{
    Int32s_t                returnCode = FALSE;
    FILE                   *fp;
    FILE                   *in_p;
    Char_p                  buf_p;
    Char_p                  buf2_p;
    Char_p                  claimsin_p;
    Char_t                  groupNumber[8];
    qHead_t                 tokenList;
    qHead_t                 providerList;
    qHead_p                 provider_q;
    qHead_t                 provider2List;
    qHead_p                 provider2_q;
    pmcTokenStruct_p        token_p;
    pmcClaimsInfoStruct_p   provider_p;
    pmcClaimsInfoStruct_p   provider2_p;
    Char_p                  tempName_p;
    Char_p                  spoolFileName_p;
    Char_p                  flagFileName_p;
    MbDateTime              dateTime;
    bool                    printFlag = FALSE;
    bool                    problem = FALSE;
    bool                    found = FALSE;
    bool                    anyProblem = FALSE;

    // Initialize list of providers found in validrpt file
    provider_q  = qInitialize( &providerList );
    provider2_q = qInitialize( &provider2List );

    mbMalloc( buf_p, 512 );
    mbMalloc( buf2_p, 512 );

    mbMalloc( tempName_p, 128 );
    mbMalloc( spoolFileName_p, 128 );
    mbMalloc( flagFileName_p, 128 );
    mbMalloc( claimsin_p, 256 );

    if( pmcCfg[CFG_MSP_GROUP_NUMBER].value == 0 )
    {
        mbDlgExclaim( "No MSP group number specified.  Cannot complete reconciliation." );
        goto exit;
    }

    // Determine the name of the archived claimsin file based on the
    // validrpt filename
    {
        Ints_t pos;

        strcpy( claimsin_p, fileName_p );
        pos = mbStrPos( claimsin_p, PMC_MSP_FILENAME_VALIDRPT );
        *(claimsin_p + pos ) = 0;
        strcat( claimsin_p, PMC_MSP_FILENAME_CLAIMSIN );
    }

    if( pmcClaimsFileInfoGet( claimsin_p, provider_q ) != TRUE )
    {
        mbDlgExclaim( "Error reading file %s.\nContact system administrator.", claimsin_p );
        goto exit;
    }

    sprintf( groupNumber, "%ld", pmcCfg[CFG_MSP_GROUP_NUMBER].value );

    if( ( in_p = fopen( fileName_p, "r" ) ) == NIL )
    {
        mbDlgExclaim( "Error reading file %s.\nContact system administrator.", fileName_p );
        goto exit;
    }

    // The first loop will extract each providers record information
    while( fgets( buf_p, 512, in_p ) != 0 )
    {
        // Null terminate buffer (in case some kind of binary data was read)
        *(buf_p + 511) = 0;

        mbStrClean( buf_p, buf2_p, TRUE );

        if( mbStrPos( buf2_p, groupNumber ) == 0 )
        {
            mbCalloc( provider2_p, sizeof(pmcClaimsInfoStruct_t ) );
            qInsertLast( provider2_q, provider2_p );

            // Found a provider
            pmcTokenInit( &tokenList, buf2_p );
            token_p = pmcTokenNext( &tokenList );   // Group

            token_p = pmcTokenNext( &tokenList );   // Clinic
            if( token_p ) provider2_p->clinicNumber = atol( mbStrDigitsOnly( token_p->string_p ) );

            token_p = pmcTokenNext( &tokenList );   // Doctor
            if( token_p ) provider2_p->providerNumber = atol( mbStrDigitsOnly( token_p->string_p ) );

            token_p = pmcTokenNext( &tokenList );   // Claims
            if( token_p ) provider2_p->claimCount = atol( mbStrDigitsOnly( token_p->string_p ) );

            token_p = pmcTokenNext( &tokenList );   // Records
            if( token_p ) provider2_p->recordCount = atol( mbStrDigitsOnly( token_p->string_p ) );

            token_p = pmcTokenNext( &tokenList );   // Service records
            if( token_p ) provider2_p->serviceRecordCount = atol( mbStrDigitsOnly( token_p->string_p ) );

            token_p = pmcTokenNext( &tokenList );   // Reciprocal records
            if( token_p ) provider2_p->reciprocalRecordCount = atol( mbStrDigitsOnly( token_p->string_p ) );

            token_p = pmcTokenNext( &tokenList );   // Comment records
            if( token_p ) provider2_p->commentRecordCount = atol( mbStrDigitsOnly( token_p->string_p )) ;

            token_p = pmcTokenNext( &tokenList );   // Total claimed
            if( token_p )
            {
                mbStrDigitsOnly( token_p->string_p );
                provider2_p->totalClaimed = atol( token_p->string_p );
            }
            pmcTokenDone( &tokenList, TRUE );
        }
    }

    if( provider_q->size != provider2_q->size )
    {
        sprintf( buf_p, "Number of submitted providers (%ld) does not match returned providers (%ld)\n",
                provider_q->size, provider2_q->size );
        mbDlgExclaim( buf_p );
        mbLog( buf_p );
    }

    qWalk( provider_p, provider_q, pmcClaimsInfoStruct_p )
    {
        problem = FALSE;
        // Start to output the file
        sprintf( tempName_p, "%s.html", pmcMakeFileName( NIL, spoolFileName_p ) );
        sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );
        sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );

        fp = fopen( spoolFileName_p, "w" );
        if( fp == NIL )
        {
            mbDlgDebug(( "Error opening file %s spool file %s", fileName_p, spoolFileName_p ));
            goto exit;
        }

        fprintf( fp, "<HTML><HEAD><Title>MSP Claims Submission Acknowledgement</TITLE></HEAD><BODY>\n" );
        fprintf( fp, "<H2><CENTER>MSP Claims Submission Acknowledgement</CENTER></H1><HR>\n" );

        fprintf( fp, "<PRE WIDTH = 80>\n" );

        fprintf( fp, "Response file:  %s\n", fileName_p );
        fprintf( fp, "Submit file:    %s\n", claimsin_p );

        dateTime.SetDateTime( mbToday(), mbTime() );
        fprintf( fp, "Printed:        %s ", dateTime.HM_TimeString( ) );
        fprintf( fp, "%s\n\n",  dateTime.MDY_DateString( ) );

        fprintf( fp,  "%s\n\n", PMC_STRING_DASHED_LINE );

        // Get provider description
        sprintf( buf_p, "select %s from %s where %s=%ld and %s != ''",
            PMC_SQL_FIELD_DESC,                         PMC_SQL_TABLE_PROVIDERS,
            PMC_SQL_PROVIDERS_FIELD_PROVIDER_NUMBER,    provider_p->providerNumber,
            PMC_SQL_FIELD_LAST_NAME );

        pmcSqlSelectStr( buf_p, buf2_p, 256, NIL );

        fprintf( fp, "Provider: %s\n", buf2_p );

        // Search for the matching provider in the VALIDRPT file
        found = FALSE;
        qWalk( provider2_p, provider2_q, pmcClaimsInfoStruct_p )
        {
            if( provider_p->providerNumber == provider2_p->providerNumber )
            {
                provider2_p->sub_p = provider_p;
                found = TRUE;
                if( provider_p->recordCount != provider2_p->recordCount )
                {
                    mbLog( "provider_p->recordCount != provider2_p->recordCount (%d %d)\n", provider_p->recordCount, provider2_p->recordCount );
                    problem = TRUE;
                }
                if( provider_p->serviceRecordCount != provider2_p->serviceRecordCount )
                {
                    mbLog( "provider_p->serviceRecordCount != provider2_p->serviceRecordCount (%d %d)\n", provider_p->serviceRecordCount, provider2_p->serviceRecordCount );
                    problem = TRUE;
                }
                if( provider_p->reciprocalRecordCount != provider2_p->reciprocalRecordCount )
                {
                    mbLog( "provider_p->reciprocalRecordCount != provider2_p->reciprocalRecordCount (%d %d)\n", provider_p->reciprocalRecordCount, provider2_p->reciprocalRecordCount );
                    problem = TRUE;
                }
                if( provider_p->commentRecordCount != provider2_p->commentRecordCount )
                {
                    mbLog( "provider_p->commentRecordCount != provider2_p->commentRecordCount (%d %d)\n", provider_p->commentRecordCount, provider2_p->commentRecordCount );
                    problem = TRUE;
                }
                break;
            }
        }

        if( found == FALSE )
        {
            provider2_p = NIL;
            problem = TRUE;
        }

        if( problem )  anyProblem = TRUE;

        fprintf( fp, "\n\n" );
        fprintf( fp, "                Submitted   Acknowledged\n" );

        fprintf( fp, "\n" );
        fprintf( fp, "Provider:        %8ld", provider_p->providerNumber );
        if( provider2_p ) fprintf( fp, "       %8ld", provider2_p->sub_p->providerNumber );
        fprintf( fp, "\n" );

        fprintf( fp, "Records:         %8ld", provider_p->recordCount );
        if( provider2_p ) fprintf( fp, "       %8ld", provider2_p->sub_p->recordCount );
        fprintf( fp, "\n" );

        fprintf( fp, "Service:         %8ld", provider_p->serviceRecordCount );
        if( provider2_p ) fprintf( fp, "       %8ld", provider2_p->sub_p->serviceRecordCount );
        fprintf( fp, "\n" );

        fprintf( fp, "Reciprocal:      %8ld", provider_p->reciprocalRecordCount );
        if( provider2_p ) fprintf( fp, "       %8ld", provider2_p->sub_p->reciprocalRecordCount );
        fprintf( fp, "\n" );

        fprintf( fp, "Comments:        %8ld", provider_p->commentRecordCount );
        if( provider2_p ) fprintf( fp, "       %8ld", provider2_p->sub_p->commentRecordCount );
        fprintf( fp, "\n" );

        fprintf( fp, "Fees:           %9.2f", (float)provider_p->totalClaimed/100.0 );
        if( provider2_p ) fprintf( fp, "      %9.2f", (float)provider2_p->sub_p->totalClaimed/100.0 );
        fprintf( fp, "\n" );

        rewind( in_p );

        fprintf( fp, "\n\n" );
        printFlag = FALSE;
        // Output some contents of the file to the printout
        while( fgets( buf_p, 512, in_p ) != 0 )
        {
            // Null terminate buffer (in case some kind of binary data was read)
            *(buf_p + 511) = 0;

            mbStrClean( buf_p, buf2_p, TRUE );

            if( mbStrPos( buf2_p, "MAG-INPUT" )  == 0 ) printFlag = TRUE;

            if( printFlag && strlen( buf2_p ) > 0 )
            {
                fprintf( fp, "%s\n", buf2_p );
            }
        }

        if( problem )
        {
            anyProblem = TRUE;
            fprintf( fp, "\n\nPROBLEMS WERE ENCOUNTERED WITH THIS SUBMISSION.\nPLEASE CONTACT THE SYSTEM ADMINISTRATOR\n" );
        }
        else
        {
            fprintf( fp, "\n\nSubmission for this provider was acknowledged as valid by the MSP.\n" );
        }

        fprintf( fp, "</PRE><HR>\n\n" );
        PMC_REPORT_FOOTER( fp );
        // End of document
        fprintf( fp, "</BODY></HTML>\n" );

        fclose( fp );

        // Must trigger the print by writing to the flags directory
        fp  = fopen( flagFileName_p, "w" );
        if( fp == NIL )
        {
            mbDlgExclaim( "Could not open flag file %s", flagFileName_p );
        }
        else
        {
            fprintf( fp, "%s", flagFileName_p );
            fclose( fp );
            fp = NIL;
            mbLog( "Printed VALIDRPT file %s\n", fileName_p );
        }
    }

    if( anyProblem )
    {
        mbDlgExclaim( "Problems were detected in the file %s.\n\nContact the system administrator.", fileName_p );
    }
    else
    {
        returnCode = TRUE;
    }

exit:

    if( fp ) fclose( fp );

    // Clear out the list of providers extracted from the VALIDRPT file
    for( ; ; )
    {
        if( qEmpty( provider2_q ) ) break;
        provider2_p = (pmcClaimsInfoStruct_p)qRemoveFirst( provider2_q );
        mbFree( provider2_p );
    }

    // Clear out the list of providers extracted from CLAIMSIN file
    for( ; ; )
    {
        if( qEmpty( provider_q ) ) break;
        provider_p = (pmcClaimsInfoStruct_p)qRemoveFirst( provider_q );
        mbFree( provider_p );
    }

    if( in_p ) fclose( in_p );

    mbFree( tempName_p );
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( claimsin_p );
    mbFree( buf_p );
    mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspValidrptFileCheckEmpty
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspValidrptFileCheckEmpty
(
    Char_p                  fileName_p
)
{
    Int32s_t                returnCode = FALSE;
    FILE                   *fp;
    Char_p                  buf_p, buf2_p;
    Char_t                  groupNumber[8];
    bool                    noDataFlag = FALSE;
    bool                    abnormalFlag = FALSE;

    mbMalloc( buf_p,  512 );
    mbMalloc( buf2_p, 512 );

    sprintf( groupNumber, "%ld", pmcCfg[CFG_MSP_GROUP_NUMBER].value );

    if( ( fp = fopen( fileName_p, "r" ) ) == NIL )
    {
        mbDlgExclaim( "Error reading file %s.\nContact system administrator.", fileName_p );
        goto exit;
    }

    // The first loop will extract each providers record information
    while( fgets( buf_p, 512, fp ) != 0 )
    {
        // Null terminate buffer (in case some kind of binary data was read)
        *(buf_p + 511) = 0;

        mbStrClean( buf_p, buf2_p, TRUE );

        if( mbStrPos( buf2_p, groupNumber ) == 0 )
        {
            break;
        }
        if( mbStrPos( buf2_p, "NO DATA ON INPUT MAG-INPUT" ) >= 0 )
        {
            noDataFlag = TRUE;
        }

        if( mbStrPos( buf2_p, "ABNORMAL END OF JOB" ) >= 0 )
        {
            abnormalFlag = TRUE;
        }
    }

    if( noDataFlag == TRUE && abnormalFlag == TRUE )
    {
        returnCode = TRUE;
    }
exit:

    if( fp ) fclose( fp );

    mbFree( buf_p );
    mbFree( buf2_p );

    return returnCode;
}


