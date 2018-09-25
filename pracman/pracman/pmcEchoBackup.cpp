//---------------------------------------------------------------------------
// File:    pmcEchoBackup.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    February 15, 2003
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#include <dos.h>
#include <dir.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcEchoImportForm.h"
#include "pmcUtilsSql.h"
#include "pmcEchoBackup.h"

//---------------------------------------------------------------------------
// Function: pmcEchoBackupDeleteFiles
//---------------------------------------------------------------------------
// Delete files from a backup CD
//---------------------------------------------------------------------------

Int32s_t pmcEchoBackupDeleteFiles
(
    Int32u_t    echoId
)
{
#if 0 // MAB:20061210: Disable this feature
    Char_p          buf1_p;
    Char_p          buf2_p;
    Char_p          buf3_p;


    mbMalloc( buf1_p, 1024 );
    mbMalloc( buf2_p, 1024 );
    mbMalloc( buf3_p, 1024 );

    // 20031221: Format the path the way it is formatted in pmcEchoFileListGet... I don't
    // like this because it is an assumption as to where the files are located on the CD

    sprintf( buf1_p, "%s", pmcCfg[CFG_CD_BURNER].str_p );
    mbStrRemoveSlashTrailing( buf1_p );
    sprintf( buf2_p, "\\%05lu", echoId );
    strcat( buf1_p, buf2_p );

    strcpy( buf2_p, buf1_p );
    sprintf( buf3_p, "-bad-%06ld-%06ld", mbToday(), mbTime() );
    strcat( buf2_p, buf3_p );

    RenameFile( buf1_p, buf2_p );

#if 0
    deleteFile_q = qInitialize( &deleteFileQueue );
    mbFileListGet( buf1_p, "*.*", deleteFile_q, TRUE );
    mbLog( "Deleteting the following echo files:\n" );
    mbFileListLog( deleteFile_q );
    mbFileListDelete( deleteFile_q, TRUE );
    mbFileListFree( deleteFile_q );
    RemoveDirectory( buf1_p );
#endif

    mbFree( buf3_p );
    mbFree( buf1_p );
    mbFree( buf2_p );
#endif
    return TRUE;
}

//---------------------------------------------------------------------------
// Function: pmcEchoBackupDeleteEntry
//---------------------------------------------------------------------------
// This function deletes an entry from the backup database when the backup
// is determined to be bad.
//---------------------------------------------------------------------------

