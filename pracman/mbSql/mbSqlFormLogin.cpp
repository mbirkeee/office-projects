//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"
#include "mbSqlFormLogin.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TFormLogin::TFormLogin( MbSqlFormLoginInfo_p info_p )
    : TForm(Owner)
{
    mAttempt = FALSE;
    mFocusPassword = FALSE;

    mInfo_p = info_p;
    EditUsername->Text = info_p->username;
    info_p->password[0] = 0;

    if( strlen( info_p->username ) == 0 )
    {
    }
    else
    {
        mFocusPassword = TRUE;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormLogin::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    if( mAttempt == TRUE )
    {
        mInfo_p->returnCode = MB_BUTTON_OK;
        strncpy( mInfo_p->username, EditUsername->Text.c_str(), MB_SQL_MAX_LEN_USERNAME - 1 );
        strncpy( mInfo_p->password, EditPassword->Text.c_str(), MB_SQL_MAX_LEN_PASSWORD - 1 );
    }
    else
    {
        mInfo_p->returnCode = MB_BUTTON_CANCEL;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormLogin::ButtonLoginClick(TObject *Sender)
{
    if(    EditUsername->Text.Length() > 0
        && EditPassword->Text.Length() > 0 )
    {
        mAttempt = TRUE;
        Close();
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormLogin::FormActivate(TObject *Sender)
{
    if( mFocusPassword == TRUE )
    {
        EditPassword->SetFocus();
        mFocusPassword = FALSE;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormLogin::EditPasswordKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if( Key == MB_CHAR_CR )
    {
        if(    EditUsername->Text.Length() > 0
            && EditPassword->Text.Length() > 0 )
        {
            mAttempt = TRUE;
            Close();
        }
    }
}
//---------------------------------------------------------------------------

