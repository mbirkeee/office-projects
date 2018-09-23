//---------------------------------------------------------------------------
// File:    pmcSplashScreen.cpp
//---------------------------------------------------------------------------
// Date:    March 26, 2001
// Author:  Michael A. Bree
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcSplashScreen.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function: TSplashScreen
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

__fastcall TSplashScreen::TSplashScreen(TComponent* Owner)
    : TForm(Owner)
{
    StatusString->Caption = "Connecting to remote database...";
    {
        Char_t  buf[128];
        sprintf( buf, "Practice Manager - Version %d.%d - %c 2001, 2002 M. A. Bree (%s %s)",
        PMC_MAJOR_VERSION, PMC_MINOR_VERSION, 0xA9, __DATE__ , __TIME__);
        CopyrightLabel->Caption = buf;
        Bevel3->Invalidate( );
        CopyrightLabel->Invalidate( );
        Label1->Invalidate( );
    }
    Invalidate( );
}
//---------------------------------------------------------------------------

