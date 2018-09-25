//---------------------------------------------------------------------------
// File:    pmcMspReturns.cpp
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
#include "pmcMspReturns.h"
#include "pmcMainForm.h"
#include "pmcUtils.h"
#include "pmcClaimListForm.h"
#include "pmcMsp.h"

Boolean_t   globUpdateDatabase = TRUE;

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsFileHandle
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsFileHandle( void )
{
    bool                foundClaimsin = FALSE;
    bool                foundValidrpt = FALSE;
    bool                foundNotice = FALSE;
    bool                foundReturns = FALSE;
    bool                foundInfo = FALSE;
    Int32s_t            returnCode = FALSE;
    Char_p              buf_p;
    Char_p              archive_p;
    Int32u_t            crc;
    Int32u_t            size;
    Int32u_t            id;

    globUpdateDatabase = TRUE;

    mbMalloc( buf_p, 256 );
    mbMalloc( archive_p, 256 );

    // Check to see what files are in the MSP directory
    pmcMspFilesLocate( &foundClaimsin, &foundInfo, &foundValidrpt, &foundReturns, &foundNotice, FALSE );

    if( foundReturns )
    {
        sprintf( archive_p, "%s", pmcMakeFileName( pmcCfg[CFG_MSP_ARCHIVE_DIR].str_p, buf_p ) );
        sprintf( buf_p, "%s.%s", archive_p, PMC_MSP_FILENAME_RETURNS );

        crc = mbCrcFile( pmcReturns_p, &size, NIL, NIL, NIL, NIL, NIL );

        if( ( id = pmcMspFileDatabaseCheck( crc, size, PMC_MSP_FILE_TYPE_RETURNS ) ) > 0 )
        {
            mbDlgExclaim( "The file %s has already been processed.", pmcReturns_p );
            mbLog( "Found returns file already processed, id: %lu\n", id );
            // Archive the file anyway.. just in case

            if( mbFileCopy( pmcReturns_p, buf_p ) != MB_RET_OK )
            {
                 mbDlgError( "Could not create archive file %s.\nContact system administrator.", archive_p );
            }
            else
            {
                unlink( pmcReturns_p );
            }
            goto exit;
        }

        // Archive the returns file
        if( mbFileCopy( pmcReturns_p, buf_p ) != MB_RET_OK )
        {
             mbDlgError( "Could not create archive file %s.\nContact system administrator.", archive_p );
             goto exit;
        }

         // Print the file
        if( pmcMspReturnsFilePrint( buf_p ) == TRUE )
        {
            // Add this file to the list in the database
            if( !pmcMspFileDatabaseAdd( buf_p, PMC_MSP_FILE_TYPE_RETURNS ) )
            {
                mbDlgExclaim( "Error adding file %s to database.\nContact system administrator.", buf_p );
                goto exit;
            }

            // Archive printed OK.  Now lets process it
            if( pmcMspReturnsFileProcess( buf_p ) != TRUE )
            {
                mbDlgExclaim( "Error processing returns file %s.\nContact system administrator.", buf_p );
            }

            // Delete file.. it archived and printed OK
            if( globUpdateDatabase )
            {
                unlink( pmcReturns_p );
            }
            else
            {
                unlink( buf_p );
            }
            returnCode = TRUE;

        }
        else
        {
            // Trouble printing archive, delete it.
            mbDlgExclaim( "Error printing MSP returns file %s\n", pmcReturns_p );
            unlink( buf_p );
        }
    }

exit:

    mbFree( buf_p );
    mbFree( archive_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    pmcMspReturnsFileDatabaseAdd
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32u_t        pmcMspFileDatabaseAdd
(
    Char_p      fileName_p,
    Int32u_t    type
)
{
    Int32u_t    crc;
    Int32u_t    size;
    Int32u_t    id = 0;
    Char_p      cmd_p;
    Char_p      name_p;

    mbMalloc( cmd_p, 256 );
    mbMalloc( name_p, 128 );

    crc = mbCrcFile( fileName_p, &size, NIL, NIL, NIL, NIL, NIL );

    // Sanity check
    if( crc == 0 )
    {
        mbDlgDebug(( "Error computing CRC on file %s\n", fileName_p ));
        goto exit;
    }

    pmcFilePathAndNameGet( fileName_p, NIL, name_p );

    // Clean the filename
    mbStrClean( name_p, cmd_p, FALSE );
    strcpy( name_p, cmd_p );

    if( globUpdateDatabase == FALSE  )
    {
        if( type == PMC_MSP_FILE_TYPE_RETURNS )
        {
            mbDlgInfo( "globUpdateDatabase == FALSE; not adding RETURNS file to database" );
        }
        id = 1;
        goto exit;
    }

    if( ( id = pmcSqlRecordCreate( PMC_SQL_TABLE_MSP_FILES, NIL ) ) == 0 ) goto exit;

    // Format command to get id from database
    sprintf( cmd_p, "update %s set %s=%lu,%s=%lu,%s=%lu,%s=\"%s\" where %s=%lu",
        PMC_SQL_TABLE_MSP_FILES,
        PMC_SQL_FIELD_CRC,      crc,
        PMC_SQL_FIELD_SIZE,     size,
        PMC_SQL_FIELD_TYPE,     type,
        PMC_SQL_FIELD_NAME,     name_p,
        PMC_SQL_FIELD_ID,       id );

    pmcSqlExec( cmd_p );

    pmcSqlRecordUnlock( PMC_SQL_TABLE_MSP_FILES, id );
    pmcSqlRecordUndelete( PMC_SQL_TABLE_MSP_FILES, id );

exit:
    mbFree( cmd_p );
    mbFree( name_p );
    return id;
}

//---------------------------------------------------------------------------
// Function:  pmcMspFileDatabaseCheck
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32u_t        pmcMspFileDatabaseCheck
(
    Int32u_t    crc,
    Int32u_t    size,
    Int32u_t    type
)
{
    Int32u_t    id = 0;
    Int32u_t    count = 0;
    Char_p      cmd_p;

    mbMalloc( cmd_p, 256 );

    // Format command to get id from database
    sprintf( cmd_p, "select %s from %s where %s=%lu and %s=%lu and %s=%lu",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_MSP_FILES,
        PMC_SQL_FIELD_CRC, crc,
        PMC_SQL_FIELD_SIZE, size,
        PMC_SQL_FIELD_TYPE, type );

    id = pmcSqlSelectInt( cmd_p, &count );

    // Sanity Check
    if( count > 1 ) mbDlgDebug(( "Error: count: %ld\n", count ));

    mbFree( cmd_p );

    return id;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsFileProcess
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsFileProcess
(
    Char_p                          fileName_p
)
{
    qHead_t                         recordQueueHead;
    qHead_p                         record_q;
    Int32s_t                        returnCode = FALSE;
    qHead_t                         tempQueue;
    qHead_p                         temp_q;
    Int32u_t                        size, i;
    pmcMspReturnsRecordStruct_p     record_p;
    pmcMspReturnsRecordStruct_p     prevRecord_p;
    TThermometer                   *thermometer_p = NIL;
    Char_p                          buf_p;
    MbSQL                           sql;

    mbMalloc( buf_p, 128 );

    mbLog( "Process MSP returns file %s\n", fileName_p );

    // Initialize queue of records and providers
    record_q = qInitialize( &recordQueueHead );
    temp_q = qInitialize( &tempQueue );

    // Read all the records from the file
    if( pmcMspReturnsFileRecordsGet( fileName_p, record_q ) == FALSE )
    {
        goto exit;
    }

    size = record_q->size;

    sprintf( buf_p, "Processing %ld claims...", size );
    thermometer_p = new TThermometer( buf_p, 0, size, FALSE );

    pmcSuspendPollInc( );

    // Lock all of the tables we are going to use.
    // 20021123:  Add lock on patients table, as it cannot be read without the lock.
    // Want to read PHN from patients table to verify claim.

    sprintf( buf_p, "lock tables %s write, %s write, %s write",
        PMC_SQL_TABLE_CLAIMS,
        PMC_SQL_TABLE_CLAIM_HEADERS,
        PMC_SQL_TABLE_PATIENTS );

    sql.Update( buf_p );

    // This loop tries to assign returned comments to particular records
    qWalk( record_p, record_q, pmcMspReturnsRecordStruct_p )
    {
        if( record_p->type == PMC_RETURN_TYPE_COMMENT )
        {
            prevRecord_p = (pmcMspReturnsRecordStruct_p)record_p->linkage.blink;
            if( prevRecord_p != (pmcMspReturnsRecordStruct_p)record_q )
            {
                if( record_p->phn == prevRecord_p->phn )
                {
                    mbMalloc( prevRecord_p->comment_p, strlen( record_p->str_p ) + 1 );
                    strcpy( prevRecord_p->comment_p, record_p->str_p );
                }
            }
        }
    }

    // Filter out the unwanted records
    for( i = 0 ; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );

        // MAB:20020901: There appears to be an MSP bug where they are sending back some
        // electronic claims marked as paper
        if( record_p->formType == PMC_RETURN_FORM_TYPE_PAPER )
        {
            if( record_p->claimNumber > 10000 ) record_p->formType = PMC_RETURN_FORM_TYPE_ELECTRONIC_9;
        }

        if(    record_p->formType == PMC_RETURN_FORM_TYPE_PAPER
            || record_p->type     == PMC_RETURN_TYPE_TOTAL
            || record_p->type     == PMC_RETURN_TYPE_COMMENT
            || record_p->type     == PMC_RETURN_TYPE_RECIPROCAL )
        {
            thermometer_p->Increment( );
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    // Filter out "MESSAGE" records that do not belong to any claim
    size = record_q->size;
    for( i = 0; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );

        if(    record_p->type == PMC_RETURN_TYPE_MESSAGE
            && record_p->claimNumber == PMC_CLAIM_NUMBER_MAX )
        {
            if( thermometer_p ) thermometer_p->Increment( );
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    // There appears to be a bug in which the msp is returning a wrong date
    // when dividing claims with multiple units up into separate responses.
    // This function attempts to compensate for the bug.
    pmcMspReturnsBugFix1( record_q );

    size = record_q->size;

    // Now process the remaining records
    for( i = 0; i < size ; i++ )
    {
        if( thermometer_p ) thermometer_p->Increment( );
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        pmcMspReturnsRecordProcess( record_p, &sql );
        qInsertLast( temp_q, record_p );
    }

    sprintf( buf_p, "unlock tables" );
    sql.Update( buf_p );

    pmcSuspendPollDec( );

    // Sanity Check
    if( record_q->size != 0 )
    {
        mbDlgDebug(( "%d unprocessed claims records!",  record_q->size ));
    }

    returnCode = TRUE;

exit:

    if( thermometer_p ) delete thermometer_p;
    pmcMspReturnsQueueClean( temp_q );
    pmcMspReturnsQueueClean( record_q );
    mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordProcess
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordProcess
(
    pmcMspReturnsRecordStruct_p     record_p,
    MbSQL                          *sql_p
)
{
    Int32u_t    id;
    Int32u_t    today;
    Char_p      buf_p, buf1_p, buf2_p, buf3_p, buf4_p;
    Int32s_t    feeSubmitted;
    Int32u_t    submitDate;
    Int32u_t    replyDate;
    Int32u_t    newStatus;
    Int32u_t    oldStatus;
    Int32u_t    unitsPaid;
    Int32u_t    unitsClaimed;
    Int32u_t    newUnitsPaid;
    Int32s_t    premiumPaid;
    Int32s_t    feePaid;
    Int32s_t    premiumFlag = FALSE;
    Boolean_t   addFee = FALSE;
    Char_p      feeString_p = NIL;
    Int32u_t    unitClawback = 0;
    Int32u_t    newFlag = FALSE;
    Boolean_t   addNewUnits = TRUE;

    mbMalloc( buf_p,  512 );
    mbMalloc( buf1_p, 128 );
    mbMalloc( buf2_p, 128 );
    mbMalloc( buf3_p, 128 );
    mbMalloc( buf4_p, 256 );
    mbMalloc( feeString_p, 128 );

    today = mbToday( );

    // Output a line to the log file to help separate claim info
    mbLog( "------------------------\n" );

    id = pmcMspReturnsRecordIdGet(  record_p,
                                   &feeSubmitted,
                                   &submitDate,
                                   &replyDate,
                                   &oldStatus,
                                   &unitsPaid,
                                   &unitsClaimed,
                                   &feePaid,
                                   &premiumPaid,
                                   &newFlag,
                                   sql_p );

    // MAB: Do nothing if a new claim was added
    if( newFlag == TRUE ) goto exit;


    if( id )
    {
        // MAB:20020504: Handle case where a multi unit claim has one
        // or more units changed.
        if( record_p->type == PMC_RETURN_TYPE_CODE_CHANGE_MULTI )
        {
            mbStrClean( record_p->feeCodeApproved, buf1_p, TRUE );
            sprintf( buf4_p, "Changed %d unit%s to code '%s' (paid %6.2f)",
                record_p->units,
                ( record_p->units > 1 ) ? "s" : "", buf1_p,
                (float)record_p->feeeApproved/100.0 );

            // Start formatting the SQL command
            sprintf( buf_p, "update %s set %s=%s+%ld,%s=%s+%ld,%s=%ld,%s=%ld,%s='%s',%s=\"%s\" "
                            "where %s=%ld",
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_CLAIMS_FIELD_FEE_PAID, PMC_SQL_CLAIMS_FIELD_FEE_PAID, record_p->feeeApproved,
                PMC_SQL_CLAIMS_FIELD_UNITS_PAID, PMC_SQL_CLAIMS_FIELD_UNITS_PAID, record_p->units,
                PMC_SQL_CLAIMS_FIELD_REPLY_DATE,        today,
                PMC_SQL_CLAIMS_FIELD_ADJ_CODE,          (Int8u_t)record_p->adjCode[0],
                PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, buf1_p,
                PMC_SQL_CLAIMS_FIELD_COMMENT_FROM_MSP,  buf4_p,
                PMC_SQL_FIELD_ID,                       id
            );

            if( globUpdateDatabase )
            {
//                pmcSqlExec( buf_p );
                sql_p->Update( buf_p );
            }
            mbLog( buf_p );
            goto exit;
        }

        if( record_p->type == PMC_RETURN_TYPE_ADDITIONAL )
        {
            if( oldStatus == PMC_CLAIM_STATUS_SUBMITTED )
            {
                mbDlgDebug(( "Got ADDITIONAL return on SUBMITTED claim %ld", id ));
                goto exit;
            }
            //  mbDlgDebug(( "Process additional record for claim %ld fee paid %ld aditional %ld\n",
            //              id, feePaid, record_p->feeApproved ));

            mbStrClean( record_p->feeCodeApproved, buf1_p, TRUE );
            mbStrClean( record_p->expCode,         buf2_p, TRUE );
            mbStrClean( record_p->runCode,         buf3_p, TRUE );

            sprintf( buf4_p, "Additional payment: %7.2f for %ld units of %s  (run: %s exp: '%s')",
                 (float)record_p->feeeApproved/100.0, record_p->units, buf1_p, buf3_p, buf2_p );

            // Start formatting the SQL command
            sprintf( buf_p, "update %s set %s=%s+%ld,%s=%ld,%s=%ld,%s='%s+',%s=\"%s\" "
                            "where %s=%ld",
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_CLAIMS_FIELD_FEE_PAID, PMC_SQL_CLAIMS_FIELD_FEE_PAID, record_p->feeeApproved,
                PMC_SQL_CLAIMS_FIELD_REPLY_DATE,        today,
                PMC_SQL_CLAIMS_FIELD_ADJ_CODE,          (Int8u_t)record_p->adjCode[0],
                PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, buf1_p,
                PMC_SQL_CLAIMS_FIELD_COMMENT_FROM_MSP,  buf4_p,
                PMC_SQL_FIELD_ID,                       id
            );

            if( globUpdateDatabase )
            {
//                pmcSqlExec( buf_p );
                sql_p->Update( buf_p );
            }
            mbLog( buf_p );
            goto exit;
        }

        if( oldStatus != PMC_CLAIM_STATUS_SUBMITTED && record_p->type == PMC_RETURN_TYPE_RECOVERY )
        {
            // Sanity check - I am not sure how to handle the case where the units paid
            // might be different than the units claimed.
            if( (Int32u_t)record_p->units != (Int32u_t)unitsPaid )
            {
                Int32u_t    unitFee1;
                Int32u_t    unitFee2;

                if( (Int32u_t)record_p->units > (Int32u_t)unitsPaid )
                {
                    mbDlgDebug(( "Dont know how to handle: units: %d claimed: %d id: %d\n", record_p->units, unitsClaimed, id ));
                    goto exit;
                }

                // MAB:20020609:  Look for a reduction in the number of units paid.
                unitFee1 = feePaid / unitsPaid;
                unitFee2 = record_p->feeeApproved / record_p->units;

                if( unitFee1 == unitFee2 )
                {
                    unitClawback = record_p->units;
                }
            }

            // MAB: 20011125:  This is extremely ugly.  But what the hell.  If we find a recovery
            // record, process it here.  Forget about trying to wedge the processing of the recovery
            // record into the code below.  Jump to exit when finished.

            mbLog( "Process recovery record for claim %ld fee paid %ld reduced by %ld units( paid %ld recovered %ld )\n",
                         id, feePaid, record_p->feeeApproved, unitsPaid, record_p->units );

            mbStrClean( record_p->feeCodeApproved, buf1_p, TRUE );
            mbStrClean( record_p->expCode,         buf2_p, TRUE );
            mbStrClean( record_p->runCode,         buf3_p, TRUE );

            // Start formatting the SQL command
            sprintf( buf_p, "update %s set %s=%ld" ,
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_CLAIMS_FIELD_REPLY_DATE, today );

            if( strcmp( record_p->expCode, PMC_MSP_EXP_CODE_PREMIUM ) == 0 )
            {
                 mbLog( "This is a premium clawback %s premium paid %7.2f recover %7.2f",
                             record_p->expCode, (float)premiumPaid/100.0, (float)record_p->feeeApproved/100.0 );

                // Sanity check
                if( premiumPaid < abs( record_p->feeeApproved ) )
                {
                    mbDlgInfo( "Error: Premium recovery of (%7.2f) greater than premium paid (%7.2f)\n",
                        (float)record_p->feeeApproved/100.0, (float)premiumPaid/100.0 );
                    goto exit;
                }

                // Must subtract premium from both premium paid field and total paid field
                sprintf( buf4_p, ",%s=%s-%ld,%s=%s-%ld",
                    PMC_SQL_CLAIMS_FIELD_FEE_PAID, PMC_SQL_CLAIMS_FIELD_FEE_PAID, abs( record_p->feeeApproved ),
                    PMC_SQL_CLAIMS_FIELD_FEE_PREMIUM, PMC_SQL_CLAIMS_FIELD_FEE_PREMIUM, abs( record_p->feeeApproved ) );
                strcat( buf_p, buf4_p );
            }
            else
            {
                mbLog( "This is a regular clawback %s paid %7.2f recover %7.2f" ,
                             record_p->expCode, (float)feePaid/100.0, (float)record_p->feeeApproved/100.0 );

                // Sanity check
                if( feePaid < abs( record_p->feeeApproved ) )
                {
                    mbDlgInfo( "Error: Payment recovery of (%7.2f) greater than paid (%7.2f)\n",
                        (float)record_p->feeeApproved/100.0, (float)feePaid/100.0 );
                    goto exit;
                }

                sprintf( buf4_p, ",%s=%s-%d,%s='%s',%s='%s'",
                    PMC_SQL_CLAIMS_FIELD_FEE_PAID, PMC_SQL_CLAIMS_FIELD_FEE_PAID, abs( record_p->feeeApproved ),
                    PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, buf1_p,
                    PMC_SQL_CLAIMS_FIELD_EXP_CODE, buf2_p );

                strcat( buf_p, buf4_p );

                if( ( feePaid - abs( record_p->feeeApproved ) ) < feeSubmitted )
                {
                    sprintf( buf4_p, ",%s=%d,%s=%d",
                        PMC_SQL_CLAIMS_FIELD_STATUS, PMC_CLAIM_STATUS_REDUCED,
                        PMC_SQL_CLAIMS_FIELD_REPLY_STATUS, PMC_CLAIM_STATUS_REDUCED );
                    strcat( buf_p, buf4_p );
                }

                if( unitClawback )
                {
                    sprintf( buf4_p, ",%s=%s-%d",
                        PMC_SQL_CLAIMS_FIELD_UNITS_PAID, PMC_SQL_CLAIMS_FIELD_UNITS_PAID, unitClawback );
                    strcat( buf_p, buf4_p );
                }

                // There seems to be a bug where some older claims do not have
                // the units paid field set.
                if( (Int32u_t)unitsPaid > (Int32u_t)record_p->units )
                {
                    mbLog( "unitsPaid: %d record_p->units: %d claim: %d\n",
                        unitsPaid, record_p->units, id );
                }
            }

            sprintf( buf4_p, ",%s='%s',%s=%ld where %s=%ld",
                PMC_SQL_CLAIMS_FIELD_RUN_CODE, buf3_p,
                PMC_SQL_CLAIMS_FIELD_ADJ_CODE, (Int8u_t)record_p->adjCode[0],
                PMC_SQL_FIELD_ID, id );

            strcat( buf_p, buf4_p );

            if( globUpdateDatabase )
            {
//                pmcSqlExec( buf_p );
                sql_p->Update( buf_p );
            }

            mbLog( buf_p );

            // End of section processing recovery record
            goto exit;
        }

        if( oldStatus != PMC_CLAIM_STATUS_SUBMITTED )
        {
            // Look for a "Premium record".  This is kind of ugly, but nowhere did
            // it state that an extra record is sent back for the premium code.  So
            // what I am finding is a premium record for a claim that has already
            // been marked as paid

            if( oldStatus == PMC_CLAIM_STATUS_PAID || oldStatus == PMC_CLAIM_STATUS_REDUCED )
            {
                if( record_p->feeeSubmitted == 0 )
                {
                    if( record_p->seqNumber == 9 || record_p->type == PMC_RETURN_TYPE_PAID )
                    {
                        if( mbStrPos( record_p->expCode, PMC_MSP_EXP_CODE_PREMIUM ) >= 0 )
                        {
                            premiumFlag = TRUE;
                            mbDlgExclaim( "Should no longer see premium records\n" );
                        }
                    }
                }
            }

            if( premiumFlag )
            {
                Char_t      expCode[32];
                Char_t      existingCode[32];

                mbStrClean( record_p->feeCodeApproved, buf1_p, TRUE );
                mbStrClean( record_p->expCode,         buf2_p, TRUE );

                // Ugliness here.  MSP may have returned an EXP code for the claim, then
                // returned an "FZ" premium exp code.  If the code is indeed a premium,
                // do not overwrite an existing code.

                sprintf( expCode, "'%s'", buf2_p );

                sprintf( buf_p, "select %s from %s where %s=%ld",
                    PMC_SQL_CLAIMS_FIELD_EXP_CODE,
                    PMC_SQL_TABLE_CLAIMS,
                    PMC_SQL_FIELD_ID, id );

                existingCode[0] = 0;
                sql_p->Query( buf_p );
                if( sql_p->RowCount( ) == 1 )
                {
                    sql_p->RowGet( );
                    strcpy(  &existingCode[0], sql_p->String( 0 ) );
                }
//                pmcSqlSelectStr( buf_p, &existingCode[0], 32, NIL );

                if( strlen( existingCode ) > 0 )
                {
                    sprintf( expCode, "'%s'", existingCode );
                }

                sprintf( buf_p, "update %s set %s=%s+%ld, %s=%ld, %s='%s', %s=%s where %s=%ld",
                     PMC_SQL_TABLE_CLAIMS,
                     PMC_SQL_CLAIMS_FIELD_FEE_PAID, PMC_SQL_CLAIMS_FIELD_FEE_PAID, record_p->feeeApproved,
                     PMC_SQL_CLAIMS_FIELD_FEE_PREMIUM, record_p->feeeApproved,
                     PMC_SQL_CLAIMS_FIELD_FEE_CODE_PREMIUM, buf1_p,
                     PMC_SQL_CLAIMS_FIELD_EXP_CODE, expCode,
                     PMC_SQL_FIELD_ID, id );

                mbLog( buf_p );
                if( globUpdateDatabase )
                {
//                    pmcSqlExec( buf_p );
                    sql_p->Update( buf_p );
                }
                goto exit;
            }
            else
            {
                // MAB:20010827: It looks like MSP is dividing a claim with multiple
                // units up into two responses (i.e., 5 units paid, 4 units not paid).
                // Therefore it is possible to get a response for a paid or reduced claim.
                if( oldStatus == PMC_CLAIM_STATUS_REDUCED &&
                    unitsPaid + record_p->units <= unitsClaimed )
                {
                    mbLog( "must be a second response for claim %ld\n", record_p->claimNumber );
                    addFee = TRUE;

                    if( record_p->feeeApproved == 0 )
                    {
                        // 20021123: They must be saying that they are not paying this unit.  In one case that
                        // I have seen, there was a previous record that said it paid 6 out of 7 units.  Thus,
                        // we do not want to add these units to the units paid.
                        addNewUnits = FALSE;
                    }
                }
                else
                {
                    mbDlgExclaim( "Error: claim status not SUBMITTED (oldStatus %ld id %ld)\n", oldStatus, id );
                }
            }
        }

        newUnitsPaid = unitsPaid;

        // 20021123: Check if new units should be added
        if( addNewUnits ) newUnitsPaid += record_p->units;

        if( newUnitsPaid > unitsClaimed )
        {
            mbDlgExclaim( "Error: newUnitsPaid > unitsClaimed (%d > %d)\n", newUnitsPaid, unitsClaimed );
            goto exit;
        }

        if( feeSubmitted != record_p->feeeSubmitted )
        {
             //mbDlgExclaim( "Fees do not match" );
             mbLog( "Submitted fees differ %ld != %ld (claim %ld provider %ld id %ld)\n",
                feeSubmitted, record_p->feeeSubmitted,
                record_p->claimNumber, record_p->doctorNumber, id );
        }

        if( submitDate == 0 )
        {
            mbDlgExclaim( "Submit date = 0" );
        }

        if( replyDate != 0 )
        {
            // We can have a non-zero reply date on a resubmission
            mbLog( "Reply date %ld != 0 (claim %ld provider %ld id %ld)\n", replyDate,
                record_p->claimNumber, record_p->doctorNumber, id );
        }

        mbStrClean( record_p->feeCodeApproved, buf1_p, TRUE );
        mbStrClean( record_p->expCode,         buf2_p, TRUE );
        mbStrClean( record_p->runCode,         buf3_p, TRUE );

        *buf4_p = 0;
        if( record_p->comment_p )
        {
            mbStrClean( record_p->comment_p, buf4_p, TRUE );
        }

        // Look for an "AD" code which means that we should use the PHN in the returned record
        if( strcmp( record_p->expCode, "AD" ) == 0 )
        {
            Char_t  phnDisplay[32];
            sprintf( buf_p, "%ld", record_p->phn );
            pmcFormatPhnDisplay( buf_p, NIL, phnDisplay );
            sprintf( buf_p, " MSP returned PHN: %s", phnDisplay );
            strcat( buf4_p, buf_p );
        }

        newStatus = PMC_CLAIM_STATUS_NONE;

        switch( record_p->type )
        {
            case PMC_RETURN_TYPE_PAID:
                if( record_p->feeeApproved >= feeSubmitted )
                {
                    newStatus = PMC_CLAIM_STATUS_PAID;
                }
                else
                {
                    newStatus = PMC_CLAIM_STATUS_REDUCED;
                }
                break;

            case PMC_RETURN_TYPE_VISIT:
            case PMC_RETURN_TYPE_HOSPITAL:
                newStatus = PMC_CLAIM_STATUS_REJECTED;
                break;

            case PMC_RETURN_TYPE_RECIPROCAL:
                mbDlgExclaim( "Got PMC_RETURN_TYPE_RECIPROCAL. Contact system administrator\n" );
                break;

            default:
                mbDlgDebug(( "Cannot processing unknown record type %ld.", record_p->type ));
                break;
        }

        if( newStatus != PMC_CLAIM_STATUS_NONE )
        {
            if( addFee == TRUE )
            {
                sprintf( feeString_p, "%s+%ld+%ld", PMC_SQL_CLAIMS_FIELD_FEE_PAID, record_p->feeeApproved, record_p->feeePremium );
            }
            else
            {
                sprintf( feeString_p, "%ld+%ld", record_p->feeeApproved, record_p->feeePremium );

                // Sanity Check... about to overwrite feePaid... check if anything there already
                if( feePaid != 0 )
                {
                    sprintf( buf_p, "About to overwrite feePaid %ld to %s claim %ld\n", feePaid, feeString_p, id );
                    mbLog( buf_p );
                    mbDlgExclaim( buf_p );
                }
            }

            // Sanity Check
            if(  record_p->feeePremium > 0 && premiumPaid > 0 )
            {
                sprintf( buf_p, "About to overwrite premiumPaid %ld to %ld claim %ld\n",
                    premiumPaid, record_p->feeePremium, id );
                mbLog( buf_p );
                mbDlgExclaim( buf_p );
            }

            //                              today  fee    premium feeCode expCode runCode adjCode status    unitspd
            sprintf( buf_p, "update %s set %s=%ld, %s=%s, %s=%ld, %s='%s',%s='%s',%s='%s',%s=%d,%s=%d,%s=%d,%s=%ld,%s=\"%s\" where %s=%ld",
                     PMC_SQL_TABLE_CLAIMS,
                     PMC_SQL_CLAIMS_FIELD_REPLY_DATE, today,
                     PMC_SQL_CLAIMS_FIELD_FEE_PAID, feeString_p,
                     PMC_SQL_CLAIMS_FIELD_FEE_CODE_PREMIUM, record_p->feeePremium,
                     PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, buf1_p,
                     PMC_SQL_CLAIMS_FIELD_EXP_CODE, buf2_p,
                     PMC_SQL_CLAIMS_FIELD_RUN_CODE, buf3_p,
                     PMC_SQL_CLAIMS_FIELD_ADJ_CODE, (Int8u_t)record_p->adjCode[0],
                     PMC_SQL_CLAIMS_FIELD_STATUS, newStatus,
                     PMC_SQL_CLAIMS_FIELD_REPLY_STATUS, newStatus,
                     PMC_SQL_CLAIMS_FIELD_UNITS_PAID, newUnitsPaid,
                     PMC_SQL_CLAIMS_FIELD_COMMENT_FROM_MSP, buf4_p,
                     PMC_SQL_FIELD_ID, id );

            if( globUpdateDatabase )
            {
//                pmcSqlExec( buf_p );
                sql_p->Update( buf_p );
            }
            mbLog( buf_p );
        }
    }
    else
    {
        // Do not need a comment or debugging here as function that finds ID
        // outputs complete information
    }

    // mbLog( "Got record type: %ld provider %ld, id: %ld\n", record_p->type, record_p->doctorNumber );

exit:

    mbFree( buf_p );
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( buf4_p );
    mbFree( feeString_p );
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordIdGet
//---------------------------------------------------------------------------
// Description:
//
// This function must locate the desired record in the database.  Because
// the MSP system is so brain dead, and they do not always return the same
// sequence number, we must locate the record by comparing various fields.
// Also, a record that has been resubmitted may require that a duplicate
// sequence number be used for the resubmission.
//---------------------------------------------------------------------------

#define MSP_READ_MACRO \
    returnId        = sql_p->Int32u( 0 );\
    feeSubmitted    = sql_p->Int32u( 1 );\
    submitDate      = sql_p->Int32u( 2 );\
    replyDate       = sql_p->Int32u( 3 );\
    status          = sql_p->Int32u( 4 );\
    unitsPaid       = sql_p->Int32u( 5 );\
    unitsClaimed    = sql_p->Int32u( 6 );\
    feePaid         = sql_p->Int32u( 7 );\
    premiumPaid     = sql_p->Int32u( 8 );\
    count++

Int32u_t pmcMspReturnsRecordIdGet
(
    pmcMspReturnsRecordStruct_p
                        record_p,
    Int32s_p            feeSubmitted_p,
    Int32u_p            submitDate_p,
    Int32u_p            replyDate_p,
    Int32u_p            status_p,
    Int32u_p            unitsPaid_p,
    Int32u_p            unitsClaimed_p,
    Int32s_p            feePaid_p,
    Int32s_p            premiumPaid_p,
    Int32u_p            newFlag_p,
    MbSQL              *sql_p
)
{
    static Int32u_t     previousId = 0;
    Int32u_t            returnId = 0;
    Int32u_t            providerId;
    Char_p              buf_p;
    Char_p              selectStr_p;
    Int32u_t            count = 0;
    Int32u_t            submitDate = 0;
    Int32u_t            replyDate = 0;
    Int32s_t            feeSubmitted = 0;
    Int32u_t            status = PMC_CLAIM_STATUS_NONE;
    Int32u_t            unitsPaid = 0;
    Int32u_t            unitsClaimed = 0;
    Int32s_t            feePaid = 0;
    Int32s_t            premiumPaid = 0;
    Boolean_t           premiumFlag = FALSE;
    Int32u_t            newFlag = FALSE;
    Int32u_t            attempt = 0;

    mbMalloc( buf_p,  2048 );
    mbMalloc( selectStr_p, 1024 );

    // Sanity check
    if( record_p->claimNumber == 0 || record_p->doctorNumber == 0 )
    {
        mbDlgDebug(( "Error: got returned claim with no claim number" ));
        goto exit;
    }

    // Convert the provider number back into an ID (this should be fixed in the future)
    providerId = pmcProviderNumberToId( record_p->doctorNumber );

    // Sanity Check
    if( providerId == 0 ) mbDlgDebug(( "Error: providerId = 0" ));

    record_p->serviceDate = ( 2000 + record_p->year ) * 10000 + record_p->month * 100 + record_p->day;

    //                             0  1  2  3  4  5  6  7  8
    sprintf( selectStr_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s ",
        PMC_SQL_FIELD_ID,                          // 0
        PMC_SQL_CLAIMS_FIELD_FEE_SUBMITTED,        // 1
        PMC_SQL_CLAIMS_FIELD_SUBMIT_DATE,          // 2
        PMC_SQL_CLAIMS_FIELD_REPLY_DATE,           // 3
        PMC_SQL_CLAIMS_FIELD_STATUS,               // 4
        PMC_SQL_CLAIMS_FIELD_UNITS_PAID,           // 5
        PMC_SQL_CLAIMS_FIELD_UNITS,                // 6
        PMC_SQL_CLAIMS_FIELD_FEE_PAID,             // 7
        PMC_SQL_CLAIMS_FIELD_FEE_PREMIUM,          // 8
        PMC_SQL_TABLE_CLAIMS );

    sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s='%s' and %s=%ld and %s=%ld",
        selectStr_p,
        PMC_SQL_FIELD_PROVIDER_ID, providerId,
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE, record_p->serviceDate,
        PMC_SQL_CLAIMS_FIELD_FEE_CODE, record_p->feeCodeSubmitted,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, record_p->claimNumber,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    sql_p->Query( buf_p );
    while( sql_p->RowGet( ) )
    {
        MSP_READ_MACRO;
    }

    if( count == 1 ) goto exit;

    // MAB:20021123: Output failed SQL command
    if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );

    mbLog( "Search using sequence number %ld\n", record_p->seqNumber );

    sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s='%s' and %s=%ld and %s=%ld and %s=%ld",
        selectStr_p,
        PMC_SQL_FIELD_PROVIDER_ID, providerId,
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE, record_p->serviceDate,
        PMC_SQL_CLAIMS_FIELD_FEE_CODE, record_p->feeCodeSubmitted,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, record_p->claimNumber,
        PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ, record_p->seqNumber,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    count = 0;
    sql_p->Query( buf_p );
    while( sql_p->RowGet() )
    {
        MSP_READ_MACRO;
    }

    if( count == 1 ) goto exit;

    // MAB:20021123: Output failed SQL command
    if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );

    // 20021123: I hate to inject a new search into the sequence of searches, but I
    // have encountered another case where they have responded with the wrong date.
    // Include the sequence number in the search in case the patient has multiple
    // claims with the same code but on different days.

    if( record_p->seqNumber >= 0 )
    {
      sprintf( buf_p, "%s where %s=%ld and %s='%s' and %s=%ld and %s=%ld and %s=%ld",
        selectStr_p,
        PMC_SQL_FIELD_PROVIDER_ID, providerId,
        PMC_SQL_CLAIMS_FIELD_FEE_CODE, record_p->feeCodeSubmitted,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, record_p->claimNumber,
        PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ, record_p->seqNumber,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
    }
    else
    {
      sprintf( buf_p, "%s where %s=%ld and %s='%s' and %s=%ld and %s=%ld",
        selectStr_p,
        PMC_SQL_FIELD_PROVIDER_ID, providerId,
        PMC_SQL_CLAIMS_FIELD_FEE_CODE, record_p->feeCodeSubmitted,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, record_p->claimNumber,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
    }
    count = 0;
    sql_p->Query( buf_p );
    while( sql_p->RowGet( ) )
    {
        MSP_READ_MACRO;
    }

    if( count == 1 ) goto exit;

    // MAB:20021123: Output failed SQL command
    if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );

    // More MSP ugliness.  It is possible that MSP has changed the fee code, then
    // paid a premium on the claim with the new fee code.  Look for that here
    if( record_p->feeeSubmitted == 0 )
    {
        if( record_p->seqNumber == 9 || record_p->type == PMC_RETURN_TYPE_PAID )
        {
            if( mbStrPos( record_p->expCode, "FZ" ) >= 0 )
            {
                premiumFlag = TRUE;
            }
        }
    }

    if( premiumFlag )
    {
        mbLog( "Search for a premium with a changed code\n" );
        sprintf( buf_p, "s% where %s=%ld and %s=%ld and %s='%s' and %s=%ld and %s=%ld",
            selectStr_p,
            PMC_SQL_FIELD_PROVIDER_ID, providerId,
            PMC_SQL_CLAIMS_FIELD_SERVICE_DATE, record_p->serviceDate,
            PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, record_p->feeCodeSubmitted,
            PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, record_p->claimNumber,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        count = 0;
        sql_p->Query( buf_p );
        while( sql_p->RowGet( ) )
        {
            MSP_READ_MACRO;
        }

        // MAB:20021123: Output failed SQL command
        if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );

        if( count >= 1 ) goto exit;
    }

    if( record_p->type == PMC_RETURN_TYPE_RECOVERY )
    {
        // MAB: 20011125:  There seems to be yet another possibility.  It looks like
        // MSP is changing the approved fee code in "recovery" records, then referencing
        // that fee code in a subsequent record.  Therefore, look for the fee
        // code approved rather than the submitted fee code.

        sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s='%s' and %s=%ld and %s=%ld",
            selectStr_p,
            PMC_SQL_FIELD_PROVIDER_ID,              providerId,
            PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,      record_p->serviceDate,
            PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, record_p->feeCodeSubmitted,
            PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,      record_p->claimNumber,
            PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        mbLog( "Going to look for a fee code changed by a recovery record:\n%s\n", buf_p );

        count= 0;
        sql_p->Query( buf_p );
        while( sql_p->RowGet( ) )
        {
            MSP_READ_MACRO;
        }

        // MAB:20021123: Output failed SQL command
        if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );

        if( count >= 1 ) goto exit;

        // MAB:20021116: Another possibility:  When reducing the number of units in a
        // hospital visit, they are returning the "last date" as the service date,
        // not the first date.
        mbLog( "Search for reduced units on a hospital code changed by a recovery record:\n%s\n", buf_p );

        sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s='%s' and %s=%ld and %s!=%ld and %s=%ld",
            selectStr_p,
            PMC_SQL_FIELD_PROVIDER_ID,                  providerId,
            PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LAST,     record_p->serviceDate,
            PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED,     record_p->feeCodeSubmitted,
            PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,          record_p->claimNumber,
            PMC_SQL_CLAIMS_FIELD_UNITS_PAID,            record_p->units,
            PMC_SQL_FIELD_NOT_DELETED,                  PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        count = 0;
        sql_p->Query( buf_p );
        while( sql_p->RowGet( ) )
        {
            MSP_READ_MACRO;
        }

        // MAB:20021123: Output failed SQL command
        if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );
        if( count >= 1 ) goto exit;
    }

    // Another possibility.  It looks like they are returning
    // "additional" replies which may not contain the submitted code.
    if( record_p->type == PMC_RETURN_TYPE_ADDITIONAL )
    {
        mbLog( "Search for a fee code changed by an 'additional' record\n" );

        if( record_p->seqNumber >= 0 )
        {
            sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s=%ld and %s=%ld and %s=%ld",
                selectStr_p,
                PMC_SQL_FIELD_PROVIDER_ID,              providerId,
                PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,      record_p->serviceDate,
                PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,      record_p->claimNumber,
                PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ,         record_p->seqNumber,
                PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
        }
        else
        {
            sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s=%ld and %s=%ld",
                selectStr_p,
                PMC_SQL_FIELD_PROVIDER_ID,              providerId,
                PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,      record_p->serviceDate,
                PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,      record_p->claimNumber,
                PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
        }

        count = 0;
        sql_p->Query( buf_p );
        while( sql_p->RowGet( ) )
        {
            MSP_READ_MACRO;
        }

         // MAB:20021123: Output failed SQL command
        if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );
        if( count >= 1 ) goto exit;

        // MAB:20020901: Another possibility.  They are just adding codes to the
        // claim!!!!  This is unbelievable!!!!
        if( count == 0 )
        {
            mbLog( "calling pmcMspAddToClaim\n" );
            count = pmcMspAddToClaim( providerId, record_p, sql_p );
        }
        if( count == 1 )
        {
            newFlag = TRUE;
        }
        if( count >= 1 ) goto exit;
    }

    // MAB:20020504: More extreme ugliness... they are taking a claim
    // with 'X' units, then changing the code of one of the units,
    // then sending back a claim with a different fee code and a
    // different date!!!

    mbLog( "Search for a multi-unit fee code change\n" );

    sprintf( buf_p, "%s where %s=%ld "      // provider ID
                    "and %s='%s' "          // fee code
                    "and %s='%s' "          // run code
                    "and %s=%ld  "          // claim number
                    "and %s>0 "             // some units paid
                    "and %s!=%s "           //
                    "and %s=%ld ",
        selectStr_p,
        PMC_SQL_FIELD_PROVIDER_ID,                  providerId,
        PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED,     record_p->feeCodeSubmitted,
        PMC_SQL_CLAIMS_FIELD_RUN_CODE,              record_p->runCode,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,          record_p->claimNumber,
        PMC_SQL_CLAIMS_FIELD_UNITS,
        PMC_SQL_CLAIMS_FIELD_UNITS,                 PMC_SQL_CLAIMS_FIELD_UNITS_PAID,
        PMC_SQL_FIELD_NOT_DELETED,                  PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    count = 0;
    sql_p->Query( buf_p );
    while( sql_p->RowGet( ) )
    {
        MSP_READ_MACRO;
    }

    // MAB:20021123: Output failed SQL command
    if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );
    if( count == 1 )
    {
        if( returnId != previousId )
        {
            mbDlgExclaim( "Check claim id: %ld (previous id: %ld)\n", returnId, previousId );
        }
        // Don't bother with math on the units
        record_p->type = PMC_RETURN_TYPE_CODE_CHANGE_MULTI;
        goto exit;
    }

    // MAB:20021207:They are returning a claim with the fee code changed.
    // Search with the sequence number but without the fee code.

    mbLog( "Search without fee code\n" );

    if( record_p->seqNumber >= 0 )
    {
      sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s=%ld and %s=%ld and %s=%ld",
        selectStr_p,
        PMC_SQL_FIELD_PROVIDER_ID, providerId,
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE, record_p->serviceDate,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, record_p->claimNumber,
        PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ, record_p->seqNumber,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
    }
    else
    {
      sprintf( buf_p, "%s where %s=%ld and %s=%ld and %s=%ld and %s=%ld",
        selectStr_p,
        PMC_SQL_FIELD_PROVIDER_ID, providerId,
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE, record_p->serviceDate,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, record_p->claimNumber,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
    }
    count = 0;

    sql_p->Query( buf_p );
    while( sql_p->RowGet( ) )
    {
        MSP_READ_MACRO;
    }

    if( count == 1 ) goto exit;

    // MAB:20021207: Output failed SQL command
    if( count != 1 )  mbLog( "Attempt %ld failed: %s = %ld\n", attempt++, buf_p, count );

exit:

    if( count == 1 )
    {
        mbLog( "Located record id: %ld\n", returnId );

        if( pmcReturnsRecordCheckPhn( returnId, record_p->phn, sql_p ) != MB_RET_OK )
        {
            mbDlgExclaim( "Warining: failed PHN check on claim %ld\n", record_p->claimNumber );
        }
        previousId = returnId;
   }

    if( count != 1 )
    {
        mbDlgExclaim( "Cannot find record %ld (count: %ld).  Please contact System Administrator\n",
            record_p->claimNumber, count );
        returnId = 0;
    }

    if( feeSubmitted_p ) *feeSubmitted_p = feeSubmitted;
    if( submitDate_p )   *submitDate_p   = submitDate;
    if( replyDate_p )    *replyDate_p    = replyDate;
    if( status_p )       *status_p       = status;
    if( unitsPaid_p )    *unitsPaid_p    = unitsPaid;
    if( unitsClaimed_p ) *unitsClaimed_p = unitsClaimed;
    if( feePaid_p )      *feePaid_p      = feePaid;
    if( premiumPaid_p )  *premiumPaid_p  = premiumPaid;
    if( newFlag_p )      *newFlag_p      = newFlag;

    mbFree( buf_p );
    mbFree( selectStr_p );

    return returnId;
}

//---------------------------------------------------------------------------
// Function:  pmcReturnsRecordCheckPhn
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcReturnsRecordCheckPhn
(
    Int32u_t    claimId,
    Int32u_t    claimPhn,
    MbSQL      *sql_p
)
{
    Char_p      buf_p;
    Char_p      result_p;
    Int32u_t    result = 0;
    Int32s_t    returnCode = MB_RET_ERR;

    mbMalloc( buf_p, 1024 );
    mbMalloc( result_p, 64 );

    if( claimPhn == 0 )
    {
        returnCode = MB_RET_OK;
        goto exit;
    }

    sprintf( buf_p, "select %s.%s from %s,%s where %s.%s=%ld and %s.%s=%s.%s",
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_PHN,
        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_TABLE_PATIENTS,

        PMC_SQL_TABLE_CLAIMS,   PMC_SQL_FIELD_ID, claimId,
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_ID, PMC_SQL_TABLE_CLAIMS, PMC_SQL_FIELD_PATIENT_ID );

    sql_p->Query( buf_p );

    if( sql_p->RowCount() != 1 ) goto exit;
    sql_p->RowGet();
    strncpy( result_p, sql_p->String( 0 ), 63 );

    result = atol( result_p );

    if( result != claimPhn ) goto exit;

    returnCode = MB_RET_OK;
exit:
    mbFree( buf_p );
    mbFree( result_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspAddToClaim
//---------------------------------------------------------------------------
// Description:
//
// The jerks at MSP are just adding new claims to the returns file!!!
// Add the claim here if possible. The tables are already locked when this
// function is called.
//---------------------------------------------------------------------------

Int32u_t    pmcMspAddToClaim
(
    Int32u_t                        providerId,
    pmcMspReturnsRecordStruct_p     record_p,
    MbSQL                          *sql_p
)
{
    Char_p      buf_p = NIL;
    Char_p      buf1_p = NIL;
    Char_p      buf2_p = NIL;
    Char_p      buf3_p = NIL;
    Int32u_t    claimHeaderId = 0;
    Int32u_t    returnCount = 0;
    Int32u_t    claimIds1[10];
    Int32u_t    claimIds2[10];
    Int32u_t    claimHeaderIds[10];
    Int32u_t    i, j;
    Int32s_t    found = FALSE;
    Int32u_t    existingClaimId = 0;
    Int32u_t    claimIndex = 0;
    Int32u_t    patientId;
    Int32u_t    newClaimId;
    Int32u_t    location;

    mbMalloc( buf_p, 4096 );
    mbMalloc( buf1_p, 128 );
    mbMalloc( buf2_p, 128 );
    mbMalloc( buf3_p, 128 );

    // Try to identify the goup of claims this new claim will belong to
    sprintf( buf_p, "select %s,%s from %s where %s=%ld and %s=%ld and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID,
        PMC_SQL_TABLE_CLAIMS,

        PMC_SQL_FIELD_PROVIDER_ID,          providerId,
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,  record_p->claimNumber,

        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    i = 0;

    sql_p->Query( buf_p );
    while( sql_p->RowGet( ) )
    {
        claimIds1[i]      = sql_p->Int32u( 0 );
        claimHeaderIds[i] = sql_p->Int32u( 1 );
        i++;
        if( i > 6 ) break;
    }

    if( i == 0 )
    {
        mbDlgDebug(( "could not identify claim header id" ));
        goto exit;
    }

    // Cross check the retrieved calim header ids
    for( j = 0 ; j < i ; j++ )
    {
        if( claimHeaderIds[0] != claimHeaderIds[j] )
        {
            mbDlgDebug(( "could not identify claim header id" ));
            goto exit;
        }
    }

    claimHeaderId = claimHeaderIds[0];

    // First see if there are any claims available
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_0,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_1,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_2,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_3,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_4,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_5,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_6,

        PMC_SQL_TABLE_CLAIM_HEADERS,

        PMC_SQL_FIELD_ID, claimHeaderId );

    i = 0;

    sql_p->Query( buf_p );

    while( sql_p->RowGet( ) )
    {
        claimIds2[0]    = sql_p->Int32u( 0 );
        claimIds2[1]    = sql_p->Int32u( 1 );
        claimIds2[2]    = sql_p->Int32u( 2 );
        claimIds2[3]    = sql_p->Int32u( 3 );
        claimIds2[4]    = sql_p->Int32u( 4 );
        claimIds2[5]    = sql_p->Int32u( 5 );
        claimIds2[6]    = sql_p->Int32u( 6 );
        i++;
    }

    // Sanity check
    if( i != 1 ) goto exit;

    for( found = FALSE, i = 0 ; i < 7 ; i++ )
    {
        if( claimIds2[i] == 0 )
        {
            claimIndex = i;
            found = TRUE;
            break;
        }
        else
        {
            existingClaimId = claimIds2[i];
        }
    }

    if( !found ) goto exit;
    if( existingClaimId == 0 ) goto exit;

    // Must get some of the details from other claims
    sprintf( buf_p, "select %s,%s from %s where %s = %ld",
        PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_CLAIMS_FIELD_SERVICE_LOCATION,
        PMC_SQL_TABLE_CLAIMS,
        PMC_SQL_FIELD_ID, existingClaimId );

    i = 0;

    sql_p->Query( buf_p );

    while( sql_p->RowGet( ) )
    {
        patientId  = sql_p->Int32u( 0 );
        location   = sql_p->Int32u( 1 );
        i++;
    }

    // Sanity check
    if( i != 1 ) goto exit;

    newClaimId = pmcSqlRecordCreate( PMC_SQL_TABLE_CLAIMS, sql_p );

    mbStrClean( record_p->feeCodeApproved, buf1_p, TRUE );
    mbStrClean( record_p->expCode,         buf2_p, TRUE );
    mbStrClean( record_p->runCode,         buf3_p, TRUE );

    // Update the claim details
    //                             0      1      2      3      4       5     6       7       8       9     10     11      12    13                                                     15      16    17     18
    sprintf( buf_p, "update %s set %s=%ld,%s=%ld,%s=%ld,%s=%ld,%s=%ld,%s=%ld,%s='%s',%s='%s',%s=%ld,%s=%ld,"
                                  "%s=%ld,%s=%ld,%s=%ld,%s='%s',%s='Claim added automatically by MSP response',"
                                                                      "%s=%ld,%s=%ld,%s=%ld,%s=%ld "
                    " where %s=%ld",
        PMC_SQL_TABLE_CLAIMS,

        PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,   // 0
        PMC_SQL_FIELD_PATIENT_ID,               patientId,                              // 1
        PMC_SQL_CLAIMS_FIELD_STATUS,            PMC_CLAIM_STATUS_PAID,                  // 2
        PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID,   claimHeaderId,                          // 3
        PMC_SQL_CLAIMS_FIELD_UNITS,             record_p->units,                        // 4
        PMC_SQL_CLAIMS_FIELD_UNITS_PAID,        record_p->units,                        // 5
        PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, buf1_p,                                 // 6
        PMC_SQL_CLAIMS_FIELD_RUN_CODE,          buf3_p,                                 // 7
        PMC_SQL_CLAIMS_FIELD_CLAIM_INDEX,       claimIndex,                             // 8
        PMC_SQL_CLAIMS_FIELD_REPLY_DATE,        mbToday( ),                             // 9
        PMC_SQL_CLAIMS_FIELD_FEE_PAID,          record_p->feeeApproved,                  // 10
        PMC_SQL_CLAIMS_FIELD_SERVICE_LOCATION,  location,                               // 11
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,      record_p->serviceDate,                  // 12
        PMC_SQL_CLAIMS_FIELD_EXP_CODE,          buf2_p,                                 // 13
        PMC_SQL_CLAIMS_FIELD_COMMENT_FROM_MSP,                                          // 14
        PMC_SQL_FIELD_PROVIDER_ID,              providerId,                             // 15
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,      record_p->claimNumber,                  // 16
        PMC_SQL_CLAIMS_FIELD_REPLY_STATUS,      PMC_CLAIM_STATUS_PAID,                  // 17
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_EARLIEST, record_p->serviceDate,              // 18
        PMC_SQL_FIELD_ID, newClaimId );

    mbLog( "Added new claim: '%s'\n", buf_p );

//    pmcSqlExec( buf_p );
    sql_p->Update( buf_p );

    // Update claim header
    sprintf( buf_p, "update %s set %s%ld=%ld where %s=%ld",
        PMC_SQL_TABLE_CLAIM_HEADERS,
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID, claimIndex, newClaimId,
        PMC_SQL_FIELD_ID, claimHeaderId );
//    pmcSqlExec( buf_p );
    sql_p->Update( buf_p );

    returnCount = 1;

exit:
    mbFree( buf_p );
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );

    return returnCount;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsFilePrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------


Int32s_t pmcMspReturnsFilePrint
(
    Char_p                          fileName_p
)
{
    qHead_t                         recordQueueHead;
    qHead_p                         record_q;
    qHead_t                         providerQueueHead;
    qHead_p                         provider_q;
    Int32s_t                        returnCode = FALSE;
    pmcProviderList_p               provider_p;
    pmcMspRunCodeList_p             entry_p;

    mbLog( "Print MSP returns file %s\n", fileName_p );

    // Initialize queue of records and providers
    record_q = qInitialize( &recordQueueHead );
    provider_q = qInitialize( &providerQueueHead );

    // Read all the records from the file
    if( pmcMspReturnsFileRecordsGet( fileName_p, record_q ) == FALSE )
    {
        goto exit;
    }

    // Determine the unique providers
    pmcMspReturnsFileProvidersGet( record_q, provider_q );

    // Look for missing run codes
    pmcMspRunCodesCheck( provider_q, TRUE );

    // Print returns output for each provider
    for( ; ; )
    {
        if( qEmpty( provider_q ) ) break;
        provider_p = (pmcProviderList_p)qRemoveFirst( provider_q );
        pmcMspReturnsFileProviderPrint( provider_p, record_q, fileName_p );

        while( !qEmpty( provider_p->runCode_q ) )
        {
            entry_p = (pmcMspRunCodeList_p)qRemoveFirst( provider_p->runCode_q );
            mbFree( entry_p );
        }
        mbFree( provider_p->description_p );
        mbFree( provider_p );
    }

    // Sanity Check
    if( record_q->size != 0 )
    {
        mbDlgDebug(( "%d unprocessed claims records!",  record_q->size ));
    }

    returnCode = TRUE;

exit:
    pmcMspReturnsQueueClean( record_q );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsFileContentsDialog
//---------------------------------------------------------------------------
// Description:
//
// 20040110: Show what providers and run codes are in the file.
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsFileContentsDialog
(
    Char_p                          fileName_p
)
{
    qHead_t                         recordQueueHead;
    qHead_p                         record_q;
    qHead_t                         providerQueueHead;
    qHead_p                         provider_q;
    Int32s_t                        returnCode = FALSE;
    pmcProviderList_p               provider_p;
    Char_p                          buf_p;
    Char_p                          buf1_p;
    pmcMspRunCodeList_p             entry_p;

    // Initialize queue of records and providers
    record_q = qInitialize( &recordQueueHead );
    provider_q = qInitialize( &providerQueueHead );
    mbMalloc( buf_p, 2048 );
    mbMalloc( buf1_p, 512 );

    // Read all the records from the file
    if( pmcMspReturnsFileRecordsGet( fileName_p, record_q ) == FALSE )
    {
        mbDlgInfo( "The file %s is not a valid RETURNS file.\n", fileName_p );
        goto exit;
    }

    // Determine the unique providers
    pmcMspReturnsFileProvidersGet( record_q, provider_q );

    // Check the run codes against the database to look for missing
    // run codes.
    pmcMspRunCodesCheck( provider_q, FALSE );

#if 0
    {
        // This code block is just a test of the run code to id conversion
        Char_t      runCode[3];
        Char_t      resultCode[3];
        Ints_t      i, j;
        Int32u_t    val;
        Char_p      temp_p;

        for( i = 0 ; i < 26 ; i++ )
        {
            for( j = 0 ; j < 26 ; j++ )
            {
                runCode[0] = (Char_t)('A' + i);
                runCode[1] = (Char_t)('A' + j);
                runCode[2] = 0;
                val = pmcRunCodeToInt( runCode );
                temp_p = pmcIntToRunCode( val, resultCode );
                if( strcmp( runCode, resultCode ) != 0 )
                {
                    mbDlgExclaim( "Run Code ID error %s %s\n", runCode, resultCode );
                }
                // mbLog( "Run Code: %s value: %03ld ('%s')\n", runCode, val, resultCode );
            }
        }
    }
#endif

    sprintf( buf_p, "Summary of MSP run codes in %s:\n\n", fileName_p );

    for( ; ; )
    {
        if( qEmpty( provider_q ) ) break;
        provider_p = (pmcProviderList_p)qRemoveFirst( provider_q );

        qWalk( entry_p, provider_p->runCode_q, pmcMspRunCodeList_p )
        {
            sprintf( buf1_p, "Provider:   %s      Run Code:   %s\t\tRecords: %ld\n",
                provider_p->description_p, entry_p->runCode, entry_p->count );
             strcat( buf_p, buf1_p );
        }
        while( !qEmpty( provider_p->runCode_q ) )
        {
            entry_p = (pmcMspRunCodeList_p)qRemoveFirst( provider_p->runCode_q );
            mbFree( entry_p );
        }

        mbFree( provider_p->description_p );
        mbFree( provider_p );
    }

    mbDlgInfo( buf_p );

    returnCode = TRUE;
exit:
    pmcMspReturnsQueueClean( record_q );
    mbFree( buf_p );
    mbFree( buf1_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspRunCodesCheck
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspRunCodesCheck
(
    qHead_p                 q_p,
    Boolean_t               ignoreFlag
)
{
    pmcProviderList_p       provider_p;
    pmcMspRunCodeList_p     entry_p;
    Int32u_t                runCode;
    Int32u_t                count;
    Int32u_t                id;
    Char_p                  buf_p = NIL;
    Int32u_t                runCodeInt[PMC_MSP_RUN_CODE_WRAP];
    Int32u_t                i, j;
    Int32u_t                codesRead;
    Int32u_t                ignoreBefore;
    Int32u_t                maxId, minId;
    Boolean_t               found;
    Char_t                  mspRunCode[4];
    MbSQL                   sql;

    mbMalloc( buf_p, 1024 );

    pmcSuspendPollInc( );

    // Must lock the run code table
    sprintf( buf_p, "lock tables %s write", PMC_SQL_TABLE_RUN_CODES );
    sql.Update( buf_p );

    // This first loop adds into the database any newly detected run codes
    qWalk( provider_p, q_p, pmcProviderList_p )
    {
        qWalk( entry_p, provider_p->runCode_q, pmcMspRunCodeList_p )
        {
            if( entry_p->count < 20 )
            {
                // Since MSP does not return the actual run code in the RETURNS file,
                // but rather with each record... and since the RETURNS file may contain
                // adjusted records from previous runs, we must "guess" which run code
                // is contained in this file.  We do this by only considering
                // run codes with more than 20 appearances.  What a brain-dead system
                mbLog( "Skipping run code '%s' count: %ld\n", entry_p->runCode, entry_p->count );
                continue;
            }

            // Convert the run code to an integer for easier processing
            runCode = pmcRunCodeToInt( &entry_p->runCode[0] );

            //mbDlgInfo( "Check provider: %ld runCode %s Count %ld int: %lu\n",
            //    provider_p->id, entry_p->runCode, entry_p->count, pmcRunCodeToInt( &entry_p->runCode[0] ) );

            // Check to see if this run code is in the database
            sprintf( buf_p, "select %s from %s where %s=%ld and %s=%ld and %s=%ld",
                PMC_SQL_FIELD_ID,
                PMC_SQL_TABLE_RUN_CODES,
                PMC_SQL_FIELD_RUN_CODE,     runCode,
                PMC_SQL_FIELD_PROVIDER_ID,  provider_p->id,
                PMC_SQL_FIELD_NOT_DELETED,  PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

            sql.Query( buf_p );
            count = sql.RowCount( );
            if( count == 0 )
            {
                // This run code is not in the database... add it
                if( ( id = pmcSqlRecordCreate( PMC_SQL_TABLE_RUN_CODES, &sql ) ) == 0 )
                {
                    mbDlgExclaim( "Failed to create run code record\n" );
                }
                else
                {
                    // Created the record... now upadate it
                    sprintf( buf_p, "update %s set %s=%ld,%s=%ld,%s=%ld where %s=%ld",
                        PMC_SQL_TABLE_RUN_CODES,
                        PMC_SQL_FIELD_RUN_CODE,     runCode,
                        PMC_SQL_FIELD_PROVIDER_ID,  provider_p->id,
                        PMC_SQL_FIELD_NOT_DELETED,  PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
                        PMC_SQL_FIELD_ID,           id );

                    if( sql.Update( buf_p ) == FALSE )
                    {
                        mbDlgExclaim( "Failed to update run code record %ld\n", id );
                    }
                }
            }
            else if( count == 1 )
            {
                // The run code is already in the database.  Can ignore for now
                // mbDlgInfo( "MSP run code %s already in database\n", entry_p->runCode );
            }
            else
            {
                // The count is greater than 1.  This should never happen
                mbDlgExclaim( "Error: MSP run code %s count: %d\n"
                              "Please contact system administrator." , entry_p->runCode, entry_p->count );
            }
        }
    } // End of loop adding new run codes

    // Next, get all run codes for the provider and check for missing codes

    if( ignoreFlag )
    {
        ignoreBefore = pmcRunCodeToInt( PMC_MSP_RUN_CODE_IGNORE_BEFORE );
    }
    else
    {
        ignoreBefore = pmcRunCodeToInt( "WA" );
    }

    qWalk( provider_p, q_p, pmcProviderList_p )
    {
        sprintf( buf_p, "select %s from %s where %s=%ld and %s=%ld",
            PMC_SQL_FIELD_RUN_CODE,
            PMC_SQL_TABLE_RUN_CODES,
            PMC_SQL_FIELD_PROVIDER_ID,  provider_p->id,
            PMC_SQL_FIELD_NOT_DELETED,  PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        sql.Query( buf_p );

        i = 0;
        while( sql.RowGet( ) )
        {
            runCodeInt[i] = sql.Int32u( 0 );
            i++;
            if( i >= PMC_MSP_RUN_CODE_WRAP )
            {
                mbDlgExclaim( "Error: too many run codes\n" );
                break;
            }
        }

        codesRead = i;

        // Don't bother checking if less than 2 codes
        if( codesRead < 2 ) continue;

        // Now determine the max ID and min read back
        for( i = 0, maxId = 0, minId = 0xFFFFFFFF ; i < codesRead ; i++ )
        {
            if( runCodeInt[i] > maxId ) maxId = runCodeInt[i];
            if( runCodeInt[i] < minId ) minId = runCodeInt[i];
        }

        for( i = maxId - 1 ; i >= minId ; i-- )
        {
            found = FALSE;
            for( j = 0 ; j < codesRead ; j++ )
            {
                if( runCodeInt[j] == i )
                {
                    found = TRUE;
                    break;
                }
            }
            if( found ) continue;

            if( i < ignoreBefore ) continue;

            // Are missing a run code
            mbDlgExclaim( "Missing MSP Run Code: %s\n\n"
                          "Please contact system administrator.\n", pmcIntToRunCode( i, mspRunCode ) );
        }
    }

    sprintf( buf_p, "unlock tables" );
    sql.Update( buf_p );
    
    pmcSuspendPollDec( );
    mbFree( buf_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsQueueClean
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsQueueClean
(
    qHead_p     queue_p
)
{
    pmcMspReturnsRecordStruct_p record_p;
    for( ; ; )
    {
        if( qEmpty( queue_p ) ) break;
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( queue_p );
        if( record_p->comment_p ) mbFree( record_p->comment_p );
        if( record_p->str_p ) mbFree( record_p->str_p );
        mbFree( record_p );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsFileRecordsGet
//---------------------------------------------------------------------------
// Description:
//
// This function reads all the records from the RETURNS file.
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsFileRecordsGet
(
    Char_p          fileName_p,
    qHead_p         record_q
)
{
    FILE                           *fp;
    Int32s_t                        returnCode = FALSE;
    Char_p                          buf_p;
    Char_p                          buf2_p;
    pmcMspReturnsRecordStruct_p     record_p;
    Int32u_t                        bytesRead;
    Int32u_t                        recordCount;
    TCursor                         origCursor;
    Int8u_t                         byte1, byte2, byte14;
    Int32u_t                        type;

    mbMalloc( buf_p, 256 );
    mbMalloc( buf2_p, 256 );

    if( ( fp = fopen( fileName_p, "r" ) ) == NIL )
    {
        mbDlgExclaim( "Error opening file %s\n", fileName_p );
        goto exit;
    }

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    // The first loop will extract each providers record information
    recordCount = 0;
    for( ; ; )
    {
        // Note that instead of getting back CR-LF, I am only getting back LF.
        // Thus must read 1 less that the actual record length
        bytesRead = fread(  buf_p, 1, PMC_RETURNS_RECORD_LENGTH - 1, fp );
        if( bytesRead != PMC_RETURNS_RECORD_LENGTH - 1 ) break;

        recordCount++;

        mbCalloc( record_p, sizeof( pmcMspReturnsRecordStruct_t ) );
        qInsertLast( record_q, record_p );

        // Attempt to parse the records
        byte1 = (Int8u_t)*(buf_p);
        byte2 = (Int8u_t)*(buf_p+1);
        byte14 = (Int8u_t)*(buf_p+13);

        type = PMC_RETURN_TYPE_UNKNOWN;
        switch( byte1 )
        {
            case '0':
            case '1':
            case '9':
                // Paid, total, or message
                switch( byte14 )
                {
                    case 'P':
                        type = PMC_RETURN_TYPE_PAID;
                        break;
                    case 'T':
                        type = PMC_RETURN_TYPE_TOTAL;
                        break;
                    case 'M':
                        type = PMC_RETURN_TYPE_MESSAGE;
                        break;
                    default:
                        break;
                }
                break;

            case '6':
                // Paid, total, message, or comment.  This detection algorithm
                // is not bulletproof!
                switch( byte14 )
                {
                    case 'P':
                        type = PMC_RETURN_TYPE_PAID;
                        break;
                    case 'T':
                        type = PMC_RETURN_TYPE_TOTAL;
                        break;
                    case 'M':
                        type = PMC_RETURN_TYPE_MESSAGE;
                        break;
                    default:
                        // Assume this is a returned comment - sanity check
                        if( byte2 == '0' )
                        {
                            type = PMC_RETURN_TYPE_COMMENT;
                        }
                        break;
                }
                break;

            case '8':
                // Paid, total, message, or reciprocal
                switch( byte14 )
                {
                    case 'P':
                        type = PMC_RETURN_TYPE_PAID;
                        break;
                    case 'T':
                        type = PMC_RETURN_TYPE_TOTAL;
                        break;
                    case 'M':
                        type = PMC_RETURN_TYPE_TOTAL;
                        break;
                    default:
                        // Assume this is a returned comment - sanity check
                        if( byte2 == '9' )
                        {
                            type = PMC_RETURN_TYPE_RECIPROCAL;
                        }
                        break;
                }
                break;

            case '5':
               // Vist, hospital
               switch( byte2 )
               {
                    case '0':
                        type = PMC_RETURN_TYPE_VISIT;
                        break;

                    case '7':
                    case '8':
                        type = PMC_RETURN_TYPE_HOSPITAL;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        // Now look at the resolved record type
        switch( type )
        {
            case PMC_RETURN_TYPE_PAID:
                pmcMspReturnsRecordPaidGet( buf_p, record_p );
                break;
            case PMC_RETURN_TYPE_TOTAL:
                pmcMspReturnsRecordTotalGet( buf_p, record_p );
                break;
            case PMC_RETURN_TYPE_MESSAGE:
                pmcMspReturnsRecordMessageGet( buf_p, record_p );
                break;
            case PMC_RETURN_TYPE_VISIT:
                pmcMspReturnsRecordVisitGet( buf_p, record_p );
                break;
            case PMC_RETURN_TYPE_HOSPITAL:
                pmcMspReturnsRecordHospitalGet( buf_p, record_p );
                break;
            case PMC_RETURN_TYPE_COMMENT:
                pmcMspReturnsRecordCommentGet( buf_p, record_p );
                break;
            case PMC_RETURN_TYPE_RECIPROCAL:
                pmcMspReturnsRecordReciprocalGet( buf_p, record_p );
                break;
            case PMC_RETURN_TYPE_UNKNOWN:
            default:
                record_p->type = PMC_RETURN_TYPE_UNKNOWN;
                mbDlgExclaim( "Encountered unknown record type: %d\n", type );
                goto exit;
        }
    }
    returnCode = TRUE;

exit:

    Screen->Cursor = origCursor;

    // Close the file if it is open
    if( fp ) fclose( fp );

    mbFree( buf_p );
    mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordPaidGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsRecordPaidGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
)
{
    Char_p                          buf1_p;

    mbMalloc( buf1_p, 128 );

    record_p->type = PMC_RETURN_TYPE_PAID;

    // Get Provider Number
    PMC_GET_FIELD( buf_p, buf1_p, 2, 5 );
    record_p->doctorNumber = atol( buf1_p );

    // Get Clinic Number
    PMC_GET_FIELD( buf_p, buf1_p, 6, 8 );
    record_p->clinicNumber = atol( buf1_p );

    // Get Name
    PMC_GET_FIELD( buf_p, record_p->name, 16, 34 );

    // Get PHN
    PMC_GET_FIELD( buf_p, buf1_p, 36, 44 );
    record_p->phn = atol( buf1_p );

    // Get Claim Number
    PMC_GET_FIELD( buf_p, buf1_p, 46, 50 );
    record_p->claimNumber = atol( buf1_p );

    // Get Seq Number
    PMC_GET_FIELD( buf_p, buf1_p, 51, 51 );
    if( *buf1_p == '?' )
    {
        // 20040102: The printed MSP file does not include the sequence number
        // so indicate this via a '?' character in the RETURNS file.
        record_p->seqNumber = -1;
    }
    else
    {
        record_p->seqNumber = atol( buf1_p );
    }

    // Get Day of service
    PMC_GET_FIELD( buf_p, buf1_p, 53, 54 );
    record_p->day = (Int8u_t)atol( buf1_p );

    // Get Month of service
    PMC_GET_FIELD( buf_p, buf1_p, 56, 57 );
    record_p->month = (Int8u_t)atol( buf1_p );

    // Get Year of service
    PMC_GET_FIELD( buf_p, buf1_p, 59, 59 );
    record_p->year = (Int16u_t)atoi( buf1_p );

    // The year is returned as a 1 digit number
    {
        Int32s_t    today, temp;

        today = (Int32s_t)mbToday( );
        today /= 10000;
        today -= 2000;

        for( ; ; )
        {
            temp = today;
            while( temp > 10 ) temp -= 10;

            if( temp == record_p->year )
            {
                record_p->year = (Int16u_t)today;
                break;
            }
            today--;
        }
    }

    // Get Units
    PMC_GET_FIELD( buf_p, buf1_p, 61, 62 );
    record_p->units = (Int16u_t)atoi( buf1_p );

    // Get Fee Code Approved
    PMC_GET_FIELD( buf_p, record_p->feeCodeApproved, 77, 80 );

    // Get Fee Code Submitted
    PMC_GET_FIELD( buf_p, record_p->feeCodeSubmitted, 64, 67 );

    // Fee Submitted
    PMC_GET_FIELD( buf_p, buf1_p, 69, 75 );
    record_p->feeeSubmitted = pmcMspReturnsFeeGet( buf1_p );

    // Fee Approved
    PMC_GET_FIELD( buf_p, buf1_p, 82, 88 );
    record_p->feeeApproved = pmcMspReturnsFeeGet( buf1_p );

    // Exp code
    PMC_GET_FIELD( buf_p, record_p->expCode, 90, 91 );

    // Adj code
    PMC_GET_FIELD( buf_p, record_p->adjCode, 94, 94 );

    // Run code
    PMC_GET_FIELD( buf_p, record_p->runCode, 96, 97 );

    // Form Type
    PMC_GET_FIELD( buf_p, buf1_p, 98, 98 );
    record_p->formType = (Int8u_t)atol( buf1_p );

    // Premium
    PMC_GET_FIELD( buf_p, buf1_p, 99, 105 );
    record_p->feeePremium = pmcMspReturnsFeeGet( buf1_p );

    // Debugging
    if( record_p->feeePremium != 0 )
    {
        mbLog( "Got premium %7.2f\n", (float)record_p->feeePremium/100.0 );
    }

    if( record_p->adjCode[0] == PMC_MSP_ADJUST_RECOVERY )
    {
        // mbDlgDebug(( "Found a recovery record '%s' %d\n", record_p->name, record_p->claimNumber ));
        // Recovery claim number will be two greater than the claim
        // that it refers to
        record_p->claimNumber -= 2;

        if( record_p->claimNumber >= ( PMC_CLAIM_NUMBER_MIN + 2 ) )
        {
            // For some reason, they are sending back electronic "recovery" records
            // marked as paper claims. So check the claim number, and if it greater
            // than 10000, assume that it refers to an electronic claim.

            //mbDlgDebug(( "Electronic claim number: %d\n", record_p->claimNumber ));
            record_p->formType = PMC_RETURN_FORM_TYPE_ELECTRONIC_9;
            record_p->type = PMC_RETURN_TYPE_RECOVERY;
        }
    }

    if( record_p->adjCode[0] == PMC_MSP_ADJUST_ADDITIONAL )
    {
        // mbDlgDebug(( "Found an additional record '%s' %d\n", record_p->name, record_p->claimNumber ));

        // Recovery claim number will be one greater than the claim that it refers to
        record_p->claimNumber -= 1;

        if( record_p->claimNumber >= ( PMC_CLAIM_NUMBER_MIN + 1 ) )
        {
            // For some reason, they are sending back electronic "recovery" records
            // marked as paper claims. So check the claim number, and if it greater
            // than 10000, assume that it refers to an electronic claim.

            //mbDlgDebug(( "Electronic claim number: %d\n", record_p->claimNumber ));
            record_p->formType = PMC_RETURN_FORM_TYPE_ELECTRONIC_9;
            record_p->type = PMC_RETURN_TYPE_ADDITIONAL;
        }
    }
    mbFree( buf1_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordTotalGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsRecordTotalGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
)
{
    Char_p                          buf1_p;

    mbMalloc( buf1_p, 128 );

    record_p->type = PMC_RETURN_TYPE_TOTAL;

    // Get Provider Number
    PMC_GET_FIELD( buf_p, buf1_p, 2, 5 );
    record_p->doctorNumber = atol( buf1_p );

    // Get Clinic Number
    PMC_GET_FIELD( buf_p, buf1_p, 6, 8 );
    record_p->clinicNumber = atol( buf1_p );

    // Fee Submitted
    PMC_GET_FIELD( buf_p, buf1_p, 65, 75 );
    record_p->feeeSubmitted = pmcMspReturnsFeeGet( buf1_p );

    // Fee Approved
    PMC_GET_FIELD( buf_p, buf1_p, 78, 88 );
    record_p->feeeApproved = pmcMspReturnsFeeGet( buf1_p );

    // Get Total type - put into name field
    PMC_GET_FIELD( buf_p, record_p->name, 54, 63 );

    mbFree( buf1_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordMessageGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsRecordMessageGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
)
{
    Char_p                          buf1_p;
    Ints_t                          len;

    mbMalloc( buf1_p, 128 );

    record_p->type = PMC_RETURN_TYPE_MESSAGE;

    // Get Provider Number
    PMC_GET_FIELD( buf_p, buf1_p, 2, 5 );
    record_p->doctorNumber = atol( buf1_p );

    // Get Clinic Number
    PMC_GET_FIELD( buf_p, buf1_p, 6, 8 );
    record_p->clinicNumber = atol( buf1_p );

    // Get Claim Number
    PMC_GET_FIELD( buf_p, buf1_p, 9, 13 );
    record_p->claimNumber = atol( buf1_p );

    // Get Actual message
    PMC_GET_FIELD( buf_p, buf1_p, 15, 94 );
    if( ( len = strlen( buf1_p ) ) > 0 )
    {
        mbCalloc( record_p->str_p, len + 1 );
        strcpy( record_p->str_p, buf1_p );
    }

    mbLog( "Message record '%s' claim %ld", record_p->str_p, record_p->claimNumber );

    mbFree( buf1_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordVisitGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsRecordVisitGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
)
{
    Char_p                          buf1_p;

    mbMalloc( buf1_p, 128 );

    record_p->type = PMC_RETURN_TYPE_VISIT;

    // Get Provider Number
    PMC_GET_FIELD( buf_p, buf1_p, 3, 6 );
    record_p->doctorNumber = atol( buf1_p );

    // Get Clinic Number
    PMC_GET_FIELD( buf_p, buf1_p, 96, 98 );
    record_p->clinicNumber = atol( buf1_p );

    // Get Claim Number
    PMC_GET_FIELD( buf_p, buf1_p, 7, 11 );
    record_p->claimNumber = atol( buf1_p );

    // Get Sequence Number
    PMC_GET_FIELD( buf_p, buf1_p, 12, 12 );
    if( *buf1_p == '?' )
    {
        record_p->seqNumber = -1;
    }
    else
    {
        record_p->seqNumber = atol( buf1_p );
    }

    // Get Name
    PMC_GET_FIELD( buf_p, record_p->name, 27, 51 );

    // Get PHN
    PMC_GET_FIELD( buf_p, buf1_p, 13, 21 );
    record_p->phn = atol( buf1_p );

    // Get Dob
    PMC_GET_FIELD( buf_p, record_p->dob, 22, 25 );

    // Get Gender
    PMC_GET_FIELD( buf_p, record_p->gender, 26, 26 );

    // Get ICD code
    PMC_GET_FIELD( buf_p, record_p->icdCode, 52, 54 );

    // Get Referring Dr number
    PMC_GET_FIELD( buf_p, buf1_p, 55, 58 );
    record_p->referringNumber = atol( buf1_p );

    // Get Day
    PMC_GET_FIELD( buf_p, buf1_p, 59, 60 );
    record_p->day = (Int8u_t)atol( buf1_p );

    // Get Month
    PMC_GET_FIELD( buf_p, buf1_p, 61, 62 );
    record_p->month = (Int8u_t)atol( buf1_p );

    // Get Year
    PMC_GET_FIELD( buf_p, buf1_p, 63, 64 );
    record_p->year = (Int16u_t)atol( buf1_p );

    // Get Units
    PMC_GET_FIELD( buf_p, buf1_p, 65, 66 );
    record_p->units = (Int16u_t)atoi( buf1_p );

    // Get Location Code
    PMC_GET_FIELD( buf_p, record_p->location, 52, 54 );

    // Get Fee Code Submitted
    PMC_GET_FIELD( buf_p, record_p->feeCodeSubmitted, 68, 71 );

    // Fee Submitted
    PMC_GET_FIELD( buf_p, buf1_p, 72, 77 );
    record_p->feeeSubmitted = pmcMspReturnsFeeGet( buf1_p );

    // Form Type
    PMC_GET_FIELD( buf_p, buf1_p, 79, 79 );
    record_p->formType = (Int8u_t)atol( buf1_p );

    // Exp code
    PMC_GET_FIELD( buf_p, record_p->expCode, 90, 91 );

    // Run code
    PMC_GET_FIELD( buf_p, record_p->runCode, 92, 93 );

    mbFree( buf1_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordHospitalGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsRecordHospitalGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
)
{
    Char_p                          buf1_p;

    mbMalloc( buf1_p, 128 );

    record_p->type = PMC_RETURN_TYPE_HOSPITAL;

    // Get Provider Number
    PMC_GET_FIELD( buf_p, buf1_p, 3, 6 );
    record_p->doctorNumber = atol( buf1_p );

    // Get Clinic Number
    PMC_GET_FIELD( buf_p, buf1_p, 96, 98 );
    record_p->clinicNumber = atol( buf1_p );

    // Get Claim Number
    PMC_GET_FIELD( buf_p, buf1_p, 7, 11 );
    record_p->claimNumber = atol( buf1_p );

    // Get Sequence Number
    PMC_GET_FIELD( buf_p, buf1_p, 12, 12 );
    if( *buf1_p == '?' )
    {
        record_p->seqNumber = -1; 
    }
    else
    {
        record_p->seqNumber = atol( buf1_p );
    }

    // Get Name
    PMC_GET_FIELD( buf_p, record_p->name, 27, 51 );

    // Get PHN
    PMC_GET_FIELD( buf_p, buf1_p, 13, 21 );
    record_p->phn = atol( buf1_p );

    // Get Dob
    PMC_GET_FIELD( buf_p, record_p->dob, 22, 25 );

    // Get Gender
    PMC_GET_FIELD( buf_p, record_p->gender, 26, 26 );

    // Get ICD code
    PMC_GET_FIELD( buf_p, record_p->icdCode, 52, 54 );

    // Get Referring Dr number
    PMC_GET_FIELD( buf_p, buf1_p, 55, 58 );
    record_p->referringNumber = atol( buf1_p );

    // Get Day
    PMC_GET_FIELD( buf_p, buf1_p, 59, 60 );
    record_p->day = (Int8u_t)atol( buf1_p );

    // Get Month
    PMC_GET_FIELD( buf_p, buf1_p, 61, 62 );
    record_p->month = (Int8u_t)atol( buf1_p );

    // Get Year
    PMC_GET_FIELD( buf_p, buf1_p, 63, 64 );
    record_p->year = (Int16u_t)atol( buf1_p );

    // Get Day
    PMC_GET_FIELD( buf_p, buf1_p, 65, 66 );
    record_p->lastDay = (Int8u_t)atol( buf1_p );

    // Get Month
    PMC_GET_FIELD( buf_p, buf1_p, 67, 68 );
    record_p->lastMonth = (Int8u_t)atol( buf1_p );

    // Get Year
    PMC_GET_FIELD( buf_p, buf1_p, 69, 70 );
    record_p->lastYear = (Int16u_t)atol( buf1_p );

    // Get Units
    PMC_GET_FIELD( buf_p, buf1_p, 71, 72 );
    record_p->units = (Int16u_t)atoi( buf1_p );

    // Get Fee Code Submitted
    PMC_GET_FIELD( buf_p, record_p->feeCodeSubmitted, 73, 76 );

    // Fee Submitted
    PMC_GET_FIELD( buf_p, buf1_p, 77, 82 );
    record_p->feeeSubmitted = pmcMspReturnsFeeGet( buf1_p );
    
    // Form Type
    PMC_GET_FIELD( buf_p, buf1_p, 84, 84 );
    record_p->formType = (Int8u_t)atol( buf1_p );

    // Exp code
    PMC_GET_FIELD( buf_p, record_p->expCode, 94, 95 );

    // Run code
    PMC_GET_FIELD( buf_p, record_p->runCode, 92, 93 );

    mbFree( buf1_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordCommentGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsRecordCommentGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
)
{
    Char_p                          buf1_p;
    Ints_t                          len;

    mbMalloc( buf1_p, 128 );

    record_p->type = PMC_RETURN_TYPE_COMMENT;

    // Get Provider Number
    PMC_GET_FIELD( buf_p, buf1_p, 3, 6 );
    record_p->doctorNumber = atol( buf1_p );

    // Get Clinic Number
    PMC_GET_FIELD( buf_p, buf1_p, 96, 98 );
    record_p->clinicNumber = atol( buf1_p );

    // Get Claim Number
    PMC_GET_FIELD( buf_p, buf1_p, 7, 11 );
    record_p->claimNumber = atol( buf1_p );

    // Get Sequence Number
    PMC_GET_FIELD( buf_p, buf1_p, 12, 12 );
    record_p->claimNumber = atol( buf1_p );

    // Get PHN
    PMC_GET_FIELD( buf_p, buf1_p, 13, 21 );
    record_p->phn = atol( buf1_p );

     // Get Actual message
    PMC_GET_FIELD( buf_p, buf1_p, 22, 95 );
    if( ( len = strlen( buf1_p ) ) > 0 )
    {
        mbCalloc( record_p->str_p, len + 1 );
        mbStrClean( buf1_p, record_p->str_p, TRUE );
    }

    mbFree( buf1_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordReciprocalGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsRecordReciprocalGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
)
{
    Char_p                          buf1_p;

    mbMalloc( buf1_p, 128 );

    record_p->type = PMC_RETURN_TYPE_RECIPROCAL;

    // Get Provider Number
    PMC_GET_FIELD( buf_p, buf1_p, 3, 6 );
    record_p->doctorNumber = atol( buf1_p );

    // Get Clinic Number
    PMC_GET_FIELD( buf_p, buf1_p, 96, 98 );
    record_p->clinicNumber = atol( buf1_p );

    // Get Claim Number
    PMC_GET_FIELD( buf_p, buf1_p, 7, 11 );
    record_p->claimNumber = atol( buf1_p );

    // Get Sequence Number
    PMC_GET_FIELD( buf_p, buf1_p, 12, 12 );
    record_p->claimNumber = atol( buf1_p );

    PMC_GET_FIELD( buf_p, record_p->prov, 22, 23 );

    // Last Name
    PMC_GET_FIELD( buf_p, record_p->name, 24, 41 );

    // First Name
    PMC_GET_FIELD( buf_p, record_p->firstName, 42, 50 );

    // Out of province health number
    PMC_GET_FIELD( buf_p, record_p->healthNum, 52, 63 );

    mbFree( buf1_p );
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: pmcMspReturnsFileProvidersGet
//---------------------------------------------------------------------------
// Description:
//
// This function identifies all the unique provider numbers in the
// the records file
//---------------------------------------------------------------------------

Int32s_t        pmcMspReturnsFileProvidersGet
(
    qHead_p     record_q,
    qHead_p     provider_q
)
{
    pmcMspReturnsRecordStruct_p     record_p;
    pmcProviderList_p               provider_p;
    bool                            found;
    Char_t                          description[256];

    qWalk( record_p, record_q, pmcMspReturnsRecordStruct_p )
    {
        if( record_p->doctorNumber == 0 ) continue;

        found = FALSE;

#if 0
        if( strlen( record_p->runCode ) && record_p->type == PMC_RETURN_TYPE_PAID )
        {
            mbLog( "Check claim runCode %s adj: %d formType: %d\n",
                record_p->runCode, record_p->adjCode[0], record_p->formType );
        }
#endif

        qWalk( provider_p, provider_q, pmcProviderList_p )
        {
            if( record_p->doctorNumber == provider_p->providerNumber )
            {
                // 20040110: Determine run code for eack provider in the file.
                // In reality, it looks like a returns file can contain claims
                // from multiple runs.
                if( strlen( record_p->runCode ) != 0 )
                {
                    pmcMspRunCodeAdd( provider_p->runCode_q, record_p->runCode );
                }
                found = TRUE;
                break;
            }
        }

        if( !found )
        {
            mbCalloc( provider_p, sizeof( pmcProviderList_t ) );
            provider_p->runCode_q = qInitialize( &provider_p->runCodeQueue );

            qInsertLast( provider_q, provider_p );
            provider_p->providerNumber = record_p->doctorNumber;

            // 20040110: Get additional provider information
            provider_p->id = pmcProviderNumberToId( provider_p->providerNumber );
            pmcProviderDescGet( provider_p->id, description );
            mbMallocStr( provider_p->description_p, description );
        }
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: pmcMspRunCodeAdd
//---------------------------------------------------------------------------
// Description:
//
// 20040110: This function ckecks tke list of run codes for the specified
// code, and if not found, the code is added to the list.
//---------------------------------------------------------------------------

Int32s_t        pmcMspRunCodeAdd
(
    qHead_p                 q_p,
    Char_p                  runCode_p
)
{
    pmcMspRunCodeList_p     entry_p;
    Boolean_t               found = FALSE;
    Int32s_t                returnCode = FALSE;
    Ints_t                  len = 0;

    // Sanity checks
    if( q_p == NIL || runCode_p == NIL ) goto exit;

    len = strlen( runCode_p );
    if( len <= 0 || len >= 4 ) goto exit;

    qWalk( entry_p, q_p, pmcMspRunCodeList_p )
    {
        //mbLog( "compare '%s' to '%s'\n", entry_p->runCode, runCode_p );
        if( strcmp( entry_p->runCode, runCode_p ) == 0 )
        {
            entry_p->count++;
            found = TRUE;
            break;
        }
    }

    if( !found )
    {
         mbCalloc( entry_p, sizeof( pmcMspRunCodeList_t ) );
         strcpy( entry_p->runCode, runCode_p );
         //mbLog( "added new record '%s' \n", entry_p->runCode );
         entry_p->count = 1;
         qInsertLast( q_p, entry_p );
    }
    returnCode = TRUE;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcMspReturnsFileProviderPrint
//---------------------------------------------------------------------------
// Description:
//
// This function prints the returned records for the specified provider
//---------------------------------------------------------------------------

Int32s_t    pmcMspReturnsFileProviderPrint
(
    pmcProviderList_p               provider_p,
    qHead_p                         record_q,
    Char_p                          fileName_p
)
{
    pmcMspReturnsRecordStruct_p     record_p;
    pmcMspRunCodeList_p             entry_p;
    Char_p                          tempName_p;
    Char_p                          spoolFileName_p;
    Char_p                          flagFileName_p;
    Char_p                          buf1_p, buf2_p;
    Int32s_t                        returnCode = FALSE;
    FILE                           *fp = NIL;
    MbDateTime                      dateTime;
    qHead_t                         tempQueue;
    qHead_p                         temp_q;
    Int32u_t                        size, i;
    Int32s_t                        claimed;
    Int32s_t                        paid;
    Int32s_t                        paidTotal = 0;

    mbMalloc( tempName_p, 128 );
    mbMalloc( spoolFileName_p, 128 );
    mbMalloc( flagFileName_p, 128 );
    mbMalloc( buf1_p, 128 );
    mbMalloc( buf2_p, 128 );

    temp_q = qInitialize( &tempQueue );

    sprintf( tempName_p, "%s.html", pmcMakeFileName( NIL, spoolFileName_p ) );
    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, tempName_p );

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Error opening claims print file '%s'", spoolFileName_p ));
        goto exit;
    }

    fprintf( fp, "<HTML><HEAD><Title>MSP Returns</TITLE></HEAD><BODY>\n" );
    fprintf( fp, "<H2><CENTER>MSP Processed Claims Report</CENTER></H1><HR>\n" );

    fprintf( fp, "<PRE WIDTH = 80>\n" );

    fprintf( fp, "Provider Name:   %s\n",  provider_p->description_p );
    fprintf( fp, "Provider Number: %ld\n", provider_p->providerNumber );

    fprintf( fp, "\n" );

    qWalk( entry_p,  provider_p->runCode_q, pmcMspRunCodeList_p )
    {
    fprintf( fp, "Run Code:        %s  \t(Records:  %ld)\n", entry_p->runCode, entry_p->count );
    }
    fprintf( fp, "\n" );

    fprintf( fp, "File name:       %s\n", fileName_p );

    dateTime.SetDateTime( mbToday(), mbTime() );
    fprintf( fp, "Printed:         %s ", dateTime.HM_TimeString( ) );
    fprintf( fp, "%s\n",  dateTime.MDY_DateString( ) );

    fprintf( fp, "\n" );
    fprintf( fp, "                                 Service Date        Fee          Fee\n" );
    fprintf( fp, "   # Claim   PHN       Name          DD/MM/YY    Claimed Code    Paid Code Exp\n" );
    fprintf( fp, "%s\n\n", PMC_STRING_DASHED_LINE );

    // Print messages
    size = record_q->size;
    for( i = 0; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if(    record_p->doctorNumber == provider_p->providerNumber
            && record_p->type == PMC_RETURN_TYPE_MESSAGE  )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following %ld MESSAGE record%s returned:\n\n", temp_q->size,
            ( temp_q->size == 1 ) ? " was" : "s were"  );
        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        pmcMspReturnsQueueClean( temp_q );
    }
    paidTotal += paid;

    // Print paper records
    size = record_q->size;
    for( i = 0; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if( record_p->doctorNumber == provider_p->providerNumber &&
            record_p->formType == PMC_RETURN_FORM_TYPE_PAPER )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following %ld paper claim%s processed:\n\n", temp_q->size,
            ( temp_q->size == 1 ) ? " was" : "s were"  );

        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        fprintf( fp, "\n                                     TOTALS:%12.2f %12.2f\n", (float)claimed/100.0, (float)paid/100.0 );
        pmcMspReturnsQueueClean( temp_q );
    }
    paidTotal += paid;

    // Print recovery records
    size = record_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if( record_p->doctorNumber == provider_p->providerNumber &&
            record_p->type         == PMC_RETURN_TYPE_RECOVERY )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following %ld previously paid claim%s reduced:\n\n", temp_q->size,
            ( temp_q->size == 1 ) ? " was" : "s were"  );

        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        fprintf( fp, "\n                                     TOTALS:%12.2f %12.2f\n", (float)claimed/100.0, (float)paid/100.0 );
        pmcMspReturnsQueueClean( temp_q );
    }
    paidTotal+= paid;

    // Next print rejected claims
    size = record_q->size;
    for( i = 0; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if(   record_p->doctorNumber == provider_p->providerNumber &&
            ( record_p->type == PMC_RETURN_TYPE_VISIT    ||
              record_p->type == PMC_RETURN_TYPE_HOSPITAL ||
              record_p->type == PMC_RETURN_TYPE_COMMENT  ||
              record_p->type == PMC_RETURN_TYPE_RECIPROCAL ) )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following %ld electronic claim%s returned (rejected):\n\n",  temp_q->size,
            ( temp_q->size == 1 ) ? " was" : "s were"  );
        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        fprintf( fp, "\n                                     TOTALS:%12.2f %12.2f\n", (float)claimed/100.0, (float)paid/100.0 );
        pmcMspReturnsQueueClean( temp_q );
    }
    paidTotal += paid;

    // Next print reduced claims
    size = record_q->size;
    for( i = 0; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if(    record_p->doctorNumber == provider_p->providerNumber
            && record_p->type == PMC_RETURN_TYPE_PAID
            && record_p->feeeApproved < record_p->feeeSubmitted )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following %ld electronic claim%s paid at a reduced rate:\n\n", temp_q->size,
            ( temp_q->size == 1 ) ? " was" : "s were" );
        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        fprintf( fp, "\n                                     TOTALS:%12.2f %12.2f\n", (float)claimed/100.0, (float)paid/100.0 );
        pmcMspReturnsQueueClean( temp_q );
    }
    paidTotal += paid;

    // Print 'additional' records
    size = record_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if( record_p->doctorNumber == provider_p->providerNumber &&
            record_p->type         == PMC_RETURN_TYPE_ADDITIONAL )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following %ld 'additional' claim%s returned:\n\n", temp_q->size,
            ( temp_q->size == 1 ) ? " was" : "s were"  );

        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        fprintf( fp, "\n                                     TOTALS:%12.2f %12.2f\n", (float)claimed/100.0, (float)paid/100.0 );
        pmcMspReturnsQueueClean( temp_q );
    }
    paidTotal += paid;

    // Next print paid claims
    size = record_q->size;
    for( i = 0; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if(    record_p->doctorNumber == provider_p->providerNumber
            && record_p->type == PMC_RETURN_TYPE_PAID  )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following %ld electronic claim%s paid at the submitted rate:\n\n", temp_q->size,
         ( temp_q->size == 1 ) ? " was" : "s were"  );
        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        fprintf( fp, "\n                                     TOTALS:%12.2f %12.2f\n", (float)claimed/100.0, (float)paid/100.0 );
        pmcMspReturnsQueueClean( temp_q );
    }
    paidTotal += paid;

    // Next print totals
    size = record_q->size;
    for( i = 0; i < size ; i++ )
    {
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( record_q );
        if(    record_p->doctorNumber == provider_p->providerNumber
            && record_p->type == PMC_RETURN_TYPE_TOTAL  )
        {
            qInsertLast( temp_q, record_p );
        }
        else
        {
            qInsertLast( record_q, record_p );
        }
    }

    claimed = 0;
    paid = 0;
    if( temp_q->size )
    {
        fprintf( fp, "\nThe following TOTALS record%s returned:\n\n", ( temp_q->size == 1 ) ? " was" : "s were"  );
        pmcMspReturnsRecordsPrint( fp, temp_q, &claimed, &paid );
        pmcMspReturnsQueueClean( temp_q );
    }

    fprintf( fp, "</PRE><HR>\n\n" );
    PMC_REPORT_FOOTER( fp );
    // End of document
    fprintf( fp, "</BODY></HTML>\n" );

    returnCode = TRUE;

    mbLog( "Total paid (total records): %9.2f  (individual records): %9.2f\n",
            (float)paid/100.0, (float)paidTotal/100.0 );

    if( paid != paidTotal )
    {
        mbDlgInfo( "Total paid total records (%9.2f) does not match individual records (%9.2f)\n",
            (float)paid/100.0, (float)paidTotal/100.0 );
    }

    mbLog( "Printed processed claims report '%s' provider %ld\n",
        fileName_p, provider_p->providerNumber );

    if( strlen( provider_p->description_p ) )
    {
        mbDlgInfo( "Processed claims report for %s sent to printer.", provider_p->description_p );
    }
    else
    {
        mbDlgInfo( "Processed claims report sent to printer." );
    }

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
    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordsPrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordsPrint
(
    FILE                       *fp,
    qHead_p                     queue_p,
    Int32s_p                    claimed_p,
    Int32s_p                    paid_p
)
{
    pmcMspReturnsRecordStruct_p record_p;
    Int32u_t                    count;

    for( count = 1 ; ; count++ )
    {
        if( qEmpty( queue_p ) ) break;
        record_p = (pmcMspReturnsRecordStruct_p)qRemoveFirst( queue_p );

        switch( record_p->type )
        {
            case PMC_RETURN_TYPE_PAID:
            case PMC_RETURN_TYPE_RECOVERY:
            case PMC_RETURN_TYPE_ADDITIONAL:
                pmcMspReturnsRecordPaidPrint( fp, count, record_p, claimed_p, paid_p );
                break;
            case PMC_RETURN_TYPE_TOTAL:
                pmcMspReturnsRecordTotalPrint( fp, count, record_p, paid_p );
                break;
            case PMC_RETURN_TYPE_MESSAGE:
                pmcMspReturnsRecordMessagePrint( fp, count, record_p );
                break;
            case PMC_RETURN_TYPE_VISIT:
                pmcMspReturnsRecordVisitPrint( fp, count, record_p, claimed_p, paid_p );
                break;
            case PMC_RETURN_TYPE_HOSPITAL:
                pmcMspReturnsRecordHospitalPrint( fp, count, record_p, claimed_p, paid_p );
                break;
            case PMC_RETURN_TYPE_COMMENT:
                pmcMspReturnsRecordCommentPrint( fp, count, record_p );
                break;
            case PMC_RETURN_TYPE_RECIPROCAL:
                pmcMspReturnsRecordReciprocalPrint( fp, count, record_p );
                break;
            case PMC_RETURN_TYPE_UNKNOWN:
            default:
                mbDlgExclaim( "Found unknown record type: %d\n", record_p->type );
                break;
        }

        // Free the records memory
        if( record_p->str_p ) mbFree( record_p->str_p );
        if( record_p->comment_p ) mbFree( record_p->comment_p );
        mbFree( record_p );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordPaidPrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordPaidPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    claimed_p,
    Int32s_p                    paid_p
)
{
    Char_p      buf1_p, buf2_p;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );

    fprintf( fp, "%4d ", count );

    if( claimed_p ) *claimed_p += record_p->feeeSubmitted;
    if( paid_p )    *paid_p    += record_p->feeeApproved;
    if( paid_p )    *paid_p    += record_p->feeePremium;

    // Claim number
    buf1_p[0] = 0;
    if( record_p->claimNumber )
    {
        if( record_p->seqNumber >= 0 )
        {
            sprintf( buf1_p, "%05d-%1d", record_p->claimNumber, record_p->seqNumber );
        }
        else
        {
            sprintf( buf1_p, "%05d-?", record_p->claimNumber );
        }
    }
    fprintf( fp, "%-7.7s ", buf1_p );

    // PHN
    buf1_p[0] = 0;
    if( record_p->phn )
    {
        sprintf( buf1_p, "%09ld", record_p->phn );
    }
    fprintf( fp, "%-9.9s ", buf1_p );

    fprintf( fp, "%-13.13s ", mbStrClean( record_p->name, buf1_p, TRUE ) );

    fprintf( fp, "%02d/%02d/%02d ", record_p->day, record_p->month, record_p->year   );

    fprintf( fp, "%2d ", record_p->units );

    fprintf( fp, "%7.2f ", (float)record_p->feeeSubmitted/ 100.0 );
    fprintf( fp, "%4.4s ", pmcFeeCodeFormatDisplay( record_p->feeCodeSubmitted, buf1_p ) );

    if( record_p->feeePremium > 0 )
    {
        fprintf( fp, "%7.2f+", (float)( record_p->feeeApproved + record_p->feeePremium )/ 100.0 );
    }
    else
    {
        fprintf( fp, "%7.2f ", (float)record_p->feeeApproved/ 100.0 );
    }

    if( strcmp( record_p->feeCodeSubmitted, record_p->feeCodeApproved ) != 0 )
    {
        fprintf( fp, "%4.4s ", pmcFeeCodeFormatDisplay( record_p->feeCodeApproved, buf1_p ) );
    }
    else
    {
        fprintf( fp, "     " );
    }

    fprintf( fp, "%2.2s", mbStrClean( record_p->expCode, buf1_p, TRUE ) );
    fprintf( fp, "%1.1s", record_p->adjCode );

    fprintf( fp, "\n" );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordMessagePrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordMessagePrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p
)
{
    Char_p      buf1_p, buf2_p;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );

    fprintf( fp, "%4d ", count );

    // Claim number
    buf1_p[0] = 0;
    if( record_p->claimNumber > 0 && record_p->claimNumber < PMC_CLAIM_NUMBER_MAX )
    {
        sprintf( buf1_p, "%05d", record_p->claimNumber );
        fprintf( fp, "%-7.7s ", buf1_p );
    }
    if( record_p->str_p )
    {
        mbStrClean( record_p->str_p, buf1_p, TRUE );
        fprintf( fp, "%-65.65s", buf1_p );
    }
    fprintf( fp, "\n" );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordTotalPrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordTotalPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    paid_p
)
{
    Char_p      buf1_p, buf2_p;

    // Sanity Check 
    if( record_p == NIL ) return FALSE;

    if( paid_p ) *paid_p  += record_p->feeeApproved;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );

    fprintf( fp, "%4d", count );

    if( mbStrPos( record_p->name, PMC_MSP_TOTAL_RECORD_STRING_TOTAL ) >= 0 )
    {
        fprintf( fp, "   Total:            Submitted: %9.2f    Approved:  %9.2f ",
            (float)record_p->feeeSubmitted/100.0, (float)record_p->feeeApproved/100.0 );
    }
    else if( mbStrPos( record_p->name, PMC_MSP_TOTAL_RECORD_STRING_SMA_DUES ) >= 0 )
    {
        fprintf( fp, "   Deduction:        SMA Dues:                         -%9.2f ",  (float)record_p->feeeApproved/100.0 );
    }
    else if( mbStrPos( record_p->name, PMC_MSP_TOTAL_RECORD_STRING_PAPER_CLAIMS ) >= 0 )
    {
        fprintf( fp, "   Deduction:        Paper claims processing fee:      -%9.2f ",  (float)record_p->feeeApproved/100.0 );
    }
    else
    {
        fprintf( fp, "   Unknown TOTAL record type " );
    }

    fprintf( fp, "\n" );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return TRUE;
}
//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordVisitPrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordVisitPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    claimed_p,
    Int32s_p                    paid_p
)
{
    Char_p      buf1_p, buf2_p;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );

    if( claimed_p ) *claimed_p += record_p->feeeSubmitted;
    if( paid_p )    *paid_p    += record_p->feeeApproved;

    fprintf( fp, "%4d ", count );

    // Claim number
    buf1_p[0] = 0;
    if( record_p->claimNumber )
    {
        if( record_p->seqNumber >= 0 )
        {
            sprintf( buf1_p, "%05d-%1d", record_p->claimNumber, record_p->seqNumber );
        }
        else
        {
            sprintf( buf1_p, "%05d-?", record_p->claimNumber );
        }
    }
    fprintf( fp, "%-7.7s ", buf1_p );

    // PHN
    buf1_p[0] = 0;
    if( record_p->phn )
    {
        sprintf( buf1_p, "%09ld", record_p->phn );
    }
    fprintf( fp, "%-9.9s ", buf1_p );

    fprintf( fp, "%-13.13s ", mbStrClean( record_p->name, buf1_p, TRUE ) );

    fprintf( fp, "%02d/%02d/%02d ", record_p->day, record_p->month, record_p->year   );

    fprintf( fp, "%2d ", record_p->units );

    fprintf( fp, "%7.2f ", (float)record_p->feeeSubmitted/ 100.0 );
    fprintf( fp, "%4.4s ", pmcFeeCodeFormatDisplay( record_p->feeCodeSubmitted, buf1_p ) );

    if( record_p->feeeApproved )
    {
        fprintf( fp, "%7.2f ", (float)record_p->feeeApproved/ 100.0 );
    }
    else
    {
        fprintf( fp, " Reject " );
    }

    if( strcmp( record_p->feeCodeSubmitted, record_p->feeCodeApproved ) != 0 )
    {
        fprintf( fp, "%4.4s ", pmcFeeCodeFormatDisplay( record_p->feeCodeApproved, buf1_p ) );
    }
    else
    {
        fprintf( fp, "     " );
    }

    fprintf( fp, "%2.2s ", mbStrClean( record_p->expCode, buf1_p, TRUE ) );
    fprintf( fp, "\n" );

//    fprintf( fp,  "%s\n", PMC_STRING_DASHED_LINE );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return TRUE;
}
//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordHospitalPrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordHospitalPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    claimed_p,
    Int32s_p                    paid_p
)
{
    Char_p      buf1_p, buf2_p;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );

    if( claimed_p ) *claimed_p += record_p->feeeSubmitted;
    if( paid_p )    *paid_p    += record_p->feeeApproved;

    fprintf( fp, "%4d ", count );

    // Claim number
    buf1_p[0] = 0;
    if( record_p->claimNumber )
    {
        if( record_p->seqNumber >= 0 )
        {
            sprintf( buf1_p, "%05d-%1d", record_p->claimNumber, record_p->seqNumber );
        }
        else
        {
            sprintf( buf1_p, "%05d-?", record_p->claimNumber );
        }
    }
    fprintf( fp, "%-7.7s ", buf1_p );

    // PHN
    buf1_p[0] = 0;
    if( record_p->phn )
    {
        sprintf( buf1_p, "%09ld", record_p->phn );
    }
    fprintf( fp, "%9.9s ", buf1_p );

    fprintf( fp, "%-13.13s ", mbStrClean( record_p->name, buf1_p, TRUE ) );

    fprintf( fp, "%02d/%02d/%02d ", record_p->day, record_p->month, record_p->year   );

    fprintf( fp, "%2d ", record_p->units );

    fprintf( fp, "%7.2f ", (float)record_p->feeeSubmitted/ 100.0 );
    fprintf( fp, "%4.4s ", pmcFeeCodeFormatDisplay( record_p->feeCodeSubmitted, buf1_p ) );

    if( record_p->feeeApproved )
    {
        fprintf( fp, "%7.2f ", (float)record_p->feeeApproved/ 100.0 );
    }
    else
    {
        fprintf( fp, " Reject " );
    }


    if( strcmp( record_p->feeCodeSubmitted, record_p->feeCodeApproved ) != 0 )
    {
        fprintf( fp, "%4.4s ", pmcFeeCodeFormatDisplay( record_p->feeCodeApproved, buf1_p ) );
    }
    else
    {
        fprintf( fp, "     " );
    }

    fprintf( fp, "%2.2s ", record_p->expCode );
    fprintf( fp, "\n" );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return TRUE;
}
//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordCommentPrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordCommentPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p
)
{
    Char_p      buf1_p, buf2_p;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );

    fprintf( fp, "%4d ", count );

    // Claim number
    buf1_p[0] = 0;
    if( record_p->claimNumber )
    {
        if( record_p->seqNumber >= 0 )
        {
            sprintf( buf1_p, "%05d-%1d", record_p->claimNumber, record_p->seqNumber );
        }
        else
        {
            sprintf( buf1_p, "%05d-?", record_p->claimNumber );
        }
    }
    fprintf( fp, "%-7.7s ", buf1_p );

    // PHN
    buf1_p[0] = 0;
    if( record_p->phn )
    {
        sprintf( buf1_p, "%09ld", record_p->phn );
    }
    fprintf( fp, "%-9.9s ", buf1_p );

    if( record_p->str_p )
    {
        mbStrClean( record_p->str_p, buf1_p, TRUE );
        fprintf( fp, "%-54.54s", buf1_p );
        if( strlen( buf1_p ) > 55 )
        {
            fprintf( fp, "\n                      %s", buf1_p + 55 );
        }
    }
    fprintf( fp, "\n" );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return TRUE;
}
//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordReciprocalPrint
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcMspReturnsRecordReciprocalPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p
)
{
    Char_p      buf1_p, buf2_p;

    mbCalloc( buf1_p, 128 );
    mbCalloc( buf2_p, 128 );

    fprintf( fp, "%4d ", count );

    // Claim number
    buf1_p[0] = 0;
    if( record_p->claimNumber )
    {
        sprintf( buf1_p, "%05d", record_p->claimNumber );
    }
    fprintf( fp, "%-7.7s ", buf1_p );

    fprintf( fp, "%12.12s ", record_p->healthNum );
    fprintf( fp, "%2.2s ", record_p->prov );

    fprintf( fp, "%-13.13s ", record_p->name );
    fprintf( fp, "%-8.8s ", record_p->firstName );

    fprintf( fp, "\n" );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsBugFix1
//---------------------------------------------------------------------------
// Description:
//
// This function corrects for a very specific bug in the RETURNS file in
// which the wrong date is being returned.  An example is listed herein.
// The number of possible manifestations of this bug are not known.
//
// Truncated name phn etc.dd/mm/yy
// ---------------------- -- -- -
// 1968564P WAL 62 105640 01 07 1 05 036H 001145{ 036H 001145{       TZ9
// 1968564P WAL 62 105641 02 07 1 05 036H 001145{ 036H 001145{       TZ9
// 1968564P WAL 62 105642 01 07 1 04 036H 000916{ 036H 000000{ HH    TZ9
// 1968564P WAL 62 105643 01 07 1 04 036H 000916{ 036H 000000{ HH    TZ9
//                         ^
//                          Wrong day
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsBugFix1
(
    qHead_p         record_q
)
{
    pmcMspReturnsRecordStruct_p     rec1_p, rec2_p, rec3_p;
    pmcMspReturnsRecordStruct_p     recArray_p[32];
    Int32u_t                        index;
    Int32u_t                        half, i;
    bool                            fixed;

    for( rec1_p  = (pmcMspReturnsRecordStruct_p)( record_q->linkage.flink );
         rec1_p != (pmcMspReturnsRecordStruct_p)( record_q );
         rec1_p  = (pmcMspReturnsRecordStruct_p)( rec1_p->linkage.flink ) )
    {
        for( rec2_p  = (pmcMspReturnsRecordStruct_p)( record_q->linkage.flink );
             rec2_p != (pmcMspReturnsRecordStruct_p)( record_q );
             rec2_p  = (pmcMspReturnsRecordStruct_p)( rec2_p->linkage.flink ) )
        {
            /* Ensure we are not comparing the entry to itself */
            if( rec1_p == rec2_p ) continue;

            if( rec1_p->doctorNumber != rec2_p->doctorNumber ) continue;
            if( rec1_p->claimNumber  != rec2_p->claimNumber  ) continue;
            if( rec1_p->day          != rec2_p->day          ) continue;
            if( rec1_p->month        != rec2_p->month        ) continue;
            if( rec1_p->year         != rec2_p->year         ) continue;
            if( rec1_p->feeeApproved != rec2_p->feeeApproved  ) continue;
            if( strcmp( rec1_p->healthNum,        rec2_p->healthNum        ) != 0 ) continue;
            if( strcmp( rec1_p->feeCodeSubmitted, rec2_p->feeCodeSubmitted ) != 0 ) continue;
            if( strcmp( rec1_p->feeCodeApproved,  rec2_p->feeCodeApproved  ) != 0 ) continue;

            mbDlgExclaim( "Duplicate RETURNs record detected for claim number %ld.\n"
                              "Attempting to fix (check log file.)", rec2_p->claimNumber );

            fixed = FALSE;

            mbLog( "Duplicate response detected '%s'\n", rec1_p->name );

            index = 0;

            for( rec3_p  = (pmcMspReturnsRecordStruct_p)( record_q->linkage.flink );
                 rec3_p != (pmcMspReturnsRecordStruct_p)( record_q );
                 rec3_p  = (pmcMspReturnsRecordStruct_p)( rec3_p->linkage.flink ) )
            {
                if( rec3_p->doctorNumber != rec2_p->doctorNumber ) continue;
                if( rec3_p->claimNumber  != rec2_p->claimNumber  ) continue;
                if( strcmp( rec3_p->feeCodeSubmitted, rec2_p->feeCodeSubmitted ) != 0 ) continue;

                mbLog( "Duplicate group entry: index: %d day %d month %d year %d\n",
                    index, rec3_p->day, rec3_p->month, rec3_p->year );

                recArray_p[index++] = rec3_p;
            }

            mbLog( "Detected %d claims in duplicate group\n", index );
            if( index & 0x00000001 )
            {
                mbDlgExclaim( "Odd number of claims in duplicate group. Don't know how to fix\n" );
            }
            else
            {
                half = index / 2;

                for( i = 0 ; i < half ; i++ )
                {
                    if( recArray_p[i + half]->day != recArray_p[i]->day )
                    {
                        mbLog( "Changing date %d -> %d index %d\n",
                            recArray_p[i + half]->day,
                            recArray_p[i]->day,
                            i + half );

                        recArray_p[i + half]->day = recArray_p[i]->day;
                    }
                }
            }

            if( fixed )
            {
                mbDlgExclaim( "Fixed incorrect RETURNS record for claim %ld", rec2_p->claimNumber );
            }
            else
            {
                mbDlgExclaim( "Could not fix incorrect RETURNS record for claim %ld", rec2_p->claimNumber );
            }
        }
    }
    return TRUE;
 }

//---------------------------------------------------------------------------
// Function:  pmcMspReturnsRecordVisitGet
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcMspReturnsFeeGet
(
    Char_p                          buf_p
)
{
    Int32s_t    returnValue = 0;

    if( buf_p == NIL ) goto exit;

    returnValue = atol( buf_p );

    // mbLog( "pmcMspReturnsFeeGet '%s' returned %lu\n", buf_p, returnValue );
exit:
    return returnValue;
}




