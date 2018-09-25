//---------------------------------------------------------------------------
// File:    pmcMedListForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2006, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Dec. 2006
//---------------------------------------------------------------------------
// Description:
//
// Display and search the medication list
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
#include "pmcMedListForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TMedListForm::TMedListForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TMedListForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Action = caFree;
}
//---------------------------------------------------------------------------

void __fastcall TMedListForm::ButtonCloseClick(TObject *Sender)
{
    Close( );    
}
//---------------------------------------------------------------------------

