//---------------------------------------------------------------------------
// File:    pmcEchoImportForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    October 26, 2002
//---------------------------------------------------------------------------
// Description:
//
// A form for importing echos into the database
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#include <dos.h>
#include <dir.h>
#include <winbase.h>
#include <winioctl.h>
#pragma hdrstop

#include "mbUtils.h"
#include "hpSonosLib.h"

#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcEchoImportForm.h"
#include "pmcUtilsSql.h"
#include "pmcEchoBackup.h"

#pragma package(smart_init)
#pragma link "CGAUGES"
#pragma resource "*.dfm"


//---------------------------------------------------------------------------
__fastcall TEchoImportForm::TEchoImportForm(TComponent* Owner)
    : TForm(Owner)
{
    pmcEchoDatabaseSpaceCheck( );

    File_q = qInitialize( &FileQueue );
    Echo_q = qInitialize( &EchoQueue );

    IgnoreNotReady_q = qInitialize( &IgnoreNotReadyQueue );

    mbLog( "Echo import clicked\n" );

    DiskDetected = PMC_DISK_DETECTED_UNKNOWN;
    DiskPollTimer->Enabled = FALSE;
    StartImportButton->Enabled = FALSE;
    CancelButton->Enabled = FALSE;
    CloseButton->Enabled = TRUE;
    BackupCheckBox->Checked = FALSE;
    BackupFlag = FALSE;
    CDLabel->Caption = "";

    ScreenCursorOrig = Screen->Cursor;
    CursorOrig = Cursor;
    Valid = FALSE;

    StatusLabel->Caption = " Examining disk...";
    StatusLabel->Color = clRed;

    // Get the free space on the target drive
    if( mbDriveFreeSpace( pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p, &DatabaseFreeSize ) != MB_RET_OK )
    {
        DatabaseFreeSize = 0;
    }

    if( pmcCfg[CFG_ECHO_BACKUP_DISABLE].value == FALSE )
    {
        // User cannot disable the backup
        BackupCheckBox->Enabled = FALSE;
    }

    if( pmcCfg[CFG_ECHO_BACKUP_SKIP].value == FALSE )
    {
        // User cannot disable the backup
        BackupSkipCheckBox->Visible = FALSE;
    }

    if( pmcCfg[CFG_ECHO_IMPORT_STOP].value == FALSE )
    {
        // User cannot stop the backup
        CancelButton->Visible = FALSE;
    }
    else
    {
        CancelButton->Visible = TRUE;
    }

    BackupFreeSize = 0;
    if( BackupFlag )
    {
        Char_t  buf[64];
        mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &BackupFreeSize );
        CDLabelUpdate( mbDriveLabel( pmcCfg[CFG_CD_BURNER].str_p, buf ), FALSE );
    }

    UpdateProgress( NIL, TRUE );

    CurrentFileLabel->Caption = "";
    CurrentStudyLabel->Caption = "";

    StatusImageIndex[PMC_ECHO_IMPORT_STATUS_READY]       = PMC_COLORSQUARE_INDEX_BLUE;
    StatusImageIndex[PMC_ECHO_IMPORT_STATUS_NOT_READY]   = PMC_COLORSQUARE_INDEX_RED;
    StatusImageIndex[PMC_ECHO_IMPORT_STATUS_IMPORTING]   = PMC_COLORSQUARE_INDEX_YELLOW;
    StatusImageIndex[PMC_ECHO_IMPORT_STATUS_IN_DATABASE] = PMC_COLORSQUARE_INDEX_GREEN;
    StatusImageIndex[PMC_ECHO_IMPORT_STATUS_INVALID]     = PMC_COLORSQUARE_INDEX_RED;

    DiskPollTimer->Enabled = TRUE;

    BackupInProgress = FALSE;
}

