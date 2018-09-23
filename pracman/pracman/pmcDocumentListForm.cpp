//---------------------------------------------------------------------------
// File:    pmcDocumentListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    August 26, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

// Platform include files
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <vcl.h>
#pragma hdrstop

// Library include files
#include "mbUtils.h"

// Project include files
#include "pmcDocumentEditForm.h"
#include "pmcTables.h"
#include "pmcBatchImportForm.h"
#include "pmcUtils.h"
#include "pmcPatientListForm.h"
#include "pmcPatientEditForm.h"
#include "pmcDateSelectForm.h"
#include "pmcWordCreateForm.h"
#include "pmcMainForm.h"
#include "pmcInitialize.h"
#include "pmcDocumentUtils.h"
#include "pmcDocumentListForm.h"
#include "pmcDocument.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Default Constructor
//---------------------------------------------------------------------------
__fastcall TDocumentListForm::TDocumentListForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
__fastcall TDocumentListForm::TDocumentListForm
(
    TComponent             *Owner,
    pmcDocumentListInfo_p   formInfo_p
)
: TForm(Owner)
{
    Active = FALSE;
    FormInfo_p = formInfo_p;
    FormInfo_p->returnCode = MB_BUTTON_CANCEL;

    {
        Ints_t  height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_DOCUMENT_LIST, &height, &width, &top, &left ) == MB_RET_OK )
        {
            SetBounds( left, top, width, height );
        }
    }

    ClearingListView = FALSE;
    NoPatientFlag = FALSE;
    SortListActive = FALSE;

    DocumentMaster_pp = NIL;
    Document_pp = NIL;

    SelectedId = 0;
    ImportedId = 0;
    TopId = 0;

#if 0
    // Set up the various sort modes
    SortDateMode = PMC_SORT_DATE_DESCENDING;
    SortNameMode = PMC_SORT_NAME_DESCENDING;                                                                                   
    SortDescMode = PMC_SORT_DESC_DESCENDING;
    SortProvMode = PMC_SORT_PROV_DESCENDING;
    SortTypeMode = PMC_SORT_TYPE_DESCENDING;
    SortIdMode   = PMC_SORT_ID_DESCENDING;
    SortMode = SortIdMode;
#endif

    CheckBox_ShowFiled->Checked     = TRUE;
    CheckBox_ShowPending->Checked   = TRUE;
    CheckBox_ShowActive->Checked    = TRUE;
    CheckBox_ShowUnknown->Checked   = TRUE;

    ClearingListView = TRUE;
    pmcDocumentListUpdate( );
    UpdateList( FALSE );
    UpdatePatient( FormInfo_p->patientId );

    Active = TRUE;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#if 0
void __fastcall TDocumentListForm::ListFree( void )
{
    pmcDocumentListStruct_p     doc_p;

    DocListView->Items->BeginUpdate( );
    DocListView->Items->Clear( );
    DocListView->Items->EndUpdate( );

    for( ; ; )
    {
        if( qEmpty( DocList_q ) ) break;

        doc_p = (pmcDocumentListStruct_p)qRemoveFirst( DocList_q );

        if( doc_p->description_p )  mbFree( doc_p->description_p );
        if( doc_p->name_p )         mbFree( doc_p->name_p );
        if( doc_p->origName_p )     mbFree( doc_p->origName_p );

        mbFree( doc_p );
    }
}
#endif

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::UpdateList( Boolean topFlag )
{
    Int32u_t                    result = FALSE;
    Int32u_t                    arraySize;
    Boolean                     hackFlag = FALSE;
    MbCursor                    cursor = MbCursor( crHourGlass );

    if( ClearingListView == TRUE )
    {
        hackFlag = TRUE;
        ClearingListView = FALSE;
    }

    if( DocListView->TopItem )
    {
        pmcDocumentItem_p doc_p;
        doc_p = (pmcDocumentItem_p)DocListView->TopItem->Data;

        if( doc_p )
        {
            TopId = doc_p->id;
        }
    }

    if( topFlag == TRUE )
    {
        TopId = 0;
    }

    if( hackFlag )
    {
        ClearingListView = TRUE;
    }

    mbFree( Document_pp );
    mbFree( DocumentMaster_pp );

    /* Get array of pointers to document items; make a copy */
    DocumentMaster_pp = pmcDocumentListGet( &ItemCountMaster );
    arraySize = sizeof( pmcDocumentItem_p ) * ItemCountMaster;
    mbMalloc( Document_pp, arraySize );

    result = TRUE;

    if( result == FALSE )
    {
        mbDlgDebug(( "Failed to read document list" ));
    }
    SortList(  );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::CancelButtonClick(TObject *Sender)
{
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    mbFree( DocumentMaster_pp );
    mbFree( Document_pp );

    mbPropertyWinSave( PMC_WINID_DOCUMENT_LIST, Height, Width, Top, Left );

    Action = caFree;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::PatientList( Int32u_t key )
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId       = FormInfo_p->patientId;
    patListInfo.providerId      = FormInfo_p->providerId;
    patListInfo.mode            = PMC_LIST_MODE_SELECT;
    patListInfo.character       = key;
    patListInfo.allowGoto       = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
        if( patListInfo.patientId != FormInfo_p->patientId )
        {
            FormInfo_p->patientId = patListInfo.patientId;
            SelectedId = 0;
            NoPatientFlag = FALSE;
            UpdatePatient( FormInfo_p->patientId );
            SortList( );
       }
    }
    else
    {
        // User pressed cancel button - do nothing
    }

    // Need to update the document list as the pat list could have created a new CONSULT document
    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( TRUE );
    }
    else
    {
        ClearingListView = FALSE;
    }
    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::PatientListButtonClick(TObject *Sender)
{
    PatientList( 0 );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::PatientEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    PatientList( 0 );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::UpdatePatient
(
    Int32u_t    patientId
)
{
    Char_p          buf_p;
    Char_p          buf2_p;
    Char_p          buf3_p;
    PmcSqlPatient_p pat_p;

    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );
    mbMalloc( buf_p,       128 );
    mbMalloc( buf2_p,       64 );
    mbMalloc( buf3_p,       64 );

    sprintf( buf_p, "" );
    sprintf( buf2_p, "" );
    sprintf( buf3_p, "" );

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
            pmcFormatPhnDisplay( pat_p->phn, pat_p->phnProv, buf3_p );
        }
        else
        {
            sprintf( buf_p, "Unknown" );
        }
    }

    PatientEdit->Text = buf_p;
    PhnLabel->Caption = buf3_p;

    mbFree( pat_p );
    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
}

