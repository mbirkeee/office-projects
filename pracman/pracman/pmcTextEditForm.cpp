//---------------------------------------------------------------------------
// File:    pmcTextEditForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Januart 26, 2003
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"
#include "pmcUtils.h"
#include "pmcMainForm.h"
#include "pmcTextEditForm.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Default Constructor
//---------------------------------------------------------------------------
__fastcall TTextEditForm::TTextEditForm(TComponent* Owner)
    : TForm(Owner)
{
}

__fastcall TTextEditForm::TTextEditForm
(
    TComponent* Owner,
    pmcTextEditInfo_p info_p
)
    : TForm(Owner)
{
    Info_p = info_p;

    Caption =  Info_p->caption_p ?  Info_p->caption_p : "";
    TextEdit->Text = Info_p->text_p ? Info_p->text_p : "";
    Label->Caption = Info_p->label_p ? Info_p->label_p : "";
}
//---------------------------------------------------------------------------
void __fastcall TTextEditForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Char_p  buf_p;

    if( Info_p->returnCode == MB_BUTTON_OK )
    {
        mbCalloc( buf_p, Info_p->resultSize );
        strncpy( buf_p, TextEdit->Text.c_str(), Info_p->resultSize - 1 );
        mbStrClean( buf_p, NIL, TRUE );
        strcpy( Info_p->result_p, buf_p );
        mbFree( buf_p );
    }
    Action = caFree;
}
//---------------------------------------------------------------------------
void __fastcall TTextEditForm::OkButtonClick(TObject *Sender)
{
    Info_p->returnCode = MB_BUTTON_OK;
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TTextEditForm::CancelButtonClick(TObject *Sender)
{
    Info_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TTextEditForm::TextEditKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if( Key == MB_CHAR_CR )
    {
        Info_p->returnCode = MB_BUTTON_OK;
        Close( );
    }
}
//---------------------------------------------------------------------------
