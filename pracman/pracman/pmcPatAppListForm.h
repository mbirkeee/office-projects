//---------------------------------------------------------------------------
#ifndef pmcPatAppListFormH
#define pmcPatAppListFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
//---------------------------------------------------------------------------
class TPatAppListForm : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TStringGrid *StringGrid1;
private:	// User declarations
public:		// User declarations
    __fastcall TPatAppListForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPatAppListForm *PatAppListForm;
//---------------------------------------------------------------------------
#endif