//---------------------------------------------------------------------------
// View the selected document
//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DocListViewDblClick(TObject *Sender)
{
    pmcDocumentItem_p           doc_p;
    TListItem                  *item_p;
    PmcDocument                 document;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    document.idSet( doc_p->id );
    document.view();

    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupEditDocClick(
      TObject *Sender)
{
    pmcDocumentItem_p           doc_p;
    TListItem                  *item_p;
    PmcDocument                 document;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    document.idSet( doc_p->id );
    document.edit();

    ClearingListView = TRUE;

    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( FALSE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    // For some reason, the elected document is getting cleared in one of the
    // above calls.
    DocListView->Selected = item_p;

    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DocListPopupSubmitClick(TObject *Sender)
{
    pmcDocumentItem_p           doc_p;
    TListItem                  *item_p;
    PmcDocument                 document;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    // Must lock document record before we can edit it in any way
    if( document.idSet( doc_p->id ) != MB_RET_OK )
    {
        mbDlgInfo( "Failed to find document id: %lu.  Document not appoved.", doc_p->id );
        return;
    }

    if( document.lock( FALSE ) != MB_RET_OK )
    {
        mbDlgInfo( "Document is locked for editing by another user; its status cannot be changed at this time." );
        return;
    }

    // OK, change the status
    document.statusSet( PMC_DOCUMENT_STATUS_PENDING );
    document.save( );

    ClearingListView = TRUE;

    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( FALSE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    // For some reason, the elected document is getting cleared in one of the
    // above calls.
    DocListView->Selected = item_p;

    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DocListPopupReviseClick(TObject *Sender)
{
    pmcDocumentItem_p           doc_p;
    TListItem                  *item_p;
    PmcDocument                 document;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    // Must lock document record before we can edit it in any way
    if( document.idSet( doc_p->id ) != MB_RET_OK )
    {
        mbDlgInfo( "Failed to find document id: %lu.  Document not appoved.", doc_p->id );
        return;
    }

    if( document.lock( FALSE ) != MB_RET_OK )
    {
        mbDlgInfo( "Document is locked for editing by another user; its status cannot be changed at this time." );
        return;
    }

    // OK, change the status
    document.statusSet( PMC_DOCUMENT_STATUS_ACTIVE );
    document.save( );

    ClearingListView = TRUE;

    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( FALSE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    // For some reason, the elected document is getting cleared in one of the
    // above calls.
    DocListView->Selected = item_p;

    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupApproveClick(TObject *Sender)
{
    TListItem                  *item_p;
    pmcDocumentItem_p           doc_p;
    PmcDocument                 document;
    
    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    if( pmcCfg[CFG_DATABASE_LOCAL].value == FALSE )
    {
        mbDlgInfo( "Cannot import documents when connected to a remote database.\n" );
        return;
    }

    // Must lock document record before we can edit it in any way
    if( document.idSet( doc_p->id ) != MB_RET_OK )
    {
        mbDlgInfo( "Failed to find document id: %lu.  Document not appoved.", doc_p->id );
        return;
    }

    if( document.lock( FALSE ) != MB_RET_OK )
    {
        mbDlgInfo( "Document is locked for editing by another user, and cannot be approved at this time." );
        return;
    }

    if( document.import( ) == MB_RET_OK )
    {
        mbDlgInfo( "Document imported into database." );
    }
    else
    {
        mbDlgInfo( "Document not imported into database." );
    }

    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( FALSE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    // For some reason, the elected document is getting cleared in one of the
    // above calls.
    DocListView->Selected = item_p;

    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------
// Handle document list popup menu
//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DocListPopupPopup(TObject *Sender)
{
    TListItem                  *item_p;
    pmcDocumentItem_p           doc_p;

    pmcPopupItemEnableAll( DocListPopup, FALSE );

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_EDIT,           TRUE );

    if( doc_p )
    {
        if(    doc_p->status ==  PMC_DOCUMENT_STATUS_ACTIVE
            || doc_p->status ==  PMC_DOCUMENT_STATUS_UNKNOWN )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_DELETE,        TRUE );
        }

        if( pmcCfg[CFG_ALLOW_DOC_UNLOCK].value == TRUE )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_UNLOCK,        TRUE );
        }

        // Check if document delete should be enabled in all cases
        if( pmcCfg[CFG_ENABLE_DOCUMENT_DELETE].value == TRUE )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_DELETE,        TRUE );
        }

        if( doc_p->status != PMC_DOCUMENT_STATUS_UNKNOWN )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_VIEW,          TRUE );
        }

        if( doc_p->status == PMC_DOCUMENT_STATUS_ACTIVE )
        {
             pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_SUBMIT,       TRUE );
        }

        if( doc_p->status == PMC_DOCUMENT_STATUS_PENDING )
        {
             pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_REVISE,       TRUE );
             pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_APPROVE,      TRUE );
        }

        if( doc_p->status == PMC_DOCUMENT_STATUS_ACTIVE ||
            doc_p->status == PMC_DOCUMENT_STATUS_PENDING )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_EDIT_DOCUMENT, TRUE );
        }

        if( doc_p->status == PMC_DOCUMENT_STATUS_FILED )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_EXTRACT,       TRUE );
        }

        if( FormInfo_p->patientId == 0 && doc_p->patientId != 0 )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_LIST_PAT,      TRUE );
        }
        if( FormInfo_p->patientId != 0 )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_LIST_ALL,      TRUE );
        }

        if( doc_p->patientId )
        {
            pmcPopupItemEnable( DocListPopup, PMC_DOC_LIST_POPUP_LIST_PAT_APP,  TRUE );
        }
    }
    return;
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupListPatAppClick(
      TObject *Sender)
{
    TListItem           *item_p;
    pmcDocumentItem_p    doc_p;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    if( ( doc_p = (pmcDocumentItem_p)item_p->Data ) == NIL ) goto exit;

    pmcViewAppointments( doc_p->patientId, TRUE, TRUE,
        FALSE, NIL, NIL, TRUE, PMC_LIST_MODE_LIST );

exit:
    return;

}
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupListAllClick(
      TObject *Sender)
{
    FormInfo_p->patientId = 0;
    UpdatePatient( FormInfo_p->patientId );
    SortList( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupListPatClick(
      TObject *Sender)
{
    TListItem           *item_p;
    pmcDocumentItem_p    doc_p;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    if( ( doc_p = (pmcDocumentItem_p)item_p->Data ) == NIL ) goto exit;

    FormInfo_p->patientId = doc_p->patientId;

    UpdatePatient( FormInfo_p->patientId );
    SortList( );

exit:
    return;
}
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupEditClick(TObject *Sender)
{
    mbFileListStruct_p          file_p;
    pmcDocumentItem_p           doc_p;
    TListItem                  *item_p;
    Int32s_t                    result = FALSE;
    Int32u_t                    formDisplayed = FALSE;
    PmcDocument                 document;

    mbCalloc( file_p, sizeof(mbFileListStruct_t) );

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    // Must lock document record before we can edit it in any way
    if( document.idSet( doc_p->id ) != MB_RET_OK )
    {
        mbDlgInfo( "Failed to find document id: %lu.  Document not appoved.", doc_p->id );
        return;
    }

    if( document.lock( FALSE ) != MB_RET_OK )
    {
        mbDlgInfo( "Document is locked for editing by another user, and cannot be approved at this time." );
        return;
    }

    result = pmcDocProcess
    (
        file_p,                         // File details
       &doc_p->id,                      // document id - (0 = NEW)
        PMC_IMPORT_DIALOG_ALWAYS,       // Show dialog or not
        PMC_IMPORT_PROVIDER_DETERMINE,
        PMC_IMPORT_PHN_BOTH,
        PMC_IMPORT_DATE_DETERMINE,
        FALSE,                          // TRUE = moved failed to fail dir
        0, 0, 0, 0,                     // ProviderId, PatientId, date, failMask
        NIL,                            // result flag - terminate auto import
        &formDisplayed,                 // result flag - form displayed
        NIL,                            // Failed directory
        NIL,                            // description
        FALSE
    );

    if( result == TRUE || formDisplayed == FALSE )
    {
        ClearingListView = TRUE;
        pmcDocumentListUpdate( );
        UpdateList( FALSE );
    }
    mbFileListFreeElement( file_p );


}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupViewClick(TObject *Sender)
{
    DocListViewDblClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::ImportButtonClick(TObject *Sender)
{
    Int32u_t    newDocumentId;

    newDocumentId = pmcDocImportSingle( FormInfo_p->patientId );

    if( newDocumentId > 0 ) ImportedId = newDocumentId;

    ClearingListView = TRUE;
    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( TRUE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::NewWordDocumentButtonClick(TObject *Sender)
{
    pmcWordCreate( FormInfo_p->patientId, 0 );

    ClearingListView = TRUE;
    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( TRUE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::Button_BatchImportClick(TObject *Sender)
{
    TBatchImportForm           *batchForm_p;
    pmcBatchImportInfoo_t       batchInfo;

    if( pmcCfg[CFG_DATABASE_LOCAL].value == FALSE )
    {
        mbDlgInfo( "Cannot import documents when connected to a remote database.\n" );
        goto exit;
    }
    batchInfo.providerId = FormInfo_p->providerId;
    batchInfo.mode = PMC_IMPORT_MODE_DOCS;
    batchForm_p = new TBatchImportForm( NIL, &batchInfo );
    batchForm_p->ShowModal( );
    delete batchForm_p;

    ClearingListView = TRUE;
    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( TRUE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    if( Active ) DocListView->SetFocus( );
exit:
    return;
}

//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::PatientEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    PatientList( (Int32u_t)Key );
    if( Active ) DocListView->SetFocus( );
}

//---------------------------------------------------------------------------
// This function is a total mess and needs to be cleaned up
//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::SortList
(
    void
)
{
//    TListItem                  *item_p;
    Char_p                      buf_p;
//    bool                        selectedFound = FALSE;
    pmcDocumentItem_p           doc_p;
    Int32u_t                    selectedIndex = 0;
    Int32u_t                    topIndex = 0;
    MbCursor                    cursor = MbCursor( crHourGlass );
    SortListActive = TRUE;

    mbMalloc( buf_p, 1024 );

    ItemCount = 0;

    for( Int32u_t i = 0 ; i < ItemCountMaster ; i ++ )
    {
        doc_p = DocumentMaster_pp[i];

        if( NoPatientFlag == TRUE )
        {
            if( doc_p->patientId !=  0 ) continue;
        }

        if( FormInfo_p->patientId != 0 )
        {
            if( doc_p->patientId !=  FormInfo_p->patientId ) continue;
        }

        if( CheckBox_ShowFiled->Checked == FALSE )
        {
            if( doc_p->status == PMC_DOCUMENT_STATUS_FILED ) continue;
        }

        if( CheckBox_ShowPending->Checked == FALSE )
        {
            if( doc_p->status == PMC_DOCUMENT_STATUS_PENDING ) continue;
        }

        if( CheckBox_ShowActive->Checked == FALSE )
        {
            if( doc_p->status == PMC_DOCUMENT_STATUS_ACTIVE ) continue;
        }

        if( CheckBox_ShowUnknown->Checked == FALSE )
        {
            if( doc_p->status == PMC_DOCUMENT_STATUS_UNKNOWN ) continue;
        }

        Document_pp[ItemCount] = doc_p;

        /* Check to see if this is the selected document */
        if( doc_p->id == SelectedId )
        {
            selectedIndex = ItemCount;
        }
        if( doc_p->id == TopId )
        {
            topIndex = ItemCount;
        }
        ItemCount++;
    }

#if 0
    for( ; ; )
    {
        if( qEmpty( DocList_q ) ) break;

        doc_p = (pmcDocumentListStruct_p)qRemoveFirst( DocList_q );

        added = FALSE;

        qWalk( tempDoc_p, temp_q, pmcDocumentListStruct_p )
        {
            switch( SortMode )
            {
                case PMC_SORT_ID_DESCENDING:
                case PMC_SORT_ID_ASCENDING:
                    added = SortId( SortMode, temp_q, doc_p, tempDoc_p, TRUE );
                    break;

                case PMC_SORT_DATE_DESCENDING:
                case PMC_SORT_DATE_ASCENDING:
                    added = SortDate( SortMode, temp_q, doc_p, tempDoc_p, TRUE );
                    break;

                case PMC_SORT_NAME_ASCENDING:
                case PMC_SORT_NAME_DESCENDING:
                    added = SortName( SortMode, temp_q, doc_p, tempDoc_p, TRUE );
                    break;

                case PMC_SORT_DESC_ASCENDING:
                case PMC_SORT_DESC_DESCENDING:
                    added = SortDesc( SortMode, temp_q, doc_p, tempDoc_p, TRUE );
                    break;

                case PMC_SORT_PROV_ASCENDING:
                case PMC_SORT_PROV_DESCENDING:
                    added = SortProv( SortMode, temp_q, doc_p, tempDoc_p, TRUE );
                    break;

                case PMC_SORT_TYPE_ASCENDING:
                case PMC_SORT_TYPE_DESCENDING:
                    added = SortType( SortMode, temp_q, doc_p, tempDoc_p, TRUE );
                    break;

                default:
                    break;
            }
            if( added ) break;
        }
        if( !added ) qInsertLast( temp_q, doc_p );
    }

    for( ; ; )
    {
       if( qEmpty( temp_q )) break;

       doc_p = (pmcDocumentListStruct_p)qRemoveLast( temp_q );
       qInsertFirst( DocList_q, doc_p );
    }

#endif

    ClearingListView = TRUE;
    DocListView->Items->BeginUpdate( );
    DocListView->Items->Clear( );

    ClearingListView = FALSE;
    DocListView->AllocBy = ItemCount;
    DocListView->Items->Count = ItemCount;

#if 0
    // Check to see if list is being sorted after a document import
    if( ImportedId )
    {
        SelectedId = ImportedId;
        ImportedId = 0;
    }

    qWalk( doc_p, DocList_q, pmcDocumentListStruct_p )
    {
        // Set the selected document id if it is not already set
        if( SelectedId == 0 ) SelectedId = doc_p->id;

        item_p = DocListView->Items->Add( );

        // Record the first item in the list
        if( firstItem_p == NIL ) firstItem_p = item_p;

        item_p->Caption =  "Filed";

        item_p->ImageIndex = PMC_MAIN_COLOR_SQUARE_INDEX_GREEN;

        sprintf( buf_p, "%s", doc_p->description_p );
        item_p->SubItems->Add( buf_p );

        sprintf( buf_p, "%d", doc_p->id );
        item_p->SubItems->Add( buf_p );

        // Date of the document
        *buf_p = 0;
#if 0
        if( doc_p->date != 0 )
        {
            dateTime_p = new MDateTime( doc_p->date, 0 );
            dateTime_p->MDY_DateString( buf_p );
            delete dateTime_p;
        }
#endif
        item_p->SubItems->Add( buf_p );

        item_p->SubItems->Add( pmcDocumentTypeDescStrings[doc_p->type] );

        *buf_p = 0;
#if 0
        if( doc_p->providerId )
        {
            pmcProviderDescGet( doc_p->providerId, buf_p );
        }
#endif
        item_p->SubItems->Add( buf_p );

        item_p->SubItems->Add( doc_p->origName_p );

        item_p->Data = (Void_p)doc_p;

        // Set selected row if found
        if( doc_p->id == SelectedId )
        {
            selectedFound = TRUE;
            DocListView->Selected = item_p;
            item_p->Selected = TRUE;
            DocListView->Selected->MakeVisible( TRUE );
        }
    }
#endif

    if( selectedIndex > 0 )
    {
//        Boolean_t           found = FALSE;

         for( Int32u_t i = 0 ; i < selectedIndex ; i++ )
         {
            doc_p = (pmcDocumentItem_p)DocListView->Items->Item[i]->Data;
//            mbLog( "doc id: %u\n", doc_p->id );
         }

         DocListView->Selected = DocListView->Items->Item[selectedIndex];
         DocListView->Selected->Selected = TRUE;
//         DocListView->Selected->MakeVisible( TRUE );
    }

    if( topIndex > 0 )
    {
  //       Boolean_t               found = FALSE;

         for( Int32u_t i = 0 ; i < ItemCount ; i++ )
         {
             DocListView->Items->Item[i]->MakeVisible( TRUE );

             doc_p = (pmcDocumentItem_p)DocListView->TopItem->Data;
             if( doc_p->id  == TopId )
             {
             break;
             }
         }

//         DocListView->Selected->Selected = TRUE;
//         DocListView->Selected->MakeVisible( TRUE );
    }

    DocListView->Items->EndUpdate( );

#if 0
    if( selectedFound == FALSE && firstItem_p != NIL )
    {
        DocListView->Selected = firstItem_p;
        firstItem_p->Selected = TRUE;
        DocListView->Selected->MakeVisible( TRUE );
    }
#endif

    if( Active ) DocListView->SetFocus( );

    mbFree( buf_p );
    SortListActive = FALSE;
}

//---------------------------------------------------------------------------

#if 0
bool __fastcall  TDocumentListForm::SortId
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcDocumentListStruct_p     doc_p,
    pmcDocumentListStruct_p     tempDoc_p,
    Int32u_t                    nameSort
)
{
    bool    added = FALSE;

    if( sortMode == PMC_SORT_ID_DESCENDING )
    {
        if( doc_p->id > tempDoc_p->id )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_ID_ASCENDING )
    {
       if( doc_p->id < tempDoc_p->id )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    return added;
}

//---------------------------------------------------------------------------

bool __fastcall  TDocumentListForm::SortDate
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcDocumentListStruct_p     doc_p,
    pmcDocumentListStruct_p     tempDoc_p,
    Int32u_t                    nameSort
)
{
    bool    added = FALSE;

    if( ( doc_p->date == tempDoc_p->date ) && nameSort == TRUE )
    {
        added = SortName( SortNameMode, temp_q, doc_p, tempDoc_p, FALSE );
    }
    else if( sortMode == PMC_SORT_DATE_DESCENDING )
    {
        if( doc_p->date > tempDoc_p->date )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_DATE_ASCENDING )
    {
       if( doc_p->date < tempDoc_p->date )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    return added;
}
//---------------------------------------------------------------------------

bool __fastcall  TDocumentListForm::SortProv
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcDocumentListStruct_p     doc_p,
    pmcDocumentListStruct_p     tempDoc_p,
    Int32u_t                    dateSort
)
{
    bool    added = FALSE;

    if( ( doc_p->providerId == tempDoc_p->providerId ) && dateSort == TRUE )
    {
        added = SortDate( SortDateMode, temp_q, doc_p, tempDoc_p, TRUE );
    }
    else if( sortMode == PMC_SORT_PROV_DESCENDING )
    {
        if( doc_p->providerId > tempDoc_p->providerId )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_PROV_ASCENDING )
    {
       if( doc_p->providerId < tempDoc_p->providerId )
       {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    return added;
}
//---------------------------------------------------------------------------

bool __fastcall  TDocumentListForm::SortType
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcDocumentListStruct_p     doc_p,
    pmcDocumentListStruct_p     tempDoc_p,
    Int32u_t                    dateSort
)
{
    bool    added = FALSE;

    if( ( doc_p->type == tempDoc_p->type ) && dateSort == TRUE )
    {
        added = SortDate( SortDateMode, temp_q, doc_p, tempDoc_p, TRUE );
    }
    else if( sortMode == PMC_SORT_TYPE_DESCENDING )
    {
        if( doc_p->type > tempDoc_p->type )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_TYPE_ASCENDING )
    {
       if( doc_p->type < tempDoc_p->type )
       {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    return added;
}

//---------------------------------------------------------------------------

bool __fastcall  TDocumentListForm::SortName
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcDocumentListStruct_p     doc_p,
    pmcDocumentListStruct_p     tempDoc_p,
    Int32u_t                    dateSort
)
{
    bool        added = FALSE;
    Int32s_t    result;

    result = strcmp( doc_p->origName_p, tempDoc_p->origName_p );

    if( result == 0 && dateSort == TRUE )
    {
        added = SortDate( SortDateMode, temp_q, doc_p, tempDoc_p, FALSE );
    }
    else if( sortMode == PMC_SORT_NAME_ASCENDING )
    {
        if( result < 0 )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_NAME_DESCENDING )
    {
        if( result > 0 )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }

    return added;
}

//---------------------------------------------------------------------------

bool __fastcall  TDocumentListForm::SortDesc
(
    Int32u_t                    sortMode,
    qHead_p                     temp_q,
    pmcDocumentListStruct_p     doc_p,
    pmcDocumentListStruct_p     tempDoc_p,
    Int32u_t                    dateSort
)
{
    bool        added = FALSE;
    Int32s_t    result;

    result = strcmp( doc_p->description_p, tempDoc_p->description_p );

    if( result == 0 && dateSort == TRUE )
    {
        added = SortDate( SortDateMode, temp_q, doc_p, tempDoc_p, TRUE );
    }
    else if( sortMode == PMC_SORT_DESC_ASCENDING )
    {
        if( result < 0 )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_DESC_DESCENDING )
    {
        if( result > 0 )
        {
            qInsertBefore( temp_q, tempDoc_p, doc_p );
            added = TRUE;
        }
    }

    return added;
}

#endif

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListViewColumnClick(TObject *Sender,
      TListColumn *Column)
{
#if 0
    if( strcmp( Column->Caption.c_str(), "Date" ) == 0 )
    {
        SortDateMode = ( SortDateMode == PMC_SORT_DATE_ASCENDING ) ? PMC_SORT_DATE_DESCENDING : PMC_SORT_DATE_ASCENDING;
        SortMode = SortDateMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Original Name" ) == 0 )
    {
        SortNameMode = ( SortNameMode == PMC_SORT_NAME_ASCENDING ) ? PMC_SORT_NAME_DESCENDING : PMC_SORT_NAME_ASCENDING;
        SortMode = SortNameMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Description" ) == 0 )
    {
        SortDescMode = ( SortDescMode == PMC_SORT_DESC_ASCENDING ) ? PMC_SORT_DESC_DESCENDING : PMC_SORT_DESC_ASCENDING;
        SortMode = SortDescMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Provider" ) == 0 )
    {
        SortProvMode = ( SortProvMode == PMC_SORT_PROV_ASCENDING ) ? PMC_SORT_PROV_DESCENDING : PMC_SORT_PROV_ASCENDING;
        SortMode = SortProvMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Type" ) == 0 )
    {
        SortTypeMode = ( SortTypeMode == PMC_SORT_TYPE_ASCENDING ) ? PMC_SORT_TYPE_DESCENDING : PMC_SORT_TYPE_ASCENDING;
        SortMode = SortTypeMode;
    }
    else if( strcmp( Column->Caption.c_str(), "ID" ) == 0 )
    {
        SortIdMode = ( SortIdMode == PMC_SORT_ID_ASCENDING ) ? PMC_SORT_ID_DESCENDING : PMC_SORT_ID_ASCENDING;
        SortMode = SortIdMode;
    }

    SortList( );
#endif

}

//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DocListPopupDeleteClick(TObject *Sender)
{
    DeleteExtract( PMC_DOC_LIST_MODE_DELETE );
}

//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DocListPopupExtractClick(TObject *Sender)
{
    DeleteExtract( PMC_DOC_LIST_MODE_EXTRACT );
}

//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DocListPopupExtDelClick(TObject *Sender)
{
     DeleteExtract( PMC_DOC_LIST_MODE_EXTRACT | PMC_DOC_LIST_MODE_DELETE );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListPopupUnlockClick(TObject *Sender)
{
    PmcDocument                 document;
    pmcDocumentItem_p           doc_p;
    TListItem                  *item_p;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting document if from form" );
        return;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    // Get document object
    if( document.idSet( doc_p->id ) != MB_RET_OK )
    {
        mbDlgInfo( "Failed to find document id: %lu", doc_p->id );
        return;
    }

    // Unlock
    if( document.unlock( ) != MB_RET_OK )
    {
        mbDlgInfo("failed to unlock document");
    }
}

//---------------------------------------------------------------------------
// Delete/Extract document
//---------------------------------------------------------------------------
void __fastcall TDocumentListForm::DeleteExtract( Int32u_t mode )
{
    pmcDocumentItem_p           doc_p;
    TListItem                  *item_p;
    Boolean_t                   extractFlag = FALSE;
    Boolean_t                   deleteFlag = FALSE;
    Boolean_t                   updateFlag = FALSE;
    PmcDocument                 document;

    if( mode & PMC_DOC_LIST_MODE_DELETE )  deleteFlag = TRUE;
    if( mode & PMC_DOC_LIST_MODE_EXTRACT ) extractFlag = TRUE;

    // Get doc from form
    if( ( item_p = DocListView->Selected ) == NIL )
    {
        mbDlgError( "Error getting doc if from form" );
        goto exit;
    }

    doc_p = (pmcDocumentItem_p)item_p->Data;

    if( document.idSet( doc_p->id ) != MB_RET_OK )
    {
        mbDlgInfo( "Error getting doc info from database" );
        goto exit;
    }

    if( extractFlag )
    {
        document.extract( );
    }

    if( deleteFlag )
    {
        if( document.del( ) == TRUE )
        {
            updateFlag = TRUE;
        }
    }

    ClearingListView = TRUE;
    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( FALSE );
        updateFlag = FALSE;
    }
    else
    {
        ClearingListView = FALSE;
    }

exit:

    if( updateFlag ) UpdateList( FALSE );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListViewSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
    pmcDocumentItem_p     doc_p;

    if( Item && SortListActive == FALSE )
    {
        doc_p = (pmcDocumentItem_p)Item->Data;
        SelectedId = doc_p->id;
    }
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::DocListViewData(TObject *Sender,
      TListItem *Item)
{
    Char_t              buf[128];
    MbDateTime          dateTime;
    pmcDocumentItem_p   doc_p;

    if( ClearingListView ) return;

    if( Item->Index >= (int)ItemCount )
    {
//        mbDlgError( "Invalid document index: %d\n", Item->Index );
        return;
    }

    if( ( doc_p = Document_pp[ Item->Index ] ) == NIL )
    {
        mbDlgError( "Invalid document pointer\n" );
        return;
    }

    Item->Caption = pmcDocumentStatusString( doc_p->status );
    Item->ImageIndex = pmcDocumentStatusImageIndex( doc_p->status );

    sprintf( buf, "%d", doc_p->id );
    Item->SubItems->Add( buf );

    if( doc_p->patient_p )
    {
        Item->SubItems->Add( doc_p->patient_p );
    }
    else
    {
//        sprintf( buf, "%d", doc_p->patientId );
//        Item->SubItems->Add( buf );
        Item->SubItems->Add( "" );
    }
    Item->SubItems->Add( doc_p->description_p );

    buf[0] = 0;
    if( doc_p->date != 0 )
    {
         dateTime.SetDate( doc_p->date );
         sprintf( buf, dateTime.MDY_DateString( ) );
    }
    Item->SubItems->Add( buf );
    Item->SubItems->Add( pmcDocumentTypeDescStrings[doc_p->type] );

    buf[0] = 0;

    if( doc_p->providerId )
    {
        pmcProviderDescGet( doc_p->providerId, buf );
    }

    Item->SubItems->Add( buf );
    Item->SubItems->Add( doc_p->origName_p );

    Item->Data = (Void_p)doc_p;

    return;
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::CheckBox_ShowFiledClick(TObject *Sender)
{
    if( !Active ) return;
    SortList( );
}
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::CheckBox_ShowPendingClick(
      TObject *Sender)
{
    if( !Active ) return;
    SortList( );
}
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::CheckBox_ShowActiveClick(
      TObject *Sender)
{
    if( !Active ) return;
    SortList( );
}
//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::CheckBox_ShowUnknownClick(
      TObject *Sender)
{
    if( !Active ) return;
    SortList( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::ButtonPatNoneClick(TObject *Sender)
{
    NoPatientFlag = TRUE;
    Active = FALSE;

    CheckBox_ShowFiled->Checked     = TRUE;
    CheckBox_ShowPending->Checked   = TRUE;
    CheckBox_ShowActive->Checked    = TRUE;
    CheckBox_ShowUnknown->Checked   = TRUE;

    FormInfo_p->patientId = 0;
    UpdatePatient( FormInfo_p->patientId );

    Active = TRUE;
    SortList( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::ButtonPatAllClick(TObject *Sender)
{
    NoPatientFlag = FALSE;
    Active = FALSE;

    CheckBox_ShowFiled->Checked     = TRUE;
    CheckBox_ShowPending->Checked   = TRUE;
    CheckBox_ShowActive->Checked    = TRUE;
    CheckBox_ShowUnknown->Checked   = TRUE;

    FormInfo_p->patientId = 0;
    UpdatePatient( FormInfo_p->patientId );

    Active = TRUE;
    SortList( );
}

//---------------------------------------------------------------------------

void __fastcall TDocumentListForm::Button_ScannedImportClick(
      TObject *Sender)
{
    TBatchImportForm           *batchForm_p;
    pmcBatchImportInfoo_t       batchInfo;

    if( pmcCfg[CFG_DATABASE_LOCAL].value == FALSE )
    {
        mbDlgInfo( "Cannot import documents when connected to a remote database.\n" );
        goto exit;
    }
    batchInfo.providerId = FormInfo_p->providerId;
    batchInfo.mode = PMC_IMPORT_MODE_SCANS;
    batchForm_p = new TBatchImportForm( NIL, &batchInfo );
    batchForm_p->ShowModal( );
    delete batchForm_p;

    ClearingListView = TRUE;
    if( pmcDocumentListUpdate(  ) )
    {
        UpdateList( TRUE );
    }
    else
    {
        ClearingListView = FALSE;
    }

    if( Active ) DocListView->SetFocus( );
exit:
    return;
}
//---------------------------------------------------------------------------


