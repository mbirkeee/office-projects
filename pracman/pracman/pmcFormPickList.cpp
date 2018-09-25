//---------------------------------------------------------------------------
// File:    pmcMedListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2006, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Jan. 2007
//---------------------------------------------------------------------------
// Description:
//
// Display and search the medication list
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "mbUtils.h"
#include "pmcUtils.h"
#include "pmcFormPickList.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TForm_PickList::TForm_PickList(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
__fastcall TForm_PickList::TForm_PickList(TComponent* Owner, pmcPickListFormInfo_p info_p )
    : TForm(Owner)
{
    mbStrList_p str_p;

    Caption = info_p->caption_p;
    Info_p = info_p;

    qWalk( str_p, info_p->str_q, mbStrList_p )
    {
        ComboBox->Items->Add( str_p->str_p );
    }
    ComboBox->ItemIndex = 0;
}
//---------------------------------------------------------------------------
void __fastcall TForm_PickList::Button_OKClick(TObject *Sender)
{
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TForm_PickList::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    mbStrList_p str_p;
    Info_p->returnCode = 0;
    qWalk( str_p, Info_p->str_q, mbStrList_p )
    {
        if( strcmp( str_p->str_p, ComboBox->Items->Strings[ComboBox->ItemIndex].c_str() ) == 0 )
        {
            Info_p->returnCode = str_p->handle;
            break;
        }
    }
    Action = caFree;
}
//---------------------------------------------------------------------------

Int32u_t pmcFormPickList
(
    Char_p      caption_p,
    qHead_p     str_q
)
{
    pmcPickListFormInfo_t   formInfo;
    TForm_PickList         *form_p;

    formInfo.caption_p = caption_p;
    formInfo.str_q      = str_q;

    form_p = new TForm_PickList( NIL, &formInfo );
    form_p->ShowModal( );
    delete form_p;

    return formInfo.returnCode;
}
