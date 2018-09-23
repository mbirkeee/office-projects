//---------------------------------------------------------------------------
// File:    pmcPatientListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 15, 2001
//---------------------------------------------------------------------------
// Form for displaying list of patients
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>

#pragma hdrstop

#include "mbUtils.h"
#include "mbSqlLib.h"

#include "pmcTables.h"
#include "pmcUtils.h"
#include "pmcGlobals.h"

#include "pmcMainForm.h"
#include "pmcPatientRecord.h"
#include "pmcPatientListForm.h"
#include "pmcPatientEditForm.h"
#include "pmcClaimEditForm.h"
#include "pmcClaimListForm.h"
#include "pmcDocumentListForm.h"
#include "pmcSeiko240.h"
#include "pmcEchoListForm.h"
#include "pmcPatientForm.h"
// #include "pmcPDF.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function: TPatientListForm::~TPatientListForm()
//---------------------------------------------------------------------------
// Description:
//
// Destructor
//---------------------------------------------------------------------------

__fastcall TPatientListForm::~TPatientListForm( Void_t )
{
    FreeMasterList( );
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::TPatientListForm()
//---------------------------------------------------------------------------
// Description:
//
// Default constructor.
//---------------------------------------------------------------------------

__fastcall TPatientListForm::TPatientListForm( TComponent*  Owner )
    : TForm(Owner)
{
    mbDlgDebug(( "Default patient list constructor called" ));
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::TPatientListForm()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

__fastcall TPatientListForm::TPatientListForm
(
    TComponent*         Owner,
    PmcPatListInfo_p    patListInfo_p
)
    : TForm(Owner)
{
    Char_t  buf[8];
    Ints_t  selStart;
    MbSQL   sql;

    {
        Ints_t  height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_PAT_LIST, &height, &width, &top, &left ) == MB_RET_OK )
        {
            SetBounds( left, top, width, height );
        }
    }

    MainForm->SkipTimer = TRUE;

    PatForm_p = NIL;
    PatFormOffset_p = NIL;
    Ready = FALSE;
    UpdateSelectedPatientIdFlag = TRUE;
    DoubleClickFlag = FALSE;
    ClearFlag = FALSE;

    TotalPatientRecords = 0;
    CurrentPatientRecords = 0;

    SearchString[0] = 0;

    MouseInRow = 0;

    // PatListInfo_p is used to return information to the calling code
    PatListInfo_p = patListInfo_p;

    PatListInfo_p->returnCode = MB_BUTTON_CANCEL;

    PatListInfo_p->gotoProviderId = 0;
    PatListInfo_p->gotoDate = 0;

    SelectedPatientId = patListInfo_p->patientId;

    if( PatListInfo_p->mode == PMC_LIST_MODE_LIST )
    {
        OkButton->Visible = FALSE;
        CancelButton->Caption = "Close";
    }

    SkipCurrentListBuild = TRUE;

    // Handle case where this list was invoked by a character press
    buf[0] = 0;
    selStart = 0;
    if( PatListInfo_p->character >= 0x41 && PatListInfo_p->character <= 0x5A ) PatListInfo_p->character += 0x20;
    if( PatListInfo_p->character >= 0x61 && PatListInfo_p->character <= 0x7A )
    {
        buf[0] = (Int8u_t)PatListInfo_p->character;
        buf[1] = 0;
        selStart = 1;
    }
    SearchEdit->Text = buf;
    SearchEdit->SelStart = selStart;

    SortIndex = PMC_CURRENT_LIST_NAME;

    SkipCurrentListBuild = FALSE;
    BuildMasterList( );
    Ready = TRUE;
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::FreeMasterList()
//---------------------------------------------------------------------------
// Description:
//
// Frees all allocated memory in form's patient list
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::FreeMasterList
(
    void
)
{
    if( PatFormOffset_p )
    {
        mbFree( PatFormOffset_p );
        PatFormOffset_p =NIL;
    }
    if( PatForm_p )
    {
        for( Ints_t i = TotalPatientRecords - 1 ; i >= 0 ; i-- )
        {
            mbFree( PatForm_p[i].firstName_p );
            mbFree( PatForm_p[i].lastName_p );
            mbFree( PatForm_p[i].lastNameSearch_p );
            mbFree( PatForm_p[i].homePhone_p );
            mbFree( PatForm_p[i].homePhoneSearch_p );
            mbFree( PatForm_p[i].phn_p );
            mbFree( PatForm_p[i].phnSearch_p );
        }
        mbFree( PatForm_p );
        PatForm_p = NIL;
    }
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::BuildMasterList()
//---------------------------------------------------------------------------
// Description:
//
// Construct master list of patients
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::BuildMasterList
(
    Void_t
)
{
    pmcPatRecordStruct_p    patRecord_p;
    Ints_t                  i;
    pmcLinkageStruct_p      linkage_p;
    AnsiString              str = "";

    if( mbLockAttempt( pmcPatListLock ) == FALSE )
    {
        mbDlgDebug(( "Patient records locked by another thread" ));
        return;
    }

    // Must free allocated memory here
    FreeMasterList( );

    // Allocate array of structures for the various strings in the patient database
    mbCalloc( PatForm_p,          sizeof( pmcPatFormStruct_t ) * ( pmcPatName_q->size ));
    mbCalloc( PatFormOffset_p,    sizeof( pmcPatFormOffset_t ) * ( pmcPatName_q->size ));

    TotalPatientRecords = pmcPatName_q->size;

    i = 0;

    // Loop through master list, making a copy of all required fields
    qWalk( linkage_p, pmcPatName_q, pmcLinkageStruct_p )
    {
        // Get pointer to start of this record
        patRecord_p = (pmcPatRecordStruct_p)linkage_p->record_p;

        // Copy the first name
        mbMalloc( PatForm_p[i].firstName_p, patRecord_p->firstNameLen );
        strcpy( PatForm_p[i].firstName_p, patRecord_p->firstName_p );

        // Copy the last name
        mbMalloc( PatForm_p[i].lastName_p, patRecord_p->lastNameLen );
        strcpy( PatForm_p[i].lastName_p, patRecord_p->lastName_p );

        mbMalloc( PatForm_p[i].lastNameSearch_p, patRecord_p->lastNameSearchLen );
        strcpy( PatForm_p[i].lastNameSearch_p, patRecord_p->lastNameSearch_p );

        mbMalloc( PatForm_p[i].homePhone_p, patRecord_p->homePhoneLen );
        strcpy( PatForm_p[i].homePhone_p, patRecord_p->homePhone_p );

        mbMalloc( PatForm_p[i].homePhoneSearch_p, patRecord_p->homePhoneSearchLen );
        strcpy( PatForm_p[i].homePhoneSearch_p, patRecord_p->homePhoneSearch_p );

        mbMalloc( PatForm_p[i].phn_p, patRecord_p->phnLen );
        strcpy( PatForm_p[i].phn_p, patRecord_p->phn_p );

        mbMalloc( PatForm_p[i].phnSearch_p, patRecord_p->phnSearchLen );
        strcpy( PatForm_p[i].phnSearch_p, patRecord_p->phnSearch_p );

        PatForm_p[i].areaCode[0] = 0;
        if( patRecord_p->displayAreaCode == TRUE )
        {
            strncpy( PatForm_p[i].areaCode, patRecord_p->areaCode, PMC_AREA_CODE_LEN );
        }

        strncpy( PatForm_p[i].title, patRecord_p->title, PMC_TITLE_LEN );

        PatForm_p[i].patientId = patRecord_p->id;

        PatFormOffset_p[i].name = patRecord_p->offset;
        i++;
    }

    // Must also get record offsets sorted by phone...
    i = 0;

    qWalk( linkage_p, pmcPatPhone_q, pmcLinkageStruct_p )
    {
        // Get pointer to start of this record
        patRecord_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        PatFormOffset_p[i].phone = patRecord_p->offset;
        i++;
    }

    // Must also get record offsets sorted by phn...
    i = 0;

    qWalk( linkage_p, pmcPatPhn_q, pmcLinkageStruct_p )
    {
        // Get pointer to start of this record
        patRecord_p = (pmcPatRecordStruct_p)linkage_p->record_p;
        PatFormOffset_p[i].phn = patRecord_p->offset;
        i++;
    }

    mbLockRelease( pmcPatListLock );
    BuildCurrentList( SortIndex, SearchEdit->Text.c_str() );
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::PatientListGridCellClick()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientListGridCellClick(TColumn *Column)
{
    mbDlgDebug(( "TPatientListForm::PatientListGridCellClick called" ));
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::Button2Click()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::ShowAllButtonClick(TObject *Sender)
{
    AnsiString str = "";

    // Simply updating the text should cause a rebuild of the current list
    SearchEdit->Text = str;

    MainForm->RefreshTable( PMC_TABLE_BIT_PATIENTS );
    SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Function:  TPatientListForm::PatientStringGridDrawCell()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientStringGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect          &Rect,
    TGridDrawState  State
)
{
    AnsiString              str  = "";
    pmcPatFormStruct_p      patForm_p;

    if( ARow == PatientStringGrid->Row )
    {
        UpdateSelectedPatient( );
    }

    if( ARow == 0 )
    {
        // This is the title row
        str = pmcPatListStrings[ACol];
    }
    else if( ARow <= CurrentPatientRecords )
    {
       // Get pointer to current record
        patForm_p = &PatForm_p[PatFormOffset_p[ARow - 1].current];

        if( ACol == 0 ) str = patForm_p->lastName_p;
        if( ACol == 1 ) str = patForm_p->firstName_p;
        if( ACol == 2 ) str = patForm_p->title;
        if( ACol == 3 ) str = patForm_p->areaCode;
        if( ACol == 4 ) str = patForm_p->homePhone_p;
        if( ACol == 5 ) str = patForm_p->phn_p;
    }
    PatientStringGrid->Canvas->TextRect( Rect, Rect.Left + 2, Rect.Top, str );
}

//---------------------------------------------------------------------------
// Function:   TPatientListForm::PatientStringGridClick()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientStringGridClick(TObject *Sender)
{
   UpdateSelectedPatient( );
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::FormDestroy()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

// Function: TPatientListForm::FormClose()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    if( PatListInfo_p )
    {
        PatListInfo_p->patientId = (ClearFlag == TRUE ) ? 0 : SelectedPatientId;
    }

    MainForm->SkipTimer = FALSE;
    mbPropertyWinSave( PMC_WINID_PAT_LIST, Height, Width, Top, Left );
    Action = caFree;
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::BuildCurrentList()
//---------------------------------------------------------------------------
// Description:
//
// Filter list based on index and search string
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::BuildCurrentList
(
    Ints_t              index,
    Char_p              str_p
)
{
    Ints_t              i, j;
    Ints_t              searchLen = 0;
    pmcPatFormStruct_p  patForm_p;
    Char_t              buf[128];
    Char_p              searchString_p = NIL;
    Ints_t              matchRow = 0;

    if( SkipCurrentListBuild ) return;

    // Format the search strings
    if( str_p )
    {
        searchLen = strlen( str_p );
        mbMalloc( searchString_p , searchLen + 1 );
        strcpy( searchString_p, str_p );
        strcpy( SearchString, str_p );
        mbStrToLower( searchString_p );
    }

    CurrentPatientRecords = TotalPatientRecords;

    switch( index )
    {
        case PMC_CURRENT_LIST_NAME:
            for( i = 0 , j = 0 ; i < TotalPatientRecords ; i++ )
            {
                if( searchLen == 0 )
                {
                    // Put all records into list
                    PatFormOffset_p[j++].current = PatFormOffset_p[i].name;

                    // Check to see  if this is the selected patient
                    if( SelectedPatientId == PatForm_p[PatFormOffset_p[i].name].patientId )
                    {
                        matchRow = i;
                    }
                }
                else
                {
                    // Get pointer to actual record
                    patForm_p = &PatForm_p[PatFormOffset_p[i].name];

                    // Check if search string anywhere within string
                    if( mbStrPos( patForm_p->lastNameSearch_p, searchString_p ) == 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].name;
                    }
                    else if( mbStrPos( patForm_p->homePhoneSearch_p, searchString_p ) >= 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].name;
                    }
                    else if( mbStrPos( patForm_p->phnSearch_p, searchString_p ) == 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].name;
                    }
                }
            }
            CurrentPatientRecords = j;
            break;

        case PMC_CURRENT_LIST_PHONE:
            for( i = 0 , j = 0 ; i < TotalPatientRecords ; i++ )
            {
                if( searchLen == 0 )
                {
                    PatFormOffset_p[j++].current = PatFormOffset_p[i].phone;
                    // Check to see  if this is the selected patient
                    if( SelectedPatientId == PatForm_p[PatFormOffset_p[i].phone].patientId )
                    {
                        matchRow = i;
                    }
                }
                else
                {
                    // Get pointer to actual record
                    patForm_p = &PatForm_p[PatFormOffset_p[i].phone];

                    if( mbStrPos( patForm_p->lastNameSearch_p, searchString_p ) == 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].phone;
                    }
                    else if( mbStrPos( patForm_p->homePhoneSearch_p, searchString_p ) >= 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].phone;
                    }
                    else if( mbStrPos( patForm_p->phnSearch_p, searchString_p ) == 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].phone;
                    }
                }
            }
            CurrentPatientRecords = j;
            break;

        case PMC_CURRENT_LIST_PHN:
            for( i = 0 , j = 0 ; i < TotalPatientRecords ; i++ )
            {
                if( searchLen == 0 )
                {
                    PatFormOffset_p[j++].current = PatFormOffset_p[i].phn;
                    // Check to see  if this is the selected patient
                    if( SelectedPatientId == PatForm_p[PatFormOffset_p[i].phn].patientId )
                    {
                        matchRow = i;
                    }
                }
                else
                {
                    // Get pointer to actual record
                    patForm_p = &PatForm_p[PatFormOffset_p[i].phn];

                    if( mbStrPos( patForm_p->lastNameSearch_p, searchString_p ) == 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].phn;
                    }
                    else if( mbStrPos( patForm_p->homePhoneSearch_p, searchString_p ) >= 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].phn;
                    }
                    else if( mbStrPos( patForm_p->phnSearch_p, searchString_p ) == 0 )
                    {
                        PatFormOffset_p[j++].current = PatFormOffset_p[i].phn;
                    }
                }
            }
            CurrentPatientRecords = j;
            break;

        default:
            mbDlgDebug(( "Invalid search index: %ld", index ));
            for( i = 0 ; i < TotalPatientRecords ; i++ )
            {
                PatFormOffset_p[i].current = PatFormOffset_p[i].name;
            }
            break;
    }

    if( searchString_p ) mbFree( searchString_p );

    if( CurrentPatientRecords )
    {
        PatientStringGrid->RowCount = CurrentPatientRecords + 1;
    }
    else
    {
        PatientStringGrid->RowCount = 2;
    }

    if( searchLen == 0 )
    {
        PatientStringGrid->TopRow = matchRow + 1;
        PatientStringGrid->Row = matchRow + 1;
    }
    else
    {
        PatientStringGrid->TopRow = 1;
        PatientStringGrid->Row = 1;
    }

    PatientStringGrid->Invalidate( );

    UpdateSelectedPatient( );

    mbStrInt32u( CurrentPatientRecords, buf );
    RecordsShownLabel->Caption = buf;

    mbStrInt32u( TotalPatientRecords, buf );
    RecordsTotalLabel->Caption = buf;

    return;
}

