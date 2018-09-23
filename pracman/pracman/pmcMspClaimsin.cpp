//---------------------------------------------------------------------------
// File:    pmcMspClaimsin.cpp
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
#include "pmcUtils.h"
#include "pmcClaimListForm.h"
#include "pmcDateSelectForm.h"
#include "pmcTables.h"

//---------------------------------------------------------------------------
// Function:  pmcClaimsFileInfoGetString()
//---------------------------------------------------------------------------
// Description:
//
// This function extracts data from from the claimsin file.  It is used
// to compare to the VALIDRPT file
//---------------------------------------------------------------------------

Int32s_t pmcClaimsFileInfoGetString
(
    Char_p      fileName_p,
    Char_p      bufOut_p
)
{
    Int32s_t    returnCode = FALSE;
    FILE       *fp = NIL;
    Char_p      buf_p;
    Char_t      buf2[8];
    Int32u_t    recordType;
    Int32u_t    recordLen;
    Int32u_t    readLen;
    Int32u_t    bytesRead;
    qHead_t     claimRecordQueueHead;
    qHead_p     record_q;
    pmcClaimRecordStruct_p  record_p;

    mbLog( "ClaimsGetInfo called\n" );

    record_q = qInitialize( &claimRecordQueueHead );

    mbMalloc( buf_p, 256 );

    if( ( fp = fopen( fileName_p, "r" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        // There are five extra characters at the start of a line in debug mode
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )  { bytesRead = (Int32u_t)fread( buf_p, 1, 5, fp ); }

        // Read the first two bytes of the record
        bytesRead = (Int32u_t)fread( buf_p, 1, 2, fp );

        if( bytesRead != 2 )
        {
            if( bytesRead != 0 ) mbDlgDebug(( "Error" ));
            break;
        }

        // At this point, we read two bytes.  Determine what kind of record this is
        // PMC_CLAIM_RECORD_CODE_HEADER                "10"
        // PMC_CLAIM_RECORD_CODE_SERVICE               "50"
        // PMC_CLAIM_RECORD_CODE_HOSPITAL              "57"
        // PMC_CLAIM_RECORD_CODE_COMMENT               "60"
        // PMC_CLAIM_RECORD_CODE_RECIPROCAL            "89"
        // PMC_CLAIM_RECORD_CODE_TRAILER               "90"

        *( buf_p + 2 ) = 0;
        if( mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_HEADER ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_HEADER;
            recordLen = PMC_CLAIM_RECORD_LEN_HEADER;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_SERVICE ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_SERVICE;
            recordLen = PMC_CLAIM_RECORD_LEN_SERVICE;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_HOSPITAL ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_HOSPITAL;
            recordLen = PMC_CLAIM_RECORD_LEN_HOSPITAL;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_COMMENT ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_COMMENT;
            recordLen = PMC_CLAIM_RECORD_LEN_COMMENT;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_RECIPROCAL ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_RECIPROCAL;
            recordLen = PMC_CLAIM_RECORD_LEN_RECIPROCAL;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_TRAILER ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_TRAILER;
            recordLen = PMC_CLAIM_RECORD_LEN_TRAILER;
        }
        else
        {
            if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
            {
                // Look for lines of debugging embedded within the file
                if(  mbStrPos( buf_p, "11" ) == 0  || mbStrPos( buf_p, "1-" ) == 0 )
                {
                    recordType = PMC_CLAIM_RECORD_TYPE_DEBUG;
                    recordLen = PMC_CLAIM_RECORD_LEN_DEBUG;
                }
                else
                {
                    mbDlgDebug(( "Invalid record detected.\n" ));
                    goto exit;
                }
            }
            else
            {
                mbDlgDebug(( "Invalid record detected.\n" ));
                goto exit;
            }
        }

        // We have already read the first two lines of the record (to get the type)
        readLen = recordLen - 2;
        bytesRead = (Int32u_t)fread( buf_p, 1, (Int32u_t)readLen, fp );
        if( bytesRead != readLen )
        {
            mbDlgDebug(( "Error" ));
            break;
        }

        // There is one extra character at the end of the line debug mode (|)
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) {fread( buf2, 1, (Int32u_t)1, fp );}

        // Must read the \n character.  It appears that even though we save a CR-LF to
        // the file, all we get back is 0x0A
        fread( buf2, 1, (Int32u_t)1, fp );

        if( recordType == PMC_CLAIM_RECORD_TYPE_DEBUG ) continue;

        mbCalloc( record_p, sizeof( pmcClaimRecordStruct_t ) );
        mbCalloc( record_p->data_p, bytesRead + 10 );
        memcpy( record_p->data_p, buf_p, bytesRead );
        record_p->type = recordType;
        record_p->length = bytesRead;
        qInsertLast( record_q, record_p );

        if( recordType == PMC_CLAIM_RECORD_TYPE_TRAILER )
        {
            pmcClaimsFileTrailerString( record_p, bufOut_p );
        }
    }
    returnCode = TRUE;

exit:

    if( fp ) fclose( fp );

    // Empty out queue
    for( ; ; )
    {
        if( qEmpty( record_q ) ) break;
        record_p = (pmcClaimRecordStruct_p)qRemoveFirst( record_q );

        if( record_p->data_p ) mbFree( record_p->data_p );
        mbFree( record_p );
    }

    mbFree( buf_p );

    return returnCode;
}


