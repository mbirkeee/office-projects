//---------------------------------------------------------------------------
// File:    pmcEchoListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    January 26, 2003
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <stdio.h>                                  
#include <io.h>
#include <stdlib.h>
#include <vcl.h>
#include <dir.h>
#include <process.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcTables.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"

#include "pmcMainForm.h"
#include "pmcPatientListForm.h"
#include "pmcDateSelectForm.h"
#include "pmcEchoImportForm.h"
#include "pmcTextEditForm.h"
#include "pmcEchoCDContentsForm.h"
#include "pmcEchoListForm.h"
#include "pmcEchoBackup.h"
#include "pmcEchoDetailsForm.h"
#include "pmcClaimEditForm.h"
#include "pmcDatabaseMaint.h"

#pragma package(smart_init)
#pragma link "CGAUGES"
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Default Constructor
//---------------------------------------------------------------------------
__fastcall TEchoListForm::TEchoListForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Function: TEchoListForm
//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------

__fastcall TEchoListForm::TEchoListForm
(
    TComponent         *Owner,
    pmcEchoListInfo_p   formInfo_p
)
: TForm(Owner)
{
    pmcEchoDatabaseSpaceCheck( );

    Active = FALSE;
    FilterRun = FALSE;
    ActionLabelUpdate( PMC_ECHO_ACTION_NONE );

    CancelButtonClicked = FALSE;
    CancelButton->Enabled = FALSE;

#if 0
    InitialHeight   = Height;
    InitialWidth    = Width;
    InitialTop      = Top;
    InitialLeft     = Left;
#endif

    // Position the window
    {
        Ints_t  height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_ECHO_LIST, &height, &width, &top, &left ) == MB_RET_OK )
        {
            SetBounds( left, top, width, height );
        }
    }

    // 20040104: Display CD variables are used to decide if the echo list
    // should be based on the CD Id
    DisplayCDId = 0;
    DisplayCDFlag = FALSE;

    FormInfo_p = formInfo_p;
    FormInfo_p->returnCode = MB_BUTTON_CANCEL;

    PatientIdOld = FormInfo_p->patientId;
    PatientSelectRadioGroup->ItemIndex =  FormInfo_p->patientId ? 1 : 0;
    PatientUpdate( FormInfo_p->patientId );

    GetInProgress = FALSE;
    UpdateInProgress = FALSE;
    PopupInProgress = 0;
    SortInProgress = FALSE;

    FilterRequired = FALSE;

    SearchEdit->Text = "";

    EchoReadBackItemIndex = -1;
    SelectedEchoId = 0;

    // Set up the sorting modes
    SortMode            = pmcEchoSortMode;
    SortReadDateMode    = pmcEchoSortReadDateMode;
    SortStudyNameMode   = pmcEchoSortStudyNameMode;
    SortPatNameMode     = pmcEchoSortPatNameMode;
    SortStudyDateMode   = pmcEchoSortStudyDateMode;
    SortIdMode          = pmcEchoSortIddMode;
    SortCommentMode     = pmcEchoSortCommentMode;

#if 0
    BackedUpCheckBox->Checked       = pmcEchoFilterBackedUpChecked;
    NotBackedUpCheckBox->Checked    = pmcEchoFilterNotBackedUpChecked;
    OnlineCheckBox->Checked         = pmcEchoFilterOnlineChecked;
    OfflineCheckBox->Checked        = pmcEchoFilterOfflineChecked;
    ReadCheckBox->Checked           = pmcEchoFilterReadChecked;
    NotReadCheckBox->Checked        = pmcEchoFilterNotReadChecked;
#else
    BackedUpCheckBox->Checked       = TRUE;
    NotBackedUpCheckBox->Checked    = TRUE;
    OnlineCheckBox->Checked         = TRUE;
    OfflineCheckBox->Checked        = TRUE;
    ReadCheckBox->Checked           = TRUE;
    NotReadCheckBox->Checked        = TRUE;
#endif

    CheckRecorded = FALSE;
    SaveCheckState = TRUE;

    Echo_q = qInitialize( &EchoQueue );

    // 20041229: Keep a list of CDs that were ignored in the "suggest CD
    // with echos to make offline" function.  This list gets purged when
    // the form is closed.
    Ignore_q = qInitialize( &IgnoreCDQueue );

    FilterRun = TRUE;

    if( formInfo_p->readBack >= 0 )
    {
        ReadBackComboBox->ItemIndex = formInfo_p->readBack;
    }
    else
    {
        ReadBackComboBox->ItemIndex = PMC_ECHO_BACK_MONTH_3;
    }
    ReadBackItemIndexOld = ReadBackComboBox->ItemIndex;

    ListGet( );

    // If all echos read, do not come up with an empty screen
    if( NotReadCount == 0 && formInfo_p->patientId == 0 )
    {
#if 0
        mbDlgExclaim( "Congratulations!  No unread echos.  Keep up the good work!" );
#endif        
        ReadCheckBox->Checked = TRUE;
        ListFilter( Echo_q );
    }

    if( NotBackedUpCount != 0 )
    {
#if 0
        if( NotBackedUpCount == 1 )
        {
            mbDlgExclaim( "There is 1 echo that is not backed up.\n"
                          "It should be backed up immediately." );
        }
        else
        {
            mbDlgExclaim( "There are %lu echos that are not backed up.\n"
                          "These should be backed up immediately.", NotBackedUpCount );
        }
#endif
    }

    BytesToProcess = 0;
    BytesProcessed = 0;

    CDIdLabel->Caption = "";
    BytesFreeLabel->Caption = "";
    EchoSizeLabel->Caption = "";
    BytesDoneLabel->Caption = "";
    ActionLabelUpdate( PMC_ECHO_ACTION_NONE );

    Active = TRUE;
}

//---------------------------------------------------------------------------
// Function: ListRead
//---------------------------------------------------------------------------
// Get a list of echos from the database
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListRead( qHead_p echo_q, Boolean_t allFlag )
{
    Char_p                  cmd_p;
    Char_p                  whereClause_p;
    Char_p                  buf_p;
    Char_p                  firstName_p;
    Char_p                  lastName_p;
    Char_p                  title_p;
    pmcEchoListStruct_p     echo_p;
    Int64u_t                modifiedTime;
    static Int32u_t         running = FALSE;
    MbSQL                   sql;
    MbDateTime              dateTime;

    if( running ) return;
    running = TRUE;

    mbMalloc( cmd_p, 2048 );
    mbMalloc( whereClause_p, 1024 );
    mbMalloc( buf_p, 1024 );
    mbMalloc( firstName_p, 128 );
    mbMalloc( lastName_p, 128 );
    mbMalloc( title_p, 32 );

    // Compute read back date
    {
        dateTime.SetDate( mbToday( ) );

        switch( ReadBackComboBox->ItemIndex )
        {
            case PMC_ECHO_BACK_MONTH_1:
                ReadBackDate = dateTime.BackMonths( 1 );
                break;
            case PMC_ECHO_BACK_MONTH_3:
                ReadBackDate = dateTime.BackMonths( 3 );
                break;
            case PMC_ECHO_BACK_MONTH_6:
                ReadBackDate = dateTime.BackMonths( 6 );
                break;
            case PMC_ECHO_BACK_YEARS_1:
                ReadBackDate = dateTime.BackYears( 1 );
                break;
            case PMC_ECHO_BACK_YEARS_2:
                ReadBackDate = dateTime.BackYears( 2 );
                break;
            case PMC_ECHO_BACK_YEARS_5:
                ReadBackDate = dateTime.BackYears( 5 );
                break;
            case PMC_ECHO_BACK_ALL:
            default:
                ReadBackDate = 0;
                break;
        }
    }

    // First, create the "select clause"
    //                          0     1     2     3     4     5     6     7    8      9
    sprintf( cmd_p, "select %s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,"
                           "%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s "
                    "from %s,%s,%s,%s "
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_ID                   // 0
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_NAME                 // 1
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_DATE                 // 2
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_TIME                 // 3
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_PATIENT_ID           // 4
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_ONLINE         // 5
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_BACKUP         // 6
        ,PMC_SQL_TABLE_PROVIDERS    ,PMC_SQL_FIELD_LAST_NAME            // 7
        ,PMC_SQL_TABLE_SONOGRAPHERS ,PMC_SQL_FIELD_LAST_NAME            // 8
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_READ_DATE      // 9
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_READ_TIME      // 10
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_NOT_DELETED          // 11
        ,PMC_SQL_TABLE_PATIENTS     ,PMC_SQL_FIELD_LAST_NAME            // 12
        ,PMC_SQL_TABLE_PATIENTS     ,PMC_SQL_FIELD_FIRST_NAME           // 13
        ,PMC_SQL_TABLE_PATIENTS     ,PMC_SQL_FIELD_TITLE                // 14
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_MODIFIED             // 15
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_SIZE                 // 16
        ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_COMMENT        // 17

        ,PMC_SQL_TABLE_ECHOS
        ,PMC_SQL_TABLE_PROVIDERS
        ,PMC_SQL_TABLE_SONOGRAPHERS
        ,PMC_SQL_TABLE_PATIENTS );

    // Create the fixed part of the where clause
    sprintf( whereClause_p, "where %s.%s=%s.%s and %s.%s=%s.%s and %s.%s=%s.%s"
        ,PMC_SQL_TABLE_PROVIDERS    ,PMC_SQL_FIELD_ID   ,PMC_SQL_TABLE_ECHOS    ,PMC_SQL_FIELD_PROVIDER_ID
        ,PMC_SQL_TABLE_SONOGRAPHERS ,PMC_SQL_FIELD_ID   ,PMC_SQL_TABLE_ECHOS    ,PMC_SQL_FIELD_SONOGRAPHER_ID
        ,PMC_SQL_TABLE_PATIENTS     ,PMC_SQL_FIELD_ID   ,PMC_SQL_TABLE_ECHOS    ,PMC_SQL_FIELD_PATIENT_ID   );

    if( PageControl->ActivePage == TabSheetAll )
    {
        DisplayCDFlag = FALSE;
        // Create variable the where clause
        if( FormInfo_p->patientId && PatientSelectRadioGroup->ItemIndex == 1 )
        {
            sprintf( buf_p, " and %s.%s=%lu and %s.%s=%lu"
                ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_NOT_DELETED      ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
                ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_PATIENT_ID       ,FormInfo_p->patientId );
        }
        else if( allFlag == TRUE )
        {
            // Want to read all echos
            sprintf( buf_p, " and %s.%s=%lu and ( %s.%s<=%lu or %s.%s>%lu or %s.%s=0 )"
                ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_NOT_DELETED      ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
                ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_READ_DATE  ,PMC_ECHO_STATUS_MAX
                ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_READ_DATE  ,ReadBackDate
                ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_BACKUP );
        }
        else
        {
            Int64u_t    readTime;

            dateTime.SetDateTime64( TableReadTime );
            dateTime.BackOneMinute();
            readTime = dateTime.Int64();

            // Read updated echos (including deleted echos and read echos before read back date)
            sprintf( buf_p, " and %s.%s>%Lu"
                ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_MODIFIED, readTime );
        }
    }
    else if( PageControl->ActivePage == TabSheetCD )
    {
        // Must add echo backups table when searching by backup CD
        sprintf( buf_p, ",%s ", PMC_SQL_TABLE_ECHO_BACKUPS );
        strcat( cmd_p, buf_p );

        DisplayCDFlag = TRUE;
        // Want to get echos on a specific CD
        sprintf( buf_p, " and %s.%s=%lu and %s.%s=%s.%s and %s.%s=%lu and %s.%s=%lu"
                 ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_NOT_DELETED ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
                 ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_ID          ,PMC_SQL_TABLE_ECHO_BACKUPS, PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID
                 ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID, DisplayCDId
                 ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_FIELD_NOT_DELETED ,PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE);
    }

    strcat( whereClause_p, buf_p );

    // Record the read time (which is actually the last table modification time).
    TableReadTime = pmcPollTableModifyTime[ PMC_TABLE_INDEX_ECHOS ];
    TableReadSize = pmcPollTableSize[ PMC_TABLE_INDEX_ECHOS ];

    // Add the where clause to the command
    strcat( cmd_p, whereClause_p );

    sql.Query( cmd_p );

    while( sql.RowGet( ) )
    {
        mbCalloc( echo_p, sizeof( pmcEchoListStruct_t ) );

        // Study name
        mbMallocStr( echo_p->name_p, sql.String( 1 ) );
        mbMallocStr( echo_p->studySortName_p, echo_p->name_p );
        mbStrAlphaOnly( echo_p->studySortName_p );
        mbStrToUpper( echo_p->studySortName_p );

        // Study date
        echo_p->id          = sql.Int32u( 0 );
        echo_p->date        = sql.Int32u( 2 );
        echo_p->time        = sql.Int32u( 3 );
        echo_p->patientId   = sql.Int32u( 4 );
        echo_p->onlineFlag  = sql.Int32u( 5 );
        echo_p->backupFlag  = sql.Int32u( 6 );

        // Reader name
        mbMallocStr( echo_p->reader_p, sql.String( 7 ) );

        // Sonographer name
        mbMallocStr( echo_p->sonographer_p, sql.String( 8 ) );

        // Date and time echo read
        echo_p->readDate    = sql.Int32u(  9 );
        echo_p->readTime    = sql.Int32u( 10 );
        echo_p->notDeleted  = sql.Int32u( 11 );

        // firstName
        strcpy( lastName_p, sql.String( 12 )  );

        // lastName
        strcpy( firstName_p, sql.String( 13 ) );

        // title
        strcpy( title_p, sql.String( 14 ) );

        if( strlen( lastName_p ) ) strcat( lastName_p, ", " );
        strcat( lastName_p, firstName_p );
        if( strlen( title_p ) && strlen( lastName_p ) )
        {
            sprintf( firstName_p, " (%s)", title_p );
            strcat( lastName_p, firstName_p );
        }
        mbMallocStr( echo_p->patientName_p, lastName_p );

        // Copy the patient name to make a sort string
        mbMallocStr( echo_p->patSortName_p, echo_p->patientName_p );
        mbStrAlphaNumericOnly( echo_p->patSortName_p );
        mbStrToUpper( echo_p->patSortName_p );

        // Get latest time of all read objects
        dateTime.SetDateTime64( sql.DateTimeInt64u( 15 ) );
        modifiedTime = dateTime.Int64();
        if( modifiedTime > TableReadTime ) TableReadTime = modifiedTime;

        echo_p->size = sql.Int32u( 16 );

        // Sonographer name
        mbMallocStr( echo_p->comment_p, sql.String( 17 ) );

        mbMallocStr( echo_p->commentSort_p, echo_p->comment_p );
        mbStrToUpper( echo_p->commentSort_p );

        qInsertFirst( echo_q, echo_p );
    }

    mbFree( cmd_p );
    mbFree( whereClause_p );
    mbFree( buf_p );
    mbFree( lastName_p );
    mbFree( firstName_p );
    mbFree( title_p );

    running = FALSE;
    return;
}

