//---------------------------------------------------------------------------
// File:    pmcEchoCDContentsForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    January 6, 2003
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"
#include "hpSonosLib.h"

#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcUtilsSql.h"
#include "pmcEchoImportForm.h"
#include "pmcEchoCDContentsForm.h"
#include "pmcEchoBackup.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TEchoCDContentsForm::TEchoCDContentsForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TEchoCDContentsForm::CheckButtonClick(TObject *Sender)
{
    Int32u_t    diskId = 0;
    Char_t      buf[128];
    if( RadioGroup->ItemIndex == 0 )
    {
        strcpy( buf, Edit->Text.c_str() );
        mbStrDigitsOnly( buf );
        diskId = atol( buf );

        if( diskId == 0 )
        {
            Edit->Text = "";
            Edit->SetFocus();
            return;
        }
    }
    pmcEchoCDContents( TRUE, diskId );
    Edit->Text = "";
    Edit->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TEchoCDContentsForm::CancelButtonClick(TObject *Sender)
{
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TEchoCDContentsForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Action = caFree;
}

//---------------------------------------------------------------------------

void __fastcall TEchoCDContentsForm::EditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    RadioGroup->ItemIndex = 0;
    if( Key == MB_CHAR_CR ) CheckButtonClick( NIL );
}

//---------------------------------------------------------------------------

void __fastcall TEchoCDContentsForm::RadioGroupClick(TObject *Sender)
{
    Edit->SetFocus( );
}

//--------------------------------------------------------------------------
// Function: pmcEchoCdContentsForm
//--------------------------------------------------------------------------
// Launch the form
//--------------------------------------------------------------------------

Int32s_t pmcEchoCDContentsForm( void )
{
    TEchoCDContentsForm    *form_p;

    form_p = new TEchoCDContentsForm( NIL );
    form_p->ShowModal( );
    delete form_p;

    return MB_RET_OK;
}

//--------------------------------------------------------------------------
// Function: pmcEchoCdContents
//--------------------------------------------------------------------------
// Display and/or print the list of studies on an echo CD
//--------------------------------------------------------------------------

