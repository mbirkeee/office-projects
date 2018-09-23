//---------------------------------------------------------------------------
// File:    pmcEchoCDContentsForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    January 6, 2003
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#ifndef pmcEchoCDContentsFormH
#define pmcEchoCDContentsFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

class TEchoCDContentsForm : public TForm
{
__published:	// IDE-managed Components
    TRadioGroup *RadioGroup;
    TEdit *Edit;
    TButton *CancelButton;
    TButton *CheckButton;
    void __fastcall CheckButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall EditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall RadioGroupClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TEchoCDContentsForm(TComponent* Owner);
};

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t pmcEchoCDContentsForm( void );
Int32s_t pmcEchoCDContents( Boolean_t printFlag, Int32u_t diskId );

#endif