//---------------------------------------------------------------------------
// Function: ListUpdate
//---------------------------------------------------------------------------
// If there are any added or deleted echos, we will have to resort the
// list.  But if there are only modified echos, then we can just update
// the list.
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListUpdate( Boolean_t timerFlag )
{
    qHead_t     echoQueue;
    qHead_p     echo_q;
    Boolean_t   resort = FALSE;

    UpdateInProgress = TRUE;

    echo_q = qInitialize( &echoQueue );

    // Get a list of modified echos
    ListRead( echo_q, FALSE );

    if( ListEchosDetectNew( echo_q ) == MB_RET_OK )
    {
        resort = TRUE;
    }

    if( ListEchosDetectDeleted( echo_q ) )
    {
        resort = TRUE;
    }

    // Modify any existing entries in the list
    resort = ListEchosModifiy( echo_q, resort );

    // Delete the list of modified echos
    ListFree( echo_q );

    // MAB:20070101: Test this
    if( timerFlag == TRUE )
    {
        resort = FALSE;
        //FilterRequired = FALSE;
    }

    if( resort )
    {
        ListSort( Echo_q );
    }
    else if( FilterRequired )
    {
        ListFilter( Echo_q );
    }
    else
    {
        EchoListView->Selected->MakeVisible( TRUE );
    }

    UpdateInProgress = FALSE;
    return;
}