//---------------------------------------------------------------------------
// StartImportButtonClick
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::StartImportButtonClick(TObject *Sender)
{
    pmcEcho_p           echo_p;
    Char_p              buf_p;
    Char_p              echoFilePath_p;
    Int32s_t            result;
    mbFileListStruct_p  file_p;
    qHead_p             q_p;
    MbDateTime          dateTime;
    qHead_t             dirCacheQueue;
    qHead_p             dirCache_q;
    TCursor             origCursor;
    Int32u_t            diskId;
    Boolean_t           promptedFlag;

    mbLog( "Echo import started\n" );
    // Directory cache to speed directory checks
    dirCache_q = qInitialize( &dirCacheQueue );

    origCursor = Screen->Cursor;

    mbMalloc( buf_p, 2048 );
    mbMalloc( echoFilePath_p, 512 );

    StartImportButton->Enabled = FALSE;
    CancelButton->Enabled = TRUE;
    CloseButton->Enabled = FALSE;
    CDEjectButton->Enabled = FALSE;
    CDLoadButton->Enabled = FALSE;
    BackupCheckBox->Enabled = FALSE;

    // Set the total number of bytes copied.
    BytesCopiedTotal = 0;
    TotalProgressCurrent = 0;
    TotalProgressTotal = BytesToCopyTotal;

    if( BackupCheckBox->Checked == TRUE && BackupSkipCheckBox->Checked == FALSE )
    {
        TotalProgressTotal *= 2;
    }

    UpdateProgress( NIL, FALSE );
    CopyStartTime = mbMsec( );

    qWalk( echo_p, Echo_q, pmcEcho_p )
    {
        // Only import echos that are ready

        if( echo_p->status != PMC_ECHO_IMPORT_STATUS_READY ) continue;

        promptedFlag = FALSE;
        // This loop ensures that there is space available on the CD for backup
        if( BackupFlag )
        {
            StatusLabel->Caption = " Checking CD for echo backup...";
            StatusLabel->Invalidate( );

            diskId = pmcEchoCDCheck( &BackupFreeSize, echo_p->size, &promptedFlag, this, CDLabelCallback, FALSE, TRUE );

            if( diskId == 0 ) goto exit;
            UpdateProgress( NIL, FALSE );
        }

        if( promptedFlag == TRUE )
        {
            if( mbDlgYesNo( "The backup CD is now ready.  Proceed with import?" ) == MB_BUTTON_NO ) goto exit;
        }

        StatusLabel->Caption = " Import in progress...";

        echo_p->status = PMC_ECHO_IMPORT_STATUS_IMPORTING;
        echo_p->item_p->ImageIndex = StatusImageIndex[ echo_p->status ];

        // We are going to proceed with the import.  Must create a new record
        if( ( echo_p->id = pmcSqlRecordCreate( PMC_SQL_TABLE_ECHOS, NIL ) ) == 0 )
        {
            mbDlgExclaim( "Error creating document record in database." );
            goto exit;
        }

        dateTime.SetDate( mbToday( ) );

        // Format the path to the actual echo files.  This path will be stored in the database
        sprintf( echoFilePath_p, "%04ld-%02ld-%02ld\\%06ld",
            dateTime.Year(), dateTime.Month(), dateTime.Day(), echo_p->id );

        // Update the progress indicators
        BytesBackupStudy = (Int64u_t)echo_p->size;
        BytesBackupStudyDone = 0;
        BytesToCopyStudy = (Int64u_t)echo_p->size;
        BytesCopiedStudy = 0;
        CurrentStudyLabel->Caption = echo_p->name_p;
        BytesToCopyFile = echo_p->dbFile_p->size64;
        BytesCopiedFile = 0;
        CurrentFileLabel->Caption = echo_p->dbFile_p->name_p;
        UpdateProgress( NIL, FALSE );

        // Add the HPSonosDB file
        result = EchoImportAddFile( echo_p->id,                          // Echo ID
                                    echo_p->dbFile_p->fullName_p,        // file source
                                    echoFilePath_p,                      // Path (for database)
                                    (Int32u_t)echo_p->dbFile_p->size64,  // Size (for database)
                                    echo_p->dbFile_p->crc,
                                    echo_p->date,
                                    echo_p->time,
                                    PMC_ECHO_FILE_TYPE_HPSONOS_DB,
                                    dirCache_q );

        q_p = echo_p->dir_p->sub_q;

        if( !q_p )
        {
            mbDlgDebug(( "Error: got NIL queue pointer" ));
            goto exit;
        }

        // Add the echo files
        qWalk( file_p, q_p, mbFileListStruct_p )
        {
            // Update the progress indicators
            BytesToCopyFile = file_p->size64;
            BytesCopiedFile = 0;
            CurrentFileLabel->Caption = file_p->name_p;
            UpdateProgress( NIL, FALSE );

            result = EchoImportAddFile( echo_p->id,                     // Echo ID
                                        file_p->fullName_p,             // file source
                                        echoFilePath_p,                 // Path (for database)
                                        (Int32u_t)file_p->size64,       // Size (for database)
                                        file_p->crc,
                                        echo_p->date,
                                        echo_p->time,
                                        PMC_ECHO_FILE_TYPE_HPSONOS_DSR,
                                        dirCache_q );

            if( result != MB_RET_OK ) goto exit;
        }

        // At this point, we have successfully imported an echo
        // Update the database record
        //                                  0      1      2      3      4      5       6        7     8     9     10     11
        sprintf( buf_p, "update %s set %s=%lu,%s=%lu,%s=%lu,%s=%lu,%s=%lu,%s=%lu,%s=\"%s\",%s=%lu,%s=%u,%s=%u,%s=%lu,%s=%lu "
                        "where %s=%lu",
             PMC_SQL_TABLE_ECHOS
            ,PMC_SQL_FIELD_CRC              , echo_p->dbFile_p->crc                  // 0
            ,PMC_SQL_ECHOS_FIELD_BYTE_SUM   , echo_p->dbFile_p->byteSum              // 1
            ,PMC_SQL_FIELD_TIME             , echo_p->time                           // 2
            ,PMC_SQL_FIELD_DATE             , echo_p->date                           // 3
            ,PMC_SQL_FIELD_SIZE             , (Int32u_t)echo_p->size                 // 4
            ,PMC_SQL_FIELD_LOCK             , 0                                      // 5
            ,PMC_SQL_FIELD_NAME             , mbStrClean( echo_p->name_p, NIL, TRUE )// 6
            ,PMC_SQL_FIELD_NOT_DELETED      , PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE   // 7
            ,PMC_SQL_ECHOS_FIELD_ONLINE     , PMC_SQL_TRUE_VALUE                     // 8
            ,PMC_SQL_ECHOS_FIELD_BACKUP     , PMC_SQL_FALSE_VALUE                    // 9
            ,PMC_SQL_FIELD_SONOGRAPHER_ID   , 1                                      // 10
            ,PMC_SQL_FIELD_PROVIDER_ID      , 1                                      // 11

            ,PMC_SQL_FIELD_ID               , echo_p->id );

        if( !pmcSqlExec( buf_p ) )
        {
            mbDlgDebug(( "Failed to update echo %ld\n", echo_p->id ));
            goto exit;
        }
        else
        {
            echo_p->status = PMC_ECHO_IMPORT_STATUS_IN_DATABASE;
            echo_p->item_p->ImageIndex = StatusImageIndex[ echo_p->status ];

            mbLog( "Updated echo %lu\n", echo_p->id );

            if( BackupFlag == TRUE  )
            {
                BackupInProgress = TRUE;
                result = pmcEchoBackup
                         (
                            echo_p->id, BackupSkip,
                            &BackupFreeSize,
                            (Void_p)this,
                            CancelCheckCallback,
                            BytesCopiedCallback,
                            CDLabelCallback,
                            FALSE, TRUE
                         );

                BackupInProgress = FALSE;
                if( result != MB_RET_OK )
                {
                    mbDlgExclaim( "Backup of echo '%s' failed.\n", echo_p->name_p );
                    goto exit;
                }
            }
        }
    }

    result = MB_RET_OK;
    CopyStartTime = 0;
exit:

    if( result == MB_RET_CANCEL )
    {
        mbDlgExclaim( "Echo import canceled\n" );
        UpdateProgress( NIL, TRUE );
    }

    CancelButton->Enabled = FALSE;
    CloseButton->Enabled = TRUE;
    CDEjectButton->Enabled = TRUE;
    CDLoadButton->Enabled = TRUE;

    if( origCursor) Cursor = origCursor;

    // Regenerate list.  This will prompt for erase if all echos imported
    EchoFilesListGet( this );

    mbFree( buf_p );
    mbFree( echoFilePath_p );

    // Free directory cache
    mbStrListFree( dirCache_q );

    return;
}