//---------------------------------------------------------------------------
// Function:  TPatientListForm::SearchEditChange()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::SearchEditChange(TObject *Sender)
{
    if( Ready )
    {
        BuildCurrentList( SortIndex, SearchEdit->Text.c_str() );
    }
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::UpdateSelectedPatient()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::UpdateSelectedPatient( void )
{
    Char_t                  buf[128];
    pmcPatFormStruct_p      patForm_p;

    if( CurrentPatientRecords == 0 )
    {
        // There are not patients in the shhown list
        ClearPatient();
    }
    else if( PatientStringGrid->Row > 0 )
    {
        // First, must get a pointer to the current patient record
        patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];

        sprintf( buf, "%s, %s", patForm_p->lastName_p, patForm_p->firstName_p );

        LastNameLabel->Caption = buf;

        sprintf( buf, "%s", patForm_p->phn_p );
        PhnLabel->Caption = buf;
        ClearFlag = FALSE;

        if( UpdateSelectedPatientIdFlag )
        {
            SelectedPatientId = patForm_p->patientId;
        }
    }
    else
    {
        mbDlgInfo("SHOULD NOT BE HERE in Update Slected  Patient\n");
    }

    if( Ready ) SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// New patient button click handler
//---------------------------------------------------------------------------
void __fastcall TPatientListForm::NewPatientButtonClick(TObject *Sender)
{
    CreateNewPatient();
}

void __fastcall TPatientListForm::CreateNewPatient( void )
{
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( PatientStringGrid->Row == 0 )
    {
        mbDlgDebug(( "Got row == 0. Should not be here" ));
        return;
    }

    patEditInfo.patientId = 0;
    patEditInfo.mode = PMC_EDIT_MODE_NEW;
    patEditInfo.providerId = PatListInfo_p->providerId;
    strcpy( patEditInfo.searchString, SearchString );
    sprintf( patEditInfo.caption, "Enter New Patient Details" );

    patEditForm = new TPatientEditForm( NULL, &patEditInfo );
    patEditForm->ShowModal();
    delete patEditForm;

    if( patEditInfo.returnCode == MB_BUTTON_OK )
    {
        SelectedPatientId = patEditInfo.patientId;

        // Clear search string so that the new patient will be selected in the list
        SkipCurrentListBuild = TRUE;
        SearchEdit->Text = "";
        SkipCurrentListBuild = FALSE;

        if( MainForm->RefreshTableForce( PMC_TABLE_BIT_PATIENTS ) )
        {
            UpdateSelectedPatientIdFlag = FALSE;
            BuildMasterList( );
            UpdateSelectedPatientIdFlag = TRUE;
        }
    }
    else
    {
       // user clicked cancel
    }
    if( Ready ) SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Function:  TPatientListForm::PatientGridPopupPopup()
//---------------------------------------------------------------------------
// Description:
//
// Popup menu has popped up.
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientGridPopupPopup(TObject *Sender)
{
    pmcPopupItemEnableAll( PatientGridPopup, FALSE );

    // Get patient over which mouse is located
    if( MouseInRow <= CurrentPatientRecords && MouseInRow > 0 )
    {
        PatientStringGrid->Row = MouseInRow;
        UpdateSelectedPatient( );

        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_VIEW,          TRUE );
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_EDIT,          TRUE );
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_DELETE,        TRUE );
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_LIST_APP,      TRUE );
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_LIST_DOC,      TRUE );

        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_PRINT,         TRUE );
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_PRINT_RECORD,  TRUE );
        if( pmcCfg[CFG_SLP_NAME].str_p )
        {
            pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_PRINT_LBL_ADDR, TRUE );
            pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_PRINT_LBL_REQ,  TRUE );
        }
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_CLAIMS_NEW,    TRUE );
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_CLAIMS_LIST,   TRUE );
        pmcPopupItemEnable( PatientGridPopup, PMC_PAT_LIST_POPUP_ECHO_LIST,     TRUE );
    }
    else
    {
        // Mouse id not over a valid patient record
    }
}

