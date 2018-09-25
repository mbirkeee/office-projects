//---------------------------------------------------------------------------
// File:    pmcTextEditForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    January 12, 2003
//---------------------------------------------------------------------------
// Description:
//
// List of echos
//---------------------------------------------------------------------------


#ifndef pmcTextEditFormH
#define pmcTextEditFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

typedef struct pmcTextEditInfo_s
{
    Int32u_t    returnCode;
    Char_p      caption_p;
    Char_p      label_p;
    Char_p      text_p;
    Char_p      result_p;
    Int32u_t    resultSize;
} pmcTextEditInfo_t, *pmcTextEditInfo_p;

//---------------------------------------------------------------------------
class TTextEditForm : public TForm
{
__published:	// IDE-managed Components
    TEdit *TextEdit;
    TButton *CancelButton;
    TButton *OkButton;
    TLabel *Label;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall TextEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:	// User declarations
    pmcTextEditInfo_p Info_p;
    
public:		// User declarations
    __fastcall TTextEditForm(TComponent* Owner);
    __fastcall TTextEditForm(TComponent* Owner, pmcTextEditInfo_p info_p );
};

#endif