//---------------------------------------------------------------------------
// EchoImportAddFile
//---------------------------------------------------------------------------
// Copies a file to the echo database and creates a database record for it.
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoImportForm::EchoImportAddFile
(
    Int32s_t    echoId,
    Char_p      sourceIn_p,
    Char_p      pathIn_p,
    Int32u_t    size,
    Int32u_t    crc,
    Int32u_t    date,
    Int32u_t    time,
    Int32u_t    type,
    qHead_p     dirCache_q
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    Int32u_t    id;
    Int32u_t    copyCrc;
    Int32u_t    copySize;
    Char_p      buf_p;
    Char_p      name_p;
    Char_p      dest_p;
    Char_p      source_p;
    Char_p      path_p;

    mbMalloc( buf_p, 2048 );
    mbMalloc( name_p, 512 );
    mbMalloc( dest_p, 512 );
    mbMalloc( source_p, 512 );
    mbMalloc( path_p, 512 );

    // Sanity checks
    if( echoId == 0 || sourceIn_p == NIL || pathIn_p == NIL ) goto exit;

    // Copy these strings since they may be changed in this function
    strcpy( path_p, pathIn_p );
    strcpy( source_p, sourceIn_p );

    // We are going to proceed with the import.  Must create a new record
    if( ( id = pmcSqlRecordCreate( PMC_SQL_TABLE_ECHO_FILES, NIL ) ) == 0 )
    {
        mbDlgExclaim( "Error creating file record in database." );
        goto exit;
    }

    // Format the file name
    sprintf( name_p, "%ld-%06ld.pmc", echoId, id );

    // Format the complete path
    sprintf( dest_p, "%s\\%s\\%s", pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p, path_p, name_p );

    // Ensure the directory exists
    if( mbFileDirCheckCreate( dest_p, dirCache_q, TRUE, TRUE, FALSE ) != MB_RET_OK )
    {
        mbDlgError( "Failed to create directory for '%s'", buf_p );
        goto exit;
    }

    // Copy the file
    returnCode = mbFileCopyCall
    (
        source_p,
        dest_p,
        &copyCrc,
        &copySize,
        (Void_p)this,
        CancelCheckCallback,
        BytesCopiedCallback
    );

    if( returnCode != MB_RET_OK ) goto exit;

    if( crc == 0 )
    {
        crc = copyCrc;
    }
    else
    {
        // Sanity Check
        if( crc != copyCrc ) mbDlgDebug(( "CRCs do not match!" ));
    }

    if( size == 0 )
    {
        size = copySize;
    }
    else
    {
        if( size != copySize )  mbDlgDebug(( "sizes do not match!" ));
    }

    // At this point, if the file was successfully created, we must
    // update the database

    //                                  0      1      2      3       4         5         6        7      8      9
    sprintf( buf_p, "update %s set %s=%lu,%s=%lu,%s=%lu,%s=%lu,%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=%lu,%s=%lu,%s=%lu,"
                                  "%s=%lu,%s=%lu "
                    "where %s=%lu", PMC_SQL_TABLE_ECHO_FILES
        ,PMC_SQL_FIELD_CRC                  ,crc                                    // 0
        ,PMC_SQL_FIELD_SIZE                 ,size                                   // 1
        ,PMC_SQL_FIELD_DATE                 ,date                                   // 2
        ,PMC_SQL_FIELD_TIME                 ,time                                   // 3
        ,PMC_SQL_ECHO_FILES_FIELD_NAME_ORIG ,mbStrClean( mbStrBackslash( source_p, NIL, FALSE ) , NIL, FALSE )
        ,PMC_SQL_FIELD_NAME                 ,mbStrClean( mbStrBackslash( name_p,   NIL, FALSE ) , NIL, FALSE )
        ,PMC_SQL_ECHO_FILES_FIELD_PATH      ,mbStrClean( mbStrBackslash( path_p,   NIL, FALSE ) , NIL, FALSE )
        ,PMC_SQL_ECHO_FILES_FIELD_ECHO_ID   ,echoId                                 // 7
        ,PMC_SQL_FIELD_TYPE                 ,PMC_FILE_TYPE_ECHO                     // 8
        ,PMC_SQL_ECHO_FILES_FIELD_SUB_TYPE  ,type                                   // 9
        ,PMC_SQL_FIELD_LOCK                 ,0                                      // 10
        ,PMC_SQL_FIELD_NOT_DELETED          ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE   // 11

        ,PMC_SQL_FIELD_ID                   ,id );

    if( !pmcSqlExec( buf_p ) )
    {
        mbDlgDebug(( "Failed to update echo file %ld\n", id ));
        goto exit;
    }
    else
    {
        mbLog( "Added file '%s' to echo %lu (id: %lu)\n", source_p, echoId, id );
    }

    returnCode = MB_RET_OK;
exit:

    mbFree( buf_p );
    mbFree( name_p );
    mbFree( dest_p );
    mbFree( source_p );
    mbFree( path_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// EchoFileListGet
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TEchoImportForm::EchoFilesListGet(TObject *Sender)
{
    static int depthCount = 0;

    depthCount++;
    //mbLog( "EchoFilesListGet() called, depthCount: %d\n", depthCount );

    // Clear out the existing list
    mbFileListFree( File_q );
    EchoListFree( Echo_q );

    Screen->Cursor = crHourGlass;
    Cursor = crHourGlass;

    StatusLabel->Caption = " Examining disk...";
    StatusLabel->Invalidate( );

    // Get the free space on the target drive
    if( mbDriveFreeSpace( pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p, &DatabaseFreeSize ) != MB_RET_OK )
    {
        DatabaseFreeSize = 0;
    }

    if( mbDriveFreeSpace( pmcCfg[CFG_ECHO_IMPORT_SOURCE].str_p, NIL ) == MB_RET_OK )
    {
        // Found a disk
        mbFileListGet( pmcCfg[CFG_ECHO_IMPORT_SOURCE].str_p, "*.*", File_q, TRUE );
        DiskDetected = PMC_DISK_DETECTED_YES;
    }
    else
    {
        // No disk
        DiskDetected = PMC_DISK_DETECTED_NO;
    }
    EchoFilesListFlatten( File_q );

    EchoFilesCheck( File_q, Echo_q );
    EchoFilesSizeCompute( Echo_q );
    EchoFilesCheckDatabase( Echo_q );
    EchoFilesStatusCount( Echo_q );

    // Add items to the list view
    {
        TListItem  *item_p;
        pmcEcho_p   echo_p;
        Char_t      buf[64];

        ListView->Items->BeginUpdate( );
        ListView->Items->Clear( );

        qWalk( echo_p, Echo_q, pmcEcho_p )
        {
            item_p = ListView->Items->Add( );
            echo_p->item_p = item_p;
            item_p->Caption = echo_p->name_p;
            item_p->ImageIndex = StatusImageIndex[ echo_p->status ];
            item_p->SubItems->Add( "" );
            item_p->SubItems->Add( echo_p->date_p );
            item_p->SubItems->Add( echo_p->time_p );
            item_p->SubItems->Add( mbStrInt32u( echo_p->size, buf ) );
        }
        ListView->Items->EndUpdate( );
    }

    EchoFilesValidate( this );

    Screen->Cursor = ScreenCursorOrig;
    Cursor = CursorOrig;
    Invalidate( );

    //mbLog( "EchoFilesListGet() done, depthCount: %d\n", depthCount );
    depthCount--;
}

//---------------------------------------------------------------------------
// EchoFilesStatusCount
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t   __fastcall TEchoImportForm::EchoFilesStatusCount( qHead_p echo_q )
{
    pmcEcho_p   echo_p;

    BytesToCopyTotal = 0;
    EchoCountReady = 0;
    EchoCountNotReady = 0;
    EchoCountInDatabase = 0;
    EchoCount = 0;

    qWalk( echo_p, echo_q, pmcEcho_p )
    {
        EchoCount++;
        switch( echo_p->status )
        {
            case PMC_ECHO_IMPORT_STATUS_READY:
                BytesToCopyTotal += (Int64u_t)echo_p->size;
                EchoCountReady++;
                break;
            case PMC_ECHO_IMPORT_STATUS_NOT_READY:
                EchoCountNotReady++;
                break;
            case PMC_ECHO_IMPORT_STATUS_IN_DATABASE:
                EchoCountInDatabase++;
                break;
            default:
                mbDlgDebug(( "Got unexpected echo status\n" ));
                break;
        }
    }
    return MB_RET_OK;
}


//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::EchoFilesValidate(TObject *Sender)
{
    Int64u_t    targetSpace;
    Char_p      buf_p;
    static int  depthCount = 0;

    mbMalloc( buf_p, 1024 );

    depthCount++;
    //mbLog( "EchoFilesValidate() called, depthCount: %d\n", depthCount );
    Valid = FALSE;

    // First check if there is a disk detected
    if( DiskDetected == PMC_DISK_DETECTED_NO || DiskDetected == PMC_DISK_DETECTED_UNKNOWN  )
    {
        StatusLabel->Caption = " No disk detected.";
        goto exit;
    }

    // Are there any echo files on this disk to import?
    if( BytesToCopyTotal == 0 )
    {
        StatusLabel->Caption = " No echo files to import.";
        goto exit;
    }

    if( DatabaseFreeSize < pmcEchoMinBytesFree )
    {
        sprintf( buf_p, " Insufficient space in database (under %d MB free ).", pmcCfg[CFG_ECHO_MIN_MB_FREE].value );
        StatusLabel->Caption = buf_p;
        goto exit;
    }
    targetSpace = DatabaseFreeSize;

    if( BytesToCopyTotal > targetSpace )
    {
        StatusLabel->Caption = " Insufficient space in database.";
        goto exit;
    }
    targetSpace -= pmcEchoMinBytesFree;

    if( BytesToCopyTotal > targetSpace )
    {
        sprintf( buf_p, " Insufficient space in database (under %d MB free after import).", pmcCfg[CFG_ECHO_MIN_MB_FREE].value );
        StatusLabel->Caption = buf_p;
        goto exit;
    }

    StatusLabel->Caption = " Ready. Click 'Start' to begin import.";
    Valid = TRUE;

    if( EchoCountReady > 0 )
    {
        StartImportButton->Enabled = TRUE;
    }

exit:

    UpdateProgress( NIL, FALSE );

    StatusLabel->Color =  Valid ? clWhite : clRed;

    // Are all the echos on this disk already in the database?
    if( EchoCount && EchoCountInDatabase == EchoCount )
    {
        if( mbDlgYesNo( "All echos imported. Delete echos on optical disk?\n" ) == MB_BUTTON_YES )
        {
            EchoFilesDelete( );
            // This could be a recursive call
            EchoFilesListGet( NIL );
        }
    }
    //mbLog( "EchoFilesValidate() complete, depthCount: %d\n", depthCount );
    depthCount--;
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// EchoFilesDelete
//---------------------------------------------------------------------------
// This function will attempt to delete all echo files from the echo
// source.  Need to create a new file list because the original list
// may have been flattened
//---------------------------------------------------------------------------
void __fastcall TEchoImportForm::EchoFilesDelete( void )
{
    qHead_t     fileQueue;
    qHead_p     file_q;

    mbLog( "EchoFilesDelete() called\n" );

    file_q = qInitialize( &fileQueue );
    if( mbFileListGet( pmcCfg[CFG_ECHO_IMPORT_SOURCE].str_p, "*.*", file_q, TRUE ) != 0 )
    {
        mbFileListLog( file_q );
        mbFileListDelete( file_q, TRUE );
    }
    mbFileListFree( file_q );

    return;
}

//---------------------------------------------------------------------------
// EchoFilesCheck
//---------------------------------------------------------------------------
// This function is called after the file structure has been flattened.
// Every directory must be totally empty, or contain an HPSONOS.DB file.
//---------------------------------------------------------------------------
Int32s_t __fastcall TEchoImportForm::EchoFilesCheck
(
    qHead_p             file_q,
    qHead_p             echo_q
)
{
    Int32s_t            returnCode = MB_RET_ERR;
    mbFileListStruct_p  file_p;
    mbFileListStruct_p  root_p;
    Int32u_t            size;
    Int32u_t            i;
    Boolean_t           add = FALSE;

    // Make a fake "root" entry in the file list
    mbCalloc( root_p, sizeof( mbFileListStruct_t ) );
    mbMallocStr( root_p->name_p, "root" );
    mbMallocStr( root_p->fullName_p, "root" );
    root_p->attrib = FA_DIREC;
    root_p->sub_q = qInitialize( &root_p->subQueue );
    qInsertLast( file_q, root_p );

    // Now loop through the list and move any root files into the root directory
    size = file_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        file_p = (mbFileListStruct_p)qRemoveFirst( file_q );
        if( file_p->attrib & FA_DIREC )
        {
            // This is a directory, just put it back in the list
            qInsertLast( file_q, file_p );
        }
        else
        {
            // This is a file; put it in the root directory
            qInsertLast( root_p->sub_q, file_p );
        }
    }

    size = file_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        file_p = (mbFileListStruct_p)qRemoveFirst( file_q );

        add = FALSE;
        if( file_p->attrib & FA_DIREC )
        {
            if( file_p->sub_q->size == 0 )
            {
                // mbLog( "Found empty directory. OK\n" );
            }
            else
            {
                if( EchoFilesDBCheck( file_p, echo_q, &add ) != MB_RET_OK )
                {
                    // Error
                }
            }
        }
        else
        {
            mbDlgDebug(( "Error Got file: '%s' Size: %Ld\n", file_p->name_p, file_p->size64 ));
        }

        // Put this entry back into the list if required.
        if( !add ) qInsertLast( file_q, file_p );
    }

    returnCode = MB_RET_OK;
//exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// EchoFileDBCheck
//---------------------------------------------------------------------------
// If a HPSONOS echo file is detected, then all of the files that it
// contains should be in the list of files passed in.
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoImportForm::EchoFilesDBCheck
(
    mbFileListStruct_p  fileIn_p,
    qHead_p             echo_q,
    Boolean_p           addedFlag_p
)
{
    Int32s_t            returnCode = MB_RET_ERR;
    HpsDBHandle_t       handle = NIL;
    HpsDBRecord_t       record;
    mbFileListStruct_p  file_p;
    mbFileListStruct_p  dbFile_p;
    Char_p              name_p;
    Char_p              temp_p;
    Boolean_t           found = FALSE;
    qHead_p             file_q;
    qHead_t             tiffQueue;
    qHead_p             tiff_q;
    mbStrList_p         tiff_p;
    mbStrList_p         pat_p;
    mbStrList_p         pat2_p;
    qHead_t             patQueue;
    qHead_p             pat_q;
    qHead_t             pat2Queue;
    qHead_p             pat2_q;
    Int32u_t            missingCount;
    Boolean_t           added = FALSE;
    pmcEcho_p           echo_p;
    Int32u_t            size;
    Char_p              buf1_p;
    Char_p              buf2_p;

    mbMalloc( buf1_p, 2048 );
    mbMalloc( buf2_p, 1024 );

    if( fileIn_p == NIL  || addedFlag_p == NIL ) goto exit;

    file_q = fileIn_p->sub_q;

    if( file_q == NIL ) goto exit;

    tiff_q = qInitialize( &tiffQueue );
    pat_q  = qInitialize( &patQueue );
    pat2_q = qInitialize( &pat2Queue );

    // Must find a database file, otherwise this directory contains files
    // that are not part of a directory
    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        if( file_p->attrib & FA_DIREC )
        {
            mbDlgDebug(( "Error: found unexpected directory entry '%s'\n", file_p->name_p ))
        }
        else
        {
            if( strcmp( file_p->name_p, HPS_DB_FILENAME ) == 0 )
            {
                dbFile_p = file_p;
                found = TRUE;
                break;
            }
        }
    }

    if( !found )
    {
        mbDlgDebug(( "Did not find %s file\n", HPS_DB_FILENAME ));
        goto exit;
    }

    // Get the CRC of the HP Sonos file
    dbFile_p->crc = mbCrcFile( dbFile_p->fullName_p, NIL, NIL, NIL, NIL, NIL, NIL );
    if( dbFile_p->crc == 0 ) goto exit;

    // Get the byteSum of the HP Sonos file.
    dbFile_p->byteSum = mbFileByteSum( dbFile_p->fullName_p );
    if( dbFile_p->byteSum == 0 ) goto exit;

    // Open the database file
    if( ( handle = hpsDBOpen( dbFile_p->fullName_p ) ) == NIL ) goto exit;

    if( hpsDBTypeGet( handle ) == HPS_DB_FILE_TYPE_ROOT )
    {
        //mbLog( "This is a root file\n" );
        returnCode = MB_RET_OK;
        goto exit;
    }

    // Get echo name
    if( ( name_p = hpsDBPatNameGet( handle ) ) == NIL )
    {
        name_p = fileIn_p->name_p;
    }

    // At this point, add this directory to the list of echos to import
    mbCalloc( echo_p, sizeof( pmcEcho_t ) );
    qInsertLast( echo_q, echo_p );
    mbMallocStr( echo_p->name_p, name_p );
    added = TRUE;

    echo_p->date = hpsDBDateGet( handle );
    echo_p->time = hpsDBTimeGet( handle );

    if( ( temp_p = hpsDBDateStringGet( handle ) ) != NIL )
    {
        mbMallocStr( echo_p->date_p, temp_p );
    }

    if( ( temp_p = hpsDBTimeStringGet( handle ) ) != NIL )
    {
        mbMallocStr( echo_p->time_p, temp_p );
    }

    echo_p->status = PMC_ECHO_IMPORT_STATUS_READY;

    // Read the records.  Make a list of all tiff files
    while( hpsDBNextRecordGet( handle, &record ) == MB_RET_OK )
    {
        mbStrListAdd( tiff_q, record.fileName );
        mbStrListAdd( pat_q, record.patId.fullName );
    }

    // Now check to see if all the tiff files exist
    missingCount = 0;
    while( !qEmpty( tiff_q ) )
    {
        tiff_p = (mbStrList_p)qRemoveFirst( tiff_q );
        // mbLog( "checking for tiff file '%s'\n", tiff_p->str_p );

        found = FALSE;
 
        qWalk( file_p, file_q, mbFileListStruct_p )
        {
            if( mbStrPos( file_p->fullName_p, tiff_p->str_p ) >= 0 )
            {
                found = TRUE;
                break;
            }
        }
        if( !found )
        {
            mbDlgExclaim( "Could not find file '%s'\n", tiff_p->str_p );
            missingCount++;
        }
        mbStrListItemFree( tiff_p );
    }

    if( missingCount )
    {
        mbDlgExclaim( "The study '%s' is missing files.\nPlease report this error to the System Administrator.\n", name_p );
        echo_p->status = PMC_ECHO_IMPORT_STATUS_NOT_READY;
        goto exit;
    }

    // Check all patient names associated with the data files
    missingCount = 0;
    size = pat_q->size;
    for( Int32u_t i = 0 ; i < size ; i++ )
    {
        pat_p = (mbStrList_p)qRemoveFirst( pat_q );
 
        qWalk( pat2_p, pat_q, mbStrList_p )
        {
            if( strcmp( pat_p->str_p, pat2_p->str_p ) != 0 )
            {
                missingCount++;
            }
        }
        qInsertLast( pat_q, pat_p );
    }

    if( missingCount )
    {
        // Format a list of files for display
        Boolean_t                   addFlag;
        Int32u_t                    result;
        Boolean_t                   ignoreWarning;
        pmcEchoIgnoreNotReady_p     ignore_p;

        ignoreWarning = FALSE;
        qWalk( ignore_p, IgnoreNotReady_q, pmcEchoIgnoreNotReady_p )
        {
            if( ignore_p->crc == dbFile_p->crc )
            {
                ignoreWarning = TRUE;
                break;
            }
        }
        if( !ignoreWarning )
        {
             sprintf( buf1_p, "The study\n\n   '%s'\n\n"
                         "may contain files belonging to more than one patient.  You can choose to ignore\n"
                         "this warning for the duration of this import session, and the study can be imported.\n"
                         "Check the list of patients in this study (below) and click YES to ignore this warning:\n\n", name_p );

            size = pat_q->size;
            for( Int32u_t i = 0 ; i < size ; i++ )
            {
                pat_p = (mbStrList_p)qRemoveFirst( pat_q );
                if( strlen( pat_p->str_p ) == 0 )
                {
                    mbFree( pat_p->str_p );
                    mbMallocStr( pat_p->str_p, "Unknown" );
                }
                addFlag = TRUE;
                qWalk( pat2_p, pat2_q, mbStrList_p )
                {
                    if( strcmp( pat_p->str_p, pat2_p->str_p ) == 0 )
                    {
                        addFlag = FALSE;
                        break;
                    }
                }
                if( addFlag )
                {
                    qInsertLast( pat2_q, pat_p );
                }
                else
                {
                    qInsertLast( pat_q, pat_p );
                }
            }
            qWalk( pat_p, pat2_q, mbStrList_p )
            {
                sprintf( buf2_p, "   %s\n", pat_p->str_p );
                strcat( buf1_p, buf2_p );
            }

            sprintf( buf2_p, "\nIgnore this warning and allow this study to be imported?" );
            strcat ( buf1_p, buf2_p );
            result = mbDlgYesNo( buf1_p );

            if( result == MB_BUTTON_NO )
            {
                echo_p->status = PMC_ECHO_IMPORT_STATUS_NOT_READY;
                goto exit;
            }
            else
            {
                mbMalloc( ignore_p, sizeof( pmcEchoIgnoreNotReady_t ));
                ignore_p->crc = dbFile_p->crc;
                qInsertLast( IgnoreNotReady_q, ignore_p );
            }
        }
    }

    returnCode = MB_RET_OK;

exit:

    mbFree( buf1_p );
    mbFree( buf2_p );

    // Add pointer to DB file to echo struct
    if( added )
    {
        qRemoveEntry( file_q, dbFile_p );
        echo_p->dir_p = fileIn_p;

        // Add pointer to echo file list to echo struct
        echo_p->dbFile_p = dbFile_p;
    }

    mbStrListFree( pat_q );
    mbStrListFree( pat2_q );

    if( handle ) hpsDBClose( handle );
    if( addedFlag_p ) *addedFlag_p = added;

    return returnCode;
}

//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    EchoListFree( Echo_q );
    mbFileListFree( File_q );

    {
        pmcEchoIgnoreNotReady_p ignore_p;
        while( !qEmpty( IgnoreNotReady_q ) )
        {
            ignore_p = (pmcEchoIgnoreNotReady_p)qRemoveFirst( IgnoreNotReady_q );
            mbFree( ignore_p );
        }
    }
    Action = caFree;
}