//---------------------------------------------------------------------------
// Function:  TPatientListForm::PatientStringGridMouseDown
//---------------------------------------------------------------------------
// Description:
//
// Mouse button has been pressed in the PatientStringGrid.
// Determine the row (i.e., patient record) from the co-ordinates
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientStringGridMouseDown
(
    TObject        *Sender,
    TMouseButton    Button,
    TShiftState     Shift,
    int             X,
    int             Y
)
{
    Ints_t          row = 0, col = 0;
    TRect           rect;
    Char_p          str_p;

    if( DoubleClickFlag == TRUE )
    {
        DoubleClickFlag = FALSE;
        return;
    }

    PatientStringGrid->MouseToCell( X, Y, col, row );
    rect = PatientStringGrid->CellRect( col, row );

    if( row >= 0 && row <= CurrentPatientRecords )
    {
        MouseInRow = row;
    }
    else
    {
        MouseInRow = -1;
    }

    if( col >= 0 && col <= 5 )
    {
        MouseInCol = col;
    }
    else
    {
        MouseInCol = -1;
    }

    if( row == 0 )
    {
        str_p = pmcPatListStrings[col];
        pmcButtonDown( PatientStringGrid->Canvas, &rect, str_p );
    }
}

//---------------------------------------------------------------------------
// Function:  TPatientListForm::PatientStringGridMouseUp
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientStringGridMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    pmcButtonUp( );

    if( MouseInRow == 0 )
    {
        if( MouseInCol == 0 || MouseInCol == 1 || MouseInCol == 2 )
        {
            SortIndex = PMC_CURRENT_LIST_NAME;
        }
        if( MouseInCol == 3 || MouseInCol == 4 )
        {
            SortIndex = PMC_CURRENT_LIST_PHONE;
        }
        if( MouseInCol == 5 )
        {
            SortIndex = PMC_CURRENT_LIST_PHN;
        }
        BuildCurrentList( SortIndex, SearchEdit->Text.c_str() );
        SearchEdit->SetFocus();
    }
    PatientStringGrid->Invalidate( );
}