//---------------------------------------------------------------------------
// Function:  pmcClaimsFileTrailerString()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t    pmcClaimsFileTrailerString
(
    pmcClaimRecordStruct_p  record_p,
    Char_p                  bufOut_p
)
{
    Char_t      buf[128];
    Char_t      buf2[32];
    Ints_t      i, j;
    Int32u_t    value;
    Char_t      buf3[4096];


    // Copy into indexable buffer.  Offset by two to account for record header
    memcpy( &buf[2], record_p->data_p, record_p->length );

    // Doctor Number
    for( i = 2, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    sprintf( buf3, "Doctor number: %s\n", buf2 );
    strcat( bufOut_p, buf3 );

    // Service Records Submitted
    for( i = 17, j = 0 ; j < 5 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    sprintf( buf3, "Records submitted (Service): %d\n", value );
    strcat( bufOut_p, buf3 );

//    sprintf( buf3, "Records submitted (Comment): %6d\n", commentCount );
//    strcat( bufOut_p, buf3 );

    // Total Records Submitted
    for( i = 12, j = 0 ; j < 5 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    sprintf( buf3, "Records submitted (Total): %d\n", value );
    strcat( bufOut_p, buf3 );

    // Total amount claimed
    for( i = 22, j = 0 ; j < 7 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    sprintf( buf3, "Total amount claimed: %.2f\n", (float)value/100.0 );
    strcat( bufOut_p, buf3 );

    return TRUE;
}


//---------------------------------------------------------------------------
// Function:  pmcClaimsFileInfoGet()
//---------------------------------------------------------------------------
// Description:
//
// This function extracts data from from the claimsin file.  It is used
// to compare to the VALIDRPT file
//---------------------------------------------------------------------------

Int32s_t pmcClaimsFileInfoGet
(
    Char_p      fileName_p,
    qHead_p     provider_q
)
{
    Int32s_t    returnCode = FALSE;
    FILE       *fp = NIL;
    Char_p      buf_p;
    Char_t      buf2[8];
    Int32u_t    recordType;
    Int32u_t    recordLen;
    Int32u_t    readLen;
    Int32u_t    bytesRead;
    qHead_t     claimRecordQueueHead;
    qHead_p     record_q;
    pmcClaimRecordStruct_p  record_p;

    mbLog( "ClaimsGetInfo called\n" );

    record_q = qInitialize( &claimRecordQueueHead );

    mbMalloc( buf_p, 256 );

    if( ( fp = fopen( fileName_p, "r" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        // There are five extra characters at the start of a line in debug mode
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )  { bytesRead = (Int32u_t)fread( buf_p, 1, 5, fp ); }

        // Read the first two bytes of the record
        bytesRead = (Int32u_t)fread( buf_p, 1, 2, fp );

        if( bytesRead != 2 )
        {
            if( bytesRead != 0 ) mbDlgDebug(( "Error" ));
            break;
        }

        // At this point, we read two bytes.  Determine what kind of record this is
        // PMC_CLAIM_RECORD_CODE_HEADER                "10"
        // PMC_CLAIM_RECORD_CODE_SERVICE               "50"
        // PMC_CLAIM_RECORD_CODE_HOSPITAL              "57"
        // PMC_CLAIM_RECORD_CODE_COMMENT               "60"
        // PMC_CLAIM_RECORD_CODE_RECIPROCAL            "89"
        // PMC_CLAIM_RECORD_CODE_TRAILER               "90"

        *( buf_p + 2 ) = 0;
        if( mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_HEADER ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_HEADER;
            recordLen = PMC_CLAIM_RECORD_LEN_HEADER;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_SERVICE ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_SERVICE;
            recordLen = PMC_CLAIM_RECORD_LEN_SERVICE;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_HOSPITAL ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_HOSPITAL;
            recordLen = PMC_CLAIM_RECORD_LEN_HOSPITAL;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_COMMENT ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_COMMENT;
            recordLen = PMC_CLAIM_RECORD_LEN_COMMENT;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_RECIPROCAL ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_RECIPROCAL;
            recordLen = PMC_CLAIM_RECORD_LEN_RECIPROCAL;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_TRAILER ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_TRAILER;
            recordLen = PMC_CLAIM_RECORD_LEN_TRAILER;
        }
        else
        {
            if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
            {
                // Look for lines of debugging embedded within the file
                if(  mbStrPos( buf_p, "11" ) == 0  || mbStrPos( buf_p, "1-" ) == 0 )
                {
                    recordType = PMC_CLAIM_RECORD_TYPE_DEBUG;
                    recordLen = PMC_CLAIM_RECORD_LEN_DEBUG;
                }
                else
                {
                    mbDlgDebug(( "Invalid record detected.\n" ));
                    goto exit;
                }
            }
            else
            {
                mbDlgDebug(( "Invalid record detected.\n" ));
                goto exit;
            }
        }

        // We have already read the first two lines of the record (to get the type)
        readLen = recordLen - 2;
        bytesRead = (Int32u_t)fread( buf_p, 1, (Int32u_t)readLen, fp );
        if( bytesRead != readLen )
        {
            mbDlgDebug(( "Error" ));
            break;
        }

        // There is one extra character at the end of the line debug mode (|)
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) {fread( buf2, 1, (Int32u_t)1, fp );}

        // Must read the \n character.  It appears that even though we save a CR-LF to
        // the file, all we get back is 0x0A
        fread( buf2, 1, (Int32u_t)1, fp );

        if( recordType == PMC_CLAIM_RECORD_TYPE_DEBUG ) continue;

        mbCalloc( record_p, sizeof( pmcClaimRecordStruct_t ) );
        mbCalloc( record_p->data_p, bytesRead + 10 );
        memcpy( record_p->data_p, buf_p, bytesRead );
        record_p->type = recordType;
        record_p->length = bytesRead;
        qInsertLast( record_q, record_p );

        if( recordType == PMC_CLAIM_RECORD_TYPE_TRAILER )
        {
            pmcClaimsFileProviderInfo( provider_q, record_q );
        }
    }
    returnCode = TRUE;

exit:

    if( fp ) fclose( fp );

    // Empty out queue
    for( ; ; )
    {
        if( qEmpty( record_q ) ) break;
        record_p = (pmcClaimRecordStruct_p)qRemoveFirst( record_q );

        if( record_p->data_p ) mbFree( record_p->data_p );
        mbFree( record_p );
    }

    mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFileProviderInfo()
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

Int32s_t pmcClaimsFileProviderInfo
(
    qHead_p     provider_q,
    qHead_p     record_q
)
{
    Int32s_t                returnCode = FALSE;
    pmcClaimRecordStruct_p  record_p;
    pmcClaimsInfoStruct_p   provider_p;
    Char_t                  buf[256];
    Char_t                  buf1[128];
    Ints_t                  i,j;

    // First, stick a new element on the provider queue
    mbCalloc( provider_p, sizeof(pmcClaimsInfoStruct_t ) );
    qInsertLast( provider_q, provider_p );

    // Process the records
    for( ; ; )
    {
        if( qEmpty( record_q ) ) break;

        record_p = (pmcClaimRecordStruct_p)qRemoveFirst( record_q );

        provider_p->recordCount++;

        // Copy into indexable buffer.  Offset by two to accont for record header
        memcpy( &buf[2], record_p->data_p, record_p->length );

        switch( record_p->type )
        {
            case PMC_CLAIM_RECORD_TYPE_HEADER:
                // Read Doctor Number
                for( i = 2, j = 0 ; j < 4 ; i++, j++ ) buf1[j] = buf[i];
                buf1[j] = 0;
                provider_p->providerNumber = atol( buf1 );

                // Read Clinic Number
                for( i = 12, j = 0 ; j < 3 ; i++, j++ ) buf1[j] = buf[i];
                buf1[j] = 0;
                provider_p->clinicNumber = atol( buf1 );
                break;

            case PMC_CLAIM_RECORD_TYPE_SERVICE:
                for( i = 71, j = 0 ; j < 6 ; i++, j++ ) buf1[j] = buf[i];
                buf1[j] = 0;
                provider_p->totalClaimed += atol( buf1 );
                provider_p->serviceRecordCount++;
                break;

            case PMC_CLAIM_RECORD_TYPE_HOSPITAL:
                for( i = 76, j = 0 ; j < 6 ; i++, j++ ) buf1[j] = buf[i];
                buf1[j] = 0;
                provider_p->totalClaimed += atol( buf1 );
                provider_p->serviceRecordCount++;
                break;

            case PMC_CLAIM_RECORD_TYPE_RECIPROCAL:
                provider_p->reciprocalRecordCount++;
                break;
            case PMC_CLAIM_RECORD_TYPE_COMMENT:
                provider_p->commentRecordCount++;
                break;
            case PMC_CLAIM_RECORD_TYPE_TRAILER:
                break;
            default:
                goto exit;
        }

        if( record_p->data_p ) mbFree( record_p->data_p );
        mbFree( record_p );
    }

    returnCode = TRUE;

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrint()
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

Int32s_t pmcClaimsFilePrint
(
    Char_p      fileName_p,
    Char_p      archiveFileName_p,
    Int32u_t    printMode
)
{
    Int32s_t    returnCode = FALSE;
    FILE       *fp = NIL;
    Char_p      buf_p;
    Char_t      buf2[8];
    Int32u_t    recordType;
    Int32u_t    recordLen;
    Int32u_t    readLen;
    Int32u_t    bytesRead;
    qHead_t     claimRecordQueueHead;
    qHead_p     record_q;
    pmcClaimRecordStruct_p  record_p;

    record_q = qInitialize( &claimRecordQueueHead );

    mbMalloc( buf_p, 256 );

    if( ( fp = fopen( fileName_p, "r" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        // There are five extra characters at the start of a line in debug mode
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
        {
            bytesRead = (Int32u_t)fread( buf_p, 1, 5, fp );
        }

        bytesRead = (Int32u_t)fread( buf_p, 1, 2, fp );

        if( bytesRead != 2 )
        {
            if( bytesRead != 0 )
            {
                mbDlgDebug(( "Error" ));
            }
            break;
        }

        // At this point, we read two bytes.  Determine what kind of record this is
        // PMC_CLAIM_RECORD_CODE_HEADER                "10"
        // PMC_CLAIM_RECORD_CODE_SERVICE               "50"
        // PMC_CLAIM_RECORD_CODE_HOSPITAL              "57"
        // PMC_CLAIM_RECORD_CODE_COMMENT               "60"
        // PMC_CLAIM_RECORD_CODE_RECIPROCAL            "89"
        // PMC_CLAIM_RECORD_CODE_TRAILER               "90"

        *( buf_p + 2 ) = 0;
        if( mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_HEADER ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_HEADER;
            recordLen = PMC_CLAIM_RECORD_LEN_HEADER;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_SERVICE ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_SERVICE;
            recordLen = PMC_CLAIM_RECORD_LEN_SERVICE;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_HOSPITAL ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_HOSPITAL;
            recordLen = PMC_CLAIM_RECORD_LEN_HOSPITAL;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_COMMENT ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_COMMENT;
            recordLen = PMC_CLAIM_RECORD_LEN_COMMENT;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_RECIPROCAL ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_RECIPROCAL;
            recordLen = PMC_CLAIM_RECORD_LEN_RECIPROCAL;
        }
        else if(  mbStrPos( buf_p, PMC_CLAIM_RECORD_CODE_TRAILER ) == 0 )
        {
            recordType = PMC_CLAIM_RECORD_TYPE_TRAILER;
            recordLen = PMC_CLAIM_RECORD_LEN_TRAILER;
        }
        else
        {
            if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
            {
                // Look for lines of debugging embedded within the file
                if(  mbStrPos( buf_p, "11" ) == 0  || mbStrPos( buf_p, "1-" ) == 0 )
                {
                    recordType = PMC_CLAIM_RECORD_TYPE_DEBUG;
                    recordLen = PMC_CLAIM_RECORD_LEN_DEBUG;
                }
                else
                {
                    mbDlgDebug(( "Invalid record detected.\n" ));
                    goto exit;
                }
            }
            else
            {
                mbDlgDebug(( "Invalid record detected.\n" ));
                goto exit;
            }
        }

        // We have already read the first two lines of the record (to get the type)
        readLen = recordLen - 2;
        bytesRead = (Int32u_t)fread( buf_p, 1, (Int32u_t)readLen, fp );
        if( bytesRead != readLen )
        {
            mbDlgDebug(( "Error" ));
            break;
        }

        // There is one extra character at the end of the line debug mode (|)
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
        {
            fread( buf2, 1, (Int32u_t)1, fp );
        }

        // Must read the \n character.  It appears that even though we save a CR-LF to
        // the file, all we get back is 0x0A
        fread( buf2, 1, (Int32u_t)1, fp );

        if( recordType == PMC_CLAIM_RECORD_TYPE_DEBUG ) continue;

        mbCalloc( record_p, sizeof( pmcClaimRecordStruct_t ) );
        mbCalloc( record_p->data_p, bytesRead + 10 );
        memcpy( record_p->data_p, buf_p, bytesRead );
        record_p->type = recordType;
        record_p->length = bytesRead;
        qInsertLast( record_q, record_p );

        if( recordType == PMC_CLAIM_RECORD_TYPE_TRAILER )
        {
            if( pmcClaimsFilePrintProvider( fileName_p, archiveFileName_p, record_q, printMode ) != TRUE ) goto exit;
        }
    }
    returnCode = TRUE;

exit:

    if( fp ) fclose( fp );

    // Empty out queue
    for( ; ; )
    {
        if( qEmpty( record_q ) ) break;
        record_p = (pmcClaimRecordStruct_p)qRemoveFirst( record_q );

        if( record_p->data_p ) mbFree( record_p->data_p );
        mbFree( record_p );
    }

    mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrintProvider()
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

Int32s_t pmcClaimsFilePrintProvider
(
    Char_p      fileName_p,
    Char_p      archiveFileName_p,
    qHead_p     record_q,
    Int32u_t    printMode
)
{
    Char_p      tempName_p;
    Char_p      spoolFileName_p;
    Char_p      flagFileName_p;
    Int32s_t    returnCode = FALSE;
    pmcClaimRecordStruct_p  record_p;
    FILE       *fp = NIL;
    Int32u_t    state;
    Int32u_t    index;
    Int32u_t    commentCount;

    mbMalloc( tempName_p, 128 );
    mbMalloc( spoolFileName_p, 128 );
    mbMalloc( flagFileName_p, 128 );

    sprintf( tempName_p, "%s.html", pmcMakeFileName( NIL, spoolFileName_p ) );
    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );

    MbDateTime dateTime = MbDateTime( mbToday(), mbTime() );

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Error opening claims print file '%s'", spoolFileName_p ));
        goto exit;
    }

    state = PMC_CLAIM_FILE_STATE_WAIT_HEADER;

    fprintf( fp, "<HTML><HEAD><Title>Claims Submission Record</TITLE></HEAD><BODY>\n" );
    fprintf( fp, "<H2><CENTER>Record of MSP Claims Submission</CENTER></H1><HR>\n" );
    fprintf( fp, "<PRE WIDTH = 80>\n" );
    fprintf( fp, "File name:      %s\n", fileName_p );

    if( printMode == PMC_CLAIM_FILE_PRINT_MODE_SUBMIT && archiveFileName_p )
    {
        fprintf( fp, "Archived as:    %s\n", archiveFileName_p );
    }
    
    fprintf( fp, "Printed:        %s ", dateTime.HM_TimeString( )  );
    fprintf( fp, "%s\n\n",  dateTime.MDY_DateString( )  );
    fprintf( fp, "%s\n\n", PMC_STRING_DASHED_LINE );

    // Process the records
    for( index = 1, commentCount = 0 ; ; )
    {
        if( qEmpty( record_q ) ) break;

        record_p = (pmcClaimRecordStruct_p)qRemoveFirst( record_q );

        // Print the actual record
        switch( record_p->type )
        {
            case PMC_CLAIM_RECORD_TYPE_HEADER:
                pmcClaimsFilePrintHeader( fp, record_p );
                break;
            case PMC_CLAIM_RECORD_TYPE_SERVICE:
                pmcClaimsFilePrintService( fp, record_p, index++ );
                break;
            case PMC_CLAIM_RECORD_TYPE_HOSPITAL:
                pmcClaimsFilePrintHospital( fp, record_p, index++ );
                break;
            case PMC_CLAIM_RECORD_TYPE_RECIPROCAL:
                pmcClaimsFilePrintReciprocal( fp, record_p, index++ );
                break;
            case PMC_CLAIM_RECORD_TYPE_COMMENT:
                pmcClaimsFilePrintComment( fp, record_p, index );
                commentCount++;
                break;
            case PMC_CLAIM_RECORD_TYPE_TRAILER:
                 pmcClaimsFilePrintTrailer( fp, record_p, commentCount );
                break;
            default:
                goto exit;
        }

        // This section ensures that the records appear in the correct order
        switch( state )
        {
            case PMC_CLAIM_FILE_STATE_WAIT_HEADER:
                if( record_p->type != PMC_CLAIM_RECORD_TYPE_HEADER ) goto exit;
                state = PMC_CLAIM_FILE_STATE_GOT_HEADER;
                break;

            case PMC_CLAIM_FILE_STATE_GOT_HEADER:
                if( record_p->type == PMC_CLAIM_RECORD_TYPE_SERVICE  ||
                    record_p->type == PMC_CLAIM_RECORD_TYPE_HOSPITAL ||
                    record_p->type == PMC_CLAIM_RECORD_TYPE_RECIPROCAL )
                {
                    state =  PMC_CLAIM_FILE_STATE_GOT_SERVICE;
                }
                else
                {
                    goto exit;
                }
                break;

            case PMC_CLAIM_FILE_STATE_GOT_SERVICE:
                if( record_p->type == PMC_CLAIM_RECORD_TYPE_SERVICE  ||
                    record_p->type == PMC_CLAIM_RECORD_TYPE_HOSPITAL ||
                    record_p->type == PMC_CLAIM_RECORD_TYPE_RECIPROCAL )
                {
                    state =  PMC_CLAIM_FILE_STATE_GOT_SERVICE;
                }
                else if( record_p->type == PMC_CLAIM_RECORD_TYPE_COMMENT )
                {
                    state = PMC_CLAIM_FILE_STATE_GOT_COMMENT;
                }
                else if(  record_p->type == PMC_CLAIM_RECORD_TYPE_TRAILER )
                {
                    state = PMC_CLAIM_FILE_STATE_GOT_TRAILER;
                }
                else
                {
                    goto exit;
                }
                break;

             case PMC_CLAIM_FILE_STATE_GOT_COMMENT:
                if( record_p->type == PMC_CLAIM_RECORD_TYPE_SERVICE  ||
                    record_p->type == PMC_CLAIM_RECORD_TYPE_HOSPITAL ||
                    record_p->type == PMC_CLAIM_RECORD_TYPE_RECIPROCAL )
                {
                    state =  PMC_CLAIM_FILE_STATE_GOT_SERVICE;
                }
                else if(  record_p->type == PMC_CLAIM_RECORD_TYPE_TRAILER )
                {
                    state = PMC_CLAIM_FILE_STATE_GOT_TRAILER;
                }
                else
                {
                    goto exit;
                }
                break;

            case PMC_CLAIM_FILE_STATE_GOT_TRAILER:
                goto exit;

            default:
                goto exit;
        }
        if( record_p->data_p ) mbFree( record_p->data_p );
        mbFree( record_p );
    }

    if( state != PMC_CLAIM_FILE_STATE_GOT_TRAILER ) goto exit;

    if( printMode == PMC_CLAIM_FILE_PRINT_MODE_SUBMIT )
    {
    fprintf( fp, "I certify, to the best of my knowledge, that the claims listed herein are a\n"
                 "true and accurate accounting of services provided to the patients indicated,\n"
                 "and that these claims have not previously been paid.\n\n\n" );

    fprintf( fp, "\n\nAuthorized Signature: _________________________________________\n" );
    fprintf( fp, "\n\n                Date: _________________________________________\n\n\n" );
    }
    else if( printMode == PMC_CLAIM_FILE_PRINT_MODE_TEST )
    {
    fprintf( fp, "Note: This printout is the result of a test submission run.  However, this\n"
                 "      computer (%s) is not configured to generate claims submission files,\n"
                 "      and no submission file was produced.\n\n", pmcCfg[CFG_HOSTNAME].str_p );
    }
    else
    {
    fprintf( fp, "Note: This printout was generated from an archived claims submission file.\n\n" );
    }

    fprintf( fp, "</PRE><HR>\n\n" );
    PMC_REPORT_FOOTER( fp );
    // End of document
    fprintf( fp, "</BODY></HTML>\n" );

    returnCode = TRUE;

    mbLog( "Printed claims submission record '%s' (archive %s)\n", fileName_p, archiveFileName_p );

exit:

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

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrintHeader()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t    pmcClaimsFilePrintHeader
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p
)
{
    Char_t      buf[128];
    Char_t      doctorNumber[8];
    Char_t      clinicNumber[8];
    Char_t      doctorName[32];
    Char_t      address[32];
    Char_t      city[32];
    Char_t      postalCode[8];
    Char_t      corpIndicator[8];
    Ints_t      i, j;
    Int32s_t    returnCode = FALSE;
    Int32s_t    found = FALSE;
    Int32u_t    corpCode;

    // Copy into indexable buffer.  Offset by two to accont for record header
    memcpy( &buf[2], record_p->data_p, record_p->length );

    // Read Doctor Number
    for( i = 2, j = 0 ; j < 4 ; i++, j++ ) doctorNumber[j] = buf[i];
    doctorNumber[j] = 0;

    // Read Clinic Number
    for( i = 12, j = 0 ; j < 3 ; i++, j++ ) clinicNumber[j] = buf[i];
    clinicNumber[j] = 0;

    // Read Name
    for( i = 15, j = 0 ; j < 25 ; i++, j++ ) doctorName[j] = buf[i];
    doctorName[j] = 0;

    // Read Address
    for( i = 40, j = 0 ; j < 25 ; i++, j++ ) address[j] = buf[i];
    address[j] = 0;

    // Read City
    for( i = 65, j = 0 ; j < 25 ; i++, j++ ) city[j] = buf[i];
    city[j] = 0;

    // Read Postal Code
    for( i = 90, j = 0 ; j < 6 ; i++, j++ ) postalCode[j] = buf[i];
    postalCode[j] = 0;

    // Read Corporation Indicator
    for( i = 97, j = 0 ; j < 1 ; i++, j++ ) corpIndicator[j] = buf[i];
    corpIndicator[j] = 0;

    // Determine the corporation code
    for( found = FALSE, i = 0 ; i < PMC_MSP_CORP_INVALID ; i++ )
    {
        if( corpIndicator[0] == pmcMspCorpCode[i] )
        {
            found = TRUE;
            break;
        }
    }

    corpCode = ( found == TRUE ) ? i : PMC_MSP_CORP_INVALID;

    fprintf( fp, "Provider Name:        %s\n", doctorName );
    fprintf( fp, "                      %s\n", address );
    fprintf( fp, "                      %s %s\n", city, postalCode );
    fprintf( fp, "Practitioner Number:  %s\n", doctorNumber );
    fprintf( fp, "Clinic Number:        %s\n", clinicNumber );
    fprintf( fp, "Corporation Type:     %s\n", pmcMspCorpCodeDescription[corpCode] );

    fprintf( fp,  "\n\n\n");
    fprintf( fp, "                                 Service-Date   Loc     Ref   Fee   Units    \n" );
    fprintf( fp, "   # Claim    PHN          DOB   First   Last   |  Diag Dr    Code  |  Claimed\n" );
    fprintf( fp,  "%s\n\n", PMC_STRING_DASHED_LINE );

    mbLog( "Printing record of MSP claims submission for '%s'\n", doctorName );
    returnCode = TRUE;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrintService()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t    pmcClaimsFilePrintService
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
)
{
    Char_t      buf[128];
    Char_t      buf2[32];
    Ints_t      i, j;
    Int32s_t    returnCode = FALSE;
    Int32u_t    value;

    // Copy into indexable buffer.  Offset by two to accont for record header
    memcpy( &buf[2], record_p->data_p, record_p->length );

    fprintf( fp, "%4d ", index );

    // Claim
    for( i = 6, j = 0 ; j < 5 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s-", buf2 );

    // Seq
    for( i = 11, j = 0 ; j < 1 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Read Phn
    for( i = 12, j = 0 ; j < 9 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    if( atol( buf2 ) == 0 )
    {
        fprintf( fp,"---------  " );
    }
    else
    {
        fprintf( fp, "%s  ", buf2 );
    }

    // Gender
    for( i = 25, j = 0 ; j < 1 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s ", buf2 );

    // Birth
    for( i = 21, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Date
    for( i = 58, j = 0 ; j < 6 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ------ ", buf2 );

    // Location
    for( i = 66, j = 0 ; j < 1 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Diag
    for( i = 51, j = 0 ; j < 3 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Ref Dr.
    for( i = 54, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Fee Code
    for( i = 67, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s ", buf2 );

    // Units
    for( i = 64, j = 0 ; j < 2 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    fprintf( fp, "%2d  ", value );

    // Claimed
    for( i = 71, j = 0 ; j < 6 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    fprintf( fp, "%7.2f", (float)value/ 100.0 );

    // Name
    for( i = 26, j = 0 ; j < 25 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "\n                         %s ", buf2 );


    fprintf( fp, "\n" );
    returnCode = TRUE;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrintReciprocal()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t    pmcClaimsFilePrintReciprocal
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
)
{
    Int32s_t    returnCode = FALSE;
    Char_t      buf[128];
    Char_t      buf2[32];
    Ints_t      i, j;

    // Copy into indexable buffer.  Offset by two to account for record header
    memcpy( &buf[2], record_p->data_p, record_p->length );

    // Province
    for( i = 21, j = 0 ; j < 2 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "                         RECIPROCAL: %s ", buf2 );

    for( i = 51, j = 0 ; j < 12 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s\n", buf2 );

    returnCode = TRUE;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrintHospital()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t    pmcClaimsFilePrintHospital
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
)
{
    Char_t      buf[128];
    Char_t      buf2[32];
    Ints_t      i, j;
    Int32s_t    returnCode = FALSE;
    Int32u_t    value;

    memcpy( &buf[2], record_p->data_p, record_p->length );

    fprintf( fp, "%4d ", index );

    // Claim
    for( i = 6, j = 0 ; j < 5 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s-", buf2 );

    // Seq
    for( i = 11, j = 0 ; j < 1 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Read Phn
    for( i = 12, j = 0 ; j < 9 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    if( atol( buf2 ) == 0 )
    {
        fprintf( fp,"---------  " );
    }
    else
    {
        fprintf( fp, "%s  ", buf2 );
    }

    // Gender
    for( i = 25, j = 0 ; j < 1 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s ", buf2 );

    // Birth
    for( i = 21, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Date
    for( i = 58, j = 0 ; j < 6 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Last Date
    for( i = 64, j = 0 ; j < 6 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s    ", buf2 );

    // Diag
    for( i = 51, j = 0 ; j < 3 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Ref Dr.
    for( i = 54, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s  ", buf2 );

    // Fee Code
    for( i = 72, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "%s ", buf2 );

    // Units
    for( i = 70, j = 0 ; j < 2 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    fprintf( fp, "%2d  ", value );

    // Claimed
    for( i = 76, j = 0 ; j < 6 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    fprintf( fp, "%7.2f", (float)value/ 100.0 );

    // Name
    for( i = 26, j = 0 ; j < 25 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "\n                         %s ", buf2 );

    fprintf( fp, "\n" );
    returnCode = TRUE;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrintComment()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t    pmcClaimsFilePrintComment
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
)
{
    Char_t      buf[128];
    Char_t      buf2[128];
    Char_t      buf3[64];
    Ints_t      i, j;
    Int32s_t    returnCode = FALSE;

    // Copy into indexable buffer.  Offset by two to accont for record header
    memcpy( &buf[2], record_p->data_p, record_p->length );

    // Read Comment
    for( i = 21, j = 0 ; j < 53 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;

    mbStrClean( buf2, buf3, TRUE );
    fprintf( fp, "                         %s\n", buf3 );

    for( i = 74, j = 0 ; j < 24 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    mbStrClean( buf2, buf3, TRUE );
    if( strlen( buf3 ) )
    {
    fprintf( fp, "                         %s\n", buf3 );
    }

    returnCode = TRUE;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsFilePrintTrailer()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t    pmcClaimsFilePrintTrailer
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                commentCount
)
{
    Char_t      buf[128];
    Char_t      buf2[32];
    Ints_t      i, j;
    Int32u_t    value;

    // Copy into indexable buffer.  Offset by two to account for record header
    memcpy( &buf[2], record_p->data_p, record_p->length );

    fprintf( fp,  "\n%s\n\n", PMC_STRING_DASHED_LINE );

    // Doctor Number
    for( i = 2, j = 0 ; j < 4 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    fprintf( fp, "Doctor number:                 %s\n", buf2 );

    // Service Records Submitted
    for( i = 17, j = 0 ; j < 5 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    fprintf( fp, "Records submitted (Service): %6d\n", value );

    fprintf( fp, "Records submitted (Comment): %6d\n", commentCount );

    // Total Records Submitted
    for( i = 12, j = 0 ; j < 5 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    fprintf( fp, "Records submitted (Total):   %6d\n", value );

    // Total amount claimed
    for( i = 22, j = 0 ; j < 7 ; i++, j++ ) buf2[j] = buf[i];
    buf2[j] = 0;
    value = atol( buf2 );
    fprintf( fp, "Total amount claimed:      %8.2f\n", (float)value/100.0 );

    fprintf( fp,  "\n%s\n\n", PMC_STRING_DASHED_LINE );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  ClaimsReciprocalRecordWrite()
//---------------------------------------------------------------------------
// Description:
//
// This function writes a service record to a claims submission file.
//---------------------------------------------------------------------------

Int32u_t __fastcall TClaimListForm::ClaimReciprocalRecordWrite
(
    FILE           *fp
)
{
    Char_p          rec_p;
    Char_p          buf_p;
    Ints_t          len;
    Int32u_t        returnCode = FALSE;

    mbMalloc( rec_p, 256 );
    mbMalloc( buf_p, 256 );

    // This is actually an out of province billing
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
    {
        fprintf( fp,  "     1--------1---------2---------3---------4---------5---------6---------7---------8---------9--------|\n" );
        fprintf( fp,  "     11222233333455555555566777777777777777777888888888900000000000011111111111111111111111111111111111|\n" );
    }
    len = sprintf( rec_p, "%s", PMC_CLAIM_RECORD_CODE_RECIPROCAL );

    // Doctor number and claim number
    len += sprintf( rec_p+len, "%04ld%05ld", ReciprocalDoctorNumber, ReciprocalClaim_p->claimNumber );

    // Sequence number
    len += sprintf( rec_p+len, "0" );

    // Filler
    len += sprintf( rec_p+len, "         "  );

    // Province
    len += sprintf( rec_p+len, "%2.2s", ReciprocalClaim_p->phnProv_p );

    // Surname
    len += sprintf( rec_p+len, "%-18.18s", ReciprocalClaim_p->lastName_p );

    // First Name
    len += sprintf( rec_p+len, "%-9.9s", ReciprocalClaim_p->firstName_p );

    // Middle Initial (Optional)
    len += sprintf( rec_p+len, " ", ReciprocalClaim_p->firstName_p );

    // Out of province health services number
    strcpy( buf_p, ReciprocalClaim_p->phn_p );
    mbStrAlphaNumericOnly( buf_p );
    len += sprintf( rec_p+len, "%-12.12s", buf_p );

    // Filler (35 spaces)
    len += sprintf( rec_p+len, "                                   " );

    // Now write the record to the file
    mbStrToUpper( rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "RCP: " );
    fprintf( fp, "%s", rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "|" );
    fprintf( fp, "\n" );

    if( strlen( rec_p ) != PMC_CLAIM_RECORD_LEN_RECIPROCAL ) goto exit;

    returnCode = TRUE;
exit:
    mbFree( buf_p );
    mbFree( rec_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  ClaimsTrailerRecordWrite()
//---------------------------------------------------------------------------
// Description:
//
// This function writes a service record to a claims submission file.
//---------------------------------------------------------------------------

Int32u_t __fastcall TClaimListForm::ClaimTrailerRecordWrite
(
    FILE           *fp,
    Int32u_t        doctorNumber,
    Int32u_t        totalFeeClaimed,
    Int32u_t        serviceRecordCount,
    Int32u_t        totalRecordCount
)
{
    Char_p          rec_p;
    Char_p          buf_p;
    Ints_t          len;
    Int32u_t        returnCode = FALSE;

    mbMalloc( rec_p, 256 );
    mbMalloc( buf_p, 256 );

    // Output a reciprocal claim if required
    if( ReciprocalRequired )
    {
        ClaimReciprocalRecordWrite( fp );
        ReciprocalRequired = FALSE;
        totalRecordCount++;
    }

    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
    {
        fprintf( fp,  "     1--------1---------2---------3---------4---------5---------6---------7---------8---------9--------|\n" );
        fprintf( fp,  "     11222233333344444555556666666777777777777777777777777777777777777777777777777777777777777777777777|\n" );
    }
    len = sprintf( rec_p, "%s", PMC_CLAIM_RECORD_CODE_TRAILER );

    // Doctor number
    len += sprintf( rec_p+len, "%04ld", doctorNumber );

    // Filler
    len += sprintf( rec_p+len, "999999", doctorNumber );

    // number of records (total)
    totalRecordCount++;     // must count this record
    len += sprintf( rec_p+len, "%05ld", totalRecordCount );

    // number of records (service)
    len += sprintf( rec_p+len, "%05ld", serviceRecordCount );

    // total fee submitted
    len += sprintf( rec_p+len, "%07ld", totalFeeClaimed );

    // Filler
    for( Ints_t  i = 0 ; i < 69 ; i++ ) strcat( rec_p, " " );

    // Now write the record to the file
    mbStrToUpper( rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "TRL: " );
    fprintf( fp, "%s", rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "|" );
    fprintf( fp, "\n" );

    if( strlen( rec_p ) != PMC_CLAIM_RECORD_LEN_TRAILER ) goto exit;

    returnCode = TRUE;
exit:
    mbFree( buf_p );
    mbFree( rec_p );

    return returnCode;
}


//---------------------------------------------------------------------------
// Function:  ClaimsServiceRecordWrite()
//---------------------------------------------------------------------------
// Description:
//
// This function writes a service record to a claims submission file.
//---------------------------------------------------------------------------

Int32u_t __fastcall TClaimListForm::ClaimServiceRecordWrite
(
    FILE               *fp,
    pmcClaimStruct_p    claim_p,
    Int32u_t            doctorNumber,
    Int32u_p            sequenceNumber_p,
    Int32u_p            recordsWritten_p,
    Int32u_p            feeClaimed_p
)
{
    Int32u_t            recordsWritten = 0;
    Int32u_t            sequenceNumber;
    Ints_t              len;
    Char_p              rec_p;
    Char_p              buf_p;
    Int32u_t            month;
    Int32u_t            year;
    Int32u_t            day;
    Int32u_t            temp1;
    Int32u_t            temp2;
    Int32u_t            feeClaimed = 0;
    MbDateTime          dateTime;
    Int32u_t            returnCode = FALSE;
    Int32u_t            outOfProvince;

    mbMalloc( rec_p, 256 );
    mbMalloc( buf_p, 256 );

    if( claim_p->appointmentType == PMC_CLAIM_TYPE_SERVICE )
    {
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
        {
            fprintf( fp,  "     1--------1---------2---------3---------4---------5---------6---------7---------8---------9--------|\n" );
            fprintf( fp,  "     11222233333455555555566667888888888888888888888888899900001111112234444555555678888888888888888888|\n" );
        }
        len = sprintf( rec_p, "%s", PMC_CLAIM_RECORD_CODE_SERVICE );
    }
    else if( claim_p->appointmentType == PMC_CLAIM_TYPE_HOSPITAL )
    {
        if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
        {
            fprintf( fp,  "     1--------1---------2---------3---------4---------5---------6---------7---------8---------9--------|\n" );
            fprintf( fp,  "     11222233333455555555566667888888888888888888888888899900001111112222223344445555556788888888888888|\n" );
        }
        len = sprintf( rec_p, "%s", PMC_CLAIM_RECORD_CODE_HOSPITAL );
    }
    else
    {
        mbDlgDebug(( "Invalid appointment type detected\n" ));
        goto exit;
    }

    // Doctor number and claim number
    len += sprintf( rec_p+len, "%04ld%05ld", doctorNumber, claim_p->claimNumber );

    // Sequence number
    if( claim_p->claimNumber == PreviousClaimNumber )
    {
        // This is a continuation of the previous claim
        PreviousSequenceNumber++;
    }
    else
    {
        // This is a new claim
        PreviousSequenceNumber = 0;
        PreviousClaimNumber = claim_p->claimNumber;

        // Output record for previous claim (if required);
        if( ReciprocalRequired )
        {
            ClaimReciprocalRecordWrite( fp );
            ReciprocalRequired = FALSE;
            recordsWritten++;
        }
    }
    sequenceNumber = PreviousSequenceNumber;
    len += sprintf( rec_p+len, "%1d", sequenceNumber );

    // Now check to see if this is an out of province claim
    if( strcmp( claim_p->phnProv_p, PMC_PHN_DEFAULT_PROVINCE ) != 0 )
    {
        ReciprocalRequired = TRUE;
        ReciprocalDoctorNumber = doctorNumber;
        ReciprocalClaim_p = claim_p;
        outOfProvince = TRUE;
    }
    else
    {
        outOfProvince = FALSE;
    }

    // Output PHN
    if( !outOfProvince )
    {
        sprintf( buf_p, "%s", claim_p->phn_p );
        mbStrAlphaNumericOnly( buf_p );
        len += sprintf( rec_p+len, "%-9.9s", buf_p );
    }
    else
    {
        len += sprintf( rec_p+len, "         ", buf_p );
    }


    // Date of birth
    dateTime.SetDate( claim_p->dob );
    month = (Int32u_t)dateTime.Month();
    temp1 = (Int32u_t)dateTime.Year();
    temp2 = temp1 / 100;
    year = temp1 - (temp2 * 100 );
    len += sprintf( rec_p+len, "%02d%02d", month, year );

    // Gender
    len += sprintf( rec_p+len, "%s", ( claim_p->gender == 0 ) ? "M" : "F" );

    // Patient Name
    sprintf( buf_p, "%s, %s", claim_p->lastName_p, claim_p->firstName_p );
    len += sprintf( rec_p+len, "%-25.25s", buf_p );

    // ICD Code
    len += sprintf( rec_p+len, "%-3.3s", claim_p->icdCode_p  );

    // Referring Dr number
    {
        Int32u_t    referringDrNumber = 0;

        referringDrNumber = claim_p->referringNumber;
        if( claim_p->refDrTypeIndex )
        {
            // A non-standard type of Dr has been specified (i.e., out of province, cancer clinic, etc.
            referringDrNumber = pmcPickListCodeGet( &pmcDrType[0], claim_p->refDrTypeIndex );
        }

        // Sanity Check
        if( referringDrNumber > PMC_MAX_SASK_DR_NUMBER )
        {
            mbDlgDebug(( "Invalid referring dr number detected (%ld) setting to 0\n", referringDrNumber ));
            referringDrNumber = 0;
        }
        if( referringDrNumber )
        {
            len += sprintf( rec_p+len, "%04ld", referringDrNumber  );
        }
        else
        {
            len += sprintf( rec_p+len, "    " );
        }
    }

    // Date of service
    dateTime.SetDate( claim_p->serviceDay );
    month = (Int32u_t)dateTime.Month();
    day = (Int32u_t)dateTime.Day();
    temp1 = (Int32u_t)dateTime.Year();
    temp2 = temp1 / 100;
    year = temp1 - (temp2 * 100 );
    len += sprintf( rec_p+len, "%02d%02d%02d", day, month, year );

    // At this point the records differ a little depending on type
    if( claim_p->appointmentType == PMC_CLAIM_TYPE_HOSPITAL )
    {
        // Last date of service
        dateTime.SetDate( claim_p->lastDay );
        month = (Int32u_t)dateTime.Month();
        day = (Int32u_t)dateTime.Day();
        temp1 = (Int32u_t)dateTime.Year();
        temp2 = temp1 / 100;
        year = temp1 - (temp2 * 100 );
        len += sprintf( rec_p+len, "%02d%02d%02d", day, month, year );
    }

    // Units of service
    len += sprintf( rec_p+len, "%02d", claim_p->units );

    if( claim_p->appointmentType == PMC_CLAIM_TYPE_SERVICE )
    {
        len += sprintf( rec_p+len, "%c", (Char_t)claim_p->locationCode );
    }

    // Fee code
    pmcFeeCodeFormatDatabase( claim_p->feeCode_p, buf_p );
    len += sprintf( rec_p+len, "%-4.4s", buf_p );

    // Fee submitted
    len += sprintf( rec_p+len, "%06d", claim_p->feeSubmitted );

    // Mode
    len += sprintf( rec_p+len, "%s", PMC_CLAIM_SUBMISSION_TYPE_PHYSICIAN );

    // Type
    len += sprintf( rec_p+len, "%s", PMC_CLAIM_SUBMISSION_TYPE );

    // Filler
    if( claim_p->appointmentType == PMC_CLAIM_TYPE_SERVICE )
    {
        len += sprintf( rec_p+len, "                   "  );    // 19 spaces
    }
    else
    {
        len += sprintf( rec_p+len, "              " );          // 14 spaces
    }

    // Now write the record to the file
    mbStrToUpper( rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "SRV: " );
    fprintf( fp, "%s", rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "|" );
    fprintf( fp, "\n" );
    recordsWritten++;

    // Right now, this statement assumes service, hospital, and reciprocal records are all the same length
    if( strlen( rec_p ) != PMC_CLAIM_RECORD_LEN_SERVICE )
    {
        mbDlgDebug(( "strlen( rec_p ) != PMC_CLAIM_RECORD_LEN_SERVICE (%d != %d)\n",
            strlen( rec_p ), PMC_CLAIM_RECORD_LEN_SERVICE ));
        goto exit;
    }

    if( strlen( claim_p->comment_p ) > 0 && recordsWritten == 1 )
    {
        returnCode = ClaimCommentRecordWrite( fp, claim_p, doctorNumber, sequenceNumber, outOfProvince );
        recordsWritten++;
        if( returnCode != TRUE )
        {
            mbDlgDebug(( " ClaimCommentRecordWrite() returned error\n" ));
            goto exit;
        }
    }

    feeClaimed = claim_p->feeSubmitted;
    returnCode = TRUE;

exit:
    if( recordsWritten_p ) *recordsWritten_p = recordsWritten;
    if( sequenceNumber_p ) *sequenceNumber_p = sequenceNumber;
    if( feeClaimed_p )    *feeClaimed_p = feeClaimed;

    mbFree( rec_p );
    mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  ClaimsCommentRecordWrite()
//---------------------------------------------------------------------------
// Description:
//
// This function writes a comment record to a claims submission file.
//---------------------------------------------------------------------------

Int32u_t __fastcall TClaimListForm::ClaimCommentRecordWrite
(
    FILE               *fp,
    pmcClaimStruct_p    claim_p,
    Int32u_t            doctorNumber,
    Int32u_t            sequenceNumber,
    Int32u_t            outOfProvince
)
{
    Ints_t              len = 0;
    Char_p              rec_p;
    Char_p              buf_p;
    Int32u_t            returnCode = FALSE;

    mbMalloc( rec_p, 256 );
    mbMalloc( buf_p, 256 );

    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
    {
        fprintf( fp,  "     1--------1---------2---------3---------4---------5---------6---------7---------8---------9--------|\n" );
        fprintf( fp,  "     11222233333455555555566666666666666666666666666666666666666666666666666666666666666666666666666666|\n" );
    }
    len = sprintf( rec_p, "%s", PMC_CLAIM_RECORD_CODE_COMMENT );

    // Doctor number and claim number
    len += sprintf( rec_p+len, "%04ld%05ld", doctorNumber, claim_p->claimNumber );

    len += sprintf( rec_p+len, "%1d", sequenceNumber );

    // Output PHN - Not sure how to handle out of province number
    if( !outOfProvince )
    {
        sprintf( buf_p, "%s", claim_p->phn_p );
        mbStrAlphaNumericOnly( buf_p );
        len += sprintf( rec_p+len, "%-9.9s", buf_p );
    }
    else
    {
        len += sprintf( rec_p+len, "         " );
    }

    len += sprintf( rec_p+len, "%-77.77s", claim_p->comment_p );

    // Now write the record to the file
    mbStrToUpper( rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "CMT: " );
    fprintf( fp, "%s", rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "|" );
    fprintf( fp, "\n" );

    if( strlen( rec_p ) != PMC_CLAIM_RECORD_LEN_COMMENT ) goto exit;

    returnCode = TRUE;

exit:

    mbFree( rec_p );
    mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcClaimsHeaderWrite()
//---------------------------------------------------------------------------
// Description:
//
// This function writes a header record to a claims submission file.
//---------------------------------------------------------------------------

Int32s_t __fastcall TClaimListForm::ClaimHeaderWrite
(
    FILE        *fp,
    Int32u_t    providerId,
    Int32u_p    recordsWritten_p,
    Int32u_t    tempClinicNumber,
    MbSQL      *sql_p
)
{
    Char_p      buf_p;
    Char_p      rec_p;
    Int32s_t    doctorNumber;
    Int32u_t    i;
    Ints_t      len = 0;
    Char_p      lastName_p = NIL;
    Char_p      firstName_p = NIL;
    Char_p      clinicName_p = NIL;
    Char_p      streetAddress_p = NIL;
    Char_p      cityProvince_p = NIL;
    Char_p      postalCode_p = NIL;
    Int32s_t    returnCode = -1;
    Int32u_t    recordsWritten = 0;
    Int32u_t    corpCode = 0;

    mbMalloc( buf_p, 256 );
    mbMalloc( rec_p, 256 );

    // First see if an out of province claim is required
    // MAB:20020901: Why in the world would a reciprocal record be
    // required before outputting a new provider?
    if( ReciprocalRequired )
    {
        if( ClaimReciprocalRecordWrite( fp ) != TRUE )
        {
            goto exit;

        }
        ReciprocalRequired = FALSE;
        recordsWritten++;
    }

    // Read the required informtion from the provider table
    //                       0  1  2  3  4  5  6  7  8
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s = %ld",
        PMC_SQL_PROVIDERS_FIELD_PROVIDER_NUMBER,    // 0
        PMC_SQL_PROVIDERS_FIELD_CLINIC_NUMBER,      // 1
        PMC_SQL_FIELD_LAST_NAME,                    // 2
        PMC_SQL_FIELD_FIRST_NAME,                   // 3
        PMC_SQL_PROVIDERS_FIELD_CLINIC_NAME,        // 4
        PMC_SQL_PROVIDERS_FIELD_STREET_ADDRESS,     // 5
        PMC_SQL_PROVIDERS_FIELD_CITY_PROVINCE,      // 6
        PMC_SQL_PROVIDERS_FIELD_POSTAL_CODE,        // 7
        PMC_SQL_PROVIDERS_FIELD_CORP_CODE,          // 8

        PMC_SQL_TABLE_PROVIDERS,
        PMC_SQL_FIELD_ID, providerId );


    i = 0;

    sql_p->Query( buf_p );
    while( sql_p->RowGet( ) )
    {
        doctorNumber = (Int32s_t)sql_p->Int32u( 0 );

        // Last Name
        mbMallocStr( lastName_p, sql_p->String( 2 ) );

        // First Name
        mbMallocStr( firstName_p, sql_p->String( 3 ) );

        // Clinic Name
        mbMallocStr( clinicName_p, sql_p->String( 4 ) );

        // Street Address
        mbMallocStr( streetAddress_p, sql_p->String( 5 ) );

        // City - Province
        mbMallocStr( cityProvince_p, sql_p->String( 6 ) );

        // Postal Code
        mbMallocStr( postalCode_p, sql_p->String( 7 ) );

        // Corporation Code
        corpCode = sql_p->Int32u( 8 );

        i++;
    }

    // Sanity Check
    if( i != 1 ) mbDlgDebug(( "Error reading providerId: %ld, i: %ld", providerId, i ));

    // OK, start to output the header record
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value )
    {
        fprintf( fp,  "     1--------1---------2---------3---------4---------5---------6---------7---------8---------9--------|\n" );
        fprintf( fp,  "     11222233333344455555555555555555555555556666666666666666666666666777777777777777777777777788888890|\n" );
    }

    len = sprintf( rec_p, "%s%04ld000000%03ld", PMC_CLAIM_RECORD_CODE_HEADER, doctorNumber, tempClinicNumber );

    sprintf( buf_p, "Dr. %s %s", firstName_p, lastName_p );
    len += sprintf( rec_p + len, "%-25.25s", buf_p );

    sprintf( buf_p, "%s", streetAddress_p );
    len += sprintf( rec_p + len, "%-25.25s", buf_p );

    sprintf( buf_p, "%s", cityProvince_p );
    len += sprintf( rec_p + len, "%-25.25s", buf_p );

    sprintf( buf_p, "%s", postalCode_p );
    mbStrAlphaNumericOnly( buf_p );
    len += sprintf( rec_p + len, "%-6.6s%1.1s%c",
        buf_p,
        PMC_CLAIM_SUBMISSION_TYPE,
        pmcMspCorpCode[corpCode] );

    // Convert to upper case
    mbStrToUpper( rec_p );

    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "HDR: ", rec_p );
    fprintf( fp, "%s", rec_p );
    if( pmcCfg[CFG_CLAIMS_DEBUG_FLAG].value ) fprintf( fp, "|", rec_p );
    fprintf( fp, "\n", rec_p );

    if( strlen( rec_p ) != PMC_CLAIM_RECORD_LEN_HEADER ) goto exit;

    recordsWritten++;
    returnCode = doctorNumber;

exit:

    mbFree( buf_p );
    mbFree( rec_p );

    mbFree( lastName_p );
    mbFree( firstName_p );
    mbFree( clinicName_p );
    mbFree( streetAddress_p );
    mbFree( cityProvince_p );
    mbFree( postalCode_p );

    if( recordsWritten_p ) *recordsWritten_p = recordsWritten;

    return returnCode;
}