//---------------------------------------------------------------------------
// Function: ListEchosDetectNew
//---------------------------------------------------------------------------
// Update the list of echos
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoListForm::ListEchosDetectNew( qHead_p echo_q )
{
    pmcEchoListStruct_p     echo1_p;
    pmcEchoListStruct_p     echo2_p;
    qHead_t                 tempQueue;
    qHead_p                 temp_q;
    static  Boolean_t       running = FALSE;
    Int32s_t                returnCode = MB_RET_ERR;
    Boolean_t               found;

    temp_q = qInitialize( &tempQueue );

    // Sanity check
    if( echo_q == NIL ) goto exit;

    // Sanity check
    if( running ) mbDlgError(( "Running = TRUE" ));
    running = TRUE;

    while( !qEmpty( echo_q ) )
    {
        echo1_p = (pmcEchoListStruct_p)qRemoveFirst( echo_q );

        if( echo1_p->notDeleted == FALSE )
        {
            // This must be a deleted echo
            qInsertLast( temp_q, echo1_p );
            continue;
        }

        found = FALSE;

        qWalk( echo2_p, Echo_q, pmcEchoListStruct_p )
        {
            if( echo1_p->id == echo2_p->id )
            {
                found = TRUE;
                break;
            }
        }

        if( found )
        {
            // This echo is already in the list. Do nothing
            qInsertLast( temp_q, echo1_p );
        }
        else
        {
            // This must be a new echo.  Add to the list of echos
            qInsertLast( Echo_q, echo1_p );
            returnCode = MB_RET_OK;
        }
    }

exit:

    if( echo_q )
    {
        // Return all processed echos to the original queue
        while( !qEmpty( temp_q ) )
        {
            echo1_p = (pmcEchoListStruct_p)qRemoveFirst( temp_q );
            qInsertLast( echo_q, echo1_p );
        }
    }

    running = FALSE;

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: ListEchosDetectDeleted
//---------------------------------------------------------------------------
// Update the list of echos
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoListForm::ListEchosDetectDeleted( qHead_p echo_q )
{
    pmcEchoListStruct_p     echo1_p;
    pmcEchoListStruct_p     echo2_p;
    qHead_t                 tempQueue;
    qHead_p                 temp_q;
    static  Boolean_t       running = FALSE;
    Int32s_t                returnCode = MB_RET_ERR;
    Boolean_t               found;

    temp_q = qInitialize( &tempQueue );

    // Sanity check
    if( echo_q == NIL ) goto exit;

    // Sanity check
    if( running ) mbDlgError(( "Running = TRUE" ));
    running = TRUE;

    while( !qEmpty( echo_q ) )
    {
        echo1_p = (pmcEchoListStruct_p)qRemoveFirst( echo_q );

        if( echo1_p->notDeleted == TRUE )
        {
            // This must be a not deleted echo
            qInsertLast( temp_q, echo1_p );
            continue;
        }

        found = FALSE;

        qWalk( echo2_p, Echo_q, pmcEchoListStruct_p )
        {
            if( echo1_p->id == echo2_p->id )
            {
                found = TRUE;
                break;
            }
        }

        if( found )
        {
            // This deleted echo is in the list... delete from list
            qRemoveEntry( Echo_q, echo2_p );
            returnCode = MB_RET_OK;
            ListFreeEntry( echo2_p );
        }
        qInsertLast( temp_q, echo1_p );
    }

exit:

    if( echo_q )
    {
        // Return all processed echos to the original queue
        while( !qEmpty( temp_q ) )
        {
            echo1_p = (pmcEchoListStruct_p)qRemoveFirst( temp_q );
            qInsertLast( echo_q, echo1_p );
        }
    }

    running = FALSE;

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: ListEchosModify
//---------------------------------------------------------------------------
// Update the list of echos
//---------------------------------------------------------------------------

Boolean_t __fastcall TEchoListForm::ListEchosModifiy( qHead_p echo_q, Boolean_t sortFlag )
{
    pmcEchoListStruct_p echo1_p;
    pmcEchoListStruct_p echo2_p;
    qHead_t             tempQueue;
    qHead_p             temp_q;
    static  Boolean_t   running = FALSE;
    Boolean_t           found;
    Char_t              buf[64];

    temp_q = qInitialize( &tempQueue );

    // Sanity check
    if( echo_q == NIL ) goto exit;

    // Sanity check
    if( running ) mbDlgError(( "Running = TRUE" ));
    running = TRUE;

    while( !qEmpty( echo_q ) )
    {
        echo1_p = (pmcEchoListStruct_p)qRemoveFirst( echo_q );

        if( echo1_p->notDeleted == FALSE )
        {
            // This must be a deleted echo...
            qInsertLast( temp_q, echo1_p );
            continue;
        }

        found = FALSE;
        qWalk( echo2_p, Echo_q, pmcEchoListStruct_p )
        {
            if( echo1_p->id == echo2_p->id )
            {
                found = TRUE;
                break;
            }
        }

        if( found )
        {
            qInsertAfter( Echo_q, echo2_p, echo1_p );
            qRemoveEntry( Echo_q, echo2_p );
            echo1_p->item_p = echo2_p->item_p;

            // Look for cases where we may have to sort or filter the list
            if(    ( echo1_p->patientId != echo2_p->patientId )
                && ( SortMode == PMC_SORT_NAME_ASCENDING || SortMode == PMC_SORT_NAME_DESCENDING ) )
            {
                sortFlag = TRUE;
            }

            if( echo1_p->readDate != echo2_p->readDate )
            {
                if( SortMode == PMC_SORT_DATE_ASCENDING || SortMode == PMC_SORT_DATE_DESCENDING )
                {
                    sortFlag = TRUE;
                }
                else
                {
                    if( echo1_p->readDate > PMC_ECHO_STATUS_MAX )
                    {
                        if( ReadCheckBox->Checked == FALSE )
                        {
                            FilterRequired = TRUE;
                        }
                        if( echo1_p->readDate < ReadBackDate )
                        {
                            FilterRequired = TRUE;
                        }
                    }
                    else
                    {
                        if( NotReadCheckBox->Checked == FALSE ) FilterRequired = TRUE;
                    }
                }
            }

            // Check the Study name
            if( SortMode == PMC_SORT_DESC_DESCENDING || SortMode == PMC_SORT_DESC_ASCENDING )
            {
                if( strcmp( echo1_p->name_p, echo2_p->name_p ) != 0 ) sortFlag = TRUE;
            }
            if( echo1_p->id == SelectedEchoId )
            {
                SelectedLabel->Caption = echo1_p->name_p;
                EchoSizeLabel->Caption = mbStrInt32u( echo1_p->size, buf );
                BytesDoneLabel->Caption = "";
                ProgressGauge->Progress = 0;
            }

            // Check the online status
            if( echo1_p->onlineFlag )
            {
                if( OnlineCheckBox->Checked == FALSE ) FilterRequired = TRUE;
            }
            else
            {
                if( OfflineCheckBox->Checked == FALSE ) FilterRequired = TRUE;
            }

            // Check the backup status
            if( echo1_p->backupFlag )
            {
                if( BackedUpCheckBox->Checked == FALSE ) FilterRequired = TRUE;
            }
            else
            {
                if( NotBackedUpCheckBox->Checked == FALSE ) FilterRequired = TRUE;
            }

            // Update item if no sort or filter required
            if( sortFlag == FALSE && FilterRequired == FALSE ) ListItemUpdate( echo1_p );
            ListFreeEntry( echo2_p );
        }
        else
        {
            // This must be a new echo.  Add to the list of echos
            qInsertLast( Echo_q, echo1_p );
            sortFlag = TRUE;
        }
    }

exit:

    if( echo_q )
    {
        // Return all processed echos to the original queue
        while( !qEmpty( temp_q ) )
        {
            echo1_p = (pmcEchoListStruct_p)qRemoveFirst( temp_q );
            qInsertLast( echo_q, echo1_p );
        }
    }
    running = FALSE;
    return sortFlag;
}

//---------------------------------------------------------------------------
// Function: ListGet
//---------------------------------------------------------------------------
// Update the list of echos
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListGet( )
{
    static Boolean_t    running = FALSE;
     TCursor            origCursor;

    if( running ) return;
    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;
    running = TRUE;

    // Sanity Check
    if( UpdateInProgress || PopupInProgress )
    {
        mbDlgDebug(( "ListGet() Failed" ));
        goto exit;
    }

    GetInProgress = TRUE;

    ListFree( Echo_q );
    ListRead( Echo_q, TRUE );
    ListSort( Echo_q );

    GetInProgress = FALSE;

exit:
    running = FALSE;
    Screen->Cursor = origCursor;

    return;
}

//---------------------------------------------------------------------------
// Function: ListSort
//---------------------------------------------------------------------------
// Sort the list of echos
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListSort( qHead_p echo_q )
{
    qHead_t                     tempQueue;
    qHead_p                     temp_q;
    pmcEchoListStruct_p         echo_p;
    pmcEchoListStruct_p         tempEcho_p;
    Boolean_t                   added;

    temp_q = qInitialize( &tempQueue );

    for( ; ; )
    {
        if( qEmpty( Echo_q ) ) break;

        echo_p = (pmcEchoListStruct_p)qRemoveFirst( Echo_q );

        added = FALSE;
        qWalk( tempEcho_p, temp_q, pmcEchoListStruct_p )
        {
            switch( SortMode )
            {
                case PMC_SORT_COMMENT_DESCENDING:
                case PMC_SORT_COMMENT_ASCENDING:
                    added = ListSortComment( SortMode, temp_q, echo_p, tempEcho_p );
                    break;

                case PMC_SORT_DATE_DESCENDING:
                case PMC_SORT_DATE_ASCENDING:
                    added = ListSortReadDate( SortMode, temp_q, echo_p, tempEcho_p );
                    break;

                case PMC_SORT_NAME_ASCENDING:
                case PMC_SORT_NAME_DESCENDING:
                    added = ListSortPatName( SortMode, temp_q, echo_p, tempEcho_p );
                    break;

                case PMC_SORT_DESC_ASCENDING:
                case PMC_SORT_DESC_DESCENDING:
                    added = ListSortStudyName( SortMode, temp_q, echo_p, tempEcho_p );
                    break;

                case PMC_SORT_STUDY_DATE_ASCENDING:
                case PMC_SORT_STUDY_DATE_DESCENDING:
                    added = ListSortStudyDate( SortMode, temp_q, echo_p, tempEcho_p );
                    break;

                case PMC_SORT_ID_ASCENDING:
                case PMC_SORT_ID_DESCENDING:
                    added = ListSortId( SortMode, temp_q, echo_p, tempEcho_p );
                    break;

                default:
                    break;
            }
            if( added ) break;
        }
        if( !added ) qInsertLast( temp_q, echo_p );
    }

    //  Move entries back into the main list
    for( ; ; )
    {
       if( qEmpty( temp_q )) break;

       echo_p = (pmcEchoListStruct_p)qRemoveLast( temp_q );
       qInsertFirst( Echo_q, echo_p );
    }

    ListFilter( Echo_q );
}

//---------------------------------------------------------------------------
// Function: ListFilter
//---------------------------------------------------------------------------
// Determine which echosto display
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListFilter( qHead_p echo_q )
{
    TListItem                  *firstItem_p = NIL;
    Boolean_t                   selectedFound = FALSE;
    Int32u_t                    firstItemId = 0;
    pmcEchoListStruct_p         echo_p;
    static Boolean_t            running = FALSE;
    Boolean_t                   display;
    Char_t                      searchString[64];
    Char_t                      searchDigitsString1[32];
    Char_t                      searchDigitsString2[32];
    Int32u_t                    searchId;
    Boolean_t                   searchFlag = FALSE;

    if( !FilterRun ) return;

    // Will get called recursively when temporarily changing check boxes.
    if( running ) return;

    running = TRUE;
    FilterRequired = FALSE;

    // Clear displayed list
    EchoListView->Items->BeginUpdate( );
    EchoListView->Items->Clear( );
    EchoListView->Items->EndUpdate( );

    EchoListView->Selected = NIL;

    ReadCount = 0;
    NotReadCount = 0;
    OnlineCount = 0;
    OfflineCount = 0;
    BackedUpCount = 0;
    NotBackedUpCount = 0;

    // Check to see if we'll be filtering on study name
    if( DisplayCDFlag == FALSE )
    {
        if( strlen( SearchEdit->Text.c_str() ) > 0 )
        {
            strcpy( searchString, SearchEdit->Text.c_str() );
            strcpy( searchDigitsString1, SearchEdit->Text.c_str() );

            mbStrAlphaNumericOnly( searchString );
            mbStrToUpper( searchString );
            mbStrDigitsOnly( searchDigitsString1 );
            searchId = atoi( searchDigitsString1 );
            searchFlag = TRUE;
        }
    }

    if(    ( strlen( SearchEdit->Text.c_str() ) )
        || ( FormInfo_p->patientId && PatientSelectRadioGroup->ItemIndex == 1 )
        || DisplayCDFlag == TRUE )
    {
        if( CheckRecorded == FALSE )
        {
            ReadCheck                       = ReadCheckBox->Checked;
            NotReadCheck                    = NotReadCheckBox->Checked;
            OnlineCheck                     = OnlineCheckBox->Checked;
            OfflineCheck                    = OfflineCheckBox->Checked;
            BackedUpCheck                   = BackedUpCheckBox->Checked;
            NotBackedUpCheck                = NotBackedUpCheckBox->Checked;
            CheckRecorded = TRUE;
        }

        SaveCheckState = FALSE;

        BackedUpCheckBox->Checked       = TRUE;
        NotBackedUpCheckBox->Checked    = TRUE;
        OnlineCheckBox->Checked         = TRUE;
        OfflineCheckBox->Checked        = TRUE;
        ReadCheckBox->Checked           = TRUE;
        NotReadCheckBox->Checked        = TRUE;

        BackedUpCheckBox->Enabled       = FALSE;
        NotBackedUpCheckBox->Enabled    = FALSE;
        OnlineCheckBox->Enabled         = FALSE;
        OfflineCheckBox->Enabled        = FALSE;
        ReadCheckBox->Enabled           = FALSE;
        NotReadCheckBox->Enabled        = FALSE;
    }
    else
    {
        BackedUpCheckBox->Enabled       = TRUE;
        NotBackedUpCheckBox->Enabled    = TRUE;
        OnlineCheckBox->Enabled         = TRUE;
        OfflineCheckBox->Enabled        = TRUE;
        ReadCheckBox->Enabled           = TRUE;
        NotReadCheckBox->Enabled        = TRUE;

        if( CheckRecorded )
        {
            // Not searching... restore filter settings
            BackedUpCheckBox->Checked       = BackedUpCheck;
            NotBackedUpCheckBox->Checked    = NotBackedUpCheck;
            OnlineCheckBox->Checked         = OnlineCheck;
            OfflineCheckBox->Checked        = OfflineCheck;
            ReadCheckBox->Checked           = ReadCheck;
            SaveCheckState = TRUE;
            NotReadCheckBox->Checked        = NotReadCheck;
            CheckRecorded = FALSE;
        }
    }

    if( DisplayCDFlag == FALSE && FormInfo_p->patientId && PatientSelectRadioGroup->ItemIndex == 1 )
    {
        if( EchoReadBackItemIndex == -1 )
        {
            EchoReadBackItemIndex = ReadBackComboBox->ItemIndex;
        }
        ReadBackComboBox->ItemIndex = PMC_ECHO_BACK_ALL;
        ReadBackComboBox->Enabled = FALSE;
    }
    else
    {
        if( EchoReadBackItemIndex >= 0 )
        {
            ReadBackComboBox->ItemIndex = pmcEchoReadBackItemIndex;
            EchoReadBackItemIndex = -1;
        }
        ReadBackComboBox->Enabled = TRUE;
    }

    // Check to see if list is being sorted after an import
    qWalk( echo_p, echo_q, pmcEchoListStruct_p )
    {
        display = TRUE;
        echo_p->item_p = NIL;

        if( DisplayCDFlag == FALSE )
        {
            // Don't consider old read echo any further; unless not backed up
            if( echo_p->readDate > PMC_ECHO_STATUS_MAX && echo_p->readDate < ReadBackDate && echo_p->backupFlag )
            {
                continue;
            }
        }

        // Search for the search string
        if( searchFlag && display == TRUE )
        {
            display = FALSE;
            if( mbStrPos( echo_p->studySortName_p, searchString ) >= 0 )
            {
                display = TRUE;
            }
            else if( mbStrPos( echo_p->patSortName_p, searchString ) >= 0 )
            {
                display = TRUE;
            }
            else if( mbStrPos( echo_p->commentSort_p, searchString ) >= 0 )
            {
                display = TRUE;
            }
            else if( searchId )
            {
                sprintf( searchDigitsString2, "%ld", echo_p->id );
                if( mbStrPos( searchDigitsString2, searchDigitsString1 ) >= 0 )
                {
                    display = TRUE;
                }
            }
        }

        // Are not going to display this echo
        if( display == FALSE ) continue;

        if( echo_p->readDate > PMC_ECHO_STATUS_MAX )
        {
            // This is a read echo
            if( ReadCheckBox->Checked == FALSE ) display = FALSE;
            ReadCount++;
        }
        else
        {
            // Not read
            if( NotReadCheckBox->Checked == FALSE ) display = FALSE;
            NotReadCount++;
        }

        if( echo_p->backupFlag )
        {
            if( BackedUpCheckBox->Checked == FALSE ) display = FALSE;
            BackedUpCount++;
        }
        else
        {
            if( NotBackedUpCheckBox->Checked == FALSE ) display = FALSE;
            NotBackedUpCount++;
        }

        if( echo_p->onlineFlag )
        {
            if( OnlineCheckBox->Checked == FALSE ) display = FALSE;
            OnlineCount++;
        }
        else
        {
            if( OfflineCheckBox->Checked == FALSE ) display = FALSE;
            OfflineCount++;
        }

        if( display == FALSE ) continue;

        // Set the selected id if it is not already set
        if( SelectedEchoId == 0 ) SelectedEchoId = echo_p->id;

        echo_p->item_p = EchoListView->Items->Add( );
        if( firstItem_p == NIL )
        {
            firstItem_p = echo_p->item_p;
            firstItemId = echo_p->id;
        }

        ListItemUpdate( echo_p );

        // Set selected row if found
        if( echo_p->id == SelectedEchoId )
        {
            EchoListView->Selected = echo_p->item_p;
            selectedFound = TRUE;
         }
    }

    // Set selected item to first item if item not found
    if( selectedFound == FALSE && firstItem_p != NIL )
    {
        EchoListView->Selected = firstItem_p;
        SelectedEchoId = firstItemId;
    }

    if( EchoListView->Selected )
    {
        // Display selected item
        EchoListView->Selected->Selected = TRUE;
        EchoListView->Selected->MakeVisible( TRUE );
    }

    if( Active ) EchoListView->SetFocus( );

    CountsUpdate( );

    running = FALSE;
}

//---------------------------------------------------------------------------
// Function: ListSortPatName
//---------------------------------------------------------------------------
// Update a single item in the list view
//---------------------------------------------------------------------------

bool __fastcall  TEchoListForm::ListSortReadDate
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcEchoListStruct_p         echo_p,
    pmcEchoListStruct_p         tempEcho_p
)
{
    Boolean_t   added = FALSE;

    if( echo_p->readDate == 0 && tempEcho_p->readDate != 0 )
    {
        added = FALSE;
    }
    else if( echo_p->readDate != 0 && tempEcho_p->readDate == 0 )
    {
        qInsertBefore( temp_q, tempEcho_p, echo_p );
        added = TRUE;
    }
    else if( echo_p->readDate == tempEcho_p->readDate )
    {
        added = ListSortPatName( PMC_SORT_NAME_ASCENDING, temp_q, echo_p, tempEcho_p );
    }
    else if( sortMode == PMC_SORT_DATE_DESCENDING )
    {
        if( echo_p->readDate > tempEcho_p->readDate )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    else
    {
       if( echo_p->readDate < tempEcho_p->readDate )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    return added;
}

//---------------------------------------------------------------------------
// Function: ListSortStudyDate
//---------------------------------------------------------------------------
// Update a single item in the list view
//---------------------------------------------------------------------------

bool __fastcall  TEchoListForm::ListSortStudyDate
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcEchoListStruct_p         echo_p,
    pmcEchoListStruct_p         tempEcho_p
)
{
    Boolean_t   added = FALSE;

    if( echo_p->date == 0 && tempEcho_p->date != 0 )
    {
        added = FALSE;
    }
    else if( echo_p->date != 0 && tempEcho_p->date == 0 )
    {
        qInsertBefore( temp_q, tempEcho_p, echo_p );
        added = TRUE;
    }
    else if( echo_p->date == tempEcho_p->date )
    {
        added = ListSortPatName( PMC_SORT_NAME_ASCENDING, temp_q, echo_p, tempEcho_p );
    }
    else if( sortMode == PMC_SORT_STUDY_DATE_DESCENDING )
    {
        if( echo_p->date > tempEcho_p->date )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    else
    {
       if( echo_p->date < tempEcho_p->date )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    return added;
}

//---------------------------------------------------------------------------
// Function: ListSortPatName
//---------------------------------------------------------------------------
// Update a single item in the list view
//---------------------------------------------------------------------------

bool __fastcall  TEchoListForm::ListSortPatName
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcEchoListStruct_p         echo_p,
    pmcEchoListStruct_p         tempEcho_p
)
{
    bool        added = FALSE;
    Int32s_t    result;
    Ints_t      len1, len2;

    len1 = strlen( echo_p->patSortName_p );
    len2 = strlen( tempEcho_p->patSortName_p );

    // Dont add if no name
    if( len1 == 0 && len2 != 0 )
    {
        // No name, do not add
        goto exit;
    }

    if( len1 == 0 && len2 == 0 )
    {
        // Both names are blank... sort by id
        added = ListSortId( PMC_SORT_ID_DESCENDING, temp_q, echo_p, tempEcho_p );
        goto exit;
    }

    if( len1 != 0 && len2 == 0 )
    {
        qInsertBefore( temp_q, tempEcho_p, echo_p );
        added = TRUE;
        goto exit;
    }

    // Compare the strings
    result = strcmp( echo_p->patSortName_p, tempEcho_p->patSortName_p );

    if( result == 0  )
    {
        // Names are the same... sort by ID
        added = ListSortId( PMC_SORT_ID_DESCENDING, temp_q, echo_p, tempEcho_p );
    }
    else if( sortMode == PMC_SORT_NAME_ASCENDING )
    {
        if( result < 0 )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_NAME_DESCENDING )
    {
        if( result > 0 )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
exit:
    return added;
}

//---------------------------------------------------------------------------
// Function: ListSortStudyName
//---------------------------------------------------------------------------
// Update a single item in the list view
//---------------------------------------------------------------------------

bool __fastcall  TEchoListForm::ListSortStudyName
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcEchoListStruct_p         echo_p,
    pmcEchoListStruct_p         tempEcho_p
)
{
    bool        added = FALSE;
    Int32s_t    result;
    Ints_t      len1, len2;

    len1 = strlen( echo_p->studySortName_p );
    len2 = strlen( tempEcho_p->studySortName_p );

    // Dont add if no name
    if( len1 == 0 && len2 != 0 )
    {
        // No name, do not add
        goto exit;
    }

    if( len1 == 0 && len2 == 0 )
    {
        // Both names are blank... sort by id
        added = ListSortId( PMC_SORT_ID_DESCENDING, temp_q, echo_p, tempEcho_p );
        goto exit;
    }

    if( len1 != 0 && len2 == 0 )
    {
        qInsertBefore( temp_q, tempEcho_p, echo_p );
        added = TRUE;
        goto exit;
    }

    // Compare the strings
    result = strcmp( echo_p->studySortName_p, tempEcho_p->studySortName_p );

    if( result == 0  )
    {
        // Names are the same... sort by ID
        added = ListSortId( PMC_SORT_ID_DESCENDING, temp_q, echo_p, tempEcho_p );
    }
    else if( sortMode == PMC_SORT_DESC_ASCENDING )
    {
        if( result < 0 )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_DESC_DESCENDING )
    {
        if( result > 0 )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
exit:
    return added;
}

//---------------------------------------------------------------------------
// Function: ListSortComment
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

bool __fastcall  TEchoListForm::ListSortComment
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcEchoListStruct_p         echo_p,
    pmcEchoListStruct_p         tempEcho_p
)
{
    bool        added = FALSE;
    Int32s_t    result;
    Ints_t      len1, len2;

    len1 = strlen( echo_p->commentSort_p );
    len2 = strlen( tempEcho_p->commentSort_p );

    // Dont add if no name
    if( len1 == 0 && len2 != 0 )
    {
        // No comment, do not add
        goto exit;
    }

    if( len1 == 0 && len2 == 0 )
    {
        // Both comment are blank... sort by name
        added = ListSortId( PMC_SORT_DESC_DESCENDING, temp_q, echo_p, tempEcho_p );
        goto exit;
    }

    if( len1 != 0 && len2 == 0 )
    {
        qInsertBefore( temp_q, tempEcho_p, echo_p );
        added = TRUE;
        goto exit;
    }

    // Compare the strings
    result = strcmp( echo_p->commentSort_p, tempEcho_p->commentSort_p );

    if( result == 0  )
    {
        // Names are the same... sort by ID
        added = ListSortId( PMC_SORT_DESC_DESCENDING, temp_q, echo_p, tempEcho_p );
    }
    else if( sortMode == PMC_SORT_COMMENT_ASCENDING )
    {
        if( result < 0 )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_COMMENT_DESCENDING )
    {
        if( result > 0 )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
exit:
    return added;
}

//---------------------------------------------------------------------------
// Function: ListSortId
//---------------------------------------------------------------------------
// Update a single item in the list view
//---------------------------------------------------------------------------

bool __fastcall  TEchoListForm::ListSortId
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcEchoListStruct_p         echo_p,
    pmcEchoListStruct_p         tempEcho_p
)
{
    Boolean_t   added = FALSE;

    if( sortMode == PMC_SORT_ID_DESCENDING )
    {
        if( echo_p->id > tempEcho_p->id )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    else
    {
       if( echo_p->id < tempEcho_p->id )
        {
            qInsertBefore( temp_q, tempEcho_p, echo_p );
            added = TRUE;
        }
    }
    return added;
}

//---------------------------------------------------------------------------
// Function: ListItemUpdate
//---------------------------------------------------------------------------
// Update a single item in the list view
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListItemUpdate( pmcEchoListStruct_p echo_p )
{
    TListItem                  *item_p;
    Char_p                      buf_p;
    MbDateTime                  dateTime;
    Int32u_t                    imageIndex;

    mbMalloc( buf_p, 1024 );

    if( echo_p->item_p == NIL ) goto exit;
    item_p = echo_p->item_p;

    item_p->SubItems->Clear( );

    // Study Name
    item_p->SubItems->Add( echo_p->name_p );

    // Patient Name
    item_p->SubItems->Add( echo_p->patientName_p );

#if OBSOLETE
    *buf_p = 0;
    if( echo_p->patientId )
    {
        imageIndex = PMC_MAIN_COLOR_SQUARE_INDEX_GREEN;
    }
    else
    {
        imageIndex = PMC_MAIN_COLOR_SQUARE_INDEX_RED;
    }
    // item_p->SubItemImages[PMC_ECHO_LIST_SUBITEM_INDEX_PATIENT] = imageIndex;
#endif

    // Study Date
    if( echo_p->date != 0 )
    {
        dateTime.SetDate( echo_p->date );
        item_p->SubItems->Add( dateTime.MDY_DateString( ) );
    }
    else
    {
        item_p->SubItems->Add( "" );
    }

    // Reader
    *buf_p = 0;
    if( echo_p->readDate != 0 )
    {
        sprintf( buf_p, "%s", echo_p->reader_p );
    }
    item_p->SubItems->Add( buf_p );

    // Online indicator
    *buf_p = 0;
    item_p->SubItems->Add( buf_p );
    if( echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
//        item_p->SubItemImages[PMC_ECHO_LIST_SUBITEM_INDEX_ONLINE] = PMC_MAIN_COLOR_SQUARE_INDEX_WHITE;
    }
    else
    {
        item_p->SubItemImages[PMC_ECHO_LIST_SUBITEM_INDEX_ONLINE] = echo_p->onlineFlag ?
            PMC_COLORSQUARE_INDEX_GREEN : PMC_COLORSQUARE_INDEX_RED;
    }

    // Backup indicator
    *buf_p = 0;
    item_p->SubItems->Add( buf_p );

    if( echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
//        item_p->SubItemImages[PMC_ECHO_LIST_SUBITEM_INDEX_] = PMC_MAIN_COLOR_SQUARE_INDEX_WHITE;
    }
    else
    {
        item_p->SubItemImages[PMC_ECHO_LIST_SUBITEM_INDEX_BACKUP] = echo_p->backupFlag ?
            PMC_COLORSQUARE_INDEX_GREEN : PMC_COLORSQUARE_INDEX_RED;
    }
    // Echo ID
    sprintf( buf_p, "%lu", echo_p->id );
    item_p->SubItems->Add( buf_p );

    sprintf( buf_p, "%s", echo_p->sonographer_p );
    item_p->SubItems->Add( buf_p );

    // Description - Comment
    sprintf( buf_p, "%s", echo_p->comment_p );
    item_p->SubItems->Add( buf_p );

    // Pointer back to queue entry
    item_p->Data = (Void_p)echo_p;

    // Do caption last... will cause an update
    *buf_p = 0;

    if( echo_p->readDate != PMC_ECHO_STATUS_NEW )
    {
        if( echo_p->readDate > PMC_ECHO_STATUS_MAX )
        {
            dateTime.SetDate( echo_p->readDate );
            sprintf( buf_p, dateTime.MDY_DateString( ) );
            imageIndex = PMC_COLORSQUARE_INDEX_GREEN;
        }
        else if( echo_p->readDate == PMC_ECHO_STATUS_IN_PROGRESS )
        {
            imageIndex = PMC_COLORSQUARE_INDEX_YELLOW;
            sprintf( buf_p, "In progress" );
        }
        else if( echo_p->readDate == PMC_ECHO_STATUS_PENDING )
        {
            imageIndex = PMC_COLORSQUARE_INDEX_BLUE;
            sprintf( buf_p, "Pending" );
        }
        else if( echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
        {
            imageIndex = PMC_COLORSQUARE_INDEX_NAVY;
            sprintf( buf_p, "No Echo" );
        }
        else
        {
            imageIndex = PMC_COLORSQUARE_INDEX_RED;
            sprintf( buf_p, "Unknown" );
        }
    }
    else
    {
        imageIndex = PMC_COLORSQUARE_INDEX_RED;
        sprintf( buf_p, "New" );
    }
    item_p->Caption = buf_p;
    item_p->ImageIndex = imageIndex;

exit:

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function: ListFree
//---------------------------------------------------------------------------
// Free the list of echos
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListFree( qHead_p echo_q )
{
    pmcEchoListStruct_p echo_p;

    if( echo_q == NIL ) return;
    while( !qEmpty( echo_q ) )
    {
        echo_p = (pmcEchoListStruct_p)qRemoveFirst( echo_q );
        ListFreeEntry( echo_p );
    }
}

//---------------------------------------------------------------------------
// Function: ListFreeEntry
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ListFreeEntry( pmcEchoListStruct_p echo_p )
{
    if( echo_p == NIL ) return;

    mbFree( echo_p->name_p );
    mbFree( echo_p->reader_p );
    mbFree( echo_p->sonographer_p );
    mbFree( echo_p->patientName_p );
    mbFree( echo_p->patSortName_p );
    mbFree( echo_p->studySortName_p );
    mbFree( echo_p->comment_p );
    mbFree( echo_p->commentSort_p );
    mbFree( echo_p );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TEchoListForm::CloseButtonClick(TObject *Sender)
{
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TEchoListForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    pmcCDList_p     cd_p;

    ListFree( Echo_q );

    // Save the sort mode for next time
    pmcEchoSortMode             = SortMode;
    pmcEchoSortReadDateMode     = SortReadDateMode;
    pmcEchoSortStudyNameMode    = SortStudyNameMode;
    pmcEchoSortPatNameMode      = SortPatNameMode;
    pmcEchoSortStudyDateMode    = SortStudyDateMode;
    pmcEchoSortIddMode          = SortIdMode;
    pmcEchoSortCommentMode      = SortCommentMode;

    // pmcEchoReadBackItemIndex    = ReadBackComboBox->ItemIndex;

    // 20041229: Delete the "ignore queue"
    while( !qEmpty( Ignore_q ) )
    {
        cd_p = (pmcCDList_p)qRemoveFirst( Ignore_q );
        mbFree( cd_p );
    }

    mbPropertyWinSave( PMC_WINID_ECHO_LIST, Height, Width, Top, Left );

    Action = caFree;
}

//---------------------------------------------------------------------------
// Function: DatabaseCheckTimerTimer
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::DatabaseCheckTimerTimer(TObject *Sender)
{
    static Boolean_t    running = FALSE;
    Int64u_t            modifyTime;

    if( running ) return;
    if( GetInProgress ) return;
    if( PopupInProgress ) return;

    running = TRUE;

    if( TableReadTime != 0 || TableReadSize != 0 )
    {
        modifyTime = pmcPollTableModifyTime[ PMC_TABLE_INDEX_ECHOS ];
        if(    TableReadTime <  modifyTime )
         // || TableReadSize != pmcPollTableSize[ PMC_TABLE_INDEX_ECHOS ] )
        {
            ListUpdate( FALSE );
        }
    }

    running = FALSE;

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------


void __fastcall TEchoListForm::EchoListPopupPopup(TObject *Sender)
{
    pmcEchoListStruct_p     echo_p;

    AutoOfflineFlag = FALSE;
    PopupInProgress++;
    pmcPopupItemEnableAll( EchoListPopup, FALSE );

    if( BackupInProgress ) goto exit;

    // Sanity Check
    if( UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgDebug(( "Busy" ));
        goto exit;
    }

    //Sanity Check
    if( PopupInProgress != 1 )
    {
        mbDlgDebug(( "Error: PopupInProgress: %d\n", PopupInProgress ));
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;


    if( echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_DETAILS,     TRUE );
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_PREVIEW,     TRUE );
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_DELETE,      TRUE );
        goto exit;
    }

    pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_COMMENT,     TRUE );
    pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_RENAME,      TRUE );
    pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_PATIENT_SET, TRUE );
    pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_DETAILS,     TRUE );
    pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_VIEW,        TRUE );

    if( echo_p->patientId )
    {
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_PATIENT_CLEAR,   TRUE );
    }

    if( echo_p->readDate > PMC_ECHO_STATUS_MAX )
    {
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_MARK_AS_NOT_READ, TRUE );
    }
    else
    {
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_MARK_AS_READ,    TRUE );
    }

    if(  echo_p->readDate > PMC_ECHO_STATUS_NEW )
    {
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_PREVIEW,     TRUE );
    }

    pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_BACKUP_CD_LIST_GET,  TRUE );

    // MAB:20061209: Backups may not be on CD
    // pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_BACKUP_VERIFY,       TRUE );

    if( echo_p->onlineFlag )
    {
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_BACKUP,              TRUE );
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_BACKUP_DATABASE,     TRUE );
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_BACKUP_NON_DATABASE, TRUE );

