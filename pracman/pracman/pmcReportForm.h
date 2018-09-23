//---------------------------------------------------------------------------
// File:    pmcReportForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    May 9, 2001
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

#ifndef pmcReportFormH
#define pmcReportFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

typedef struct pmcReportFormInfo_s
{
    Int32u_t    returnCode;
    Char_p      fileName_p;
    Char_p      caption_p;
} pmcReportFormInfo_t, *pmcReportFormInfo_p;

class TReportForm : public TForm
{
__published:	// IDE-managed Components
    TMemo *Memo;
    TBevel *Bevel1;
    TButton *Button1;
    TButton *Button2;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations

    __fastcall TReportForm(TComponent* Owner);
    __fastcall TReportForm(TComponent* Owner, pmcReportFormInfo_p reportFormInfo_p );
};

#endif
