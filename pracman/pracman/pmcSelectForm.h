//---------------------------------------------------------------------------
#ifndef pmcSelectFormH
#define pmcSelectFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

typedef struct pmcSelectFormInfo_s
{
    Int32u_t    returnCode;
} pmcSelectFormInfo_t, *pmcSelectFormInfo_p;

class TSelectForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CancelButton;
    TListView *ListViewIn;
    TListView *ListView2;
    TBevel *Bevel1;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
    pmcSelectFormInfo_p FormInfo_p;

public:		// User declarations
    __fastcall TSelectForm(TComponent* Owner);
    __fastcall TSelectForm(TComponent* Owner, pmcSelectFormInfo_p info_p );
};

//---------------------------------------------------------------------------
//extern PACKAGE TSelectForm *SelectForm;
//---------------------------------------------------------------------------

#endif