Int32s_t pmcEchoBackupDeleteEntry
(
    Int32u_t    echoId,
    Int32u_t    cdId
)
{
#if 0
    Char_p      buf_p = NIL;
    Int32u_t    id;
    Int32u_t    count;
    Int32s_t    returnCode = FALSE;

    mbCalloc( buf_p, 2048 );

    // First determine the id of the database entry
    sprintf( buf_p, "select %s from %s where %s=%ld and %s=%ld and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_ECHO_BACKUPS,
        PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID, echoId,
        PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID,   cdId,
        PMC_SQL_FIELD_NOT_DELETED,          PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    id = pmcSqlSelectInt( buf_p, &count );
    if( count != 1 )
    {
        mbDlgExclaim( "Error locating echo backup echo ID: %ld CD ID: %ld\n", echoId, cdId );
        goto exit;
    }

    pmcSqlRecordDelete( PMC_SQL_TABLE_ECHO_BACKUPS, id );

    returnCode = TRUE;

exit:

    mbFree( buf_p );
    return returnCode;
#endif
    return TRUE;    
}

//---------------------------------------------------------------------------
// Function: pmcEchoBackupCount
//---------------------------------------------------------------------------
// This function returns the number of CDs this echo is backed up on.
//---------------------------------------------------------------------------

Int32s_t pmcEchoBackupCount
(
    Int32u_t    echoId
)
{
    Char_p      buf_p = NIL;
    Int32u_t    count;
    Int32s_t    returnCode = -1;

    mbCalloc( buf_p, 2048 );

    // First determine the id of the database entry
    sprintf( buf_p, "select %s from %s where %s=%ld and %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_ECHO_BACKUPS,
        PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID, echoId,
        PMC_SQL_FIELD_NOT_DELETED,          PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    pmcSqlSelectInt( buf_p, &count );

    returnCode = (Int32s_t)count;
    mbFree( buf_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcEchoBackupVerifyRestore
//---------------------------------------------------------------------------
// This function either verifies the backup on CD or restores it to disk
//---------------------------------------------------------------------------

Int32s_t pmcEchoBackupVerifyRestore
(
    Int32u_t        echoId,
    Int32u_t        cdIdIn,
    Void_p          this_p,
    Int32s_t        (*cancelCallback)( Void_p this_p ),
    Int32s_t        (*bytesCallback)( Void_p this_p, Int32u_t bytes ),
    Int32s_t        (*cdLabelCallback)( Void_p this_p, Char_p str_p, Int32u_t bytesFree ),
    Int32u_p        cancelFlag_p,
    Int32u_t        restoreFlag
)
{
    Char_p          echoName_p = NIL;
    Char_p          buf1_p = NIL;
    Char_p          buf2_p = NIL;
    TCursor         origCursor;
    qHead_p         file_q;
    qHead_t         fileQueue;
    pmcEchoFile_p   file_p;
    Int64u_t        spaceAvailable = 0;
    pmcCDList_p     cd_p;
    qHead_p         cd_q;
    qHead_t         cdQueue;
    Boolean_t       found = FALSE;
    Int32u_t        cdId;
    Int32u_t        cancelFlag;
    Int32s_t        returnCode = FALSE;
    Int32u_t        crc;
    Int32u_t        size;

    mbMalloc( echoName_p,   512 );
    mbCalloc( buf1_p,       2048 );
    mbCalloc( buf2_p,       2048 );

    file_q = qInitialize( &fileQueue );
    cd_q = qInitialize( &cdQueue );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    pmcEchoNameGet( echoId, echoName_p, 512 );

    // Get a list if CDs this echo is backed up on
    pmcEchoCDListGet( cd_q, echoId );
    if( cd_q->size == 0 )
    {
        mbDlgInfo( "%s\n\nThis echo is not backed up.", echoName_p );
        goto exit;
    }

    // If called without a CD CD check the drive for an existing ID
    cdId = pmcEchoCDIDGet( &spaceAvailable );

    if( cdId )
    {
        if( (Void_p)cdLabelCallback )
        {
            mbDriveLabel( pmcCfg[CFG_CD_BURNER].str_p, buf1_p );
            cdLabelCallback( this_p, buf1_p, (Int32u_t)spaceAvailable );
        }
    }

    if( cdIdIn && ( cdId != cdIdIn ))
    {
        mbDlgInfo( "CD in drive mismatch" );
        goto exit;
    }

    for( ; ; )
    {
        if( cdId )
        {
            found = FALSE;
            // Check to see if the echo is backed up on this CD
            qWalk( cd_p, cd_q, pmcCDList_p )
            {
                // See if this CD is one on which the echo is backed up
                if( cd_p->id == cdId )
                {
                    found = TRUE;
                    break;
                }
            }
            if( found == TRUE ) break;

            if( mbDlgYesNo( "%s\n\nThis echo is not backed up on this CD. Try another CD?", echoName_p ) == MB_BUTTON_NO ) goto exit;
            mbDriveOpen( TRUE, pmcCfg[CFG_CD_BURNER].str_p );
            cdId = 0;
        }
        else
        {
            // No CD ID

            sprintf( buf2_p, "" );

            qWalk( cd_p, cd_q, pmcCDList_p )
            {
                sprintf( buf1_p, "   " PMC_ECHO_CD_LABEL "\n", cd_p->id );
                strcat( buf2_p, buf1_p );
            }

            sprintf( buf1_p, "%s\n\nThis echo is backed up on CD%s:\n\n",
                     echoName_p,
                    ( cd_q->size == 1 ) ? "" : "s" );
            strcat( buf1_p, buf2_p );

            sprintf( buf2_p, "\nPlease insert %s CD%s into the CD drive and click OK.",
                    ( cd_q->size == 1 ) ? "this" : "one of these",
                    ( cd_q->size == 1 ) ? "" : "s" );
            strcat( buf1_p, buf2_p );

            if( mbDlgOkCancel( buf1_p ) == MB_BUTTON_CANCEL ) goto exit;

            // Check for a CD
            cdId = pmcEchoCDCheck( NIL, 0,  NIL, this_p, cdLabelCallback, FALSE, TRUE );
        }
    }

    // 20061105:  Attempt to put echo viewer on CD
    {
      // Put echo viewer on CD
        Char_t  buf[1024];
        if( pmcCfg[CFG_ECHO_RESTRICTED].str_p )
        {
            sprintf( buf, "%s", pmcCfg[CFG_CD_BURNER].str_p );
            mbStrRemoveSlashTrailing( buf );
            strcat( buf, "\\EchoView.exe" );

            if( !FileExists( buf ) )
            {
                mbFileCopy( pmcCfg[CFG_ECHO_RESTRICTED].str_p, buf );
            }
        }
    }

    // Get list of echo files
    pmcEchoFileListGet( echoId, file_q, NIL );

    sprintf( buf2_p, "%s\n\nCancel verification of echo backup?\n", echoName_p );

    returnCode = TRUE;
    qWalk( file_p, file_q, pmcEchoFile_p )
    {
        if( !FileExists( file_p->target_p ) )
        {
            sprintf( buf1_p, "Cannot find echo backup file '%s'\n", file_p->target_p );
            returnCode = FALSE;
            break;
        }

        if( restoreFlag )
        {
            mbLog( "Copy file '%s' to '%s'", file_p->target_p, file_p->source_p );

            {
                Char_t  targetPath[256];
                sprintf( targetPath, "%s\\%s", pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p, file_p->path_p );
                mbFileDirCheckCreate( targetPath, NIL, FALSE, TRUE, FALSE );
            }

            mbFileCopyCall
            (
                file_p->target_p,
                file_p->source_p,
                &crc, &size, this_p,
                cancelCallback,
                bytesCallback
            );

            if( crc != file_p->crc )
            {
                if( mbDlgYesNo( "There is a problem with file: %s.\n\nContinue restore?", file_p->source_p ) == MB_BUTTON_NO )
                {
                    sprintf( buf1_p, "CRC failed for file '%s'\n", file_p->target_p );
                    returnCode = FALSE;
                    break;
                }
            }
        }
        else
        {
            if( mbCrcFile( file_p->target_p, NIL, this_p, cancelCallback, bytesCallback, &cancelFlag, buf2_p ) != file_p->crc )
            {
                if( cancelFlag == TRUE ) break;

                sprintf( buf1_p, "CRC failed for file '%s'\n", file_p->target_p );
                returnCode = FALSE;
                break;
            }
        }
    }

    // 20031221: Deal with a bad backup
    if( returnCode == FALSE )
    {
        mbDlgExclaim( "%s\n\nThere is a problem with backup of this echo on CD ECHO-%04ld:\n\n"
                      "%s\n"
                      //"This echo will now be removed from the list of backed up echos.\n"
                      "Please inform system administrator.\n", echoName_p, cdId, buf1_p );

        pmcEchoBackupDeleteFiles( echoId );
        pmcEchoBackupDeleteEntry( echoId, cdId );
    }
    else
    {
#if 0
        if( cancelFlag == FALSE )
        {
            mbDlgInfo( "The backup of echo '%s' is good\n", echoName_p );
        }
        else
        {
            mbDlgInfo( "Verifation of echo '%s' backup cancelled\n", echoName_p );
        }
#endif
    }

exit:
    Screen->Cursor = origCursor;

    pmcEchoCDListFree( cd_q );
    pmcEchoFileListFree( file_q );

    mbFree( echoName_p );
    mbFree( buf1_p );
    mbFree( buf2_p );

    if( cancelFlag_p ) *cancelFlag_p = cancelFlag;
    return returnCode;
}

//---------------------------------------------------------------------------
// pmcEchoCDIDGet
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32u_t    pmcEchoCDIDGet( Int64u_p spaceAvailable_p )
{
    Int32u_t    echoCDID = 0;
    Char_p      name_p;
    FILE       *fp = NIL;
    Int64u_t    spaceAvailable = 0;

    mbMalloc( name_p, 512 );

    // First see if we can access the CD drive
    if( mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &spaceAvailable ) != MB_RET_OK ) goto exit;

    // Format the name of the ID file
    sprintf( name_p, "%s", pmcCfg[CFG_CD_BURNER].str_p );
    mbStrRemoveSlashTrailing( name_p );
    strcat( name_p, "\\" );
    strcat( name_p, PMC_ECHO_CD_ID_FILE );

    if( ( fp = fopen( name_p, "r" ) ) == NIL ) goto exit;

    while( fgets( name_p, 512, fp ) != 0 )
    {
        echoCDID = atol( name_p );
        break;
    }
exit:
    if( spaceAvailable_p ) *spaceAvailable_p = spaceAvailable;
    mbFree( name_p );
    if( fp ) fclose( fp );
    return echoCDID;
}

//---------------------------------------------------------------------------
// pmcEchoCDIDSet
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t    pmcEchoCDIDSet( Int32u_t id )
{
    Int32s_t    returnCode = MB_RET_ERR;
    Int32u_t    attributes = 0;
    Char_p      name_p;
    FILE       *fp = NIL;
    Int64u_t    spaceAvailable = 0;

    mbMalloc( name_p, 512 );

    // First see if we can access the CD drive
    if( mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &spaceAvailable ) != MB_RET_OK ) goto exit;

    // Check that there is space available
    if( spaceAvailable == 0 ) goto exit;

    // Check that there is not already an ID
    if( pmcEchoCDIDGet( NIL ) != 0 ) goto exit;

    // Format the name of the ID file
    sprintf( name_p, "%s", pmcCfg[CFG_CD_BURNER].str_p );
    mbStrRemoveSlashTrailing( name_p );
    strcat( name_p, "\\" );
    strcat( name_p, PMC_ECHO_CD_ID_FILE );

    if( ( fp = fopen( name_p, "w" ) ) == NIL ) goto exit;

    // Actually write the ID to the disk
    fprintf( fp, "%ld\n%ld\n", id, pmcSystemId );

    returnCode = MB_RET_OK;

exit:
    if( fp )
    {
        fclose( fp );

        // Make the ID file read only
        attributes = GetFileAttributes( name_p );
        attributes |= FILE_ATTRIBUTE_READONLY;
        SetFileAttributes( name_p, attributes );
    }
    mbFree( name_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// pmcEchoCDCheck
//---------------------------------------------------------------------------
// This function checks to see if the CD in the drive is writable, how
// much space is available, and if it is recorded as an echo backup CD
//---------------------------------------------------------------------------

Int32u_t    pmcEchoCDCheck
(
    Int64u_p        spaceAvailable_p,
    Int64u_t        spaceRequired,
    Boolean_p       promptedFlag_p,
    Void_p          this_p,
    Int32s_t       (*cdLabelCallback)( Void_p this_p, Char_p str_p, Int32u_t bytesFree ),
    Int32u_t        thisFlag,
    Int32u_t        databaseCDFlag
)
{
    Int32u_t        diskId = 0;
    Int32u_t        returnId = 0;
    Int32s_t        result;
    Int32s_t        failedCount = 0;
    TCursor         origCursor;
    Int64u_t        spaceAvailable;
    Int64u_t        size;
    Char_p          buf_p;
    Boolean_t       eject = FALSE;
    Boolean_t       load  = FALSE;
    Boolean_t       promptedFlag = FALSE;

    mbMalloc( buf_p, 1024 );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    for( ; ; )
    {
        if( eject )
        {
            Screen->Cursor = crHourGlass;
            mbDriveOpen( TRUE, pmcCfg[CFG_CD_BURNER].str_p );
            load = TRUE;
            eject = FALSE;
        }

        if( load )
        {
            promptedFlag = TRUE;
            result = mbDlgOkCancel( "Click OK once a CD has been placed in the drive." );
            Screen->Cursor = crHourGlass;
            mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );

            if( result == MB_BUTTON_CANCEL ) goto exit;
            load = FALSE;
            failedCount = 0;
        }

        // Read the available space on the backup drive
        mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );
        result = mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &spaceAvailable );

        if( result != MB_RET_OK )
        {
            failedCount++;
            if( failedCount == 2 )
            {
                promptedFlag = TRUE;
                result = mbDlgYesNo( "No CD detected in the drive. Try again?" );
                if( result == MB_BUTTON_NO ) goto exit;
                eject = TRUE;
            }
            else
            {
                Sleep( 3000 );
                eject = FALSE;
                load = FALSE;
            }
            continue;
        }

        // If we made it to this point, we must have detected a CD.  Attempt to
        // get the CD's echo backup ID
        diskId = pmcEchoCDIDGet( &spaceAvailable );

        if( (Void_p)cdLabelCallback )
        {
            mbDriveLabel( pmcCfg[CFG_CD_BURNER].str_p, buf_p );
            cdLabelCallback( this_p, buf_p, (Int32u_t)spaceAvailable );
        }

        if( diskId == 0 )
        {
            if( spaceAvailable == 0 )
            {
                // No ID and no space
                promptedFlag = TRUE;
                result = mbDlgYesNo( "This CD is not an echo backup CD.  Try a different CD?" );
                if( result == MB_BUTTON_NO ) goto exit;
                eject = TRUE;
                continue;
            }
            else
            {
                // No ID but space available.  Could be a blank disk.  Check to
                // see if there are any files on it.
                qHead_t     fileQueue;
                qHead_p     file_q;

                file_q = qInitialize( &fileQueue );

                // Generate a new file list
                size = mbFileListGet( pmcCfg[CFG_CD_BURNER].str_p,  "*.*", file_q, TRUE );
                mbFileListFree( file_q );

                if( size > 0 )
                {
                    if( databaseCDFlag )
                    {
                        // A writeable CD used for other purposes
                        promptedFlag = TRUE;
                        result = mbDlgYesNo( "This CD is not an echo backup CD.  Try a different CD?" );
                        if( result == MB_BUTTON_NO ) goto exit;
                        eject = TRUE;
                        continue;
                    }
                    else
                    {
                        // Can use this CD for a non database backup
                        returnId = 1;
                        break;
                    }
                }
                else
                {
                    if( databaseCDFlag )
                    {
                        promptedFlag = TRUE;
                        result = mbDlgYesNo( "This CD can be used backup up echos, but it has not been used before.\n"
                                         "Would you like to use this CD to backup echos?" );
                        if( result == MB_BUTTON_YES )
                        {
                            diskId = pmcSqlRecordCreate( PMC_SQL_TABLE_ECHO_CDS, NIL );
                            if( pmcEchoCDIDSet( diskId ) != MB_RET_OK )
                            {
                                mbDlgError( "Failed to create record\n" );
                                goto exit;
                            }
                            else
                            {
                                sprintf( buf_p, PMC_ECHO_CD_LABEL, diskId );
                                SetVolumeLabel( pmcCfg[CFG_CD_BURNER].str_p, buf_p );
                                mbDlgInfo( "This CD has been formatted for echo backups.\n"
                                            "Please label this CD:\n\n        %s\n\n", buf_p );

                                if( (Void_p)cdLabelCallback )
                                {
                                    mbDriveLabel( pmcCfg[CFG_CD_BURNER].str_p, buf_p );
                                    mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &spaceAvailable );
                                    cdLabelCallback( this_p, buf_p, (Int32u_t)spaceAvailable );
                                }
                                returnId = diskId;
                                break;
                            }
                        }
                        else
                        {
                            result = mbDlgYesNo( "Try a different CD?" );
                            if( result == MB_BUTTON_NO ) goto exit;
                            eject = TRUE;
                            continue;
                        }
                    }
                    else
                    {
                        // Can use this CD for a non database backup
                        returnId = 1;
                        goto exit;
                    }
                }
            }
        }
        else
        {
            if( databaseCDFlag )
            {
                //  Detected an ID... see if its valid.
                sprintf( buf_p, "select %s from %s where %s=%ld\n",
                    PMC_SQL_FIELD_ID,
                    PMC_SQL_TABLE_ECHO_CDS,
                    PMC_SQL_FIELD_ID, diskId );

                if( pmcSqlSelectInt( buf_p, NIL ) != diskId )
                {
                    promptedFlag = TRUE;
                    result = mbDlgYesNo( "This CD cannot be used to backup echos.  Try a different CD?" );
                    if( result == MB_BUTTON_NO ) goto exit;
                    eject = TRUE;
                    continue;
                }

               if( spaceAvailable == 0 && spaceRequired > 0 )
                {
                    promptedFlag = TRUE;
                    result = mbDlgYesNo( "This echo backup CD is full.  Try a different CD?" );
                    if( result == MB_BUTTON_NO ) goto exit;
                    eject = TRUE;
                    continue;
                }
                else
                {
                    // This is an echo CD with space available.
                    if( spaceRequired > spaceAvailable )
                    {
                        promptedFlag = TRUE;
                        result = mbDlgYesNo( "There is not enough space on this CD to backup %s echo.  Try a different CD?", thisFlag ? "this" : "the next" );
                        if( result == MB_BUTTON_NO ) goto exit;
                        eject = TRUE;
                        continue;
                    }
                    else
                    {
                        returnId = diskId;
                        break;
                    }
                }
            }
            else
            {
                promptedFlag = TRUE;
                result = mbDlgYesNo( "This echo backup CD cannot be used for a non-database backup.  Try a different CD?" );
                if( result == MB_BUTTON_NO ) goto exit;
                eject = TRUE;
                continue;
            }
        }
    }

    if( !databaseCDFlag && returnId == 1 )
    {
        if( spaceAvailable == 0 )
        {
            promptedFlag = TRUE;
            mbDlgInfo( "This CD is not writable." );
            returnId = 0;
            goto exit;
        }
        else
        {
            // This is an echo CD with space available.
            if( spaceRequired > spaceAvailable )
            {
                promptedFlag = TRUE;
                mbDlgInfo( "There is not enough space on this CD to backup %s echo.", thisFlag ? "this" : "the next" );
                returnId = 0;
                goto exit;
            }
        }
    }
exit:

    Screen->Cursor = origCursor;
    if( spaceAvailable_p ) *spaceAvailable_p = spaceAvailable;
    if( promptedFlag_p ) *promptedFlag_p = promptedFlag;
    mbFree( buf_p );
    return returnId;
}

