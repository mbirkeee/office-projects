//---------------------------------------------------------------------------
// File:    pmcIcdForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    April 2002
//---------------------------------------------------------------------------
// Description:
//
// Display and search the ICD codes
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcTables.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcColors.h"
#include "pmcInitialize.h"
#include "pmcIcdForm.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Default Constructor
//---------------------------------------------------------------------------

__fastcall TIcdForm::TIcdForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Default Constructor
//---------------------------------------------------------------------------

__fastcall TIcdForm::TIcdForm( TComponent* Owner, pmcIcdListInfo_p info_p )
    : TForm(Owner)
{
    pmcIcdStruct_p   icd_p;
    pmcIcdStruct_p   icd2_p;

    SelectedLabel->Caption = FALSE;

    Active = FALSE;

    // Position the window
    {
        Ints_t  height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_ICD_LIST, &height, &width, &top, &left ) == MB_RET_OK )
        {
            SetBounds( left, top, width, height );
        }
    }

    Icd_q = qInitialize( &IcdQueueHead );
    Unused_q = qInitialize( &UnusedQueueHead );

    Info_p = info_p;

    SelectedIcd_p = NIL;

    if( Info_p->mode == PMC_LIST_MODE_LIST )
    {
        OkButton->Visible = FALSE;
        CancelButton->Caption = "Close";
    }
    else
    {
        CancelButton->Caption = "Cancel";
    }

    // Get the list of matching entries
    qWalk( icd2_p, pmcIcdSingle_q, pmcIcdStruct_p )
    {
        mbMalloc( icd_p, sizeof(pmcIcdStruct_t ) );

        // Copy the actual code from the pracman master ICD list
        strcpy( icd_p->code, icd2_p->code );
        mbStrToUpper( icd_p->code );

        // Point this description to the master list description
        mbMalloc( icd_p->description_p, strlen( icd2_p->description_p ) + 1 );
        mbMalloc( icd_p->search_p,      strlen( icd2_p->description_p ) + 1 );

        strcpy( icd_p->description_p, icd2_p->description_p );
        strcpy( icd_p->search_p, icd2_p->description_p );
        mbStrToUpper( icd_p->search_p );
        mbStrAlphaNumericOnly( icd_p->search_p );
        // Add entry to the list
        qInsertLast( Icd_q, icd_p );
    }

    SortCodeMode = PMC_SORT_CODE_ASCENDING;
    SortDescMode = PMC_SORT_DESC_ASCENDING;
    SortMode = SortCodeMode;

    if( strlen( Info_p->codeIn ) )
    {
        SearchEdit->Text = Info_p->codeIn;
    }
    ListUpdate( );

    Active = TRUE;
}

//---------------------------------------------------------------------------

void __fastcall TIcdForm::ListFree( void )
{
    pmcIcdStruct_p   icd_p;

    // Clear the list view
    IcdListView->Items->BeginUpdate( );
    IcdListView->Items->Clear( );
    IcdListView->Items->EndUpdate( );

    while( !qEmpty( Unused_q ) )
    {
        icd_p = (pmcIcdStruct_p)qRemoveFirst( Unused_q );
        qInsertLast( Icd_q, icd_p );
    }

    // Free the internal list
    while( !qEmpty( Icd_q ) )
    {
        icd_p = (pmcIcdStruct_p)qRemoveLast( Icd_q );
        mbFree( icd_p->description_p );
        mbFree( icd_p->search_p );
        mbFree( icd_p );
    }
}

//---------------------------------------------------------------------------

void __fastcall TIcdForm::ListUpdate( void )
{
    pmcIcdStruct_p      icd_p;
    Char_p              search_p;
    Ints_t              len, i;

    mbMalloc( search_p, 128 );

    strcpy( search_p, SearchEdit->Text.c_str( ) );
    mbStrToUpper( search_p );
    mbStrAlphaNumericOnly( search_p );

    while( !qEmpty( Unused_q ) )
    {
        icd_p = (pmcIcdStruct_p)qRemoveFirst( Unused_q );
        qInsertLast( Icd_q, icd_p );
    }

    len = Icd_q->size;

    for( i = 0 ; i < len ; i++ )
    {
        icd_p = (pmcIcdStruct_p)qRemoveFirst( Icd_q );
        if( mbStrPos( icd_p->search_p, search_p ) >= 0 )
        {
            qInsertLast( Icd_q, icd_p );
        }
        else if( mbStrPos( icd_p->code, search_p ) >= 0 )
        {
            qInsertLast( Icd_q, icd_p );
        }
        else
        {
            qInsertLast( Unused_q, icd_p );
        }
    }

    ListSort( );
    mbFree( search_p );
}

