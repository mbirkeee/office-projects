//---------------------------------------------------------------------------
// File:    pmcAppListForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    August 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcAppListFormH
#define pmcAppListFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>

typedef struct pmcAppListStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    date;
    Int32u_t    duration;
    Int32u_t    time;
    Int32u_t    completedState;
    Int32u_t    type;
    Int32u_t    notDeleted;
    Int32u_t    confirmedPhoneDate;
    Int32u_t    confirmedPhoneTime;
    Int32u_t    confirmedPhoneId;
    Int32u_t    confirmedLetterDate;
    Int32u_t    confirmedLetterTime;
    Int32u_t    confirmedLetterId;
    Int32u_t    providerId;
    Int32u_t    conflictCount;
    Int32u_t    conflict;
    Int32u_t    unavail;
    Int32u_t    appCount;
    Int32u_t    startTimeMin;
    Int32u_t    endTimeMin;
    Char_p      providerDesc_p;
    Char_p      referringDesc_p;
    Char_p      comment_p;
    Char_t      timeslots[PMC_TIMESLOTS_PER_DAY+1];
} pmcAppListStruct_t, *pmcAppListStruct_p;


typedef struct pmcAppListInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    patientId;
    Int32u_t    providerId;
    Int32u_t    date;
    Int32u_t    appointId;
    Int32u_t    mode;
    bool        showFuture;
    bool        showPast;
    bool        allowGoto;
    bool        gotGoto;
    bool        allowPatientSelect;
} pmcAppListInfo_t, *pmcAppListInfo_p;

class TAppListForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CancelButton;
    TBevel *Bevel1;
    TDrawGrid *AppListGrid;
    TGroupBox *GroupBox1;
    TBevel *Bevel3;
    TLabel *Label1;
    TBevel *Bevel4;
    TLabel *Label2;
    TButton *PatientEditButton;
    TLabel *PhnLabel;
    TLabel *PhoneLabel;
    TGroupBox *GroupBox2;
    TCheckBox *PastAppsCheckBox;
    TCheckBox *FutureAppsCheckBox;
    TBevel *Bevel5;
    TBevel *Bevel6;
    TLabel *FutureAppsCountLabel;
    TLabel *PastAppsCountLabel;
    TPopupMenu *AppListFormPopup;
    TMenuItem *AppListFormPopupGoto;
    TMenuItem *N2;
    TMenuItem *AppListFormPopupType;
    TMenuItem *AppListFormPopupDr;
    TMenuItem *AppListFormPopupComment;
    TMenuItem *N3;
    TMenuItem *AppListFormPopupCut;
    TMenuItem *AppListFormPopupDelete;
    TMenuItem *N4;
    TMenuItem *AppListFormPopupConfirm;
    TMenuItem *AppListFormPopupArrived;
    TMenuItem *AppListFormPopupComplete;
    TMenuItem *AppListFormPopupPatient;
    TMenuItem *AppListFormPopupPatientEdit;
    TMenuItem *AppListFormPopupPatientView;
    TMenuItem *N5;
    TMenuItem *AppListFormPopupPatientPrintRecord;
    TMenuItem *AppListFormPopupPatientPrintAddLbl;
    TMenuItem *AppListFormPopupPatientPrintReqLbl;
    TMenuItem *AppListFormPopupPatientPrintAppLetter;
    TMenuItem *AppListFormPopupTypeNew;
    TMenuItem *AppListFormPopupTypeUrgent;
    TMenuItem *AppListFormPopupTypeReview;
    TMenuItem *N6;
    TMenuItem *AppListFormPopupTypeClear;
    TMenuItem *AppListFormPopupDrView;
    TMenuItem *AppListFormPopupDrEdit;
    TMenuItem *AppListFormPopupDrChange;
    TMenuItem *AppListFormPopupDrClear;
    TMenuItem *N7;
    TMenuItem *AppListFormPopupDrPrintRecord;
    TMenuItem *AppListFormPopupDrPrintAddLbl;
    TMenuItem *Edit3;
    TMenuItem *View3;
    TMenuItem *AppListFormPopupConfirmPhone;
    TMenuItem *AppListFormPopupConfirmLetter;
    TMenuItem *N8;
    TMenuItem *AppListFormPopupConfirmPhoneCancel;
    TMenuItem *AppListFormPopupConfirmLetterCancel;
    TMenuItem *N9;
    TMenuItem *AppListFormPopupConfirmCheck;
    TGroupBox *GroupBox3;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Label7;
    TBevel *Bevel7;
    TBevel *Bevel8;
    TBevel *Bevel9;
    TLabel *AppDateLabel;
    TLabel *AppTimeLabel;
    TLabel *AppProviderLabel;
    TEdit *PatientNameEdit;
    TBitBtn *PatientSelectButton;
    TLabel *DoubleClickLabel;
    TButton *PatientViewButton;
    TButton *OkButton;
    TCheckBox *CancelledAppsCheckBox;
    TBevel *Bevel2;
    TLabel *CancelledAppsCountLabel;
    TMenuItem *AppListFormPopupCancel;
    TGroupBox *GroupBox4;
    TDrawGrid *AppLegendDrawGrid;
    TMenuItem *N1;
    TMenuItem *AppListFormPopupHistory;
    TCheckBox *DeletedAppsCheckBox;
    TBevel *Bevel10;
    TLabel *DeletedAppsCountLabel;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall AppListGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall AppListGridMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall AppListGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall PastAppsCheckBoxClick(TObject *Sender);
    void __fastcall FutureAppsCheckBoxClick(TObject *Sender);
    void __fastcall PatientEditButtonClick(TObject *Sender);
    void __fastcall AppListFormPopupPopup(TObject *Sender);
    void __fastcall AppListFormPopupCutClick(TObject *Sender);
    void __fastcall AppListFormPopupDeleteClick(TObject *Sender);
    void __fastcall AppListGridDblClick(TObject *Sender);
    void __fastcall PatientNameEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PatientSelectButtonClick(TObject *Sender);
    void __fastcall AppListFormPopupGotoClick(TObject *Sender);
    void __fastcall AppListFormPopupCompleteClick(TObject *Sender);
    void __fastcall AppListFormPopupConfirmCheckClick(TObject *Sender);
    void __fastcall AppListFormPopupConfirmPhoneClick(TObject *Sender);
    void __fastcall AppListFormPopupConfirmLetterClick(TObject *Sender);
    void __fastcall AppListFormPopupConfirmLetterCancelClick(
          TObject *Sender);
    void __fastcall AppListFormPopupConfirmPhoneCancelClick(
          TObject *Sender);
    void __fastcall AppListFormPopupTypeNewClick(TObject *Sender);
    void __fastcall AppListFormPopupTypeUrgentClick(TObject *Sender);
    void __fastcall AppListFormPopupTypeReviewClick(TObject *Sender);
    void __fastcall AppListFormPopupTypeClearClick(TObject *Sender);
    void __fastcall PatientViewButtonClick(TObject *Sender);
    void __fastcall AppListFormPopupPatientEditClick(TObject *Sender);
    void __fastcall AppListFormPopupPatientViewClick(TObject *Sender);
    void __fastcall AppListFormPopupPatientPrintRecordClick(
          TObject *Sender);
    void __fastcall AppListFormPopupPatientPrintAddLblClick(
          TObject *Sender);
    void __fastcall AppListFormPopupPatientPrintReqLblClick(
          TObject *Sender);
    void __fastcall AppListFormPopupPatientPrintAppLetterClick(
          TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall AppListFormPopupCancelClick(TObject *Sender);
    void __fastcall CancelledAppsCheckBoxClick(TObject *Sender);
    void __fastcall AppLegendDrawGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall AppListFormPopupHistoryClick(TObject *Sender);
    void __fastcall DeletedAppsCheckBoxClick(TObject *Sender);

private:	// User declarations
    void __fastcall AppListGet( void );
    void __fastcall AppListFree( void );
    void __fastcall AppListSort( void );
    void __fastcall UpdatePatient( void );
    void __fastcall UpdateSelectedAppointment( void );

    void __fastcall AppListConflictsPatient( void );
    void __fastcall AppListConflictsTimeslots( void );
    void __fastcall AppListConflictsProvider( Char_p provDate_p );
    
    bool __fastcall AppListSortDateAscending
    (
        pmcAppListStruct_p  app1_p,
        pmcAppListStruct_p  app2_p
    );

    bool __fastcall AppListSortDateDescending
    (
        pmcAppListStruct_p  app1_p,
        pmcAppListStruct_p  app2_p
    );

    void __fastcall AppListFormPopupAppTypeSet
    (
        Int32u_t            appointId,
        Int32u_t            appType
    );

    void __fastcall PatientEditView
    (
        Int32u_t            mode
    );

    void __fastcall AppListFormPopupDeleteCancel
    (
        Int32u_t            cancelFlag
    );

    // Pointer to array of pointers
    pmcAppListStruct_p   *AppArray_pp;

    bool        AppListInvalid;
    bool        SkipAppListGet;
    bool        Active;
    bool        DoubleClickFlag;
    bool        PastAutoEnable;

    qHead_p     App_q;
    qHead_t     AppQueueHead;
    Int32u_t    SortCode;
    Int32u_t    PreviousSortCode;
    Ints_t      AppArraySize;
    Ints_t      MouseInCol;
    Ints_t      MouseInRow;

    Int32u_t    PastAppsCount;
    Int32u_t    FutureAppsCount;
    Int32u_t    CancelledAppsCount;
    Int32u_t    DeletedAppsCount;

    mbLock_t    AppListLock;
    Int32u_t    Today;

    pmcAppListInfo_p    FormInfo_p;

public:		// User declarations
    __fastcall TAppListForm(TComponent* Owner);
    __fastcall TAppListForm(TComponent* Owner, pmcAppListInfo_p formInfo_p );

};

