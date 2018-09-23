//---------------------------------------------------------------------------
// File:    pmcAppLettersForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 11, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcAppLettersFormH
#define pmcAppLettersFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
#include "CGAUGES.h"
#include <Dialogs.hpp>

typedef struct pmcPatAppLetter_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    patientId;
    Int32u_t    providerId;
    Int32u_t    referringId;
    Int32u_t    startTime;
    Int32u_t    day;
    Int32u_t    confLetterDay;
    Int32u_t    confLetterTime;
    Int32u_t    confLetterId;
    Int32u_t    result;
    unsigned __int64    dateTimeInt64;
    Char_p      subStr[ PMC_SUB_STR_COUNT ];
    Boolean_t   subStrRequired[ PMC_SUB_STR_COUNT ];

} pmcPatAppLetter_t, *pmcPatAppLetter_p;


class TAppLettersForm : public TForm
{
__published:	// IDE-managed Components
    TMaskEdit *StartDateEdit;
    TMaskEdit *EndDateEdit;
    TButton *StartDateButton;
    TButton *EndDateButton;
    TRadioGroup *PrintRadio;
    TBevel *Bevel2;
    TButton *CancelButton;
    TButton *StartButton;
    TLabel *AppointmentCountLabel;
    TComboBox *ProviderCombo;
    TMaskEdit *AddressMergeEdit;
    TSaveDialog *AddressMergeFileDialog;
    TButton *Button1;
    TGroupBox *GroupBox1;
    TGroupBox *GroupBox2;
    TCGauge *Thermometer;
    TCheckBox *MarkAsConfirmedCheckBox;
    TCheckBox *PrintLabelsCheckBox;
    TCheckBox *PrintReportCheckBox;
    TCheckBox *SkipLetterPrintCheckBox;
    TCheckBox *ReportAddressesCheckBox;
    void __fastcall StartDateButtonClick(TObject *Sender);
    void __fastcall EndDateButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall StartButtonClick(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall PrintRadioClick(TObject *Sender);
    void __fastcall ProviderComboChange(TObject *Sender);
    void __fastcall AddressMergeFileDialogCanClose(TObject *Sender,
          bool &CanClose);
    void __fastcall SkipLetterPrintCheckBoxClick(TObject *Sender);
    void __fastcall MarkAsConfirmedCheckBoxClick(TObject *Sender);

private:	// User declarations
    Char_t      AddressMergeFileName[128];
    Int32u_t    StartDate;
    Int32u_t    EndDate;
    Int32u_t    ProviderId;
    Int32u_t    SucceededCount;
    Int32u_t    FailedCount;
    qHead_t     AppQueueHead;
    qHead_p     App_q;
    void __fastcall  AppointmentListGet( void );
    void __fastcall  AppointmentListFree( void );
    void __fastcall  PrintReport( void );

public:		// User declarations
    __fastcall TAppLettersForm(TComponent* Owner);
    __fastcall TAppLettersForm(TComponent* Owner, Int32u_t selectedDate, Int32u_t providerId );
    __fastcall ~TAppLettersForm( void );
};

//---------------------------------------------------------------------------

void    pmcPatAppLetterMake
(
        pmcPatAppLetter_p   app_p,
        bool                markAsConfirmed,
        bool                skipPrint
);

void    pmcPatAppLetterFree
(
        pmcPatAppLetter_p   app_p
);

#endif
