//---------------------------------------------------------------------------
// File:    pmcIcdForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2006, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Dec. 24, 2006
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcMedListFormH
#define pmcMedListFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>

#include "pmcInitialize.h"

//---------------------------------------------------------------------------
class TMedListForm : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TListView *ListView1;
    TGroupBox *GroupBox1;
    TEdit *SearchEdit;
    TGroupBox *GroupBox2;
    TLabel *SelectedLabel;
    TGroupBox *GroupBox3;
    TButton *Button3;
    TButton *ButtonClose;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall ButtonCloseClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TMedListForm(TComponent* Owner);
};

#endif // pmcMedListFormH