//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::RefreshButtonClick(TObject *Sender)
{
    EchoFilesListGet( this );
}

//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::DiskPollTimerTimer(TObject *Sender)
{
    Int64u_t    backupSpace = BackupFreeSize;

    //mbLog( "DiskPollTimerTimer called" );
    if( mbDriveFreeSpace( pmcCfg[CFG_ECHO_IMPORT_SOURCE].str_p, NIL ) != MB_RET_OK )
    {
        DiskDetected = PMC_DISK_DETECTED_NO;
        EchoFilesValidate( NIL );
    }
    else
    {
        // Detected a disk... should we read the files?
        if( DiskDetected != PMC_DISK_DETECTED_YES )
        {
            mbLog( "Calling EchoFilesListGet from DiskPollTimerTimer\n" );
            EchoFilesListGet( this );
        }
    }

    // Attempt to determine the free space on the CD
    if( mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &BackupFreeSize ) != MB_RET_OK )
    {
        BackupFreeSize = 0;
        CDLabelUpdate( "", FALSE );
    }
    else
    {
        Char_t  buf[64];
        CDLabelUpdate( mbDriveLabel( pmcCfg[CFG_CD_BURNER].str_p, buf ), FALSE );
    }
    if( backupSpace != BackupFreeSize ) UpdateProgress( NIL, FALSE );

    //mbLog( "DiskPollTimerTimer done" );
}

