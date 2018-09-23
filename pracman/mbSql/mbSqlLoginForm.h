//---------------------------------------------------------------------------

#ifndef mbSqlLoginFormH
#define mbSqlLoginFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
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
private:	// User declarations
public:		// User declarations
    __fastcall TFormLogin(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormLogin *FormLogin;
//---------------------------------------------------------------------------
#endif