//---------------------------------------------------------------------------
// pmcEchoBackup
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t                pmcEchoBackup
(
    Int32u_t            echoId,
    Int32u_t            backupSkip,
    Int64u_p            backupFreeSpace_p,
    Void_p              this_p,
    Int32s_t            (*cancelCallback)( Void_p this_p ),
    Int32s_t            (*bytesCallback)( Void_p this_p, Int32u_t bytes ),
    Int32s_t            (*cdLabelCallback)( Void_p this_p, Char_p str_p, Int32u_t bytesFree ),
    Int32u_t            thisFlag,
    Int32u_t            databaseCDFlag
)
{
    Char_p              buf_p;
    Char_p              source_p;
    Char_p              target_p;
    Int32s_t            returnCode = MB_RET_ERR;
    Int32s_t            result;
    Char_p              echoName_p = NIL;
    Int32u_t            count;
    Int32u_t            online;
    qHead_t             fileQueue;
    qHead_p             file_q;
    pmcEchoFile_p       file_p;
    Int32u_t            echoSize1 = 0;
    Int32u_t            echoSize2 = 0;
    qHead_t             cdFileQueue;
    qHead_p             cdFile_q;
    Int32u_t            diskId;
    Int32u_t            copyCrc;
    Int32u_t            copySize;
    Int32u_t            backupId;
    TCursor             origCursor;
    Boolean_t           deleteFilesFlag = FALSE;
    MbSQL               sql;

    cdFile_q = qInitialize( &cdFileQueue );

    mbMalloc( buf_p, 1024 );
    mbMalloc( source_p, 1024 );
    mbMalloc( target_p, 1024 );

    file_q = qInitialize( &fileQueue );

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    // Get the study name and the online status
    sprintf( buf_p, "select %s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_NAME,
        PMC_SQL_ECHOS_FIELD_ONLINE,
        PMC_SQL_FIELD_SIZE,
        PMC_SQL_TABLE_ECHOS,
        PMC_SQL_FIELD_ID, echoId );

    count = 0;

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        // Get the study name
        if( echoName_p == NIL ) mbMallocStr( echoName_p, sql.String(0) );

        // Get the online status of this echo
        online = sql.Int32u(1);

        // Get total echo size
        echoSize1 = sql.Int32u(2);

        count++;
    }

    // Sanity check
    if( count != 1 )
    {
        mbDlgExclaim( "Error backing up echo ID: %d\n", echoId );
        goto exit;
    }

    if( online != PMC_SQL_TRUE_VALUE )
    {
        mbDlgExclaim( "%s\n\nThis echo is offline and cannot be backed up.\n", echoName_p );
        goto exit;
    }

    if( pmcEchoFileListGet( echoId, file_q, &echoSize2 ) != MB_RET_OK )
    {
        mbDlgExclaim( "Error reading echo files\n" );
        goto exit;
    }

    // Sanity check
    if( echoSize1 != echoSize2 ) mbDlgDebug(( "Echo sizes do not match (id: %ld)", echoId ));

    // Ensure there is room on the CD
    diskId = pmcEchoCDCheck( backupFreeSpace_p, echoSize1, NIL, this_p, cdLabelCallback, thisFlag, databaseCDFlag );

    if( diskId == 0 ) goto exit;

    if( databaseCDFlag )
    {
        // Check to see if this echo is already on this disk
        sprintf( buf_p, "select %s from %s where %s=%lu and %s=%lu and %s=%lu",
            PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID,
            PMC_SQL_TABLE_ECHO_BACKUPS,
            PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID,     echoId,
            PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID,       diskId,
            PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        pmcSqlSelectInt( buf_p, &count );

        if( count > 1 )
        {
            mbDlgDebug(( "Error: count: %lu\n", count ));
        }
        else if( count == 1 )
        {
            mbDlgInfo( "%s\n\nThis echo is already backed up on this CD.\n\n"
                       "It cannot be backed up on this CD again,\n"
                       "but it can be backed up on a different CD.", echoName_p );
            goto exit;
        }
    }

    // Get a list of all files currently on the CD
    if( mbFileListGet( pmcCfg[CFG_CD_BURNER].str_p, "*.*", cdFile_q, TRUE ) == 0 )
    {
        if( databaseCDFlag )
        {
            mbDlgDebug(( "Error reading files on CD\n" ));
            goto exit;
        }
    }

    // Check the filenames to ensure no conflict with CD

    qWalk( file_p, file_q, pmcEchoFile_p )
    {
        if( mbFileListSearch( cdFile_q, file_p->target_p ) == MB_RET_OK )
        {
            mbDlgExclaim( "A file with the name\n\n  %s\n\nalready exists on this CD. The backup cannot continue.\n\n"
                           "This is probably due to a previous backup attempt that did not complete."
                           "You should attempt to backup this echo on a differnt echo CD", file_p->target_p );
            goto exit;
        }
    }

    if( backupSkip == TRUE )
    {
        mbDlgExclaim( "Skipping CD write because BackupSkip == TRUE" );
        returnCode = MB_RET_OK;
        goto exit;
    }

    // Do the actual file copies
    qWalk( file_p, file_q, pmcEchoFile_p )
    {
        // Ensure the directory exists
        if( mbFileDirCheckCreate( file_p->target_p, NIL, TRUE, TRUE, FALSE ) != MB_RET_OK )
        {
            mbDlgError( "Failed to create directory for '%s'", buf_p );
            goto exit;
        }

        if( !FileExists( file_p->source_p ) )
        {
            mbDlgError( "Backup failed.  Cannot find database file\n\n'%s'\n\nPlease contact system administrator.", file_p->source_p );
            deleteFilesFlag = TRUE;
            goto exit;
        }

        // Copy the file
        result = mbFileCopyCall
        (
            file_p->source_p, file_p->target_p,
            &copyCrc, &copySize,
            this_p,
            cancelCallback,
            bytesCallback
        );

        Screen->Cursor = crHourGlass;

        // Sanity check
        if( copyCrc != file_p->crc )
        {
            mbDlgError( "CRC error on file %s.\nBackup of echo %s failed\n", file_p->target_p, echoName_p );
            deleteFilesFlag = TRUE;
            goto exit;
        }

        if( copySize != file_p->size )
        {
            mbDlgError( "Size error on file %s.\nBackup of echo %s failed\n", file_p->target_p, echoName_p );
            deleteFilesFlag = TRUE;
            goto exit;
        }

        if( result != MB_RET_OK )
        {
            mbDlgError( "%s\n\nBackup of this echo failed.\n", echoName_p );
            deleteFilesFlag = TRUE;
            goto exit;
        }
    }

    // Write a "PMC" data file to the CD
    {
        FILE *fp;
        sprintf( target_p, "%s", pmcCfg[CFG_CD_BURNER].str_p );
        mbStrRemoveSlashTrailing( target_p );
        sprintf( buf_p, "\\%05ld\\STUDY_DATA.PMC", echoId  );
        strcat( target_p, buf_p );

        fp = fopen( target_p, "w" );
        if( fp )
        {
            fprintf( fp, "STUDY_NAME: %s\n", echoName_p );
            fclose( fp );
        }
    }

    // Put echo viewer on CD
    if( pmcCfg[CFG_ECHO_RESTRICTED].str_p )
    {
        sprintf( target_p, "%s", pmcCfg[CFG_CD_BURNER].str_p );
        mbStrRemoveSlashTrailing( target_p );
        strcat( target_p, "\\EchoView.exe" );

        if( !FileExists( target_p ) )
        {
            mbFileCopy( pmcCfg[CFG_ECHO_RESTRICTED].str_p, target_p );
        }
    }

    if( databaseCDFlag )
    {
        // Successfully copied all files onto the CD
        if( ( backupId = pmcSqlRecordCreate( PMC_SQL_TABLE_ECHO_BACKUPS, NIL ) ) == 0 )
        {
            mbDlgExclaim( "Error creating file record in database." );
            goto exit;
        }

        sprintf( buf_p, "update %s set %s=%lu,%s=%lu,%s=%lu where %s=%lu"
            ,PMC_SQL_TABLE_ECHO_BACKUPS
            ,PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID   ,diskId
            ,PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID ,echoId
            ,PMC_SQL_FIELD_NOT_DELETED          ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
            ,PMC_SQL_FIELD_ID                   ,backupId );

        if( !pmcSqlExec( buf_p ) )
        {
            mbDlgDebug(( "Failed to update echo backup record %ld\n", backupId ));
            goto exit;
        }

        // Also set the backup flag to true for this echo.
        pmcSqlExecInt
        (
            PMC_SQL_TABLE_ECHOS,
            PMC_SQL_ECHOS_FIELD_BACKUP,
            PMC_SQL_TRUE_VALUE,
            echoId
        );
    }
    returnCode = MB_RET_OK;

exit:

    if( returnCode == MB_RET_OK )
    {
        if( databaseCDFlag )
        {
            mbLog( "%s: Backed up this echo sucessfully on database CD ECHO-%04ld\n", echoName_p, diskId );
        }
        else
        {
            mbLog( "%s: Backed up this echo successfully on a non-database CD.\n", echoName_p );
        }
    }
    else
    {
        mbLog( "Failed to back up echo %ld\n", echoId );
        // 20031222: Attempt to get rid of any partial echos that may have been copied
        if( deleteFilesFlag )
        {
            pmcEchoBackupDeleteFiles( echoId );
        }
    }

    pmcEchoFileListFree( file_q );

    mbFree( buf_p );
    mbFree( source_p );
    mbFree( target_p );
    mbFree( echoName_p );

    // Free the list of files on the CD
    mbFileListFree( cdFile_q );

    Screen->Cursor = origCursor;

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcEchoCDListGet
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t pmcEchoCDListGet( qHead_p cd_q, Int32u_t echoId )
{
    Char_p          buf1_p = NIL;
    Int32s_t        returnCode = MB_RET_ERR;
    pmcCDList_p     cd_p;
    pmcCDList_p     cd2_p;
    qHead_t         tempQueue;
    qHead_p         temp_q;
    Boolean_t       added;
    MbSQL           sql;

    if( cd_q == NIL ) goto exit;

    temp_q = qInitialize( &tempQueue );

    mbMalloc( buf1_p, 1024 );

    if( echoId == 0 )
    {
        // This will get a list of all echo CDs
        sprintf( buf1_p, "select %s from %s where %s>=1",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_ECHO_CDS,
            PMC_SQL_FIELD_ID );
    }
    else
    {
        sprintf( buf1_p, "select %s from %s where %s=%lu and %s=%lu",
            PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID,
            PMC_SQL_TABLE_ECHO_BACKUPS,
            PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID,     echoId,
            PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
    }

    if( sql.Query( buf1_p ) == FALSE ) goto exit;

    while( sql.RowGet() )
    {
        mbMalloc( cd_p, sizeof( pmcCDList_t ) );
        qInsertLast( temp_q, cd_p );

        cd_p->id = sql.Int32u( 0 );
    }

    // Now sort the entries into the specified queue
    while( !qEmpty( temp_q ) )
    {
        added = FALSE;
        cd_p = (pmcCDList_p)qRemoveFirst( temp_q );
        qWalk( cd2_p, cd_q, pmcCDList_p )
        {
            if( cd_p->id < cd2_p->id )
            {
                qInsertBefore( cd_q, cd2_p, cd_p );
                added = TRUE;
                break;
            }
        }
        if( !added ) qInsertLast( cd_q, cd_p );
    }

    returnCode = MB_RET_OK;
exit:
    mbFree( buf1_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcEchoCDListFree
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t pmcEchoCDListFree( qHead_p cd_q )
{
    pmcCDList_p     cd_p;

    if( cd_q == NIL ) return MB_RET_ERR;
    while( !qEmpty( cd_q ) )
    {
        cd_p = (pmcCDList_p)qRemoveFirst( cd_q );
        mbFree( cd_p );
    }
    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// Function: pmcEchoNameGet
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Char_p  pmcEchoNameGet( Int32u_t echoId, Char_p echoName_p, Ints_t length )
{
    Char_p      buf_p = NIL;

    mbMalloc( buf_p, 1024 );

    sprintf( buf_p, "select %s from %s where %s=%lu",
        PMC_SQL_FIELD_NAME,
        PMC_SQL_TABLE_ECHOS,
        PMC_SQL_FIELD_ID, echoId );

    pmcSqlSelectStr( buf_p, echoName_p, length, NIL );

    mbFree( buf_p );

    return echoName_p;
}

//---------------------------------------------------------------------------
// Function: pmcEchoFileListFree
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t pmcEchoFileListFree( qHead_p file_q )
{
    Int32s_t        returnCode = MB_RET_ERR;
    pmcEchoFile_p   file_p;

    if( file_q == NIL ) goto exit;

    // Delete the list of files
    while( !qEmpty( file_q ) )
    {
        file_p = (pmcEchoFile_p)qRemoveFirst( file_q );
        mbFree( file_p->name_p );
        mbFree( file_p->orig_p );
        mbFree( file_p->path_p );
        mbFree( file_p->source_p );
        mbFree( file_p->target_p );
        mbFree( file_p );
    }
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}
//---------------------------------------------------------------------------
// Function: pmcEchoFileListGet
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t pmcEchoFileListGet( Int32u_t echoId, qHead_p file_q, Int32u_p totalSize_p )
{
    Int32s_t        returnCode = MB_RET_ERR;
    Char_p          buf_p = NIL;
    Char_p          source_p = NIL;
    Char_p          target_p = NIL;
    Int32u_t        totalSize = 0;
    pmcEchoFile_p   file_p;
    MbSQL           sql;
    
    if( file_q == NIL ) goto exit;
    mbMalloc( buf_p, 2048 );
    mbMalloc( source_p, 1024 );
    mbMalloc( target_p, 1024 );

    // Get the filenames belonging to this echo
    //                       0  1  2  3  4  5
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s from %s where %s=%lu and %s=%lu and %s=%lu"
        ,PMC_SQL_FIELD_ID                       // 0
        ,PMC_SQL_FIELD_CRC                      // 1
        ,PMC_SQL_FIELD_SIZE                     // 2
        ,PMC_SQL_FIELD_NAME                     // 3
        ,PMC_SQL_ECHO_FILES_FIELD_NAME_ORIG     // 4
        ,PMC_SQL_ECHO_FILES_FIELD_PATH          // 5

        ,PMC_SQL_TABLE_ECHO_FILES
        ,PMC_SQL_FIELD_TYPE                 ,PMC_FILE_TYPE_ECHO
        ,PMC_SQL_FIELD_NOT_DELETED          ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
        ,PMC_SQL_ECHO_FILES_FIELD_ECHO_ID   ,echoId );

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet() )
    {
        mbCalloc( file_p, sizeof( pmcEchoFile_t ) );
        qInsertLast( file_q, file_p );

        file_p->id   = sql.Int32u( 0 );
        file_p->crc  = sql.Int32u( 1 );
        file_p->size = sql.Int32u( 2 );

        mbMallocStr( file_p->name_p, sql.String( 3 ) );
        mbStrBackslash( file_p->name_p, NIL, TRUE );

        mbMallocStr( file_p->orig_p, sql.String( 4 ) );
        mbStrBackslash( file_p->orig_p, NIL, TRUE );

        mbMallocStr( file_p->path_p, sql.String( 5 ) );
        mbStrBackslash( file_p->path_p, NIL, TRUE );

        totalSize += file_p->size;
    }

    // Loop to determine source and destination file names
    qWalk( file_p, file_q, pmcEchoFile_p )
    {
        // Format the source file name
        sprintf( source_p, "%s", pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p );
        mbStrRemoveSlashTrailing( source_p );
        sprintf( buf_p, "\\%s\\%s", file_p->path_p, file_p->name_p );
        strcat( source_p, buf_p );
        mbMallocStr( file_p->source_p, source_p );

        // Format destination file name
        sprintf( target_p, "%s", pmcCfg[CFG_CD_BURNER].str_p );
        mbStrRemoveSlashTrailing( target_p );
        sprintf( buf_p, "\\%05ld\\%s", echoId, mbFileNamePathStrip( file_p->orig_p, source_p ) );
        strcat( target_p, buf_p );
        mbMallocStr( file_p->target_p, target_p );
    }


    returnCode = MB_RET_OK;
    if( totalSize_p ) *totalSize_p = totalSize;
exit:
    mbFree( buf_p );
    mbFree( source_p );
    mbFree( target_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcEchoDatabaseSpaceCheck
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t    pmcEchoDatabaseSpaceCheck( void )
{
    Int64u_t    databaseFreeSize;
    Int32s_t    returnCode = FALSE;

    // Get the free space on the echo disk
    if( mbDriveFreeSpace( pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p, &databaseFreeSize ) != MB_RET_OK )
    {
        mbLog( "Error reading database free space\n" );
    }
    else
    {
#if 0
        if( databaseFreeSize < 5000000000L )
        {
            mbDlgInfo( "The system is low on disk space for storing echos.\n"
                       "Please inform system administrator." );

        }
#endif
        returnCode = TRUE;
    }
    return returnCode;
}
