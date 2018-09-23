//---------------------------------------------------------------------------
#ifndef pmcAppointLetterFormH
#define pmcAppointLetterFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
#include "CGAUGES.h"
//---------------------------------------------------------------------------
class TPrintAppLettersForm : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TMaskEdit *StartDateEdit;
    TMaskEdit *EndDateEdit;
    TButton *StartDateButton;
    TButton *EndDateButton;
    TRadioGroup *PrintRadio;
    TRadioGroup *MarkConfirmedRadio;
    TRadioGroup *PrintLabelsRadio;
    TBevel *Bevel2;
    TButton *CancelButton;
    TButton *OkButton;
    TCGauge *CGauge1;
    TBevel *Bevel3;
    TLabel *AppointmentCountLabel;
private:	// User declarations
public:		// User declarations
    __fastcall TPrintAppLettersForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TPrintAppLettersForm *PrintAppLettersForm;
//---------------------------------------------------------------------------
#endif