//---------------------------------------------------------------------------
void __fastcall TIcdForm::ListSort( void )
{
    pmcIcdStruct_p      icd_p;
    pmcIcdStruct_p      tempIcd_p;
    TListItem          *firstItem_p;
    TListItem          *matchItem_p;
    TListItem          *item_p;
    qHead_t             tempQueue;
    qHead_p             temp_q;
    bool                added;
    Char_t              buf[32];

    temp_q = qInitialize( &tempQueue );

    for( ; ; )
    {
        if( qEmpty( Icd_q ) ) break;

        icd_p = (pmcIcdStruct_p)qRemoveFirst( Icd_q );

        added = FALSE;
        qWalk( tempIcd_p, temp_q, pmcIcdStruct_p )
        {
            switch( SortMode )
            {
                case PMC_SORT_CODE_ASCENDING:
                case PMC_SORT_CODE_DESCENDING:
                    added = SortCode( SortMode, temp_q, icd_p, tempIcd_p, TRUE );
                    break;
                case PMC_SORT_DESC_ASCENDING:
                case PMC_SORT_DESC_DESCENDING:
                    added = SortDesc( SortMode, temp_q, icd_p, tempIcd_p, TRUE );
                    break;
            }
            if( added ) break;
        }
        if( !added ) qInsertLast( temp_q, icd_p );
    }

    for( ; ; )
    {
       if( qEmpty( temp_q )) break;
       icd_p = (pmcIcdStruct_p)qRemoveLast( temp_q );
       qInsertFirst( Icd_q, icd_p );
    }

    IcdListView->Items->BeginUpdate( );
    IcdListView->Items->Clear( );
    IcdListView->Items->EndUpdate( );

    firstItem_p = NIL;
    matchItem_p = NIL;

    qWalk( icd_p, Icd_q, pmcIcdStruct_p )
    {
        item_p = IcdListView->Items->Add( );

        // Record first item in the list
        if( firstItem_p == NIL ) firstItem_p = item_p;
        if( icd_p == SelectedIcd_p ) matchItem_p = item_p;

        item_p->Caption = icd_p->code;

        if( strlen( Info_p->codeIn ) )
        {
            if( strcmp( Info_p->codeIn, icd_p->code ) == 0 )
            {
                Info_p->codeIn[0] = 0;
                matchItem_p = item_p;
                SelectedIcd_p = icd_p;
            }
        }
        item_p->SubItems->Add( icd_p->description_p );
        item_p->Data = (Void_p)icd_p;
    }

    if( matchItem_p == NIL ) matchItem_p = firstItem_p;

    if( matchItem_p != NIL )
    {
        IcdListView->Selected = matchItem_p;
        matchItem_p->Selected = TRUE;
        IcdListView->Selected->MakeVisible( TRUE );
    }
    else
    {
        sprintf( Info_p->code, "" );
        SelectedLabel->Caption = Info_p->code;
    }

    sprintf( buf, "%ld", Icd_q->size + Unused_q->size );
    EntriesCountLabel->Caption = buf;

    sprintf( buf, "%ld", Icd_q->size );
    EntriesShownLabel->Caption = buf;

    if( Active ) SearchEdit->SetFocus();
}

//---------------------------------------------------------------------------