#if 0  // MAB:20061209: Do not allow making an echo offline
        if( echo_p->readDate != 0 && echo_p->backupFlag )
        {
            pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_MAKE_OFFLINE,    TRUE );
        }
#endif
    }
    else
    {
        pmcPopupItemEnable( EchoListPopup, PMC_ECHO_LIST_POPUP_MAKE_ONLINE ,        TRUE );
    }

exit:

    PopupInProgress--;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupPatientSetClick( TObject *Sender)
{
    EchoListPopupPatientEdit( TRUE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupPatientClearClick( TObject *Sender)
{
    EchoListPopupPatientEdit( FALSE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupPatientEdit( Boolean_t patPromptFlag )
{
    pmcEchoListStruct_p     echo_p;
    Int32u_t                patientId = 0;
    TPatientListForm       *patListForm_p;
    PmcPatListInfo_t        patListInfo;
    Boolean_t               gotLock = FALSE;

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( pmcSqlEchoLock( echo_p->id, echo_p->name_p ) == FALSE ) goto exit;
    gotLock = TRUE;

    if( patPromptFlag )
    {
        // Now put up the patient list form
        patListInfo.patientId    = echo_p->patientId;
        patListInfo.providerId   = 0;
        patListInfo.mode         = PMC_LIST_MODE_SELECT;
        patListForm_p = new TPatientListForm( this, &patListInfo );
        patListForm_p->ShowModal( );
        delete patListForm_p;

        if( patListInfo.returnCode == MB_BUTTON_OK && patListInfo.patientId != echo_p->patientId )
        {
            patientId = patListInfo.patientId;
        }
        else
        {
            goto exit;
        }
    }

    // Do the update
    pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_FIELD_PATIENT_ID, patientId, echo_p->id );

    mbLog( "Update echo %lu set patient %lu\n", echo_p->id, patientId );
exit:

    if( gotLock ) pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, echo_p->id );
    PopupInProgress--;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListViewChange(TObject *Sender,
      TListItem *Item, TItemChange Change)
{
    pmcEchoListStruct_p     echo_p;
    Char_t                  buf[64];

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    SelectedEchoId = echo_p->id;
    SelectedLabel->Caption = echo_p->name_p;
    EchoSizeLabel->Caption = mbStrInt32u( echo_p->size, buf );
    BytesDoneLabel->Caption = "";
    ProgressGauge->Progress = 0;

exit:
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupMarkAsReadClick(
      TObject *Sender)
{
    EchoListPopupReadEdit( TRUE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupMarkAsNotReadClick(
      TObject *Sender)
{
    EchoListPopupReadEdit( FALSE );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupReadEdit( Boolean_t readFlag )
{
    pmcEchoListStruct_p     echo_p;
    Int32u_t                readDate = 0;
    Boolean_t               gotLock = FALSE;

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( pmcSqlEchoLock( echo_p->id, echo_p->name_p ) == FALSE ) goto exit;
    gotLock = TRUE;

    if( readFlag )
    {
        TDateSelectForm    *dateSelectForm_p;
        Char_t              string[128];
        pmcDateSelectInfo_t dateSelectInfo;

        dateSelectInfo.mode      = PMC_DATE_SELECT_PARMS;
        dateSelectInfo.string_p  = string;
        dateSelectInfo.dateIn    = ( echo_p->readDate > PMC_ECHO_STATUS_MAX ) ? echo_p->readDate : mbToday( );
        dateSelectInfo.caption_p = NIL;

        dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );

        dateSelectForm_p->ShowModal( );
        delete dateSelectForm_p;

        if( dateSelectInfo.returnCode == MB_BUTTON_OK && dateSelectInfo.dateOut != echo_p->readDate )
        {
            readDate = dateSelectInfo.dateOut;
            if( readDate > mbToday() )
            {
                if( mbDlgYesNo( "The selected date is in the future.  Use this date anyway?" ) == MB_BUTTON_NO ) goto exit;
            }
        }
        else
        {
            goto exit;
        }
    }

    // Do the update
    pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_READ_DATE, readDate, echo_p->id );

    if( readFlag )
    {
        ReadCount++;
        NotReadCount--;
    }
    else
    {
        NotReadCount++;
        ReadCount--;
    }
    CountsUpdate( );

    // Set flag indicating if list should be refiltered
    if(  readFlag &&  ReadCheckBox->Checked    == FALSE ) FilterRequired = TRUE;
    if( !readFlag &&  NotReadCheckBox->Checked == FALSE ) FilterRequired = TRUE;

    mbLog( "Update echo %lu read date %lu\n", echo_p->id, readDate );

exit:

    if( gotLock ) pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, echo_p->id );

    PopupInProgress--;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListViewDblClick(TObject *Sender)
{
    ViewEcho( TRUE );
}

void __fastcall TEchoListForm::ViewEcho( Boolean_t dblClickFlag )
{

    pmcEchoListStruct_p     echo_p;
//    Boolean_t               gotLock = FALSE;
    Char_p                  cmd_p;
    Char_p                  source_p;
    Char_p                  target_p;
    Ints_t                  startWidth = 0;
    Ints_t                  endWidth = 0;
    Boolean_t               editComment = FALSE;
    TCursor                 cursorOrig;

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    mbMalloc( source_p, 512 );
    mbMalloc( target_p, 512 );
    mbMalloc( cmd_p, 2048 );

    PopupInProgress++;

    if( BackupInProgress ) goto exit;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;


    // Look for a double click on the comment column
    for( Ints_t i = 0 ; i < EchoListView->Columns->Count ; i++ )
    {
        endWidth += EchoListView->Columns->Items[i]->Width;
        if( EchoListView->Columns->Items[i]->Caption == "Comment" )
        {
            if( MouseX >= startWidth && MouseX <= endWidth )
            {
                editComment = TRUE;
                break;
            }
        }
        startWidth = endWidth;
    }

    if( editComment && dblClickFlag == TRUE )
    {
        PopupInProgress--;
//        EditComment( );
        pmcEchoDetailsForm( echo_p->id );
        PopupInProgress++;
        goto exit;
    }

    if( !pmcCfg[CFG_ECHO_VIEWER].str_p )
    {
        mbDlgInfo( "No echo viewer configured on this computer." );
        goto exit;
    }

    if( !pmcCfg[CFG_ECHO_PATH].str_p )
    {
        mbDlgInfo( "No path to echos" );
        goto exit;
    }

    // Check if online
    if( echo_p->onlineFlag == FALSE )
    {
        mbDlgInfo( "%s\n\n This echo is offine and cannot be viewed.", echo_p->name_p );
        goto exit;
    }

#if 0
    if( echo_p->readDate )
    {
        Screen->Cursor = cursorOrig;
        sprintf( cmd_p, "%s\n\nThis echo has been marked as read.\nView it anyway?", echo_p->name_p );
        if( mbDlgYesNo( cmd_p ) == MB_BUTTON_NO )
        {
            goto exit;
        }
        else
        {
            cursorOrig = Screen->Cursor;
            Screen->Cursor = crHourGlass;
       }
    }
#endif


//    if( pmcSqlEchoLock( echo_p->id, echo_p->name_p ) == FALSE ) goto exit;
//    gotLock = TRUE;

    if( pmcCfg[CFG_DATABASE_LOCAL].value == TRUE )
    {
        // temporary section to copy echo to the desktop
  //      Int32u_t        fileCount;
        Int32u_t        count;
        Char_p          name_p;
        Char_p          orig_p;
        Char_p          path_p;
 //       TThermometer   *thermometer_p = NIL;
        MbSQL           sql;

        mbMalloc( name_p, 256 );
        mbMalloc( orig_p, 256 );
        mbMalloc( path_p, 256 );

//        sprintf( cmd_p, "select %s from %s where %s=%lu"
//                            ,PMC_SQL_ECHO_FILES_FILED_COUNT
//                            ,PMC_SQL_TABLE_ECHO_FILES
//                            ,PMC_SQL_ECHO_FILES_FIELD_ECHO_ID
//                            ,echo_p->id );

//        fileCount = pmcSqlSelectInt( cmd_p, NIL );

//        sprintf( source_p, "Loading echo: %s", echo_p->name_p );
//        thermometer_p = new TThermometer( source_p, 0, fileCount, FALSE );

        sprintf( cmd_p, "select %s,%s,%s from %s where %s=%lu"
                            ,PMC_SQL_ECHO_FILES_FIELD_NAME
                            ,PMC_SQL_ECHO_FILES_FIELD_NAME_ORIG
                            ,PMC_SQL_ECHO_FILES_FIELD_PATH
                            ,PMC_SQL_TABLE_ECHO_FILES
                            ,PMC_SQL_ECHO_FILES_FIELD_ECHO_ID
                            ,echo_p->id );
        count = 0;
        sql.Query( cmd_p );
        while( sql.RowGet( ) )
        {
 //           thermometer_p->Increment( );

            strcpy( name_p, sql.String( 0 ) );
            strcpy( orig_p, sql.String( 1 ) );

            mbFileNamePathStrip( orig_p, cmd_p );

            strcpy( path_p, sql.String( 2 ) );

            // Make target
            sprintf( target_p, "%s\\%s\\%s", pmcCfg[CFG_DESKTOP].str_p, echo_p->name_p, cmd_p );
            mbStrBackslash( target_p, NIL, TRUE );

            // Make source
  //          sprintf( source_p, "%s\\%s\\%s",  pmcCfg[CFG_ECHO_IMPORT_TARGET].str_p, path_p, name_p );
            sprintf( source_p, "%s\\%s",  pmcCfg[CFG_ECHO_PATH].str_p, path_p );
            mbStrBackslash( source_p, NIL, TRUE );

            //mbDlgDebug(( "name: %s\norig: %s\npath: %starget: %s\nsource: %s\n",
            //    name_p, orig_p, path_p, target_p, source_p ));

 //           mbFileDirCheckCreate( target_p, NIL, TRUE, TRUE, FALSE );
 //           mbFileCopy( source_p, target_p );

            count++;
        }

 //       delete thermometer_p;

 //       if( count != fileCount )
 //       {
 //           mbDlgDebug(( "Error" ));
 //           goto exit;
  //      }

        mbFree( name_p );
        mbFree( path_p );
        mbFree( orig_p );
    } // end of temp section

    {
        int i;
        strcpy( cmd_p, pmcCfg[CFG_ECHO_VIEWER].str_p );
        for( i = strlen( cmd_p ) - 1 ; *(cmd_p + i ) != MB_CHAR_BACKSLASH ; i-- )
        {
        }
        *(cmd_p + i) = 0;
    }

    getcwd( target_p, 256 );
    chdir( cmd_p );

    mbLog( "Calling %s for %s\n", pmcCfg[CFG_ECHO_VIEWER].str_p, echo_p->name_p );


    spawnle( P_NOWAIT,
                pmcCfg[CFG_ECHO_VIEWER].str_p,
                pmcCfg[CFG_ECHO_VIEWER].str_p,
                source_p,
                NIL,
                NULL, NULL );

    chdir( target_p );

#if 0
    // Delete temp files
    {
        sprintf( target_p, "%s\\%s", pmcCfg[CFG_DESKTOP].str_p, echo_p->name_p );
        mbStrBackslash( target_p, NIL, TRUE );

        qHead_t     fileQueue;
        qHead_p     file_q;

        file_q = qInitialize( &fileQueue );

        if( mbFileListGet( target_p, "*.*", file_q, TRUE ) != 0 )
        {
            // mbFileListLog( file_q );
            mbFileListDelete( file_q, TRUE );
        }
        mbFileListFree( file_q );
        RemoveDirectory( target_p );

    }
#endif

#if 0
    if( echo_p->readDate == 0 )
    {
        if( mbDlgYesNo( "%s\n\nMark this echo as read?", echo_p->name_p ) == MB_BUTTON_YES )
        {
            sprintf( cmd_p, "update echos set read_date = %lu,read_time = %lu where id = %ld\n", mbToday(), mbTime(), echo_p->id );
            mbLog( "Update echo %lu read date %lu\n", echo_p->id, mbToday() );
            pmcSqlExec( cmd_p );
        }
    }
#endif

exit:

//    if( gotLock ) pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, echo_p->id );

    Screen->Cursor = cursorOrig;

    PopupInProgress--;

    mbFree( cmd_p );
    mbFree( source_p );
    mbFree( target_p );
}

//---------------------------------------------------------------------------
// Function: EchoListViewColumnClick
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListViewColumnClick(TObject *Sender,
      TListColumn *Column)
{
    Boolean_t  sort = TRUE;
    if( UpdateInProgress || PopupInProgress || GetInProgress )
    {
        mbDlgDebug(( "Busy" ));
        return;
    }

    UpdateInProgress = TRUE;

    if( strcmp( Column->Caption.c_str(), "Read Status" ) == 0 )
    {
        SortReadDateMode = ( SortReadDateMode == PMC_SORT_DATE_ASCENDING ) ? PMC_SORT_DATE_DESCENDING : PMC_SORT_DATE_ASCENDING;
        SortMode = SortReadDateMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Study Name" ) == 0 )
    {
        SortStudyNameMode = ( SortStudyNameMode == PMC_SORT_DESC_ASCENDING ) ? PMC_SORT_DESC_DESCENDING : PMC_SORT_DESC_ASCENDING;
        SortMode = SortStudyNameMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Patient" ) == 0 )
    {
        SortPatNameMode = ( SortPatNameMode == PMC_SORT_NAME_ASCENDING ) ? PMC_SORT_NAME_DESCENDING : PMC_SORT_NAME_ASCENDING;
        SortMode = SortPatNameMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Date" ) == 0 )
    {
        SortStudyDateMode = ( SortStudyDateMode == PMC_SORT_STUDY_DATE_ASCENDING ) ? PMC_SORT_STUDY_DATE_DESCENDING : PMC_SORT_STUDY_DATE_ASCENDING;
        SortMode = SortStudyDateMode;
    }
    else if( strcmp( Column->Caption.c_str(), "ID" ) == 0 )
    {
        SortIdMode = ( SortIdMode == PMC_SORT_ID_ASCENDING ) ? PMC_SORT_ID_DESCENDING : PMC_SORT_ID_ASCENDING;
        SortMode = SortIdMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Comment" ) == 0 )
    {
        SortCommentMode = ( SortCommentMode == PMC_SORT_COMMENT_ASCENDING ) ? PMC_SORT_COMMENT_DESCENDING : PMC_SORT_COMMENT_ASCENDING;
        SortMode = SortCommentMode;
    }
    else
    {
        sort = FALSE;
    }

    if( sort )
    {
       SelectedEchoId = 0;
       ListSort( Echo_q );
    }

    UpdateInProgress = FALSE;
    return;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ReadCheckBoxClick(TObject *Sender)        { CheckBoxClick( PMC_ECHO_CHECK_BOX_READ       );}
void __fastcall TEchoListForm::NotReadCheckBoxClick(TObject *Sender)     { CheckBoxClick( PMC_ECHO_CHECK_BOX_NOT_READ   );}
void __fastcall TEchoListForm::OnlineCheckBoxClick(TObject *Sender)      { CheckBoxClick( PMC_ECHO_CHECK_BOX_ONLINE     );}
void __fastcall TEchoListForm::OfflineCheckBoxClick(TObject *Sender)     { CheckBoxClick( PMC_ECHO_CHECK_BOX_OFFLINE    );}
void __fastcall TEchoListForm::BackedUpCheckBoxClick(TObject *Sender)    { CheckBoxClick( PMC_ECHO_CHECK_BOX_BACKED_UP  );}
void __fastcall TEchoListForm::NotBackedUpCheckBoxClick(TObject *Sender) { CheckBoxClick( PMC_ECHO_CHECK_BOX_NOT_BACKED_UP);}

void __fastcall TEchoListForm::CheckBoxClick( Int32u_t  boxCode )
{
    static running = FALSE;

    if( !Active ) return;
    if( running ) return;
    running = TRUE;

    switch( boxCode )
    {
        case PMC_ECHO_CHECK_BOX_READ:
            if( ReadCheckBox->Checked == FALSE )        NotReadCheckBox->Checked     = TRUE;
            break;
        case PMC_ECHO_CHECK_BOX_NOT_READ:
            if( NotReadCheckBox->Checked == FALSE )     ReadCheckBox->Checked        = TRUE;
            break;
        case PMC_ECHO_CHECK_BOX_ONLINE:
            if( OnlineCheckBox->Checked == FALSE )      OfflineCheckBox->Checked     = TRUE;
            break;
        case PMC_ECHO_CHECK_BOX_OFFLINE:
            if( OfflineCheckBox->Checked == FALSE )     OnlineCheckBox->Checked      = TRUE;
            break;
        case PMC_ECHO_CHECK_BOX_BACKED_UP:
            if( BackedUpCheckBox->Checked == FALSE )    NotBackedUpCheckBox->Checked = TRUE;
            break;
        case PMC_ECHO_CHECK_BOX_NOT_BACKED_UP:
            if( NotBackedUpCheckBox->Checked == FALSE ) BackedUpCheckBox->Checked    = TRUE;
            break;
    }

    if( SaveCheckState )
    {
        pmcEchoFilterBackedUpChecked    = BackedUpCheckBox->Checked;
        pmcEchoFilterNotBackedUpChecked = NotBackedUpCheckBox->Checked;
        pmcEchoFilterOnlineChecked      = OnlineCheckBox->Checked;
        pmcEchoFilterOfflineChecked     = OfflineCheckBox->Checked;
        pmcEchoFilterReadChecked        = ReadCheckBox->Checked ;
        pmcEchoFilterNotReadChecked     = NotReadCheckBox->Checked;
    }

    SelectedEchoId = 0;
    ListFilter( Echo_q );

    running = FALSE;
}

//---------------------------------------------------------------------------
// Function: CountsUpdate
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::CountsUpdate( void )
{
    Char_p  buf_p;

    mbMalloc( buf_p, 128 );
    ReadCountLabel->Caption         = mbStrInt32u( ReadCount,       buf_p );
    NotReadCountLabel->Caption      = mbStrInt32u( NotReadCount,    buf_p );
    OnlineCountLabel->Caption       = mbStrInt32u( OnlineCount,     buf_p );
    OfflineCountLabel->Caption      = mbStrInt32u( OfflineCount,    buf_p );
    BackedUpCountLabel->Caption     = mbStrInt32u( BackedUpCount,   buf_p );
    NotBackedUpCountLabel->Caption  = mbStrInt32u( NotBackedUpCount,buf_p );
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function: ActionLabelUpdate
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ActionLabelUpdate( Int32u_t code )
{
    switch( code )
    {
        case PMC_ECHO_ACTION_VERIFY:
            ActionLabel->Caption = "Verify...";
            break;
        case PMC_ECHO_ACTION_BACKUP:
            ActionLabel->Caption = "Backup...";
            break;
        case PMC_ECHO_ACTION_NONE:
        default:
            ActionLabel->Caption = "";
            break;
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::SearchEditChange(TObject *Sender)
{
    if( Active )
    {
        ListFilter( Echo_q );
        SearchEdit->SetFocus( );
    }
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::SearchClearButtonClick(TObject *Sender)
{
    SearchEdit->Text = "";
    if( Active ) EchoListView->SetFocus( );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ReadBackComboBoxChange(TObject *Sender)
{
    //ReadBackComboBox->Text = ReadBackComboBox->Items[ReadBackComboBox->ItemIndex].Text;
    if( ReadBackComboBox->ItemIndex != ReadBackItemIndexOld )
    {
        ListGet( );
        ReadBackItemIndexOld = ReadBackComboBox->ItemIndex;
    }
    if( Active ) EchoListView->SetFocus( );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::PatientSelectRadioGroupClick(TObject *Sender)
{
    if( !Active ) return;

    if( PatientSelectRadioGroup->ItemIndex == 0 )
    {
        if( PatientIdOld != 0 )
        {
            PatientIdOld = 0;
            ListGet( );
        }
    }
    else
    {
        if( FormInfo_p->patientId )
        {
            if( PatientIdOld != FormInfo_p->patientId )
            {
                PatientIdOld = FormInfo_p->patientId;
                ListGet( );
            }
        }
        else
        {
            PatientSelectRadioGroup->ItemIndex = 0;
        }
    }
    EchoListView->SetFocus();
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::PatientList( Int32u_t key )
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId = 0;
    patListInfo.providerId = 0;
    patListInfo.mode = PMC_LIST_MODE_SELECT;
    patListInfo.character = key;
    patListInfo.allowGoto = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
        if( patListInfo.patientId != FormInfo_p->patientId )
        {
            FormInfo_p->patientId = patListInfo.patientId;
            PatientUpdate( FormInfo_p->patientId );
            if( PatientSelectRadioGroup->ItemIndex != 1 )
            {
                PatientSelectRadioGroup->ItemIndex = 1;
            }
            else
            {
                ListGet( );
            }
        }
    }
    else
    {
        // User pressed cancel button - do nothing
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::PatientListButtonClick(TObject *Sender)
{
    PatientList( 0 );
    EchoListView->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------


void __fastcall TEchoListForm::PatientEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    PatientList( Key );
    EchoListView->SetFocus();
}

//---------------------------------------------------------------------------
// Function: PatientUpdate
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::PatientUpdate
(
    Int32u_t    patientId
)
{
    Char_p          buf_p;
    Char_p          buf2_p;
    PmcSqlPatient_p pat_p;

    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );
    mbMalloc( buf_p,       128 );
    mbMalloc( buf2_p,       64 );

    sprintf( buf_p, "" );
    sprintf( buf2_p, "" );

    if( patientId )
    {
        if( pmcSqlPatientDetailsGet( patientId, pat_p ) == TRUE )
        {
            sprintf( buf_p, "" );
            if( strlen( pat_p->lastName ) )
            {
                sprintf( buf2_p, "%s, ", pat_p->lastName );
                strcat( buf_p, buf2_p );
            }
            if( strlen( pat_p->firstName ) )
            {
                sprintf( buf2_p, "%s ", pat_p->firstName );
                strcat( buf_p, buf2_p );
            }
            if( strlen( pat_p->title ) )
            {
                sprintf( buf2_p, "(%s)", pat_p->title );
                strcat( buf_p, buf2_p );
            }
        }
        else
        {
            sprintf( buf_p, "Unknown" );
        }
    }

    PatientEdit->Text = buf_p;

    mbFree( pat_p );
    mbFree( buf_p );
    mbFree( buf2_p );
}

//---------------------------------------------------------------------------
// Function: EchoListPopupBackupClick
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupBackupDatabaseClick(
      TObject *Sender)
{
    EchoListPopupBackupClick( TRUE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupBackupNonDatabaseClick(
      TObject *Sender)
{
    EchoListPopupBackupClick( FALSE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupBackupClick( Int32u_t databaseFlag )
{
    pmcEchoListStruct_p     echo_p;
    Boolean_t               gotLock = FALSE;
    Int32s_t                result;
    Int64u_t                backupFreeSpace;

    PopupInProgress++;
    ActionLabelUpdate( PMC_ECHO_ACTION_BACKUP );

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( echo_p->onlineFlag == FALSE )
    {
        mbDlgInfo( "%s\n\nThis echo is offline and cannot be backed up.", echo_p->name_p );
        goto exit;
    }

    if( pmcSqlEchoLock( echo_p->id, echo_p->name_p ) == FALSE ) goto exit;
    gotLock = TRUE;

    // Give user a chance to back out
    BytesToProcess = echo_p->size;
    {
        Char_t buf[64];
        EchoSizeLabel->Caption = mbStrInt32u( BytesToProcess, buf );
    }

    // Set up the thermometer.
    BytesProcessed = 0;
    ProgressGauge->Progress = 0;
    BytesDoneLabel->Caption = "";

    if( mbDlgYesNo( "%s\n\nBackup this echo to a %s CD?", echo_p->name_p,
            databaseFlag ? "database" : "non-database" ) == MB_BUTTON_NO ) goto exit;

    ControlsStore( );
    ControlsDisable( );

    BackupInProgress = TRUE;

    result = pmcEchoBackup( echo_p->id,
                            FALSE,
                            &backupFreeSpace,
                            (Void_p)this,
                            CancelCheckCallback,
                            BytesProcessedCallback,
                            CDLabelCallback,
                            TRUE,
                            databaseFlag );

    ControlsRestore( );

    BackupInProgress = FALSE;

    if( result == MB_RET_OK )
    {
        mbDlgInfo( "%s\n\nBackup of this echo successfull.", echo_p->name_p );
    }
    else
    {
         mbDlgInfo( "%s\n\nBackup of this echo failed.", echo_p->name_p );
         BytesDoneLabel->Caption = "";
         ProgressGauge->Progress = 0;
    }

exit:

    // 20031222: Don't clear size when backup finished
    //EchoSizeLabel->Caption = "";

    if( gotLock ) pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, echo_p->id );

    PopupInProgress--;
    ActionLabelUpdate( PMC_ECHO_ACTION_NONE );
    return;
}

//---------------------------------------------------------------------------
// Function: CancelCheckCallback
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
Int32s_t TEchoListForm::CancelCheckCallback( Void_p this_p )
{
    TEchoListForm *form_p;

    Int32s_t    result = FALSE;
    if( this_p )
    {
        form_p = static_cast<TEchoListForm *>(this_p);
        result = form_p->CancelCheckCallbackReal( );
    }
    return result;
}

//---------------------------------------------------------------------------
// Fuction: CancelCheckCallbackReal
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoListForm::CancelCheckCallbackReal( )
{
    Int32s_t    returnCode = FALSE;
    MSG         msg;
    Boolean_t   processed = FALSE;
    Int32s_t    startMsg = 0;
    Int32s_t    endMsg = 0;

    for( ; ; )
    {
        if( !PeekMessage( &msg, NULL, startMsg, endMsg, PM_REMOVE ) ) break;

        if( !processed )
        {
            TranslateMessage( (LPMSG)&msg );
            DispatchMessage(  (LPMSG)&msg );
        }
    }

    if( CancelButtonClicked == TRUE )
    {
        returnCode = TRUE;
    }
    CancelButtonClicked = FALSE;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: BytesCopiedCallback
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
Int32s_t TEchoListForm::BytesProcessedCallback( Void_p this_p, Int32u_t bytes )
{
    TEchoListForm *form_p;

    Int32s_t    returnCode = MB_RET_ERR;
    if( this_p )
    {
        form_p = static_cast<TEchoListForm *>(this_p);
        returnCode = form_p->BytesProcessedCallbackReal( bytes );
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: BytesCopiedCallbackReal
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t __fastcall TEchoListForm::BytesProcessedCallbackReal( Int32u_t bytes )
{
    Int64u_t    percent;
    Ints_t      progress;
    Char_t      buf[64];

    BytesProcessed += bytes;
    progress = 0;

    if( BytesToProcess > 0 )
    {
        if( BytesProcessed >= BytesToProcess )
        {
            progress = 100;
        }
        else
        {
            percent = ( (Int64u_t)BytesToProcess - (Int64u_t)BytesProcessed );
            percent *= (Int64u_t)100;
            percent /= (Int64u_t)BytesToProcess;
            progress = 100 - (Ints_t)percent;
            if( progress == 100 &&  BytesProcessed < BytesToProcess ) progress = 99;
        }
    }
    ProgressGauge->Progress = progress;

    BytesDoneLabel->Caption = mbStrInt32u( BytesProcessed, buf );

    // Only update bytes free if a backup is in progress
    if( ActinCode == PMC_ECHO_ACTION_BACKUP )
    {
        if( BytesFree > bytes )
        {
            BytesFree -= bytes;
        }
        BytesFreeLabel->Caption = mbStrInt32u( BytesFree, buf );
    }

    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// Function: CDLabelCallback
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t TEchoListForm::CDLabelCallback( Void_p this_p, Char_p str_p, Int32u_t bytesFree )
{
    TEchoListForm *form_p;

    Int32s_t    returnCode = MB_RET_OK;
    if( this_p )
    {
        form_p = static_cast<TEchoListForm *>(this_p);
        form_p->CDLabelUpdate( str_p, bytesFree );
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// CDLabelUpdate
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::CDLabelUpdate( Char_p str_p, Int32u_t bytesFree )
{
    Char_t  buf[64];
    CDIdLabel->Caption = str_p;
    BytesFreeLabel->Caption = mbStrInt32u( bytesFree, buf );
    BytesFree = bytesFree;

}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EjectButtonClick(TObject *Sender)
{
    mbDriveOpen( TRUE, pmcCfg[CFG_CD_BURNER].str_p );
    CDIdLabel->Caption = "";
    BytesFreeLabel->Caption = "";

}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::LoadButtonClick(TObject *Sender)
{
    TCursor     origCursor;
    Int64u_t    freeSpace;
    Int32u_t    failedCount = 0;
    Int32s_t    result;
    Int32u_t    diskId = 0;

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    for( ; ; )
    {
        Screen->Cursor = crHourGlass;

        // This call should block while the CD drive is not ready
        mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );
        result = mbDriveFreeSpace( pmcCfg[CFG_CD_BURNER].str_p, &freeSpace );

        // Only attempt to read disk if mbDriveFreeSpace returned no error
        if( result == MB_RET_OK )
        {
            if( failedCount) Sleep( 3000 );
            diskId = pmcEchoCDIDGet( NIL );
        }
        else
        {
            if( failedCount == 4 )
            {
                mbDlgExclaim( "Unable to read CD" );
                goto exit;
            }
            Sleep( 3000 );
            failedCount++;
            continue;
        }

        if( diskId )
        {
            if( freeSpace == 0 )
            {
                mbDlgInfo( "This CD is not writable and cannot be used for backups." );
            }
            result = MB_RET_OK;
            break;
        }

        if( freeSpace > 0 )
        {
            qHead_t cdQueue;
            qHead_p cdFile_q = qInitialize( &cdQueue );

            // Get a list of all files currently on the CD
            if( mbFileListGet( pmcCfg[CFG_CD_BURNER].str_p, "*.*", cdFile_q, TRUE ) == 0 )
            {
                result = mbDlgYesNo( "This CD is writeable and might be suitable for database backups.\n"
                                     "Check a different CD?" );
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
                            "The CD in the drive may not be an echo database CD,\n"
                            "or the CD drive may not be ready.\n\n"
                            "Retry?" ) == MB_BUTTON_NO ) goto exit;
        }

        Screen->Cursor = crHourGlass;

        mbDriveOpen( TRUE,  pmcCfg[CFG_CD_BURNER].str_p );
        result = mbDlgOkCancel( "Click OK once a CD has been placed in the drive." );

        Screen->Cursor = crHourGlass;
        mbDriveOpen( FALSE, pmcCfg[CFG_CD_BURNER].str_p );

        if( result == MB_BUTTON_CANCEL )
        {
            result = MB_RET_ERR;
            Screen->Cursor = origCursor;
            goto exit;
        }
    }

exit:

    if( result == MB_RET_OK )
    {
        Char_t  buf[64];
        CDIdLabel->Caption = mbDriveLabel( pmcCfg[CFG_CD_BURNER].str_p, buf );
        BytesFree = (Int32u_t)freeSpace;
        BytesFreeLabel->Caption = mbStrInt32u( BytesFree, buf );
    }
    Screen->Cursor = origCursor;

}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupBackupCDListGetClick(
      TObject *Sender)
{
    Char_p                  buf1_p;
    Char_p                  buf2_p;
    pmcEchoListStruct_p     echo_p;
    qHead_t                 cdQueue;
    qHead_p                 cd_q;
    pmcCDList_p             cd_p;

    mbCalloc( buf1_p,  2048 );
    mbCalloc( buf2_p,  2048 );

    cd_q = qInitialize( &cdQueue );

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    // Get a list of the CDs this echo is backed up on
    pmcEchoCDListGet( cd_q, echo_p->id );

    qWalk( cd_p, cd_q, pmcCDList_p )
    {
        sprintf( buf1_p, "   " PMC_ECHO_CD_LABEL "\n", cd_p->id );
        strcat( buf2_p, buf1_p );
    }

    if( cd_q->size == 0 )
    {
        mbDlgInfo( "%s\n\nThis echo is not backed up on CD.", echo_p->name_p );

    }
    else
    {
        sprintf( buf1_p, "%s\n\nThis echo is backed up on CD%s:\n\n",
            echo_p->name_p,
            ( cd_q->size == 1 ) ? "" : "s" );
        strcat( buf1_p, buf2_p );
        mbDlgInfo( buf1_p );
    }

exit:
    pmcEchoCDListFree( cd_q );

    mbFree( buf1_p );
    mbFree( buf2_p );

    PopupInProgress--;
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupRenameClick(TObject *Sender)
{
    pmcEchoListStruct_p     echo_p;
    Boolean_t               gotLock = FALSE;
    TTextEditForm          *Form_p;
    pmcTextEditInfo_t       info;
    Char_p                  result_p;
    Char_p                  buf_p;

    mbMalloc( result_p, 128 );
    mbMalloc( buf_p, 1024 );

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( pmcSqlEchoLock( echo_p->id, echo_p->name_p ) == FALSE ) goto exit;
    gotLock = TRUE;

    info.caption_p = "Echo Name";
    info.text_p = echo_p->name_p;
    info.label_p = "Name";
    info.result_p = result_p;
    info.resultSize = 128;

    Form_p = new TTextEditForm( this, &info );
    Form_p->ShowModal( );
    delete Form_p;

    if( info.returnCode == MB_RET_OK )
    {
        if( strcmp( echo_p->name_p, result_p ) != 0 )
        {
            sprintf( buf_p, "update %s set %s=\"%s\" where %s=%ld",
                PMC_SQL_TABLE_ECHOS,
                PMC_SQL_FIELD_NAME,     result_p,
                PMC_SQL_FIELD_ID,       echo_p->id );

            pmcSqlExec( buf_p );
            mbLog( "Update echo %lu name '%s'\n", echo_p->id, result_p );
        }
    }

exit:
    mbFree( result_p );
    mbFree( buf_p );
    if( gotLock ) pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, echo_p->id );
    PopupInProgress--;
}

//---------------------------------------------------------------------------
// Function: CheckButtonClick
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::CheckButtonClick(TObject *Sender)
{
    pmcEchoCDContentsForm();
}

//---------------------------------------------------------------------------
// Function: pmcEchoListForm
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t pmcEchoListForm
(
    Int32u_t    patientId
)
{
    TEchoListForm          *form_p;
    pmcEchoListInfo_t       formInfo;

    formInfo.patientId = patientId;

    formInfo.readBack = ( patientId == 0 ) ? PMC_ECHO_BACK_MONTH_3 : PMC_ECHO_BACK_ALL;

    form_p = new TEchoListForm( NIL, &formInfo );

    form_p->ShowModal( );
    delete form_p;

    return MB_RET_OK;
}

void __fastcall TEchoListForm::FormResize(TObject *Sender)
{
#if 0
    Ints_t  heightDiff;
    Ints_t  widthDiff;

    if( Active ) return;

    heightDiff = Height - InitialHeight;
    widthDiff  = Width - InitialWidth;

    EchoListView->Width  += widthDiff;
    EchoListView->Height += heightDiff;

    CloseButton->Top  += heightDiff;
    CloseButton->Left += widthDiff;
#endif
}

//---------------------------------------------------------------------------
// Function: EchoListPopupBackupVerifyClick
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupBackupVerifyClick(
      TObject *Sender)
{
    pmcEchoListStruct_p     echo_p;
    Int32s_t                result;
    Int32u_t                cancelFlag;

    PopupInProgress++;
    ActionLabelUpdate( PMC_ECHO_ACTION_VERIFY );

    BytesProcessed = 0;
    ProgressGauge->Progress = 0;
    BytesDoneLabel->Caption = "";

    // First, must determine the echo ID
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    // Give user a chance to back out
    BytesToProcess = echo_p->size;
    {
        Char_t buf[64];
        EchoSizeLabel->Caption = mbStrInt32u( BytesToProcess, buf );
    }

    mbLog( "Echo backup verify: '%s'\n", echo_p->name_p );
    CancelButtonEnable( TRUE );
    result = pmcEchoBackupVerifyRestore
    (
        echo_p->id, 0, (Void_p)this,
        CancelCheckCallback,
        BytesProcessedCallback,
        CDLabelCallback,
        &cancelFlag, FALSE
    );
    CancelButtonEnable( FALSE );

    if( result == TRUE )
    {
        if( cancelFlag == TRUE )
        {
            mbDlgInfo( "%s\n\nVerification of backup cancelled.\n", echo_p->name_p );
        }
        else
        {
            mbDlgInfo( "%s\n\nBackup of this echo is good.\n", echo_p->name_p );
        }
    }
    else
    {
        //mbDlgExclaim( "Error verifying echo '%s' backup.\n", echo_p->name_p );
        // Ok, now check to see how many CDs this echo is backed up on

        result = pmcEchoBackupCount( echo_p->id );
        if( result == 0 )
        {
            if( echo_p->backupFlag == TRUE )
            {
                mbDlgExclaim( "%s\n\nThis echo is not backed up.\n", echo_p->name_p );
                echo_p->backupFlag = FALSE;
                FilterRequired = TRUE;
                pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_BACKUP, 0, echo_p->id );
                // Adjust the counts
                BackedUpCount--;
                NotBackedUpCount++;
                CountsUpdate( );
            }
        }
    }

exit:
    PopupInProgress--;
    ActionLabelUpdate( PMC_ECHO_ACTION_NONE );
    return;
}

//---------------------------------------------------------------------------
// Function: GetSelectedEcho
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

pmcEchoListStruct_p __fastcall TEchoListForm::GetSelectedEcho( void )
{
    TListItem              *item_p;
    pmcEchoListStruct_p     echo_p = NIL;

    item_p = EchoListView->Selected;
    if( item_p ) echo_p = (pmcEchoListStruct_p)item_p->Data;

    return echo_p;
}

void __fastcall TEchoListForm::CancelButtonClick(TObject *Sender)
{
    CancelButtonClicked = TRUE;
}

//---------------------------------------------------------------------------
// Function: CancelButtonEnable
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::CancelButtonEnable( Boolean_t enableFlag )
{
    CancelButtonClicked = FALSE;
    if( enableFlag )
    {
        ControlsStore( );
        ControlsDisable( );
        CancelButton->Enabled = TRUE;
    }
    else
    {

        ControlsRestore( );
        CancelButton->Enabled = FALSE;
    }
}

//---------------------------------------------------------------------------
// Function: ControlsStore
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ControlsStore( void )
{
    StoreEjectButtonEnabled          = EjectButton->Enabled         ;
    StoreLoadButtonEnabled           = LoadButton->Enabled          ;
    StoreCheckButtonEnabled          = CheckButton->Enabled         ;
    StoreReadCheckBoxEnabled         = ReadCheckBox->Enabled        ;
    StoreNotReadCheckBoxEnabled      = NotReadCheckBox->Enabled     ;
    StoreOnlineCheckBoxEnabled       = OnlineCheckBox->Enabled      ;
    StoreOfflineCheckBoxEnabled      = OfflineCheckBox->Enabled     ;
    StoreBackedUpCheckBoxEnabled     = BackedUpCheckBox->Enabled    ;
    StoreNotBackedUpCheckBoxEnabled  = NotBackedUpCheckBox->Enabled ;
    StorePatientListButtonEnabled    = PatientListButton->Enabled   ;
    StoreReadBackComboBoxEnabled     = ReadBackComboBox->Enabled    ;
    StoreSearchClearButtonEnabled    = SearchClearButton->Enabled   ;
    StoreSearchEditEnabled           = SearchEdit->Enabled          ;
    StoreCloseButtonEnabled          = CloseButton->Enabled         ;
    StorePatientEditEnabled          = PatientEdit->Enabled         ;
    StoreEchoListPanelEnabled        = EchoListPanel->Enabled       ;

    StorePatientSelectRadioGroupEnabled = PatientSelectRadioGroup->Enabled;

    StorePageControlEnabled          = PageControl->Enabled         ;
    StoreCDEditEnabled               = CDEdit->Enabled              ;
    StoreDisplayCDButtonEnabled      = DisplayCDButton->Enabled     ;
    StoreSuggestButtonEnabled        = SuggestButton->Enabled       ;
    StoreOfflineButtonEnabled        = OfflineButton->Enabled       ;
    return;
}

//---------------------------------------------------------------------------
// Function: ControlsRestore
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ControlsRestore( void )
{
    EjectButton->Enabled          = StoreEjectButtonEnabled         ;
    LoadButton->Enabled           = StoreLoadButtonEnabled          ;
    CheckButton->Enabled          = StoreCheckButtonEnabled         ;
    ReadCheckBox->Enabled         = StoreReadCheckBoxEnabled        ;
    NotReadCheckBox->Enabled      = StoreNotReadCheckBoxEnabled     ;
    OnlineCheckBox->Enabled       = StoreOnlineCheckBoxEnabled      ;
    OfflineCheckBox->Enabled      = StoreOfflineCheckBoxEnabled     ;
    BackedUpCheckBox->Enabled     = StoreBackedUpCheckBoxEnabled    ;
    NotBackedUpCheckBox->Enabled  = StoreNotBackedUpCheckBoxEnabled ;
    PatientListButton->Enabled    = StorePatientListButtonEnabled   ;
    ReadBackComboBox->Enabled     = StoreReadBackComboBoxEnabled    ;
    SearchClearButton->Enabled    = StoreSearchClearButtonEnabled   ;
    SearchEdit->Enabled           = StoreSearchEditEnabled          ;
    CloseButton->Enabled          = StoreCloseButtonEnabled         ;
    PatientEdit->Enabled          = StorePatientEditEnabled         ;
    EchoListPanel->Enabled        = StoreEchoListPanelEnabled       ;

    PatientSelectRadioGroup->Enabled = StorePatientSelectRadioGroupEnabled;

    PageControl->Enabled            = StorePageControlEnabled       ;
    CDEdit->Enabled                 = StoreCDEditEnabled            ;
    DisplayCDButton->Enabled        = StoreDisplayCDButtonEnabled   ;
    SuggestButton->Enabled          = StoreSuggestButtonEnabled     ;
    OfflineButton->Enabled          = StoreOfflineButtonEnabled     ;

    return;
}

//---------------------------------------------------------------------------
// Function: ControlsStore
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::ControlsDisable( void )
{
    EjectButton->Enabled          = FALSE;
    LoadButton->Enabled           = FALSE;
    CheckButton->Enabled          = FALSE;
    ReadCheckBox->Enabled         = FALSE;
    NotReadCheckBox->Enabled      = FALSE;
    OnlineCheckBox->Enabled       = FALSE;
    OfflineCheckBox->Enabled      = FALSE;
    BackedUpCheckBox->Enabled     = FALSE;
    NotBackedUpCheckBox->Enabled  = FALSE;
    PatientListButton->Enabled    = FALSE;
    ReadBackComboBox->Enabled     = FALSE;
    SearchClearButton->Enabled    = FALSE;
    SearchEdit->Enabled           = FALSE;
    CloseButton->Enabled          = FALSE;
    PatientEdit->Enabled          = FALSE;
    EchoListPanel->Enabled        = FALSE;

    PatientSelectRadioGroup->Enabled  = FALSE;

    PageControl->Enabled            = FALSE;
    CDEdit->Enabled                 = FALSE;
    DisplayCDButton->Enabled        = FALSE;
    OfflineButton->Enabled          = FALSE;
    SuggestButton->Enabled          = FALSE;

    return;
}

//---------------------------------------------------------------------------
// Function: EchoListPopupMakeOfflineClick
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupMakeOfflineClick( TObject *Sender )
{
    pmcEchoListStruct_p     echo_p;
    Int32s_t                result;
    Int32u_t                cancelFlag;

    PopupInProgress++;
    OfflineCanceled = FALSE;
    ActionLabelUpdate( PMC_ECHO_ACTION_VERIFY );

    BytesProcessed = 0;
    ProgressGauge->Progress = 0;
    BytesDoneLabel->Caption = "";

    // First, must determine the echo ID
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( echo_p->readDate == 0 )
    {
        mbDlgInfo( "%s\n\nThis echo has not been read and cannot be taken offline.\n", echo_p->name_p );
        goto exit;
    }

    mbLog( "Make echo offline: '%s'\n", echo_p->name_p );
    // Give user a chance to back out
    BytesToProcess = echo_p->size;
    {
        Char_t buf[64];
        EchoSizeLabel->Caption = mbStrInt32u( BytesToProcess, buf );
    }

    CancelButtonEnable( TRUE );
    result = pmcEchoBackupVerifyRestore
    (
        echo_p->id, 0, (Void_p)this,
        CancelCheckCallback,
        BytesProcessedCallback,
        CDLabelCallback,
        &cancelFlag, FALSE
    );
    CancelButtonEnable( FALSE );

    if( result == TRUE )
    {
        qHead_p         file_q;
        qHead_t         fileQueue;
        pmcEchoFile_p   file_p;

        if( cancelFlag == TRUE )
        {
            mbDlgInfo( "%s\n\nVerification of echo backup cancelled.\n"
                      "Echo will not be taken offline.", echo_p->name_p );
            OfflineCanceled = TRUE;
            goto exit;
        }

        if( AutoOfflineFlag == FALSE )
        {
            if( mbDlgYesNo( "%s\n\nThe backup of this echo is good.\n"
                            "Proceed with taking this echo offline?", echo_p->name_p ) == MB_BUTTON_NO )
            {
                goto exit;
            }
        }

        file_q = qInitialize( &fileQueue );

        pmcEchoFileListGet( echo_p->id, file_q, NIL );

        qWalk( file_p, file_q, pmcEchoFile_p )
        {
            mbLog( "Deleting file: %s\n", file_p->source_p );
            unlink( file_p->source_p );
        }
        pmcEchoFileListFree( file_q );

        echo_p->onlineFlag = FALSE;
        FilterRequired = TRUE;
        pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_ONLINE, 0, echo_p->id );

        // Adjust the counts
        OfflineCount++;
        OnlineCount--;
        CountsUpdate( );

        if( AutoOfflineFlag == FALSE )
        {
            mbDlgInfo( "%s\n\nThis echo is now offline.\n", echo_p->name_p );
        }
    }
    else
    {
        OfflineCanceled = TRUE;
        // Ok, now check to see how many CDs this echo is backed up on
        result = pmcEchoBackupCount( echo_p->id );
        if( result == 0 )
        {
            if( echo_p->backupFlag == TRUE )
            {
                mbDlgExclaim( "%s\n\nThis echo is not backed up.\n", echo_p->name_p );
                echo_p->backupFlag = FALSE;
                FilterRequired = TRUE;
                pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_BACKUP, 0, echo_p->id );
                // Adjust the counts
                BackedUpCount--;
                NotBackedUpCount++;
                CountsUpdate( );
            }
        }
    }

exit:
    PopupInProgress--;
    ActionLabelUpdate( PMC_ECHO_ACTION_NONE );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupMakeOnlineClick( TObject *Sender )
{
    pmcEchoListStruct_p     echo_p;
    Int32s_t                result;
    Int32u_t                cancelFlag;

    PopupInProgress++;
    ActionLabelUpdate( PMC_ECHO_ACTION_VERIFY );

    BytesProcessed = 0;
    ProgressGauge->Progress = 0;
    BytesDoneLabel->Caption = "";

    // First, must determine the echo ID
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( echo_p->onlineFlag == TRUE )
    {
        mbDlgExclaim( "%s\n\nThis echo is already online\n", echo_p->name_p );
        goto exit;
    }

    CancelButtonEnable( TRUE );
    CancelButton->Enabled = FALSE;
    result = pmcEchoBackupVerifyRestore
    (
        echo_p->id, 0, (Void_p)this,
        CancelCheckCallback,
        BytesProcessedCallback,
        CDLabelCallback,
        &cancelFlag, TRUE
    );
    CancelButtonEnable( FALSE );

    if( result != TRUE )
    {
        mbDlgExclaim( "%s\n\nAn error occured while restoring this echo.", echo_p->name_p );
    }
    else
    {
        echo_p->onlineFlag = FALSE;
        FilterRequired = TRUE;
        pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_ONLINE, 1, echo_p->id );

        // Adjust the counts
        OfflineCount--;
        OnlineCount++;
        CountsUpdate( );

        mbDlgInfo( "%s\n\nThis echo was successfully restored from backup.\n", echo_p->name_p );
    }

exit:
    PopupInProgress--;
    ActionLabelUpdate( PMC_ECHO_ACTION_NONE );

    return;
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::PageControlChange( TObject *Sender )
{
    OfflineButton->Enabled = FALSE;
    if( PageControl->ActivePage == TabSheetAll )
    {
        //mbDlgInfo( "Got 'all' tab sheet\n" );
        ListGet( );
    }
    else if( PageControl->ActivePage == TabSheetCD )
    {
        //mbDlgInfo( "Got 'CD' tab sheet\n" );
        DisplayCD( );
    }
    else
    {
        mbDlgExclaim( "Invalid tab sheet\n" );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::CDEditKeyDown( TObject *Sender, WORD &Key, TShiftState Shift )
{
    if( Key == MB_CHAR_CR )
    {
        //mbDlgInfo( "Got CR" );
        DisplayCD( );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::DisplayCDButtonClick( TObject *Sender )
{
    //mbDlgInfo( "Get List button clicked" );
    DisplayCD( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::DisplayCD( Void )
{
    Int32u_t    cdId;
    Char_p      buf_p;
    Int32u_t    count;
    Char_t      number[32];

    mbMalloc( buf_p, 1024 );

    strcpy( number, CDEdit->Text.c_str() );
    mbStrDigitsOnly( number );

    if( strlen( number ) == 0 )
    {
        //mbDlgInfo( "no cd id, skip display\n" );
        goto exit;
    }

    cdId = atoi( number );
    if( cdId == 0 )
    {
        CDEdit->Text = "";
        goto exit;
    }

    //mbDlgInfo( "want to display cd id: %d\n", cdId );

    // First check to see if this is a valid cd id
    sprintf( buf_p, "select %s from %s where %s=%ld",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_ECHO_CDS,
        PMC_SQL_FIELD_ID, cdId );

    pmcSqlSelectInt( buf_p, &count );
    if( count != 1 )
    {
        mbDlgInfo( "There is no Echo CD with ID#: %ld", cdId );
        CDEdit->Text = "";
        goto exit;
    }

    DisplayCDId = cdId;

    ListGet( );

    OfflineButton->Enabled = ( EchoListView->Items->Count ) ? TRUE : FALSE;

exit:
    mbFree( buf_p );
    CDEdit->SetFocus();
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// This function searches all the backup CDs to find the lowest numbered
// CD that contains echos that can be made offline (these are echos that
// are backed up and read).
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::SuggestButtonClick(TObject *Sender)
{
    qHead_t         queue;
    qHead_p         q_p = NIL;
    pmcCDList_p     cd_p;
    pmcCDList_p     cd2_p;
    TThermometer   *thermometer_p = NIL;
    Char_t          buf[256];
    Boolean_t       found = FALSE;
    Boolean_t       cancelled = FALSE;
    Boolean_t       stopped = FALSE;
    Boolean_t       ignore;
    Int32u_t        count;

    q_p = qInitialize( &queue );

    // Get a list of all CDs
    pmcEchoCDListGet( q_p, 0 );

    sprintf( buf, "Examining %ld echo CDs...", q_p->size );
    thermometer_p = new TThermometer( buf, 0, q_p->size, TRUE );

    // Loop through the list of CDs
    qWalk( cd_p, q_p, pmcCDList_p )
    {
        if( thermometer_p->Increment( ) == FALSE )
        {
            if( mbDlgYesNo( "Cancel check ?" ) == MB_BUTTON_YES )
            {
                cancelled = TRUE;
                break;
            }
        }

        // 20041229: Do we want to ignore this CD?
        ignore = false;
        qWalk( cd2_p, Ignore_q, pmcCDList_p )
        {
            if( cd_p->id == cd2_p->id )
            {
                ignore = true;
                break;
            }
        }
        if( ignore ) continue;

        sprintf( buf, "select %s.%s from %s,%s where %s.%s=%s.%s and %s.%s!=0 and %s.%s!=0 and %s.%s!=0 "
                      "and %s.%s=%ld and %s.%s=%d and %s.%s=%d"

            ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_ID
            ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_TABLE_ECHO_BACKUPS

            ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_ID, PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID
            ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_ONLINE
            ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_BACKUP
            ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_ECHOS_FIELD_READ_DATE
            ,PMC_SQL_TABLE_ECHOS        ,PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
            ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
            ,PMC_SQL_TABLE_ECHO_BACKUPS ,PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID, cd_p->id );

        pmcSqlSelectInt( buf, &count );

        if( count > 0 )
        {
            if( count == 1 )
            {
                sprintf( buf, "The CD ECHO-%04ld has 1 echo that can be marked offline.\n\n"
                              "Select this CD?", cd_p->id );
            }
            else
            {
                sprintf( buf, "The CD ECHO-%04ld has %ld echos that can be marked offline.\n\n"
                              "Select this CD?", cd_p->id, count );
            }
            if( mbDlgYesNo( buf ) == MB_BUTTON_YES )
            {
                found = TRUE;
                break;
            }
            else
            {
                mbMalloc( cd2_p, sizeof( pmcCDList_t ) );
                cd2_p->id = cd_p->id;
                qInsertLast( Ignore_q, cd2_p );

                if( mbDlgYesNo( "Stop examining echo CDs?" ) == MB_BUTTON_YES )
                {
                    stopped = TRUE;
                    break;
                }
            }
        }
    }

    if( thermometer_p ) delete thermometer_p;

    if( !cancelled )
    {
        if( !found  )
        {
            if( !stopped )
            {
                sprintf( buf, "Unable to suggest a CD with echos suitable for marking offline.\n" );
                mbDlgInfo( buf );
            }
        }
        else
        {
            sprintf( buf, "%ld", cd_p->id );
            CDEdit->Text = buf;
            DisplayCD( );
        }
    }
    pmcEchoCDListFree( q_p );
}

//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EditCommentClick(TObject *Sender)
{
    EditComment( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EditComment( void )
{
    pmcEchoListStruct_p     echo_p;
    TTextEditForm          *Form_p;
    pmcTextEditInfo_t       info;
    Char_p                  result_p = NIL;
    Char_p                  buf_p = NIL;
    Boolean_t               gotLock = FALSE;

    mbMalloc( result_p, 256 );
    mbMalloc( buf_p, 256 );

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( pmcSqlEchoLock( echo_p->id, echo_p->name_p ) == FALSE ) goto exit;
    gotLock = TRUE;

    sprintf( buf_p, "Echo: %s", echo_p->name_p );

    info.caption_p = buf_p;
    info.text_p = echo_p->comment_p;
    info.label_p = "Comment";
    info.result_p = result_p;
    info.resultSize = 256;

    Form_p = new TTextEditForm( this, &info );
    Form_p->ShowModal( );
    delete Form_p;

    if( info.returnCode == MB_RET_OK )
    {
        mbStrClean( result_p, NIL, TRUE );
        if( strcmp( echo_p->comment_p, result_p ) != 0 )
        {
            sprintf( buf_p, "update %s set %s=\"%s\" where %s=%ld",
                PMC_SQL_TABLE_ECHOS,
                PMC_SQL_ECHOS_FIELD_COMMENT,   result_p,
                PMC_SQL_FIELD_ID,             echo_p->id );

            pmcSqlExec( buf_p );
            mbLog( "Update echo %lu name '%s'\n", echo_p->id, result_p );
       }
    }

exit:
    mbFree( result_p );
    mbFree( buf_p );
    if( gotLock ) pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, echo_p->id );
    PopupInProgress--;
}
void __fastcall TEchoListForm::EchoListViewMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    MouseX = X;
    MouseY = Y;    
}
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::OfflineButtonClick(TObject *Sender)
{
    Ints_t      i;
    TListItem   *item_p;
    pmcEchoListStruct_p     echo_p;

    for( i = 0 ; i < EchoListView->Items->Count ; i++ )
    {
        item_p = EchoListView->Items->Item[i];
        EchoListView->Selected = item_p;
        echo_p = (pmcEchoListStruct_p)item_p->Data;

        //mbDlgInfo( "Echo '%s': online: %s backed up: %s\n",
        //    echo_p->name_p,
        //    echo_p->onlineFlag ? "TRUE" : "FALSE",
        //    echo_p->backupFlag ? "TRUE" : "FALSE" );

        if( Active ) EchoListView->SetFocus( );

        if( echo_p->onlineFlag && echo_p->backupFlag && echo_p->readDate )
        {
            // mbDlgInfo( "Offline echo '%s'\n", echo_p->name_p );
            AutoOfflineFlag = TRUE;
            EchoListPopupMakeOfflineClick( NIL );
            if( OfflineCanceled == TRUE ) break;
        }
        CountsUpdate( );
    }
}

//---------------------------------------------------------------------------

void __fastcall TEchoListForm::EchoListPopupDetailsClick(TObject *Sender)
{
    pmcEchoListStruct_p     echo_p;

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( pmcEchoDetailsForm( echo_p->id ) == TRUE )
    {
//        FilterRequired = TRUE;
    }

exit:
    PopupInProgress--;
}
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::Popup_ViewEchoClick(TObject *Sender)
{
    ViewEcho( FALSE );
}
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::Popup_PreviewReportClick(TObject *Sender)
{
    pmcEchoListStruct_p     echo_p;
    Char_t                  watermark[32];

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
        sprintf( watermark, "No Echo" );
    }
    else  if ( echo_p->readDate == PMC_ECHO_STATUS_PENDING )
    {
        sprintf( watermark, "Pending" );
    }
    else
    {
        sprintf( watermark, "%s",  ( echo_p->readDate > PMC_ECHO_STATUS_MAX ) ? "" : "DRAFT" );
    }
    pmcEchoReportGenerate( echo_p->id, NIL, watermark, TRUE, FALSE );

exit:
    PopupInProgress--;
}
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::Button2Click(TObject *Sender)
{
    pmcEchoImport( NIL );
}
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::Button1Click(TObject *Sender)
{
    Int32u_t    echoId;
    Char_p      buf_p = NIL;
    Char_p      buf2_p = NIL;

    mbMalloc( buf_p, 1024 );
    mbMalloc( buf2_p, 1024 );

    if( mbDlgYesNo( "Create report for echo not yet imported?" ) != MB_BUTTON_YES )
    {
        goto exit;
    }

    if( ( echoId = pmcSqlRecordCreate( PMC_SQL_TABLE_ECHOS, NIL ) ) == 0 )
    {
        mbDlgExclaim( "Error creating document record in database." );
        goto exit;
    }

    sprintf( buf2_p, "PRE-ECHO REPORT" );

    //                                  0      1      2      3      4      5       6       7      8     9
    sprintf( buf_p, "update %s set %s=%lu,%s=%lu,%s=%lu,%s=%lu,%s=%lu,%s=%lu,%s=\"%s\",%s=%lu,%s=%u,%s=%u,"
                                  "%s=%lu,%s=%lu,%s=%lu "
                    "where %s=%lu",
             PMC_SQL_TABLE_ECHOS
            ,PMC_SQL_FIELD_CRC              , 0                                      // 0
            ,PMC_SQL_ECHOS_FIELD_BYTE_SUM   , 0                                      // 1
            ,PMC_SQL_FIELD_TIME             , 0                                      // 2
            ,PMC_SQL_FIELD_DATE             , 0                                      // 3
            ,PMC_SQL_FIELD_SIZE             , 0                                      // 4
            ,PMC_SQL_FIELD_LOCK             , 0                                      // 5
            ,PMC_SQL_FIELD_NAME             , mbStrClean( buf2_p, NIL, TRUE )        // 6
            ,PMC_SQL_FIELD_NOT_DELETED      , PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE   // 7
            ,PMC_SQL_ECHOS_FIELD_ONLINE     , PMC_SQL_FALSE_VALUE                    // 8
            ,PMC_SQL_ECHOS_FIELD_BACKUP     , PMC_SQL_FALSE_VALUE                    // 9
            ,PMC_SQL_FIELD_SONOGRAPHER_ID   , 1                                      // 10
            ,PMC_SQL_FIELD_PROVIDER_ID      , 1                                      // 11
            ,PMC_SQL_ECHOS_FIELD_READ_DATE  , PMC_ECHO_STATUS_NO_ECHO                // 12

            ,PMC_SQL_FIELD_ID               , echoId );

    if( !pmcSqlExec( buf_p ) )
    {
        mbDlgDebug(( "Failed to update echo %ld\n", echoId ));
        goto exit;
    }

    if( pmcEchoDetailsForm( echoId ) == TRUE )
    {
        FilterRequired = TRUE;
    }

exit:
    mbFree( buf_p );
    mbFree( buf2_p );
    return;
}
//---------------------------------------------------------------------------

void __fastcall TEchoListForm::Popup_DeleteClick(TObject *Sender)
{
    pmcEchoListStruct_p     echo_p;

    PopupInProgress++;

    //Sanity Check
    if( PopupInProgress != 1 || UpdateInProgress || GetInProgress || SortInProgress )
    {
        mbDlgInfo( "The system is busy... please try again." );
        goto exit;
    }

    // Get selected echo
    if( ( echo_p = GetSelectedEcho( ) ) == NIL ) goto exit;

    if( mbDlgYesNo( "Are you sure you want to delete echo '%s'?", echo_p->name_p ) != MB_BUTTON_YES ) goto exit;

    if( pmcSqlEchoLock( echo_p->id, echo_p->name_p ) == FALSE )
    {
        mbDlgError( "Failed to lock echo\n" );
        goto exit;
    }

    pmcSqlRecordDelete( PMC_SQL_TABLE_ECHOS, echo_p->id );

exit:

    PopupInProgress--;
}

//---------------------------------------------------------------------------
void __fastcall TEchoListForm::Button_PatientAssignClick(TObject *Sender)
{
    pmcDatabaseEchosAssignPatients( );
}

//---------------------------------------------------------------------------

void __fastcall TEchoListForm::Button3Click(TObject *Sender)
{
    Ints_t                  i;
    TListItem               *item_p;
    pmcEchoListStruct_p     echo_p;
    Boolean_t               result;

    ControlsStore( );
    ControlsDisable( );

    for( i = 0 ; i < EchoListView->Items->Count ; i++ )
    {
        item_p = EchoListView->Items->Item[i];
        EchoListView->Selected = item_p;
        echo_p = (pmcEchoListStruct_p)item_p->Data;

        if( echo_p->onlineFlag == TRUE ) continue;

        BytesProcessed = 0;

        result = pmcEchoBackupVerifyRestore
        (
            echo_p->id, 0, (Void_p)this,
            CancelCheckCallback,
            BytesProcessedCallback,
            CDLabelCallback,
            NIL, TRUE
        );

        if( result != TRUE )
        {
            mbLog( "restore failed\n" );
            mbDlgInfo( "%s\n\nRestore of this echo failed.\n", echo_p->name_p );
        }
        else
        {
            mbLog( "restore succeeded\n" );
            echo_p->onlineFlag = TRUE;
            //FilterRequired = TRUE;
            pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_ONLINE, 1, echo_p->id );

            // Adjust the counts
            OfflineCount--;
            OnlineCount++;
            //CountsUpdate( );
        }
    }
    ControlsRestore( );

    mbDlgInfo( "Restore complete\n" );
}
//---------------------------------------------------------------------------

