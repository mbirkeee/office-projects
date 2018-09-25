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

#include "pmcInitialize.h"

typedef struct pmcPickListFormInfo_s
{
    Char_p      caption_p;
    qHead_p     str_q;
    Int32u_t    returnCode;
} pmcPickListFormInfo_t, *pmcPickListFormInfo_p;

//---------------------------------------------------------------------------
class TForm_PickList : public TForm
{
__published:	// IDE-managed Components
    TComboBox *ComboBox;
    TButton *Button_OK;
    void __fastcall Button_OKClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
    pmcPickListFormInfo_p Info_p;
    
public:		// User declarations
    __fastcall TForm_PickList(TComponent* Owner);
    __fastcall TForm_PickList(TComponent* Owner, pmcPickListFormInfo_p info_p );
};
//---------------------------------------------------------------------------

Int32u_t pmcFormPickList( Char_p caption_p, qHead_p str_q );

#endif

