//---------------------------------------------------------------------------

#ifndef mbSqlFormLoginH
#define mbSqlFormLoginH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

#define MB_SQL_MAX_LEN_USERNAME 64
#define MB_SQL_MAX_LEN_PASSWORD 64
#define MB_SQL_MAX_LEN_DATABASE 64

typedef struct MbSqlFormLoginInfo_s
{
    Int32u_t    returnCode;
    Char_t      username[MB_SQL_MAX_LEN_USERNAME];
    Char_t      password[MB_SQL_MAX_LEN_PASSWORD];
    Char_t      database[MB_SQL_MAX_LEN_DATABASE];
} MbSqlFormLoginInfo_t, *MbSqlFormLoginInfo_p;

//---------------------------------------------------------------------------
class TFormLogin : public TForm
{
__published:	// IDE-managed Components
    TEdit *EditUsername;
    TEdit *EditPassword;
    TButton *ButtonLogin;
    TLabel *LabelUsername;
    TLabel *LabelPassword;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall ButtonLoginClick(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall EditPasswordKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// User declarations
    Boolean_t               mAttempt;
    Boolean_t               mFocusPassword;
    MbSqlFormLoginInfo_p    mInfo_p;
public:		// User declarations
    __fastcall TFormLogin( MbSqlFormLoginInfo_p info_p );
};
//---------------------------------------------------------------------------
//extern PACKAGE TFormLogin *FormLogin;
//---------------------------------------------------------------------------
#endif

