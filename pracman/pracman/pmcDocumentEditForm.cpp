//---------------------------------------------------------------------------
// File:    pmcDocumentEditForm.cpp
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
#include "pmcDocumentUtils.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Default constructor
//---------------------------------------------------------------------------
__fastcall TDocumentEditForm::TDocumentEditForm(TComponent* Owner)
: TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
__fastcall TDocumentEditForm::TDocumentEditForm
(
    TComponent             *Owner,
    pmcDocumentEditInfo_p   editInfo_p
)
: TForm(Owner)
{
    Char_p          buf1_p;
    Char_p          buf2_p;
    MbDateTime      dateTime;

    mbMalloc( buf1_p, 128 );
    mbMalloc( buf2_p, 128 );

    EditInfo_p = editInfo_p;

    EditInfo_p->terminateBatch = FALSE;

    TerminateBatchCheckBox->Checked = FALSE;
    TerminateBatchCheckBox->Visible = ( EditInfo_p->batchMode ) ? TRUE : FALSE;
    FailButton->Visible             = ( EditInfo_p->batchMode ) ? TRUE : FALSE;

    if( EditInfo_p->newFlag == TRUE ) OkButton->Caption = "Import";

    // Get the name of the file
    OriginalNameEdit->Text = EditInfo_p->file_p->name_p;
    DescriptionEdit->Text = EditInfo_p->description_p;

    // Set the document type pick list
    {
        Int32u_t tempType = editInfo_p->file_p->type;

        if(    tempType == PMC_DOCUMENT_TYPE_CONSLT
            || tempType == PMC_DOCUMENT_TYPE_FOLLOWUP
            || tempType== PMC_DOCUMENT_TYPE_PDF_FIXED )
        {
            tempType =  PMC_DOCUMENT_TYPE_PDF;
        }
        pmcPickListBuild( DocumentTypeComboBox, &pmcDocumentTypes[0], tempType );
        DocumentTypeComboBoxIndex = DocumentTypeComboBox->ItemIndex;
    }

    FailReasonUpdate( );
    PatientUpdate( EditInfo_p->patientId );
    DateUpdate( EditInfo_p->date );

    EditInfo_p->providerId = pmcProviderListBuild( ProviderComboBox, EditInfo_p->providerId, FALSE, TRUE );
    ProviderIndex = ProviderComboBox->ItemIndex;

    *buf1_p = 0;
    if( EditInfo_p->createdDate != 0 )
    {
        dateTime.SetDateTime( EditInfo_p->createdDate, EditInfo_p->createdTime );
        sprintf( buf1_p, "%s %s", dateTime.YMD_DateString( ), dateTime.HMS_TimeString( ) );
    }
    CreatedLabel->Caption = buf1_p;

    *buf1_p = 0;
    if( EditInfo_p->modifiedDate != 0 )
    {
        dateTime.SetDateTime( EditInfo_p->modifiedDate, EditInfo_p->modifiedTime );
        sprintf( buf1_p, "%s %s", dateTime.YMD_DateString( ), dateTime.HMS_TimeString( ) );
    }
    ModifiedLabel->Caption = buf1_p;

    sprintf( buf1_p, "%d", EditInfo_p->id );
    IdLabel->Caption = buf1_p;

    if( EditInfo_p->lockFailed )
    {
        OkButton->Visible = FALSE;
        CancelButton->Caption = "Close";

        OriginalNameEdit->Enabled = FALSE;
        DescriptionEdit->Enabled = FALSE;
        DocumentTypeComboBox->Enabled = FALSE;
        ProviderComboBox->Enabled = FALSE;
        PatientEdit->Enabled = FALSE;
        PatientListButton->Enabled = FALSE;
        DateEdit->Enabled = FALSE;
        DateEditButton->Enabled = FALSE;
        ProviderClearButton->Enabled = FALSE;
        PatientClearButton->Enabled = FALSE;
        PatientEditButton->Enabled = FALSE;
    }

    ViewButton->Enabled = ( EditInfo_p->file_p->fullName_p == NIL ) ? FALSE : TRUE;

    // Disable the controls for CONSULT and FIXED documents
    if(    EditInfo_p->file_p->type == PMC_DOCUMENT_TYPE_CONSLT
        || EditInfo_p->file_p->type == PMC_DOCUMENT_TYPE_FOLLOWUP
        || EditInfo_p->file_p->type == PMC_DOCUMENT_TYPE_PDF_FIXED )
    {
        this->disableControls( );
    }
    
    mbFree( buf1_p );
    mbFree( buf2_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::FailReasonUpdate( void )
{
    Int32u_t    i, mask;
    TListItem  *item_p;

    FailListView->Items->BeginUpdate( );
    FailListView->Items->Clear( );

    for( i = 0, mask = 1 ; i < 32 ; i++ )
    {
        if( mask == PMC_IMPORT_FAIL_INVALID ) break;

        if( EditInfo_p->failReasonMask & mask )
        {
            item_p = FailListView->Items->Add( );
            item_p->Caption = pmcImportFailStrings[i];
        }
        mask <<= 1;
    }
    FailListView->Items->EndUpdate( );
}


//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::CancelButtonClick(TObject *Sender)
{
    mbLog( "Document edit form cancel button clicked\n" );
    EditInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Char_p  buf_p;
    bool    cancel = FALSE;

    mbMalloc( buf_p, 512 );

    if( TerminateBatchCheckBox->Checked )EditInfo_p->terminateBatch = TRUE;

    Action = ( cancel ) ? caNone : caFree;

    if( Action == caFree  )
    {
        FailListView->Items->BeginUpdate( );
        FailListView->Items->Clear( );
        FailListView->Items->EndUpdate( );

        if( EditInfo_p->returnCode == MB_BUTTON_OK )
        {
            // Update the original file name
            mbFree( EditInfo_p->file_p->name_p );
            mbStrClean( OriginalNameEdit->Text.c_str(), buf_p, FALSE );
            mbMalloc( EditInfo_p->file_p->name_p, strlen( buf_p ) + 1 );
            strcpy( EditInfo_p->file_p->name_p, buf_p );

            // Update the description
            mbFree( EditInfo_p->description_p );
            mbStrClean( DescriptionEdit->Text.c_str(), buf_p, TRUE );
            mbMalloc( EditInfo_p->description_p, strlen( buf_p ) + 1 );
            strcpy( EditInfo_p->description_p, buf_p );
        }
    }
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::DocumentTypeComboBoxChange
(
    TObject *Sender
)
{
   if( DocumentTypeComboBox->ItemIndex >= 0 &&
       pmcDocumentTypes[DocumentTypeComboBox->ItemIndex].code != PMC_DOCUMENT_TYPE_ANY )
    {
        // Index is valid... has it changed?
        if( DocumentTypeComboBoxIndex != DocumentTypeComboBox->ItemIndex )
        {
            DocumentTypeComboBoxIndex = DocumentTypeComboBox->ItemIndex;
        }
    }
    else
    {
        // Index is invalid... restore previous value
        DocumentTypeComboBox->ItemIndex = DocumentTypeComboBoxIndex;
    }

    // Restore text
    DocumentTypeComboBox->Text = pmcDocumentTypes[DocumentTypeComboBox->ItemIndex].description_p;

    EditInfo_p->file_p->type = pmcFileIndexToType( DocumentTypeComboBox->ItemIndex );

    DocumentTypeComboBox->Invalidate( );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::OkButtonClick(TObject *Sender)
{
    mbLog( "Document edit form OK button clicked\n" );
    EditInfo_p->returnCode = MB_BUTTON_OK;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::ViewButtonClick(TObject *Sender)
{
    pmcDocumentView( EditInfo_p->file_p, TRUE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::PatientListButtonClick(TObject *Sender)
{
    PatientList( 0 );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::PatientUpdate
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
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::PatientEditButtonClick(TObject *Sender)
{
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( EditInfo_p->patientId )
    {
        patEditInfo.patientId = EditInfo_p->patientId;
        patEditInfo.mode = PMC_EDIT_MODE_EDIT;
        sprintf( patEditInfo.caption, "Patient Details" );

        patEditForm = new TPatientEditForm( NULL, &patEditInfo );
        patEditForm->ShowModal();
        delete patEditForm;

        if( patEditInfo.returnCode == MB_BUTTON_OK )
        {
            PatientUpdate( EditInfo_p->patientId );
        }
        else
        {
            // User must have clicked cancel button
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::DateUpdate( Int32u_t date )
{
    if( date == 0 ) date = mbToday();
    MbDateTime dateTime = MbDateTime( date, 0 );
    DateEdit->Text = dateTime.MDY_DateString( );
    EditInfo_p->date = date;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::DateEditButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    Char_t              string[128];
    pmcDateSelectInfo_t dateSelectInfo;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = string;
    dateSelectInfo.dateIn = EditInfo_p->date;
    dateSelectInfo.caption_p = NIL;

    dateSelectForm_p = new TDateSelectForm
    (
        NULL,
        &dateSelectInfo
    );

    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        EditInfo_p->date = dateSelectInfo.dateOut;
        DateUpdate( EditInfo_p->date );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::DateEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    DateEditButtonClick( Sender );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::ProviderComboBoxChange(TObject *Sender)
{
    Int32u_t    newProviderId;
    Char_p      buf_p;

    mbMalloc( buf_p, 64 );

    newProviderId = pmcProviderIdGet( ProviderComboBox->Text.c_str() );

    // If the provider id is unknown, set it back to what it was before
    if( newProviderId == 0 )
    {
        ProviderComboBox->ItemIndex = ProviderIndex;
    }
    else
    {
        ProviderIndex = ProviderComboBox->ItemIndex;
        EditInfo_p->providerId = newProviderId;
    }
    ProviderComboBox->Text = pmcProviderDescGet( EditInfo_p->providerId, buf_p );

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::PatientEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    PatientList( 0 );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::PatientClearButtonClick(TObject *Sender)
{
    EditInfo_p->patientId = 0;
    PatientUpdate( EditInfo_p->patientId );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::ProviderClearButtonClick(TObject *Sender)
{
    EditInfo_p->providerId = 0;
    ProviderComboBox->ItemIndex = -1;
    ProviderIndex = ProviderComboBox->ItemIndex;
    ProviderComboBox->Text = "";
}
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::PatientEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    PatientList( (Int32u_t)Key );
}
//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::PatientList( Int32u_t key )
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    if( EditInfo_p->patientId == 0 )
    {
        patListInfo.patientId = pmcPatIdRetrieve( FALSE );
    }
    else
    {
        patListInfo.patientId = EditInfo_p->patientId;
    }
    patListInfo.providerId = EditInfo_p->providerId;
    patListInfo.mode = PMC_LIST_MODE_SELECT;
    patListInfo.character = key;
    patListInfo.allowGoto = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
        if( patListInfo.patientId != EditInfo_p->patientId )
        {
            EditInfo_p->patientId = patListInfo.patientId;
            PatientUpdate( EditInfo_p->patientId );

            // Store the patient ID for possible reuse
            pmcPatIdStore( EditInfo_p->patientId );
        }
    }
    else
    {
        // User pressed cancel button - do nothing
    }
}

//---------------------------------------------------------------------------

void __fastcall TDocumentEditForm::FailButtonClick(TObject *Sender)
{
    mbLog( "Document edit form fail button clicked\n" );
    EditInfo_p->returnCode = MB_BUTTON_FAILED;
    Close( );
}

//---------------------------------------------------------------------------
// This function is used by the "CONSULT" document import to disable
// the appropriate controls.  In the future, this function might be
// made more generic
//---------------------------------------------------------------------------
Int32s_t __fastcall TDocumentEditForm::disableControls( void )
{
    DocumentTypeComboBox->Enabled = FALSE;
    PatientEdit->Enabled = FALSE;
    PatientClearButton->Enabled = FALSE;
    PatientEditButton->Enabled = FALSE;
    ProviderComboBox->Enabled = FALSE;
    ProviderClearButton = FALSE;
    DateEdit->Enabled = FALSE;
    DateEditButton->Enabled = FALSE;
    PatientListButton->Enabled = FALSE;

    return MB_RET_OK;
}
