//---------------------------------------------------------------------------
// File:    pmcAboutForm.cpp
//---------------------------------------------------------------------------
// Date:    March 26, 2001
// Author:  Michael A. Bree
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "pmcAboutForm.h"
#include "pmcGlobals.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function: TAboutForm
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

__fastcall TAboutForm::TAboutForm(TComponent* Owner)
    : TForm(Owner)
{
    Char_t  buf[256];
    
    sprintf( buf, "Practice Manager - Version %d.%d\n%c 2001-2007 Michael A. Bree\n(Build Date: %s Build Time: %s)",
        PMC_MAJOR_VERSION, PMC_MINOR_VERSION, 0xA9, __DATE__ , __TIME__);
        
    CopyrightLabel->Caption = buf;
}

//---------------------------------------------------------------------------
void __fastcall TAboutForm::OkButtonClick(TObject *Sender)
{
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Action = caFree;    
}
//---------------------------------------------------------------------------
