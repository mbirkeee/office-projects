//---------------------------------------------------------------------------
#ifndef pmcSplashScreenH
#define pmcSplashScreenH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TSplashScreen : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel2;
    TLabel *StatusString;
    TBevel *Bevel3;
    TLabel *Label1;
    TLabel *CopyrightLabel;
private:	// User declarations
public:		// User declarations
    __fastcall TSplashScreen(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSplashScreen *SplashScreen;
//---------------------------------------------------------------------------
#endif