bool __fastcall  TIcdForm::SortCode
(
    Int32u_t        sortMode,
    qHead_p         temp_q,
    pmcIcdStruct_p  icd_p,
    pmcIcdStruct_p  tempIcd_p,
    Int32u_t        descSort
)
{
    bool        added = FALSE;
    Int32s_t    result;

    result = strcmp( icd_p->code, tempIcd_p->code );

    if( result == 0 && descSort == TRUE )
    {
        added = SortDesc( SortDescMode, temp_q, icd_p, tempIcd_p, FALSE );
    }
    else if( sortMode == PMC_SORT_CODE_ASCENDING )
    {
        if( result < 0 )
        {
            qInsertBefore( temp_q, tempIcd_p, icd_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_CODE_DESCENDING )
    {
        if( result > 0 )
        {
            qInsertBefore( temp_q, tempIcd_p, icd_p );
            added = TRUE;
        }
    }

    return added;
}
//---------------------------------------------------------------------------

bool __fastcall  TIcdForm::SortDesc
(
    Int32u_t        sortMode,
    qHead_p         temp_q,
    pmcIcdStruct_p  icd_p,
    pmcIcdStruct_p  tempIcd_p,
    Int32u_t        codeSort
)
{
    bool            added = FALSE;
    Int32s_t        result;

    result = strcmp( icd_p->description_p, tempIcd_p->description_p );

    if( result == 0 && codeSort == TRUE )
    {
        added = SortCode( SortCodeMode, temp_q, icd_p, tempIcd_p, FALSE );
    }
    else if( sortMode == PMC_SORT_DESC_ASCENDING )
    {
        if( result < 0 )
        {
            qInsertBefore( temp_q, tempIcd_p, icd_p );
            added = TRUE;
        }
    }
    else if( sortMode == PMC_SORT_DESC_DESCENDING )
    {
        if( result > 0 )
        {
            qInsertBefore( temp_q, tempIcd_p, icd_p );
            added = TRUE;
        }
    }

    return added;
}

//---------------------------------------------------------------------------

void __fastcall TIcdForm::CancelButtonClick(TObject *Sender)
{
    Info_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TIcdForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    ListFree( );
    mbPropertyWinSave( PMC_WINID_ICD_LIST, Height, Width, Top, Left );
    Action = caFree;
}
//---------------------------------------------------------------------------
void __fastcall TIcdForm::SearchEditChange(TObject *Sender)
{
    ListUpdate( );
}
//---------------------------------------------------------------------------


void __fastcall TIcdForm::IcdListViewColumnClick
(
    TObject     *Sender,
    TListColumn *Column
)
{
    if( strcmp( Column->Caption.c_str(), "ICD Code" ) == 0 )
    {
        SortCodeMode = ( SortCodeMode == PMC_SORT_CODE_ASCENDING ) ? PMC_SORT_CODE_DESCENDING : PMC_SORT_CODE_ASCENDING;
        SortMode = SortCodeMode;
    }
    else if( strcmp( Column->Caption.c_str(), "Description" ) == 0 )
    {
        SortDescMode = ( SortDescMode == PMC_SORT_DESC_ASCENDING ) ? PMC_SORT_DESC_DESCENDING : PMC_SORT_DESC_ASCENDING;
        SortMode = SortDescMode;
    }
    ListSort( );
}
//---------------------------------------------------------------------------

void __fastcall TIcdForm::IcdListViewSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
    pmcIcdStruct_p  icd_p;

    sprintf( Info_p->code, "" );
    if( Item != NIL )
    {
        icd_p = (pmcIcdStruct_p)Item->Data;
        strcpy( Info_p->code, icd_p->code );
        SelectedIcd_p = icd_p;
    }
    SelectedLabel->Caption = Info_p->code;

    if( Active ) SearchEdit->SetFocus( );
}
//---------------------------------------------------------------------------


void __fastcall TIcdForm::OkButtonClick(TObject *Sender)
{
    Info_p->returnCode = MB_BUTTON_OK;
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TIcdForm::IcdListViewDblClick(TObject *Sender)
{
    if( Info_p->mode == PMC_LIST_MODE_SELECT )
    {
        Info_p->returnCode = MB_BUTTON_OK;
        Close( );
    }
}
//---------------------------------------------------------------------------

void __fastcall TIcdForm::SearchEditKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    TItemStates selected = TItemStates() << isNone;

    if( Key == VK_UP )
    {
        if( IcdListView->Selected != IcdListView->TopItem )
        {
           IcdListView->SetFocus( );
             IcdListView->Selected = IcdListView->GetNextItem( IcdListView->Selected , sdAbove, selected );
            SearchEdit->SetFocus( );
        }
        Key = 0;
    }
    else if( Key == VK_DOWN )
    {
        IcdListView->SetFocus( );
        IcdListView->Selected = IcdListView->GetNextItem( IcdListView->Selected , sdBelow, selected );
        SearchEdit->SetFocus( );
        Key = 0;
    }
#if 0
    else if( Key == VK_NEXT )
    {
        if( IcdListView->Row < IcdListView->RowCount - 25 )
        {
            IcdListView->Row += 25;
        }
        else
        {
            IcdListView->Row = IcdListView->RowCount - 1 ;
        }
    }
    else if( Key == VK_PRIOR )
    {
        if( IcdListView->Row > 26 )
        {
            IcdListView->Row -= 25;
        }
        else
        {
            IcdListView->Row = 1;
        }
    }
#endif
    else if( Key == VK_RETURN )
    {
        Info_p->returnCode = MB_BUTTON_OK;
        Close( );
    }
}
//---------------------------------------------------------------------------


