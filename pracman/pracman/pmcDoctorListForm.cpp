//---------------------------------------------------------------------------
// File:    pmcDoctorListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for managing the doctor list form
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <alloc.h>
#include <mem.h>

#pragma hdrstop

#include "mbUtils.h"

#include "pmcTables.h"
#include "pmcUtils.h"
#include "pmcMainForm.h"
#include "pmcDoctorListForm.h"
#include "pmcDoctorEditForm.h"
#include "pmcSeiko240.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function: TDoctorListForm::~TDoctorListForm()
//---------------------------------------------------------------------------
// Description:
//
// Destructor
//---------------------------------------------------------------------------

__fastcall TDoctorListForm::~TDoctorListForm( Void_t )
{
    FreeMasterList( );
}

//---------------------------------------------------------------------------
__fastcall TDoctorListForm::TDoctorListForm(TComponent* Owner)
    : TForm(Owner)
{
    mbDlgDebug(( "Default constructor called" ));
}

//---------------------------------------------------------------------------

__fastcall TDoctorListForm::TDoctorListForm
(
    TComponent*         Owner,
    pmcDocListInfo_p    docListInfo_p
)
    : TForm(Owner)
{
    {
        Ints_t  height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_DOCTOR_LIST, &height, &width, &top, &left ) == MB_RET_OK )
        {
            SetBounds( left, top, width, height );
        }
    }

    DocForm_p = NIL;
    DocFormOffset_p = NIL;
    Ready = FALSE;
    UpdateSelectedDoctorIdFlag = TRUE;

    ClearFlag = FALSE;
    DoubleClickFlag = FALSE;

    TotalDoctorRecords = 0;
    CurrentDoctorRecords = 0;

    MouseInRow = 0;

    SkipCurrentListBuild = TRUE;

    FormInfo_p = docListInfo_p;
    FormInfo_p->returnCode = MB_BUTTON_CANCEL;

    SelectedDoctorId = docListInfo_p->doctorId;

    if( FormInfo_p->mode == PMC_LIST_MODE_LIST )
    {
        OkButton->Visible = FALSE;
        CancelButton->Caption = "Close";
    }

    SearchEdit->Text = "";
    SortIndex = PMC_CURRENT_LIST_NAME;

    SkipCurrentListBuild = FALSE;
    BuildMasterList( );
    Ready = TRUE;
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::FreeMasterList()
//---------------------------------------------------------------------------
// Description:
//
// Frees all allocated memory in form's patient list
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::FreeMasterList
(
    void
)
{
    if( DocFormOffset_p )
    {
        mbFree( DocFormOffset_p );
        DocFormOffset_p =NIL;
    }
    if( DocForm_p )
    {
        for( Ints_t i = TotalDoctorRecords - 1 ; i >= 0  ; i-- )
        {
            mbFree( DocForm_p[i].firstName_p );
            mbFree( DocForm_p[i].lastName_p );
            mbFree( DocForm_p[i].lastNameSearch_p );
            mbFree( DocForm_p[i].workPhone_p );
            mbFree( DocForm_p[i].workFax_p );
            mbFree( DocForm_p[i].workPhoneSearch_p );
            mbFree( DocForm_p[i].num_p );
        }
        mbFree( DocForm_p );
        DocForm_p = NIL;
    }
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::BuildMasterList()
//---------------------------------------------------------------------------
// Description:
//
// Construct master list of patients
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::BuildMasterList
(
    Void_t
)
{
    pmcDocRecordStruct_p    docRecord_p;
    Ints_t                  i;
    pmcLinkageStruct_p      linkage_p;
    AnsiString              str = "";

    if( mbLockAttempt( pmcDocListLock ) == FALSE )
    {
        mbDlgDebug(( "Doctor records locked by another thread" ));
        return;
    }

    // Must free allocated memory here
    FreeMasterList( );

    // Allocate array of structures for the various strings in the patient database
    mbCalloc( DocForm_p,          sizeof( pmcDocFormStruct_t ) * ( pmcDocName_q->size ));
    mbCalloc( DocFormOffset_p,    sizeof( pmcDocFormOffset_t ) * ( pmcDocName_q->size ));

    TotalDoctorRecords = pmcDocName_q->size;

    i = 0;

    // Loop through master list, making a copy of all required fields

    qWalk( linkage_p, pmcDocName_q, pmcLinkageStruct_p )
    {
        // Get pointer to start of this record
        docRecord_p = (pmcDocRecordStruct_p)linkage_p->record_p;

#if PMC_DEBUG
        if( docRecord_p->magicNumber != PMC_DOC_RECORD_MAGIC_NUMBER )
        {
            mbDlgDebug(( "Invalid magic number\n" ));
        }
#endif

        // Copy the first name
        mbMalloc( DocForm_p[i].firstName_p, docRecord_p->firstNameLen );
        strcpy( DocForm_p[i].firstName_p, docRecord_p->firstName_p );

        // Copy the last name
        mbMalloc( DocForm_p[i].lastName_p, docRecord_p->lastNameLen );
        strcpy( DocForm_p[i].lastName_p, docRecord_p->lastName_p );

        mbMalloc( DocForm_p[i].lastNameSearch_p, docRecord_p->lastNameSearchLen );
        strcpy( DocForm_p[i].lastNameSearch_p, docRecord_p->lastNameSearch_p );

        mbMalloc( DocForm_p[i].workPhoneSearch_p, docRecord_p->workPhoneSearchLen );
        strcpy( DocForm_p[i].workPhoneSearch_p, docRecord_p->workPhoneSearch_p );

        mbMalloc( DocForm_p[i].workFax_p, docRecord_p->workFaxLen );
        strcpy( DocForm_p[i].workFax_p, docRecord_p->workFax_p );

        DocForm_p[i].faxAreaCode[0] = 0;
        if( docRecord_p->displayFaxAreaCode == TRUE )
        {
            strncpy( DocForm_p[i].faxAreaCode, docRecord_p->faxAreaCode, PMC_AREA_CODE_LEN );
        }

        mbMalloc( DocForm_p[i].num_p, docRecord_p->numLen );
        strcpy( DocForm_p[i].num_p, docRecord_p->num_p );

        mbMalloc( DocForm_p[i].workPhone_p, docRecord_p->workPhoneLen );
        strcpy( DocForm_p[i].workPhone_p, docRecord_p->workPhone_p );

        DocForm_p[i].workAreaCode[0] = 0;
        if( docRecord_p->displayWorkAreaCode == TRUE )
        {
            strncpy( DocForm_p[i].workAreaCode, docRecord_p->workAreaCode, PMC_AREA_CODE_LEN );
        }

        DocForm_p[i].doctorId = docRecord_p->id;
        DocFormOffset_p[i].name = docRecord_p->offset;

        i++;
    }

    // Must also get record offsets sorted by phone...
    i = 0;

    qWalk( linkage_p, pmcDocPhone_q, pmcLinkageStruct_p )
    {
        // Get pointer to start of this record
        docRecord_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        DocFormOffset_p[i].phone = docRecord_p->offset;
        i++;
    }

    // Must also get record offsets sorted by phn...
    i = 0;

    qWalk( linkage_p, pmcDocNum_q, pmcLinkageStruct_p )
    {
        // Get pointer to start of this record
        docRecord_p = (pmcDocRecordStruct_p)linkage_p->record_p;
        DocFormOffset_p[i].num = docRecord_p->offset;
        i++;
    }

    mbLockRelease( pmcDocListLock );
    BuildCurrentList( SortIndex,  SearchEdit->Text.c_str() );
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::Button2Click()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::ShowAllButtonClick(TObject *Sender)
{
    AnsiString str = "";

    // Simply updating the text should cause a rebuild of the current list
    SearchEdit->Text = str;

    MainForm->RefreshTable( PMC_TABLE_BIT_PATIENTS );
    if( Ready ) SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Function:  TDoctorListForm::DoctorStringGridDrawCell()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorStringGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect          &Rect,
    TGridDrawState  State
)
{
    AnsiString              str  = "";
    pmcDocFormStruct_p      docForm_p;

    if( ARow == DoctorStringGrid->Row )
    {
        UpdateSelectedDoctor( );
    }

    if( ARow == 0 )
    {
        // This is the title row
        str = pmcDocListStrings[ACol];
    }
    else if( ARow <= CurrentDoctorRecords )
    {
       // Get pointer to current record
        docForm_p = &DocForm_p[DocFormOffset_p[ARow - 1].current];

        if( ACol == 0 ) str = docForm_p->lastName_p;
        if( ACol == 1 ) str = docForm_p->firstName_p;
        if( ACol == 2 ) str = docForm_p->workAreaCode;
        if( ACol == 3 ) str = docForm_p->workPhone_p;
        if( ACol == 4 ) str = docForm_p->faxAreaCode;
        if( ACol == 5 ) str = docForm_p->workFax_p;
        if( ACol == 6 ) str = docForm_p->num_p;
    }
    DoctorStringGrid->Canvas->TextRect( Rect, Rect.Left + 2, Rect.Top, str );
}

//---------------------------------------------------------------------------
// Function:   TDoctorListForm::DoctorStringGridClick()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorStringGridClick(TObject *Sender)
{
   nbDlgDebug(( "Called, col:%ld row :%ld",
        DoctorStringGrid->Col, DoctorStringGrid->Row ));

   UpdateSelectedDoctor( );
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::FormDestroy()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::FormDestroy(TObject *Sender)
{
    nbDlgDebug(( "DoctorListForm Destroy called" ));
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::FormClose()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    if( FormInfo_p )
    {
        FormInfo_p->doctorId = (ClearFlag == TRUE ) ? 0 : SelectedDoctorId;
    }
    mbPropertyWinSave( PMC_WINID_DOCTOR_LIST, Height, Width, Top, Left );
    Action = caFree;
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::BuildCurrentList()
//---------------------------------------------------------------------------
// Description:
//
// Filter list based on index and search string
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::BuildCurrentList
(
    Ints_t              index,
    Char_p              str_p
)
{
    Ints_t              i, j;
    Ints_t              searchLen = 0;
    pmcDocFormStruct_p  docForm_p;
    Char_t              buf[128];
    Char_p              searchString_p = NIL;
    Ints_t              matchRow = 0;

    if( SkipCurrentListBuild ) return;

    nbDlgDebug(( "Build current list called, Selected Doctor ID: %ld", SelectedDoctorId ));

    // Format the search strings
    if( str_p )
    {
        searchLen = strlen( str_p );
        mbMalloc( searchString_p , searchLen + 1 );
        strcpy( searchString_p, str_p );
        mbStrToLower( searchString_p );
    }

    nbDlgDebug(( "RadioGroup1CLick called %d", SearchFieldButtons->ItemIndex ));

    CurrentDoctorRecords = TotalDoctorRecords;

    switch( index )
    {
        case PMC_CURRENT_LIST_NAME:
            for( i = 0 , j = 0 ; i < TotalDoctorRecords ; i++ )
            {
                if( searchLen == 0 )
                {
                    // Put all records into list
                    DocFormOffset_p[j++].current = DocFormOffset_p[i].name;

                    // Check to see  if this is the selected doctor
                    if( SelectedDoctorId == DocForm_p[DocFormOffset_p[i].name].doctorId ) matchRow = i;
                }
                else
                {
                    // Get pointer to actual record
                    docForm_p = &DocForm_p[DocFormOffset_p[i].name];

                    // Check if search string anywhere within string
                    if( mbStrPos( docForm_p->lastNameSearch_p, searchString_p ) == 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].name;
                    }
                    else if( mbStrPos( docForm_p->workPhoneSearch_p, searchString_p ) >= 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].name;
                    }
                    else if( mbStrPos( docForm_p->num_p, searchString_p ) == 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].name;
                    }
                }
            }
            CurrentDoctorRecords = j;
            break;

        case PMC_CURRENT_LIST_PHONE:
            for( i = 0 , j = 0 ; i < TotalDoctorRecords ; i++ )
            {
                if( searchLen == 0 )
                {
                    DocFormOffset_p[j++].current = DocFormOffset_p[i].phone;
                    // Check to see  if this is the selected doctor
                    if( SelectedDoctorId == DocForm_p[DocFormOffset_p[i].phone].doctorId ) matchRow = i;
                }
                else
                {
                    // Get pointer to actual record
                    docForm_p = &DocForm_p[DocFormOffset_p[i].phone];

                    // Check if search string in phone search string
                    if( mbStrPos( docForm_p->lastNameSearch_p, searchString_p ) == 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].phone;
                    }
                    else if( mbStrPos( docForm_p->workPhoneSearch_p, searchString_p ) >= 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].phone;
                    }
                    else if( mbStrPos( docForm_p->num_p, searchString_p ) == 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].phone;
                    }
                }
            }
            CurrentDoctorRecords = j;
            break;

        case PMC_CURRENT_LIST_NUM:
            for( i = 0 , j = 0 ; i < TotalDoctorRecords ; i++ )
            {
                if( searchLen == 0 )
                {
                    DocFormOffset_p[j++].current = DocFormOffset_p[i].num;
                    // Check to see  if this is the selected doctor
                    if( SelectedDoctorId == DocForm_p[DocFormOffset_p[i].num].doctorId ) matchRow = i;
                }
                else
                {
                    // Get pointer to actual record
                    docForm_p = &DocForm_p[DocFormOffset_p[i].num];

                    // Check if search string in phone search string
                    if( mbStrPos( docForm_p->lastNameSearch_p, searchString_p ) == 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].num;
                    }
                    else if( mbStrPos( docForm_p->workPhoneSearch_p, searchString_p ) >= 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].num;
                    }
                    else if( mbStrPos( docForm_p->num_p, searchString_p ) == 0 )
                    {
                        DocFormOffset_p[j++].current = DocFormOffset_p[i].num;
                    }
                }
            }
            CurrentDoctorRecords = j;
            break;

        default:
            mbDlgDebug(( "Invalid search index: %ld", index ));
            for( i = 0 ; i < TotalDoctorRecords ; i++ )
            {
                DocFormOffset_p[i].current = DocFormOffset_p[i].name;
            }
            break;
    }

    if( searchString_p ) mbFree( searchString_p );

    if( CurrentDoctorRecords )
    {
        DoctorStringGrid->RowCount = CurrentDoctorRecords + 1;
    }
    else
    {
        DoctorStringGrid->RowCount = 2;
    }

    if( searchLen == 0 )
    {
        DoctorStringGrid->TopRow = matchRow + 1;
        DoctorStringGrid->Row = matchRow + 1;
    }
    else
    {
        DoctorStringGrid->TopRow = 1;
        DoctorStringGrid->Row = 1;
    }

    DoctorStringGrid->Invalidate( );

    UpdateSelectedDoctor( );

    mbStrInt32u( CurrentDoctorRecords, buf );
    RecordsShownLabel->Caption = buf;

    mbStrInt32u( TotalDoctorRecords, buf );
    RecordsTotalLabel->Caption = buf;

    return;
}

