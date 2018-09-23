//---------------------------------------------------------------------------
// File:    splashMain.cpp
//---------------------------------------------------------------------------
// Date:    March 26, 2001
// Author:  Michael A. Bree
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#include <stdio.h>
#include <io.h>
#include <vcl.h>

#pragma hdrstop

#include "mbTypes.h"
#include "splashMain.h"
#include "splashNames.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    Count = 0;
    // Check to see if the flag file exists
    if( access( SPLASH_FLAG_NAME , 0 ) == 0 )
    {
        unlink( SPLASH_FLAG_NAME );
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Timer1Timer(TObject *Sender)
{
    Count++;

    // Timer goes off every second.  Kill program if 60 minutes
    // expires.  Want the splash screen to stay up if the
    // user leaves the login prompt there for a while

    if( Count == 60000 )
    {
        Close( );
    }

    // Check to see if the flag file exists
    if( access( SPLASH_FLAG_NAME, 0 ) == 0 )
    {
        unlink( SPLASH_FLAG_NAME );
        Close( );
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SpeedButton1Click(TObject *Sender)
{
    Close( );    
}
//---------------------------------------------------------------------------