//---------------------------------------------------------------------------

Int32s_t TEchoImportForm::BytesCopiedCallback( Void_p this_p, Int32u_t bytes )
{
    TEchoImportForm *form_p;

    Int32s_t    returnCode = MB_RET_ERR;
    if( this_p )
    {
        form_p = static_cast<TEchoImportForm *>(this_p);
        returnCode = form_p->BytesCopiedCallbackReal( bytes );
    }
    return returnCode;
}

//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoImportForm::BytesCopiedCallbackReal( Int32u_t bytes )
{
    Int32u_t    msec;

    if( BackupInProgress )
    {
        BytesBackupStudyDone += (Int64u_t)bytes;
        if( BackupFreeSize > (Int64u_t)bytes ) BackupFreeSize -= (Int64u_t)bytes;
    }
    else
    {
        BytesCopiedTotal += (Int64u_t)bytes;
        BytesCopiedFile  += (Int64u_t)bytes;
        BytesCopiedStudy += (Int64u_t)bytes;
        if( DatabaseFreeSize > (Int64u_t)bytes ) DatabaseFreeSize -= (Int64u_t)bytes;
    }
    TotalProgressCurrent += (Int64u_t)bytes;

    msec = mbMsec( );
    if( msec - LastUpdateTime > 1000 )
    {
        UpdateProgress( NIL, FALSE );
        LastUpdateTime = msec;
    }
    else
    {
        UpdateThermometers( NIL );
    }

    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// Function: CDLabelCallback
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
Int32s_t TEchoImportForm::CDLabelCallback( Void_p this_p, Char_p str_p, Int32u_t bytesFree )
{
    TEchoImportForm *form_p;

    Int32s_t    returnCode = MB_RET_OK;
    if( this_p )
    {
        form_p = static_cast<TEchoImportForm *>(this_p);
        form_p->CDLabelUpdate( str_p, TRUE );
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: FileNameCallback
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
Int32s_t TEchoImportForm::FileNameCallback( Void_p this_p, Char_p fileName_p )
{
    TEchoImportForm *form_p;

    Int32s_t    returnCode = MB_RET_ERR;
    if( this_p )
    {
        form_p = static_cast<TEchoImportForm *>(this_p);
        returnCode = form_p->FileNameCallbackReal( fileName_p );
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: FileNameCallbackReal
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoImportForm::FileNameCallbackReal( Char_p fileName_p )
{
    Char_p      buf_p;
    Int32s_t    returnCode = MB_RET_ERR;

    mbMalloc( buf_p, 1024 );
    if( fileName_p )
    {
        sprintf( buf_p, " Copying file: %s", fileName_p );
        StatusLabel->Caption = buf_p;
        StatusLabel->Color = clWhite;
        returnCode = MB_RET_OK;
    }
    mbFree( buf_p );
    return returnCode;
}
//---------------------------------------------------------------------------
// Function: CancelCheckCallback
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
Int32s_t TEchoImportForm::CancelCheckCallback( Void_p this_p )
{
    TEchoImportForm *form_p;

    Int32s_t    result = FALSE;
    if( this_p )
    {
        form_p = static_cast<TEchoImportForm *>(this_p);
        result = form_p->CancelCheckCallbackReal( );
    }
    return result;
}
//---------------------------------------------------------------------------
// Fuction: CancelButtonCheck( )
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoImportForm::CancelCheckCallbackReal( )
{
    Int32s_t    returnCode = FALSE;
    MSG         msg;
    Boolean_t   processed;
    TRect       button;
    TPoint      buttonOrigin;
    Int32s_t    startMsg = 0;
    Int32s_t    endMsg = 0;

    for( ; ; )
    {
        if( !PeekMessage( &msg, NULL, startMsg, endMsg, PM_REMOVE ) ) break;
        if( msg.hwnd != Handle && IsChild( Handle, msg.hwnd ) == 0 )
        {
            if( msg.message == WM_LBUTTONUP || msg.message == WM_LBUTTONDOWN ) continue;
        }
        processed = FALSE;

        if( msg.message == WM_LBUTTONUP || msg.message == WM_LBUTTONDOWN )
        {
            button = CancelButton->ClientRect;
            buttonOrigin = CancelButton->ClientOrigin;

            button.Top      += buttonOrigin.y;
            button.Bottom   += buttonOrigin.y;
            button.Left     += buttonOrigin.x;
            button.Right    += buttonOrigin.x;

            // For some reason on win9x, the message point seems to be in the wrong spot
            // Read the current cursor position
            if( ( mbWinVersion( ) ) == MB_WINDOWS_9X )
            {
                POINT pt;
                GetCursorPos( &pt );
                msg.pt.x = pt.x;
                msg.pt.y = pt.y;
            }
            if(    msg.pt.x > button.Left && msg.pt.x < button.Right
                && msg.pt.y > button.Top  && msg.pt.y < button.Bottom )
            {
                if( msg.message == WM_LBUTTONDOWN )
                {
                    // processed = TRUE;
                    CancelButton->Down = TRUE;
                    CancelButtonDown = TRUE;
                    CancelButton->Invalidate();
                }
                else if( msg.message == WM_LBUTTONUP )
                {
                    // processed = TRUE;
                    CancelButton->Down = FALSE;
                    CancelButton->Invalidate( );
                    if( CancelButtonDown == TRUE )
                    {
                        returnCode = TRUE;
                        goto exit;
                    }
                    CancelButtonDown = FALSE;
                }
            }
            else
            {
                // pmcLog( "message not on button\n" );
                if( CancelButtonDown == TRUE )
                {
                    // processed = TRUE;
                    CancelButton->Down = FALSE;
                    CancelButton->Invalidate( );
                    CancelButtonDown = FALSE;
                }
            } // end - if( on button )
        } // end - if( BUTTONUP || BUTTONDOWN )

        if( !processed )
        {
            TranslateMessage( (LPMSG)&msg );
            DispatchMessage(  (LPMSG)&msg );
        }
    }
exit:

    if( CancelButton->Visible == FALSE ) returnCode = FALSE;

    return returnCode;
}

//---------------------------------------------------------------------------

void pmcEchoImport( TCursor *cursor_p )
{
    TEchoImportForm    *form_p;
    Char_p              buf_p;
    FILE               *fp;

    mbMalloc( buf_p, 1024 );

    if(    pmcCfg[CFG_ECHO_IMPORT_SOURCE].str_p == NIL
        || pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p == NIL
        || pmcCfg[CFG_CD_BURNER].str_p == NIL )
    {
        mbDlgExclaim( "This computer is not configured to import Echos.\n" );
        goto exit;
    }

    // Ensure we can write to the target directory
    pmcMakeFileName( pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p, buf_p );
    if( ( fp = fopen( buf_p, "wb" ) ) == NIL )
    {
        mbDlgExclaim( "Unable to write in the Echo database: '%s'\n"
                      "Not proceeding with Echo import.", pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p );
        goto exit;
    }
    fclose( fp );
    unlink( buf_p );

    form_p = new TEchoImportForm( NIL );
    form_p->ShowModal( );
    delete form_p;

exit:
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------

void pmcEchoFilesList( qHead_p file_q )
{
    mbFileListStruct_p file_p;

    qWalk( file_p, file_q, mbFileListStruct_p )
    {
        if( file_p->attrib & FA_DIREC )
        {
            mbLog( "Got directory: '%s' Size: %Ld\n", file_p->fullName_p, file_p->size64 );
            if( file_p->sub_q ) pmcEchoFilesList( file_p->sub_q );
        }
        else
        {
            mbLog( "Got file: '%s' Size: %Ld\n", file_p->fullName_p, file_p->size64 );
        }
    }
}

//---------------------------------------------------------------------------
// This function checks to see if the echos are already in the database.
// It does this by checking the CRC and byteSum of the HPSONONS.DB file
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::EchoFilesCheckDatabase( qHead_p echo_q  )
{
    pmcEcho_p           echo_p;
    Char_p              buf_p;
    Int32u_t            count;

    mbMalloc( buf_p, 2048 );

    qWalk( echo_p, echo_q, pmcEcho_p )
    {
        sprintf( buf_p, "select %s from %s where %s=%lu and %s=%lu and %s=%lu",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_ECHOS,
            PMC_SQL_FIELD_CRC,              echo_p->dbFile_p->crc,
            PMC_SQL_ECHOS_FIELD_BYTE_SUM,   echo_p->dbFile_p->byteSum,
            PMC_SQL_FIELD_NOT_DELETED,      PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        pmcSqlSelectInt( buf_p, &count );

        if( count == 1 )
        {
            echo_p->status = PMC_ECHO_IMPORT_STATUS_IN_DATABASE;
        }
        else if( count > 1 )
        {
            mbDlgDebug(( "Error: got echo count %ld\n", count ));
        }
    }
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// This function computes the total size of each echo in the echo queue.
// The directory sizes are not accurate because they may contain sizes
// included in subdirectories.
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::EchoFilesSizeCompute( qHead_p echo_q  )
{
    pmcEcho_p           echo_p;
    mbFileListStruct_p  file_p;
    qHead_p             q_p;

    qWalk( echo_p, echo_q, pmcEcho_p )
    {
        echo_p->size = (Int32u_t)echo_p->dbFile_p->size64;

        q_p = echo_p->dir_p->sub_q;

        if( !q_p )
        {
            mbDlgDebug(( "Error: got NIL queue pointer" ));
            continue;
        }
        qWalk( file_p, q_p, mbFileListStruct_p )
        {
            echo_p->size += (Int32u_t)file_p->size64;
        }
    }
}

//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::CDLabelUpdate( Char_p str_p, Boolean_t progressFlag )
{
    CDLabel->Caption = str_p;
    if( progressFlag ) UpdateProgress( NIL, FALSE );
}

//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::UpdateProgress
(
    TObject    *Sender,
    Boolean_t   reset
)
{
    Char_t      buf1[128];

    if( reset )
    {
        BytesCopiedFile = 0;
        BytesCopiedStudy = 0;
        BytesCopiedTotal = 0;

        BytesToCopyFile = 0;
        BytesToCopyStudy = 0;
        BytesToCopyTotal = 0;

        BytesBackupStudy = 0;
        BytesBackupStudyDone = 0;

        TotalProgressCurrent = 0;
        TotalProgressTotal = 0;

        LastRemainingTimeUpdate = 0;
        CopyStartTime = 0;
    }

    DatabaseFreeSpaceLabel->Caption = mbStrInt64u( DatabaseFreeSize, buf1 );
    BackupFreeSpaceLabel->Caption   = mbStrInt64u( BackupFreeSize, buf1 );
    TotalBytesToCopyLabel->Caption  = mbStrInt64u( BytesToCopyTotal, buf1 );
    TotalBytesCopiedLabel->Caption  = mbStrInt64u( BytesCopiedTotal, buf1 );

#if 0
    if( CopyStartTime > 0 )
    {
        currentTime = mbMsec();
        if( currentTime - LastRemainingTimeUpdate > 1000 )
        {
            msec = (Int64u_t)currentTime - (Int64u_t)CopyStartTime;
            if( msec > 2000 )
            {
                rate = BytesCopiedTotal/msec;
                msec = (BytesToCopyTotal - BytesCopiedTotal)/(rate + 1);
                sec = msec / (Int64u_t)1000;
                LastRemainingTimeUpdate = currentTime;

                if( sec ) TimeRemainingLabel->Caption = mbSecToTimeRemaining( (Int32u_t)sec, buf1 );
            }
        }
    }
    else
    {
        TimeRemainingLabel->Caption = "";
    }
#endif

    UpdateThermometers( NIL );

    return;
}

//---------------------------------------------------------------------------
// Update the file, studt, and total thermometers.
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::UpdateThermometers(TObject *Sender)
{
    Int64u_t    percent;
    Ints_t      progress;

    progress = 0;
    if( BytesToCopyFile > 0 )
    {
        if( BytesCopiedFile >= BytesToCopyFile )
        {
            progress = 100;
        }
        else
        {
            percent = ( BytesToCopyFile - BytesCopiedFile );
            percent *= 100;
            percent /= BytesToCopyFile;
            progress = 100 - (Ints_t)percent;
            if( progress == 100 &&  BytesCopiedFile < BytesToCopyFile ) progress = 99;
        }
    }
    CurrentFileGauge->Progress = progress;

    progress = 0;
    if( BytesToCopyStudy > 0 )
    {
        if( BytesCopiedStudy >= BytesToCopyStudy )
        {
            progress = 100;
        }
        else
        {
            percent = ( BytesToCopyStudy - BytesCopiedStudy );
            percent *= 100;
            percent /= BytesToCopyStudy;
            progress = 100 - (Ints_t)percent;
            if( progress == 100 &&  BytesCopiedStudy < BytesToCopyStudy ) progress = 99;
        }
    }
    CurrentStudyGauge->Progress = progress;

    progress = 0;
    if( TotalProgressTotal > 0 )
    {
        if( TotalProgressCurrent >= TotalProgressTotal )
        {
            progress = 100;
        }
        else
        {
            percent = ( TotalProgressTotal - TotalProgressCurrent );
            percent *= 100;
            percent /= TotalProgressTotal;
            progress = 100 - (Ints_t)percent;
            if( progress == 100 && TotalProgressCurrent < TotalProgressTotal ) progress = 99;
        }
    }
    TotalProgressGauge->Progress = progress;

    progress = 0;
    if( BytesBackupStudyDone > 0 )
    {
        if( BytesBackupStudyDone >= BytesBackupStudy )
        {
            progress = 100;
        }
        else
        {
            percent = ( BytesBackupStudy - BytesBackupStudyDone );
            percent *= 100;
            percent /= BytesBackupStudy;
            progress = 100 - (Ints_t)percent;
            if( progress == 100 &&  BytesBackupStudyDone < BytesBackupStudy ) progress = 99;
        }
    }
    BackupGauge->Progress = progress;

    return;
}

//---------------------------------------------------------------------------
// Free the list of echos
//---------------------------------------------------------------------------
Int32s_t __fastcall TEchoImportForm::EchoListFree( qHead_p echo_q )
{
    pmcEcho_p   echo_p;
    Int32s_t    returnCode = MB_RET_ERR;

    if( echo_q == NIL ) goto exit;

    while( !qEmpty( echo_q ) )
    {
        echo_p = (pmcEcho_p)qRemoveFirst( echo_q );
        mbFileListFreeElement( echo_p->dir_p );
        mbFileListFreeElement( echo_p->dbFile_p );
        mbFree( echo_p->name_p );
        mbFree( echo_p->date_p );
        mbFree( echo_p->time_p );
        mbFree( echo_p );
    }
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//--------------------------------------------------------------------------
// This function "flattens" the file list.  All entries that are
// directories are added to the list at the root level
//--------------------------------------------------------------------------

Int32s_t __fastcall TEchoImportForm::EchoFilesListFlatten
(
    qHead_p     file_q
)
{
    qHead_t             tempQueue;
    qHead_p             temp_q;
    mbFileListStruct_p  file_p;

    temp_q = qInitialize( &tempQueue );

    EchoFilesListFlattenRecurse( file_q, temp_q );

    while( !qEmpty( temp_q ) )
    {
        file_p = (mbFileListStruct_p)qRemoveFirst( temp_q );
        qInsertLast( file_q, file_p );
    }

#if 0
    // Test
    for( file_p = (mbFileListStruct_p)qInit( file_q ) ;
                                      qDone( file_q ) ;
         file_p = (mbFileListStruct_p)qNext( file_q ) )
    {
        if( file_p->attrib & FA_DIREC )
        {
            mbLog( "Got dir '%s' path '%s'\n", file_p->name_p, file_p->fullName_p );
        }
    }
#endif

    return TRUE;
}

//--------------------------------------------------------------------------
// Function: EchoFilesListFlattenRecurse
//---------------------------------------------------------------------------
Int32s_t __fastcall TEchoImportForm::EchoFilesListFlattenRecurse
(
    qHead_p     file_q,
    qHead_p     temp_q
)
{
    mbFileListStruct_p  file_p;
    Int32u_t            i, size;

    size = file_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        file_p = (mbFileListStruct_p)qRemoveFirst( file_q );
        if( file_p->attrib & FA_DIREC )
        {
            // This is a directory
            qInsertLast( temp_q, file_p );
            EchoFilesListFlattenRecurse( file_p->sub_q, temp_q );
        }
        else
        {
            // This is a file
            qInsertLast( file_q, file_p );
        }
    }
    return MB_RET_OK;
}

//---------------------------------------------------------------------------
void __fastcall TEchoImportForm::CloseButtonClick(TObject *Sender)
{
     Close( );
}
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::BackupCheckBoxClick(TObject *Sender)
{
    BackupFlag = BackupFlag ? FALSE : TRUE;
}
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::BackupSkipCheckBoxClick(TObject *Sender)
{
    BackupSkip = BackupSkip ? FALSE : TRUE;
}
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::CDEjectButtonClick(TObject *Sender)
{
    mbDriveOpen( TRUE, pmcCfg[CFG_CD_BURNER].str_p );
    CDLabel->Caption = "";
    BackupFreeSize = 0;
    UpdateProgress( NIL, FALSE );
}
//---------------------------------------------------------------------------

void __fastcall TEchoImportForm::CDLoadButtonClick(TObject *Sender)
{
    TCursor origCursor;

    mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );
    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    if( mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &BackupFreeSize ) != MB_RET_OK )
    {
        BackupFreeSize = 0;
        CDLabelUpdate( "", FALSE );
    }
    else
    {
        Char_t  buf[64];
        CDLabelUpdate( mbDriveLabel( pmcCfg[CFG_CD_BURNER].str_p, buf ), FALSE );
    }
    UpdateProgress( NIL, FALSE );
    Screen->Cursor = origCursor;
}
//---------------------------------------------------------------------------