//---------------------------------------------------------------------------
// Function:  TDoctorListForm::SearchEditChange()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::SearchEditChange(TObject *Sender)
{
    if( Ready )
    {
        BuildCurrentList( SortIndex, SearchEdit->Text.c_str() );
    }
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::UpdateSelectedDoctor()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::UpdateSelectedDoctor( void )
{
    Char_t                  buf[128];
    pmcDocFormStruct_p      docForm_p;

    if( DoctorStringGrid->Row > 0 )
    {
        // First, must get a pointer to the current doctor record
        docForm_p = &DocForm_p[DocFormOffset_p[DoctorStringGrid->Row - 1].current];

        sprintf( buf, "%s, %s", docForm_p->lastName_p, docForm_p->firstName_p );

        DoctorNameLabel->Caption = buf;
        ClearFlag = FALSE;

        sprintf( buf, "%s", docForm_p->num_p );
        DoctorNumberLabel->Caption = buf;

        if( UpdateSelectedDoctorIdFlag )
        {
            SelectedDoctorId = docForm_p->doctorId;
        }
        if( Ready ) SearchEdit->SetFocus();
    }
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::NewDoctorButtonClick()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::NewDoctorButtonClick(TObject *Sender)
{
    pmcDocEditInfo_t       docEditInfo;
    TDoctorEditForm       *docEditForm;

    if( DoctorStringGrid->Row == 0 )
    {
        mbDlgDebug(( "Got row == 0. Should not be here" ));
        return;
    }

    docEditInfo.id = 0;
    docEditInfo.mode = PMC_EDIT_MODE_NEW;
    sprintf( docEditInfo.caption, "Enter New Doctor Details" );

    docEditForm = new TDoctorEditForm( NULL, &docEditInfo );
    docEditForm->ShowModal();
    delete docEditForm;

    if( docEditInfo.returnCode == MB_BUTTON_OK )
    {
        nbDlgDebug(( "Back from new doctor View Click, doctor ID: %ld", docEditInfo.doctorId  ));

        SelectedDoctorId = docEditInfo.id;

        // Clear search string so that the new patient will be selected in the list
        SkipCurrentListBuild = TRUE;
        SearchEdit->Text = "";
        SkipCurrentListBuild = FALSE;

        if( MainForm->RefreshTableForce( PMC_TABLE_BIT_DOCTORS ) )
        {
            UpdateSelectedDoctorIdFlag = FALSE;
            BuildMasterList( );
            UpdateSelectedDoctorIdFlag = TRUE;
        }
    }
    if( Ready ) SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Function:  TDoctorListForm::DoctorGridPopupPopup()
//---------------------------------------------------------------------------
// Description:
//
// Popup menu has popped up.
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorGridPopupPopup(TObject *Sender)
{
    pmcPopupItemEnableAll( DoctorGridPopup, FALSE );

    // Get doctor over which mouse is located
    if( MouseInRow <= CurrentDoctorRecords && MouseInRow > 0 )
    {
        DoctorStringGrid->Row = MouseInRow;
        UpdateSelectedDoctor( );

        pmcPopupItemEnable( DoctorGridPopup, PMC_DR_LIST_POPUP_VIEW,            TRUE );
        pmcPopupItemEnable( DoctorGridPopup, PMC_DR_LIST_POPUP_EDIT,            TRUE );
        // pmcPopupItemEnable( DoctorGridPopup, PMC_DR_LIST_POPUP_DELETE,          TRUE );

        if( pmcCfg[CFG_SLP_NAME].str_p)
        {
            pmcPopupItemEnable( DoctorGridPopup, PMC_DR_LIST_POPUP_PRINT_LBL_ADDR,  TRUE );
        }
    }
    else
    {
        // Mouse id not over a valid doctor record
    }
    nbDlgDebug(( "DoctorStringGrid row: %d\nTotal rows: %d\n",
        DoctorStringGrid->Row, DoctorStringGrid->RowCount ));
}

//---------------------------------------------------------------------------
// Function:  TDoctorListForm::DoctorStringGridMouseDown
//---------------------------------------------------------------------------
// Description:
//
// Mouse button has been pressed in the DoctorStringGrid.
// Determine the row (i.e., doctor record) from the co-ordinates
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorStringGridMouseDown
(
    TObject        *Sender,
    TMouseButton    Button,
    TShiftState     Shift,
    int             X,
    int             Y
)
{
    Ints_t    row = 0, col = 0;
    TRect           rect;
    Char_p          str_p;

    if( DoubleClickFlag == TRUE )
    {
        DoubleClickFlag = FALSE;
        return;
    }

    DoctorStringGrid->MouseToCell( X, Y, col, row );
    rect = DoctorStringGrid->CellRect( col, row );

    nbDlgDebug(( "Mouse Down in row %d col %d\n", row, col ));

    if( row >= 0 && row <= CurrentDoctorRecords )
    {
        MouseInRow = row;
    }
    else
    {
        MouseInRow = -1;
    }

    if( col >= 0 && col <= 6 )
    {
        MouseInCol = col;
    }
    else
    {
        MouseInCol = -1;
    }

    if( row == 0 )
    {
        str_p = pmcDocListStrings[col];
        pmcButtonDown( DoctorStringGrid->Canvas, &rect, str_p );
    }
}

//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorStringGridMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{

    pmcButtonUp( );

    if( MouseInRow == 0 )
    {
        if( MouseInCol == 0 || MouseInCol == 1  )
        {
            SortIndex = PMC_CURRENT_LIST_NAME;
        }
        if( MouseInCol == 2 || MouseInCol == 3 )
        {
            SortIndex = PMC_CURRENT_LIST_PHONE;
        }
        if( MouseInCol == 6 )
        {
            SortIndex = PMC_CURRENT_LIST_NUM;
        }
        BuildCurrentList( SortIndex, SearchEdit->Text.c_str() );
        if( Ready ) SearchEdit->SetFocus();

    }
    DoctorStringGrid->Invalidate( );
}

//---------------------------------------------------------------------------
// Function:  TDoctorListForm::ViewClick()
//---------------------------------------------------------------------------
// Description:
//
// View doctor record has been selected from the popup menu.
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DrListPopupViewClick(TObject *Sender)
{
    pmcDocFormStruct_p      docForm_p;
    pmcDocEditInfo_t        docEditInfo;
    TDoctorEditForm        *docEditForm;

    if( DoctorStringGrid->Row == 0 )
    {
        mbDlgDebug(( "Got row == 0. Should not be here" ));
        return;
    }

    // First, must get a pointer to the current doctor record
    docForm_p = &DocForm_p[DocFormOffset_p[DoctorStringGrid->Row - 1].current];
    docEditInfo.id = docForm_p->doctorId;

    sprintf( docEditInfo.caption, "View Doctor Details - %s, %s",
        docForm_p->lastName_p, docForm_p->firstName_p );

    docEditInfo.mode = PMC_EDIT_MODE_VIEW;

    docEditForm = new TDoctorEditForm( NULL, &docEditInfo );
    docEditForm->ShowModal();
    delete docEditForm;

    nbDlgDebug(( "Got Doctor View Click, doctor ID: %d (%s, %s)",
        docForm_p->doctorId, docForm_p->lastName_p, docForm_p->firstName_p ));
}

//---------------------------------------------------------------------------
// Function: TDoctorListForm::RefreshButtonClick()
//---------------------------------------------------------------------------
// Description:
//
// Get updated doctor list from main form.
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::RefreshButtonClick(TObject *Sender)
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

void __fastcall TDoctorListForm::FormActivate(TObject *Sender)
{
    /* Check for updated doctor records when form recevies focus */
    if( MainForm->RefreshTable( PMC_TABLE_BIT_PATIENTS ) )
    {
        nbDlgDebug(( "Doctor Edit List Activate detected doctor list update" ));
        BuildMasterList( );
    }
    if( Ready ) SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::OkButtonClick(TObject *Sender)
{
    nbDlgDebug(( "OK button click called" ));

    FormInfo_p->returnCode = MB_BUTTON_OK;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::CancelButtonClick(TObject *Sender)
{
    FormInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}

void __fastcall TDoctorListForm::DrListPopupEditClick(TObject *Sender)
{
    DoctorEdit( );
}


void __fastcall TDoctorListForm::DrListPopupDeleteClick(TObject *Sender)
{
    mbDlgDebug(( "delete doctor selected" ));
}

//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DrListPopupPrintAddressLabelClick(
      TObject *Sender)
{
    Int32u_t            doctorId;
    pmcDocFormStruct_p  docForm_p;

    // First, must get a pointer to the current doctor record
    docForm_p = &DocForm_p[DocFormOffset_p[DoctorStringGrid->Row - 1].current];
    doctorId = docForm_p->doctorId;

    if( doctorId )
    {
        pmcLabelPrintDrAddress( doctorId );
    }
}
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorStringGridDblClick(TObject *Sender)
{
    DoubleClickFlag = TRUE;

    if( MouseInRow == -1 || MouseInRow == 0 )
    {
        return;
    }

    if( FormInfo_p->mode == PMC_LIST_MODE_SELECT )
    {
        FormInfo_p->returnCode = MB_BUTTON_OK;
        Close( );
    }
    else
    {
        DoctorEdit( );
    }
}
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorEdit( void )
{
    pmcDocFormStruct_p      docForm_p;
    pmcDocEditInfo_t        docEditInfo;
    TDoctorEditForm        *docEditForm;

    if( DoctorStringGrid->Row == 0 )
    {
        mbDlgDebug(( "Got row == 0. Should not be here" ));
        return;
    }

    // First, must get a pointer to the current doctor record
    docForm_p = &DocForm_p[DocFormOffset_p[DoctorStringGrid->Row - 1].current];
    docEditInfo.id = docForm_p->doctorId;
    docEditInfo.mode = PMC_EDIT_MODE_EDIT;
    sprintf( docEditInfo.caption, "Edit Doctor Details - %s, %s",
        docForm_p->lastName_p, docForm_p->firstName_p );

    if( pmcSqlRecordLock( PMC_SQL_TABLE_DOCTORS, docEditInfo.id, TRUE ) )
    {
        docEditForm = new TDoctorEditForm( NULL, &docEditInfo );
        docEditForm->ShowModal();
        delete docEditForm;
        pmcSqlRecordUnlock( PMC_SQL_TABLE_DOCTORS, docEditInfo.id );
        if( docEditInfo.returnCode == MB_BUTTON_OK )
        {
            SelectedDoctorId = docEditInfo.id;

            // Clear search string so that the new patient will be selected in the list
            SkipCurrentListBuild = TRUE;
            SearchEdit->Text = "";
            SkipCurrentListBuild = FALSE;

            if( MainForm->RefreshTableForce( PMC_TABLE_BIT_DOCTORS ) )
            {
                UpdateSelectedDoctorIdFlag = FALSE;
                BuildMasterList( );
                UpdateSelectedDoctorIdFlag = TRUE;
            }
        }
        else
        {
            // User must have clicked cancel button
        }
    }
    else
    {
        mbDlgExclaim( "Doctor record locked for editing  by another client" );
    }

    nbDlgDebug(( "Got Doctor View Click, doctor ID: %d (%s, %s)",
        docForm_p->doctorId, docForm_p->lastName_p, docForm_p->firstName_p ));
}


void __fastcall TDoctorListForm::ClearButtonClick(TObject *Sender)
{
    ClearFlag = TRUE;
    DoctorNameLabel->Caption = "";
    DoctorNumberLabel->Caption = "";
}
//---------------------------------------------------------------------------


void __fastcall TDoctorListForm::SearchEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    if( Key == VK_UP )
    {
        if( DoctorStringGrid->Row > 1 ) DoctorStringGrid->Row--;
        Key = 0;
    }
    else if( Key == VK_DOWN )
    {
        if( DoctorStringGrid->Row < DoctorStringGrid->RowCount - 1 )
        {
            DoctorStringGrid->Row++;
        }
        Key = 0;
    }
    else if( Key == VK_NEXT )
    {
        if( DoctorStringGrid->Row < DoctorStringGrid->RowCount - 25 )
        {
            DoctorStringGrid->Row += 25;
        }
        else
        {
            DoctorStringGrid->Row = DoctorStringGrid->RowCount - 1 ;
        }
    }
    else if( Key == VK_PRIOR )
    {
        if( DoctorStringGrid->Row > 26 )
        {
            DoctorStringGrid->Row -= 25;
        }
        else
        {
         DoctorStringGrid->Row = 1;
        }
    }
    else if( Key == VK_RETURN )
    {
        FormInfo_p->returnCode = MB_BUTTON_OK;
        Close( );
    }
}
//---------------------------------------------------------------------------

void __fastcall TDoctorListForm::DoctorStringGridKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    if( Key == VK_RETURN )
    {
        FormInfo_p->returnCode = MB_BUTTON_OK;
        Close( );
    }
    else
    {
        SearchEdit->SetFocus();
    }
}
//---------------------------------------------------------------------------

