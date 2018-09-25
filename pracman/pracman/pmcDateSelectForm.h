//---------------------------------------------------------------------------
// Function: pmcDateSelectForm.h
//---------------------------------------------------------------------------
// Date: Feb. 15, 2001
//---------------------------------------------------------------------------
// Description:
//
// Form for choosing date.
//---------------------------------------------------------------------------

#ifndef pmcDateSelectFormH
#define pmcDateSelectFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CCALENDR.h"
#include <ExtCtrls.hpp>
#include <Grids.hpp>

#include "mbTypes.h"

typedef struct pmcDateSelectInfo_s
{
    Int32u_t    mode;
    Int32u_t    clearEnabled;
    Int32u_t    dateIn;
    Int32u_t    returnCode;
    Int32u_t    dateOut;
    Char_p      string_p;
    Char_p      caption_p;
} pmcDateSelectInfo_t, *pmcDateSelectInf_p;

//---------------------------------------------------------------------------
class TDateSelectForm : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TCCalendar *Calendar;
    TScrollBar *YearScrollBar;
    TButton *OkButton;
    TButton *TodayButton;
    TBevel *Bevel2;
    TButton *JanButton;
    TButton *FebButton;
    TButton *MarButton;
    TButton *AprButton;
    TButton *MayButton;
    TButton *JuneButton;
    TButton *JulyButton;
    TButton *AugButton;
    TButton *SeptButton;
    TButton *OctButton;
    TButton *NovButton;
    TButton *DecButton;
    TLabel *DateLabel;
    TButton *CancelButton;
    TLabel *Label3;
    TLabel *Label4;
    TButton *ClearButton;
    void __fastcall FormClose( TObject *Sender, TCloseAction &Action );
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall YearScrollBarChange(TObject *Sender);
    void __fastcall TodayButtonClick(TObject *Sender);
    void __fastcall JanButtonClick(TObject *Sender);
    void __fastcall FebButtonClick(TObject *Sender);
    void __fastcall MarButtonClick(TObject *Sender);
    void __fastcall AprButtonClick(TObject *Sender);
    void __fastcall MayButtonClick(TObject *Sender);
    void __fastcall JuneButtonClick(TObject *Sender);
    void __fastcall JulyButtonClick(TObject *Sender);
    void __fastcall AugButtonClick(TObject *Sender);
    void __fastcall SeptButtonClick(TObject *Sender);
    void __fastcall OctButtonClick(TObject *Sender);
    void __fastcall NovButtonClick(TObject *Sender);
    void __fastcall DecButtonClick(TObject *Sender);
    void __fastcall CalendarChange(TObject *Sender);
    void __fastcall CalendarKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall CalendarKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall CalendarDblClick(TObject *Sender);
    void __fastcall ClearButtonClick(TObject *Sender);

private:	// User declarations
    void __fastcall UpdateDateString( void );
    void __fastcall CalendarDone( void );

    Int32u_t        OrigYear;
    Int32u_t        OrigMonth;
    Int32u_t        OrigDate;
    pmcDateSelectInf_p DateSelectInfo_p;

    Int32u_t        KeyDownDate;
    Int32u_t        KeyDownKey;
    Int32u_t        SelectedDate;
    bool            SkipUpdate;
    bool            ClearFlag;

public:		// User declarations
    __fastcall TDateSelectForm
    (
        TComponent*         Owner,
        pmcDateSelectInf_p dateSelectInfo_p
    );
  
    void __fastcall TDateSelectForm::UpdateDate
    (
        Int32u_t    year,
        Int32u_t    month,
        Int32u_t    day
    );
    __fastcall TDateSelectForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDateSelectForm *DateSelectForm;
//---------------------------------------------------------------------------
#define PMC_DATE_SELECT_TODAY       0
#define PMC_DATE_SELECT_PARMS       1
#define PMC_SELECT_DATE_YEAR_MAX    2100
#define PMC_SELECT_DATE_YEAR_MIN    1890

#endif