Int32s_t pmcEchoCDContents( Boolean_t printFlag, Int32u_t diskId )
{
    Int32s_t    returnCode = MB_RET_ERR;
    Char_p      buf_p;
    Char_p      buf2_p;
    Char_p      buf3_p;
    TCursor     origCursor;
    Int64u_t    freeSpace = 0;
    Int32u_t    count;
    qHead_t     echoQueue;
    qHead_p     echo_q;
    pmcEcho_p   echo_p;
    MbDateTime  dateTime;
    Char_p      spoolFileName_p;
    Char_p      flagFileName_p;
    Char_p      fileName_p;
    FILE       *fp = NIL;
    Boolean_t   sizeChecked = FALSE;
    Int32s_t    result;
    Int32u_t    failedCount = 0;
    MbSQL       sql;
 
    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 1024 );
    mbMalloc( buf3_p, 1024 );
    mbMalloc( spoolFileName_p, 256 );
    mbMalloc( flagFileName_p, 256 );
    mbMalloc( fileName_p, 256 );

    dateTime.SetDate( mbToday() );

    // directCDPid = mbProcessIdGet( "directcd.exe" );
    echo_q = qInitialize( &echoQueue );

    if( diskId == 0 )
    {
        for( ; ; )
        {
            origCursor = Screen->Cursor;
            Screen->Cursor = crHourGlass;
            sizeChecked = FALSE;

            // Wait for directCD to be idle before accessing drive
            // if( directCDPid ) mbProcessIdWaitIdle( directCDPid, 1000, 10000 );

            // This call should block while the CD drive is not ready
            mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );
            result = mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &freeSpace );
            sizeChecked = TRUE;

            // Only attempt to read disk if mbDriveFreeSpace returned no error
            if( result == MB_RET_OK )
            {
                if( failedCount) Sleep( 3000 );
                diskId = pmcEchoCDIDGet( NIL );
            }
            else
            {
                failedCount++;
            }

            if( diskId ) break;

            // Screen->Cursor = origCursor;

            if( freeSpace )
            {
                qHead_t cdQueue;
                qHead_p cdFile_q = qInitialize( &cdQueue );

                // Get a list of all files currently on the CD
                if( mbFileListGet( pmcCfg[CFG_CD_BURNER].str_p, "*.*", cdFile_q, TRUE ) == 0 )
                {
                    result = mbDlgYesNo( "This CD is writeable and might be suitable for CD backups.\n"
                                         "Check a differrent CD?" );
                }
                else
                {
                    result = mbDlgYesNo( "This CD is writeable but cannot be used for database backups.\n"
                                         "It might be suitable for non-database backups.  Check a different CD?" );
                }
                mbFileListFree( cdFile_q );
                if( result == MB_BUTTON_NO ) goto exit;
            }
            else
            {
                if( mbDlgYesNo( "Could not read CD.\n\n"
                                "The CD in the drive may not be an echo CD,\n"
                                "or the CD drive may not be ready.\n\n"
                                "Retry?" ) == MB_BUTTON_NO ) goto exit;
            }
            Screen->Cursor = crHourGlass;

            mbDriveOpen( TRUE, pmcCfg[CFG_CD_BURNER].str_p );

            result = mbDlgOkCancel( "Click OK once a CD has been placed in the drive." );

            Screen->Cursor = crHourGlass;
            mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );

            if( result == MB_BUTTON_CANCEL )
            {
                returnCode = MB_RET_ERR;
                Screen->Cursor = origCursor;
                goto exit;
            }
        }
    }
    else
    {
        sprintf( buf_p, "select %s from %s where %s=%ld",
            PMC_SQL_FIELD_ID,
            PMC_SQL_TABLE_ECHO_CDS,
            PMC_SQL_FIELD_ID, diskId );

        pmcSqlSelectInt( buf_p, &count );
        if( count != 1 )
        {
            mbDlgInfo( "There is no Echo CD with label 'ECHO-%04ld'", diskId );
            goto exit;
        }
    }

    // Command to get echo information
    //                          0     1     2     3     4     5
    sprintf( buf_p, "select %s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s from %s,%s where %s.%s=%lu and %s.%s=%s.%s and %s.%s=%ld"
        ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID     // 0
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_NAME                     // 1
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_DATE                     // 2
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_SIZE                     // 3
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_TIME                     // 4
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_ID                       // 5

        ,PMC_SQL_TABLE_ECHO_BACKUPS
        ,PMC_SQL_TABLE_ECHOS

        ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID   ,diskId

        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_ID
        ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID
        ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    count = 0;

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        mbCalloc( echo_p, sizeof( pmcEcho_t ) );
        qInsertLast( echo_q, echo_p );

        // Get the study name
        mbMallocStr( echo_p->name_p, sql.String( 1 ) );

        echo_p->date =  sql.Int32u( 2 );
        echo_p->size =  sql.Int32u( 3 );
        echo_p->time =  sql.Int32u( 4 );
        echo_p->id   =  sql.Int32u( 5 );

        count++;
    }

    sprintf( buf_p,  "Echo CD:        ECHO-%04ld\n",    diskId );
    if( sizeChecked )
    {
        sprintf( buf2_p, "Free Space:   %s (bytes)\n\n", mbStrInt64u( freeSpace, buf3_p ) );
    }
    else
    {
        sprintf( buf2_p, "Free Space:   Unknown\n\n" );
    }
    strcat( buf_p, buf2_p );

    if( echo_q->size )
    {
        sprintf( buf2_p, "This Echo CD contains the following %s:\n\n", ( echo_q->size > 1 ) ? "studies" : "study" );
    }
    else
    {
        sprintf( buf2_p, "This Echo CD contains no studies.\n\n" );
    }
    strcat( buf_p, buf2_p );

    qWalk( echo_p, echo_q, pmcEcho_p )
    {
        sprintf( buf2_p, "     %s", echo_p->name_p );
        strcat( buf_p, buf2_p );
        dateTime.SetDate( echo_p->date );
        sprintf( buf2_p, "  (%s, ID: %lu)\n", dateTime.MDY_DateString( ) , echo_p->id );
        strcat( buf_p, buf2_p );
    }

    if( count == 0 ) printFlag = FALSE;

    if( printFlag )
    {
        sprintf( buf2_p, "\nWould you like to print this information?\n" );
        strcat( buf_p, buf2_p );
        if( mbDlgYesNo( buf_p ) == MB_BUTTON_NO ) printFlag = FALSE;
    }
    else
    {
        mbDlgInfo( buf_p );
    }

    if( printFlag == FALSE )
    {
        returnCode = MB_RET_OK;
        goto exit;
    }

    sprintf( fileName_p, "%s.html", pmcMakeFileName( NIL, buf_p ) );
    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Error opening file '%s'", spoolFileName_p ));
        goto exit;
    }

    fprintf( fp, "<HTML><HEAD><Title>Echo CD Contents</TITLE></HEAD><BODY>\n" );
    fprintf( fp, "<H2><CENTER>Echo CD:  ECHO-%04ld</CENTER></H2><HR>\n", diskId );

    fprintf( fp, "<PRE WIDTH = 80>\n\n\n" );

    dateTime.SetDate( mbToday( ) );
    dateTime.SetTime( mbTime( ) );

    fprintf( fp, "Printed:     %s %s\n", dateTime.MDY_DateStringLong( ), dateTime.HM_TimeString( ) );
    if( sizeChecked )
    {
    fprintf( fp, "Free space:  %s (bytes)\n\n\n", mbStrInt64u( freeSpace, buf3_p ) );
    }
    else
    {
    fprintf( fp, "Free space:  Unknown\n\n\n" );
    }

    fprintf( fp, "Study Name                             Date         Time   Size (bytes)       ID\n" );
    fprintf( fp, "________________________________________________________________________________\n\n" );

    qWalk( echo_p, echo_q, pmcEcho_p )
    {
        fprintf( fp, "%-29.29s", echo_p->name_p );

        dateTime.SetDateTime( echo_p->date, echo_p->time );
        fprintf( fp, "%14.14s", dateTime.MDY_DateString( ) );
        fprintf( fp, "%13.13s", dateTime.HM_TimeString( ) );
        fprintf( fp, "%15.15s  %7lu\n",  mbStrInt32u( echo_p->size, buf2_p ), echo_p->id );
    }

    fprintf( fp, "</PRE><HR>\n" );
    PMC_REPORT_FOOTER( fp );
    // End of document
    fprintf( fp, "</BODY></HTML>\n" );

    if( fp ) fclose( fp );

    mbDlgInfo( "Study list for Echo CD ECHO-%04ld sent to printer.", diskId );

    // Write flag file to trigger despooler
    fp = fopen( flagFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Could not open flag file '%s'", flagFileName_p ));
    }
    else
    {
        fprintf( fp, "%s", flagFileName_p );
        fclose( fp );
    }

    returnCode = MB_RET_OK;

exit:

    while( !qEmpty( echo_q ) )
    {
        echo_p = (pmcEcho_p)qRemoveFirst( echo_q );
        if( echo_p->name_p ) mbFree( echo_p->name_p );
        mbFree( echo_p );
    }

    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( fileName_p );

    return returnCode;
}