//---------------------------------------------------------------------------
// Function:  TPatientListForm::ViewClick()
//---------------------------------------------------------------------------
// Description:
//
// View patient record has been selected from the popup menu.
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupViewClick(TObject *Sender)
{
    pmcPatFormStruct_p      patForm_p;
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( PatientStringGrid->Row == 0 )
    {
        mbDlgDebug(( "Got row == 0. Should not be here" ));
        return;
    }

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];
    patEditInfo.patientId = patForm_p->patientId;

    sprintf( patEditInfo.caption, "View Patient Details - %s, %s",
        patForm_p->lastName_p, patForm_p->firstName_p );

    patEditInfo.mode = PMC_EDIT_MODE_VIEW;

    patEditForm = new TPatientEditForm( NULL, &patEditInfo );
    patEditForm->ShowModal();
    delete patEditForm;

    //mbDlgDebug(( "Got Patient View Click, patient ID: %d (%s, %s)",
    //    patForm_p->patientId, patForm_p->lastName_p, patForm_p->firstName_p ));
}

//---------------------------------------------------------------------------
// Function: TPatientListForm::RefreshButtonClick()
//---------------------------------------------------------------------------
// Description:
//
// Get updated patient list from main form.
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::RefreshButtonClick(TObject *Sender)
{
    BuildMasterList( );
    if( Ready ) SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::FormActivate(TObject *Sender)
{
    /* Check for updated patient records when form recevies focus */
    if( MainForm->RefreshTable( PMC_TABLE_BIT_PATIENTS ) )
    {
        BuildMasterList( );
    }
    if( Ready ) SearchEdit->SetFocus( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::OkButtonClick(TObject *Sender)
{
    PatListInfo_p->returnCode = MB_BUTTON_OK;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::CancelButtonClick(TObject *Sender)
{
    PatListInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupEditClick(TObject *Sender)
{
    PatListEditPatient( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::DeleteClick(TObject *Sender)
{
    pmcPatFormStruct_p      patForm_p;
    Char_p                  cmd_p;

    mbMalloc( cmd_p, 256 );

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];

    // Check to see if this patient has any appointments in the database
    // Query the appointents table
    sprintf( cmd_p, "select %s,%s,%s,%s from %s where %s=%ld and %s=1",
        PMC_SQL_APPS_FIELD_START_TIME,
        PMC_SQL_FIELD_DATE,
        PMC_SQL_FIELD_PROVIDER_ID,
        PMC_SQL_APPS_FIELD_DURATION,
        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_PATIENT_ID, patForm_p->patientId,
        PMC_SQL_FIELD_NOT_DELETED );

    if( pmcSqlMatchCount( cmd_p ) )
    {
        mbDlgExclaim( "Cannot delete patient with scheduled appointments." );
        goto exit;
    }

    sprintf( cmd_p, "select %s from %s where %s=%ld and %s=1",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_CLAIMS,
        PMC_SQL_FIELD_PATIENT_ID, patForm_p->patientId,
        PMC_SQL_FIELD_NOT_DELETED );

    if( pmcSqlMatchCount( cmd_p ) )
    {
        mbDlgExclaim( "Cannot delete a patient with paid or pending claims." );
        goto exit;
    }

    sprintf( cmd_p, "select %s from %s where %s=%ld and %s=1",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_DOCUMENTS,
        PMC_SQL_FIELD_PATIENT_ID, patForm_p->patientId,
        PMC_SQL_FIELD_NOT_DELETED );

    if( pmcSqlMatchCount( cmd_p ) )
    {
        mbDlgExclaim( "Cannot delete a patient with documents in the database." );
        goto exit;
    }

    sprintf( cmd_p, "select %s from %s where %s=%ld and %s=1",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_ECHOS,
        PMC_SQL_FIELD_PATIENT_ID, patForm_p->patientId,
        PMC_SQL_FIELD_NOT_DELETED );

    if( pmcSqlMatchCount( cmd_p ) )
    {
        mbDlgExclaim( "Cannot delete a patient with echos in the database." );
        goto exit;
    }

    if( mbDlgOkCancel( "Delete patient record for %s %s?",
        patForm_p->firstName_p, patForm_p->lastName_p ) == MB_BUTTON_OK )
    {
        pmcSqlRecordDelete( PMC_SQL_TABLE_PATIENTS, patForm_p->patientId );
        mbLog( "Deleted patient id: %ld\n", patForm_p->patientId );
        if( MainForm->RefreshTableForce( PMC_TABLE_BIT_PATIENTS ) )
        {
            BuildMasterList( );
        }
    }

exit:
    mbFree( cmd_p );
    SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupListAppClick(TObject *Sender)
{
    pmcPatFormStruct_p      patForm_p;

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];

    pmcViewAppointments( patForm_p->patientId, TRUE, FALSE,
        PatListInfo_p->allowGoto,
        &PatListInfo_p->gotoProviderId,
        &PatListInfo_p->gotoDate, TRUE, PMC_LIST_MODE_LIST );

    if( PatListInfo_p->gotoProviderId != 0 && PatListInfo_p->gotoDate != 0 && PatListInfo_p->allowGoto == TRUE )
    {
        Close( );
    }

    // Call invalidate in attempt to get form to redraw properly in W2K
    PatientStringGrid->Invalidate( );
    Invalidate( );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupPrintRecordClick(TObject *Sender)
{
    pmcPatFormStruct_p      patForm_p;
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];
    //pmcPatRecordHTML( patForm_p->patientId );

    patEditInfo.patientId = patForm_p->patientId;
    patEditInfo.mode = PMC_EDIT_MODE_VIEW;

    patEditForm = new TPatientEditForm( NULL, &patEditInfo );

    patEditForm->PrintDocument( PMC_PDF_TEMPLATE_PAT_RECORD );
    delete patEditForm;

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupClaimsNewClick(TObject *Sender)
{
    pmcClaimEditInfo_t    claimEditInfo;
    TClaimEditForm       *claimEditForm;
    pmcPatFormStruct_p    patForm_p;

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];

    claimEditInfo.mode = PMC_CLAIM_EDIT_MODE_NEW;
    claimEditInfo.patientId = patForm_p->patientId;
    claimEditInfo.providerId = 0;
    claimEditForm = new TClaimEditForm( this, &claimEditInfo );
    claimEditForm->ShowModal();
    delete claimEditForm;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupClaimsListClick
(
    TObject                *Sender
)
{
    pmcClaimListInfo_t      claimListInfo;
    TClaimListForm         *claimListForm_p;
    pmcPatFormStruct_p      patForm_p;
    MbDateTime              dateTime;

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];

    claimListInfo.patientId = patForm_p->patientId;

    dateTime.SetDate( mbToday( ) );

    claimListInfo.latestDate = mbToday( );
    claimListInfo.earliestDate = dateTime.BackYears( 10 );

    claimListInfo.providerId = 0;
    claimListInfo.providerAllFlag = TRUE;
    claimListInfo.patientAllFlag = FALSE;

    claimListForm_p = new TClaimListForm( NULL, &claimListInfo );
    claimListForm_p->ShowModal();
    delete claimListForm_p;

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupEchoListClick(
      TObject *Sender)
{
    pmcPatFormStruct_p      patForm_p;

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];
    pmcEchoListForm( patForm_p->patientId );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupPrintAddrLabelClick
(
    TObject                *Sender
)
{
    pmcPatFormStruct_p      patForm_p;

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];
    pmcLabelPrintPatAddress( patForm_p->patientId );
}

//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupPrintReqLabelClick
(
      TObject *Sender
)
{
    pmcPatFormStruct_p      patForm_p;

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];
    pmcLabelPrintPatReq( 0 , 0, patForm_p->patientId );
}

//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientStringGridDblClick
(
      TObject *Sender
)
{
    pmcPatFormStruct_p      patForm_p;
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];

    DoubleClickFlag = TRUE;

    if( MouseInRow == -1 || MouseInRow == 0 )
    {
        return;
    }

    if( PatListInfo_p->mode == PMC_LIST_MODE_LIST )
    {
        if( pmcNewPatForm )
        {
            pmcPatientFormNonModal( patForm_p->patientId );
        }
        else
        {
            PatListEditPatient( );
        }
    }
    else
    {
        // We are in select mode so just close the list
        PatListInfo_p->returnCode = MB_BUTTON_OK;
        Close( );
    }
}
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListEditPatient( void )
{
    pmcPatFormStruct_p      patForm_p;
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( PatientStringGrid->Row == 0 )
    {
        mbDlgDebug(( "Got row == 0. Should not be here" ));
        return;
    }

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];
    patEditInfo.patientId = patForm_p->patientId;
    patEditInfo.mode = PMC_EDIT_MODE_EDIT;
    sprintf( patEditInfo.caption, "Edit Patient Details - %s, %s",
        patForm_p->lastName_p, patForm_p->firstName_p );

    patEditForm = new TPatientEditForm( NULL, &patEditInfo );
    patEditForm->ShowModal();
    delete patEditForm;

    if( patEditInfo.returnCode == MB_BUTTON_OK )
    {
        SelectedPatientId = patEditInfo.patientId;

        SkipCurrentListBuild = TRUE;
        SearchEdit->Text = "";
        SkipCurrentListBuild = FALSE;

        if( MainForm->RefreshTableForce( PMC_TABLE_BIT_PATIENTS ) )
        {
            UpdateSelectedPatientIdFlag = FALSE;
            BuildMasterList( );
            UpdateSelectedPatientIdFlag = TRUE;
        }
    }
    else
    {
        // User must have clicked cancel button
    }

    nbDlgDebug(( "Got Patient View Click, patient ID: %d (%s, %s)",
        patForm_p->patientId, patForm_p->lastName_p, patForm_p->firstName_p ));
}
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatListPopupListDocClick(TObject *Sender)
{
    TDocumentListForm          *form_p;
    pmcDocumentListInfo_t       formInfo;
    pmcPatFormStruct_p          patForm_p;

    // First, must get a pointer to the current patient record
    patForm_p = &PatForm_p[PatFormOffset_p[PatientStringGrid->Row - 1].current];

    formInfo.patientId = patForm_p->patientId;
    form_p = new TDocumentListForm( NIL, &formInfo );
    form_p->ShowModal( );
    delete form_p;
}

//---------------------------------------------------------------------------

void __fastcall TPatientListForm::SearchEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    if( Key == VK_UP )
    {
        if( PatientStringGrid->Row > 1 ) PatientStringGrid->Row--;
        Key = 0;
    }
    else if( Key == VK_DOWN )
    {
        if( PatientStringGrid->Row < PatientStringGrid->RowCount - 1 )
        {
            PatientStringGrid->Row++;
        }
        Key = 0;
    }
    else if( Key == VK_NEXT )
    {
        if( PatientStringGrid->Row < PatientStringGrid->RowCount - 25 )
        {
            PatientStringGrid->Row += 25;
        }
        else
        {
            PatientStringGrid->Row = PatientStringGrid->RowCount - 1 ;
        }
    }
    else if( Key == VK_PRIOR )
    {
        if( PatientStringGrid->Row > 26 )
        {
            PatientStringGrid->Row -= 25;
        }
        else
        {
         PatientStringGrid->Row = 1;
        }
    }
    else if( Key == VK_RETURN )
    {
        // mbLog("MAB: Got a CR in the Search Edit\n");
        if( CurrentPatientRecords == 0 )
        {
            CreateNewPatient();
        }
        else
        {
            if( this->PatListInfo_p->mode == PMC_LIST_MODE_SELECT )
            {
                PatListInfo_p->returnCode = MB_BUTTON_OK;
                Close( );
            }
            else
            {
                PatListEditPatient( );
            }
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::PatientStringGridKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    if( Key == VK_RETURN )
    {
        // mbLog("MAB: Got a CR in GridKeyDown\n");
        
        if( this->PatListInfo_p->mode == PMC_LIST_MODE_SELECT )
        {
            PatListInfo_p->returnCode = MB_BUTTON_OK;
            Close( );
        }
        else
        {
            PatListEditPatient( );
        }
    }
    else
    {
        SearchEdit->SetFocus();
    }
}
//---------------------------------------------------------------------------

void __fastcall TPatientListForm::ButtonClearClick(TObject *Sender)
{
    ClearPatient();

}

//---------------------------------------------------------------------------

void __fastcall TPatientListForm::ClearPatient( void )
{
    ClearFlag = TRUE;
    LastNameLabel->Caption = "";
    PhnLabel->Caption = "";
}

