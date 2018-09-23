//---------------------------------------------------------------------------
// File:    pmcSelectForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    September 22, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcTables.h"
#include "pmcGlobals.h"
//#include "pmcThermometer.h"
#include "pmcUtils.h"
#include "pmcSelectForm.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TSelectForm::TSelectForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
__fastcall TSelectForm::TSelectForm(TComponent* Owner, pmcSelectFormInfo_p info_p )
    : TForm(Owner)
{
    TListItem                  *item_p;

    FormInfo_p = info_p;

    item_p = ListViewIn->Items->Add( );
    item_p->Caption = "test1";

    item_p = ListViewIn->Items->Add( );
    item_p->Caption = "test2";

}
//---------------------------------------------------------------------------

void __fastcall TSelectForm::CancelButtonClick(TObject *Sender)
{
    FormInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TSelectForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    Action = caFree;
}
//---------------------------------------------------------------------------
