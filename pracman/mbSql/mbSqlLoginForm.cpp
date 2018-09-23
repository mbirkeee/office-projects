//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mbSqlLoginForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//TFormLogin *FormLogin;

//---------------------------------------------------------------------------
__fastcall TFormLogin::TFormLogin(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormLogin::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    mbDlgDbg( ( "close clicked" ));
}
//---------------------------------------------------------------------------