enum
{
     PMC_APP_SORT_NONE = 0
    ,PMC_APP_SORT_DATE_ASCENDING
    ,PMC_APP_SORT_DATE_DESCENDING
};

#define PMC_APP_LIST_FORM_POPUP_GOTO                "AppListFormPopupGoto"
#define PMC_APP_LIST_FORM_POPUP_CUT                 "AppListFormPopupCut"
#define PMC_APP_LIST_FORM_POPUP_DELETE              "AppListFormPopupDelete"
#define PMC_APP_LIST_FORM_POPUP_CANCEL              "AppListFormPopupCancel"
#define PMC_APP_LIST_FORM_POPUP_PATIENT             "AppListFormPopupPatient"
#define PMC_APP_LIST_FORM_POPUP_PATIENT_VIEW        "AppListFormPopupPatientView"
#define PMC_APP_LIST_FORM_POPUP_PATIENT_EDIT        "AppListFormPopupPatientEdit"

#define PMC_APP_LIST_FORM_POPUP_DR                  "AppListFormPopupDr"
#define PMC_APP_LIST_FORM_POPUP_DR_EDIT             "AppListFormPopupDrEdit"
#define PMC_APP_LIST_FORM_POPUP_DR_VIEW             "AppListFormPopupDrView"
#define PMC_APP_LIST_FORM_POPUP_DR_CHANGE           "AppListFormPopupDrChange"
#define PMC_APP_LIST_FORM_POPUP_DR_CLEAR            "AppListFormPopupDrClear"
#define PMC_APP_LIST_FORM_POPUP_DR_PRINT_RECORD     "AppListFormPopupDrPrintRecord"
#define PMC_APP_LIST_FORM_POPUP_DR_PRINT_ADD_LBL    "AppListFormPopupDrPrintAddLbl"

#define PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_RECORD     "AppListFormPopupPatientPrintRecord"
#define PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_ADD_LBL    "AppListFormPopupPatientPrintAddLbl"
#define PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_REQ_LBL    "AppListFormPopupPatientPrintReqLbl"
#define PMC_APP_LIST_FORM_POPUP_PATIENT_PRINT_APP_LETTER "AppListFormPopupPatientPrintAppLetter"

#define PMC_APP_LIST_FORM_POPUP_COMPLETE                "AppListFormPopupComplete"
#define PMC_APP_LIST_FORM_POPUP_HISTORY                 "AppListFormPopupHistory"

#define PMC_APP_LIST_FORM_POPUP_CONFIRM                 "AppListFormPopupConfirm"
#define PMC_APP_LIST_FORM_POPUP_CONFIRM_PHONE           "AppListFormPopupConfirmPhone"
#define PMC_APP_LIST_FORM_POPUP_CONFIRM_LETTER          "AppListFormPopupConfirmLetter"
#define PMC_APP_LIST_FORM_POPUP_CONFIRM_PHONE_CANCEL    "AppListFormPopupConfirmPhoneCancel"
#define PMC_APP_LIST_FORM_POPUP_CONFIRM_LETTER_CANCEL   "AppListFormPopupConfirmLetterCancel"
#define PMC_APP_LIST_FORM_POPUP_CONFIRM_CHECK           "AppListFormPopupConfirmCheck"

#define PMC_APP_LIST_FORM_POPUP_TYPE            "AppListFormPopupType"
#define PMC_APP_LIST_FORM_POPUP_TYPE_NEW        "AppListFormPopupTypeNew"
#define PMC_APP_LIST_FORM_POPUP_TYPE_URGENT     "AppListFormPopupTypeUrgent"
#define PMC_APP_LIST_FORM_POPUP_TYPE_REVIEW     "AppListFormPopupTypeReview"
#define PMC_APP_LIST_FORM_POPUP_TYPE_CLEAR      "AppListFormPopupTypeClear"


#endif
