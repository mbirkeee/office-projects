//---------------------------------------------------------------------------
// File:    pmcClaimsListForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------


#ifndef pmcClaimListFormH
#define pmcClaimListFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Mask.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>

typedef struct pmcClaimListInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    providerId;
    Int32u_t    providerAllFlag;
    Int32u_t    patientId;
    Int32u_t    patientAllFlag;
    Int32u_t    latestDate;
    Int32u_t    earliestDate;
} pmcClaimListInfo_t, *pmcClaimListInfo_p;

typedef struct pmcClaimSruct_s
{
    qLinkage_t  linkage;
    Char_p      firstName_p;
    Char_p      lastName_p;
    Char_p      phn_p;
    Char_p      phnProv_p;
    Char_p      feeCode_p;
    Char_p      feeCodeApproved_p;
    Char_p      expCode_p;
    Char_p      providerName_p;
    Char_p      comment_p;
    Char_p      icdCode_p;
    Char_p      runCode_p;
    Int32u_t    id;
    Int32u_t    gender;
    Int32u_t    dob;
    Int32u_t    patientId;
    Int32u_t    providerId;
    Int32u_t    submitDay;
    Int32u_t    replyDay;
    Int32u_t    serviceDay;
    Int32u_t    lastDay;
    Int32u_t    claimNumber;
    Int16u_t    claimSequence;
    Int16u_t    claimIndex;
    Int32u_t    feeSubmitted;
    Int32u_t    feePaid;
    Int32u_t    feePremium;
    Int16u_t    referringNumber;
    Int16u_t    units;
    Int16u_t    billingNumber;
    Int32u_t    claimHeaderId;
    Int16u_t    appointmentType;
    Int16u_t    locationCode;
    Int16u_t    refDrTypeIndex;
    Int16u_t    subCount;
    Int32u_t    status;
    Int32u_t    submitStatus;
    Int32u_t    referringId;
} pmcClaimStruct_t, *pmcClaimStruct_p;

typedef struct pmcClaimRecordStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    type;
    Int32u_t    length;
    Int8u_p     data_p;
} pmcClaimRecordStruct_t, *pmcClaimRecordStruct_p;

#define PMC_CLAIM_LIST_LINES 18

class TClaimListForm : public TForm
{
__published:	// IDE-managed Components
    TDrawGrid *ClaimDrawGrid;
    TButton *CancelButton;
    TButton *OkButton;
    TButton *NewButton;
    TButton *PrintLongButton;
    TButton *PickupButton;
    TButton *SubmitButton;
    TEdit *PatientEdit;
    TRadioGroup *PatientRadioGroup;
    TGroupBox *GroupBox2;
    TPopupMenu *ClaimGridPopup;
    TMenuItem *ClaimGridPopupEdit;
    TMenuItem *ClaimGridPopupNew;
    TRadioGroup *ProviderRadioGroup;
    TComboBox *ProviderComboBox;
    TBitBtn *PatientSelectButton;
    TLabel *Label3;
    TLabel *Label8;
    TMenuItem *ClaimGridPopupView;
    TSaveDialog *SaveDialog;
    TEdit *DebugEdit;
    TMenuItem *N1;
    TMenuItem *ClaimGridPopupListPatient;
    TMenuItem *N2;
    TMenuItem *ClaimGridPopupListAppointments;
    TGroupBox *DisplayedClaimsGroupBox;
    TLabel *Label9;
    TLabel *NotReadyLabel;
    TLabel *ReadyLabel;
    TLabel *SubmittedLabel;
    TLabel *RejectedLabel;
    TLabel *PaidLabel;
    TLabel *NotReadyFeeClaimedLabel;
    TLabel *ReadyFeeClaimedLabel;
    TLabel *SubmittedFeeClaimedLabel;
    TLabel *RejectedFeeClaimedLabel;
    TLabel *PaidFeeClaimedLabel;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label11;
    TLabel *ReducedLabel;
    TLabel *ReducedFeeClaimedLabel;
    TBevel *Bevel2;
    TLabel *Label6;
    TLabel *TotalFeeClaimedLabel;
    TLabel *TotalFeePaidLabel;
    TLabel *TotalClaimsLabel;
    TPageControl *ClaimSearchPageControl;
    TTabSheet *ServiceDatesPage;
    TTabSheet *SubmitDatePage;
    TTabSheet *ReplyDatePage;
    TLabel *Label12;
    TLabel *Label13;
    TBitBtn *LatestDateButton;
    TBitBtn *EarliestDateButton;
    TMaskEdit *EarliestDateEdit;
    TMaskEdit *LatestDateEdit;
    TLabel *ReducedFeePaidLabel;
    TLabel *PaidFeePaidLabel;
    TCheckBox *NotReadyCheckBox;
    TCheckBox *ReadyCheckBox;
    TCheckBox *SubmittedCheckBox;
    TCheckBox *RejectedCheckBox;
    TCheckBox *ReducedCheckBox;
    TLabel *Label7;
    TLabel *Label10;
    TLabel *Label14;
    TLabel *Label15;
    TBevel *Bevel3;
    TMenuItem *ClaimGridPopupViewExp;
    TButton *PrintShortButton;
    TBevel *Bevel4;
    TBevel *Bevel5;
    TBevel *Bevel6;
    TBevel *Bevel7;
    TBevel *Bevel8;
    TBevel *Bevel9;
    TBevel *Bevel10;
    TBevel *Bevel11;
    TBevel *Bevel12;
    TBevel *Bevel13;
    TBevel *Bevel14;
    TBevel *Bevel15;
    TBevel *Bevel16;
    TBevel *Bevel17;
    TBevel *Bevel18;
    TBevel *Bevel19;
    TBevel *Bevel20;
    TBevel *Bevel21;
    TLabel *StatusLabel0;
    TLabel *StatusLabel1;
    TLabel *StatusLabel2;
    TLabel *StatusLabel3;
    TLabel *StatusLabel4;
    TLabel *StatusLabel5;
    TLabel *StatusLabel6;
    TLabel *StatusLabel7;
    TLabel *StatusLabel8;
    TLabel *StatusLabel9;
    TLabel *StatusLabel10;
    TLabel *StatusLabel11;
    TLabel *StatusLabel12;
    TLabel *StatusLabel13;
    TLabel *StatusLabel14;
    TLabel *StatusLabel15;
    TLabel *StatusLabel16;
    TLabel *StatusLabel17;
    TTabSheet *ClaimNumberPage;
    TEdit *ClaimNumberEdit;
    TLabel *Label16;
    TMaskEdit *SubmitDateEdit;
    TLabel *Label17;
    TBitBtn *SubmitDateButton;
    TMaskEdit *ReplyDateEdit;
    TBitBtn *ReplyDateButton;
    TLabel *Label18;
    TGroupBox *ListsGroupBox;
    TButton *PatientListButton;
    TButton *DoctorListButton;
    TCheckBox *RejectedAcceptCheckBox;
    TCheckBox *ReducedAcceptCheckBox;
    TLabel *Label19;
    TLabel *RejectedAcceptLabel;
    TLabel *RejectedAcceptFeeClaimedLabel;
    TLabel *ReducedAcceptLabel;
    TLabel *ReducedAcceptFeeClaimedLabel;
    TLabel *ReducedAcceptFeePaidLabel;
    TCheckBox *PaidCheckBox;
    TMenuItem *N4;
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall ProviderComboBoxChange(TObject *Sender);
    void __fastcall ProviderRadioGroupClick(TObject *Sender);
    void __fastcall PatientSelectButtonClick(TObject *Sender);
    void __fastcall PatientRadioGroupClick(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall ClaimDrawGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall PaidCheckBoxClick(TObject *Sender);
    void __fastcall SubmittedCheckBoxClick(TObject *Sender);
    void __fastcall NotReadyCheckBoxClick(TObject *Sender);
    void __fastcall ReadyCheckBoxClick(TObject *Sender);
    void __fastcall RejectedCheckBoxClick(TObject *Sender);
    void __fastcall PatientListButtonClick(TObject *Sender);
    void __fastcall NewButtonClick(TObject *Sender);
    void __fastcall ClaimDrawGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ClaimGridPopupPopup(TObject *Sender);
    void __fastcall ClaimGridPopupEditClick(TObject *Sender);
    void __fastcall ClaimGridPopupNewClick(TObject *Sender);
    void __fastcall ClaimDrawGridRowMoved(TObject *Sender, int FromIndex,
          int ToIndex);
    void __fastcall ClaimDrawGridTopLeftChanged(TObject *Sender);
    void __fastcall ClaimGridPopupViewClick(TObject *Sender);
    void __fastcall SubmitButtonClick(TObject *Sender);
    void __fastcall ClaimGridPopupListPatientClick(TObject *Sender);
    void __fastcall PickupButtonClick(TObject *Sender);
    void __fastcall PrintLongButtonClick(TObject *Sender);
    void __fastcall ClaimGridPopupListAppointmentsClick(TObject *Sender);
    void __fastcall LatestDateButtonClick(TObject *Sender);
    void __fastcall LatestDateEditChange(TObject *Sender);
    void __fastcall EarliestDateEditChange(TObject *Sender);
    void __fastcall EarliestDateButtonClick(TObject *Sender);
    void __fastcall ReducedCheckBoxClick(TObject *Sender);
    void __fastcall ClaimGridPopupViewExpClick(TObject *Sender);
    void __fastcall ClaimDrawGridClick(TObject *Sender);
    void __fastcall PrintShortButtonClick(TObject *Sender);
    void __fastcall ClaimDrawGridDblClick(TObject *Sender);
    void __fastcall ClaimNumberEditExit(TObject *Sender);
    void __fastcall ClaimNumberEditKeyPress(TObject *Sender, char &Key);
    void __fastcall ClaimSearchPageControlChange(TObject *Sender);
    void __fastcall ReplyDateButtonClick(TObject *Sender);
    void __fastcall SubmitDateButtonClick(TObject *Sender);
    void __fastcall SubmitDateEditChange(TObject *Sender);
    void __fastcall ReplyDateEditChange(TObject *Sender);
    void __fastcall StatusLabel0Click(TObject *Sender);
    void __fastcall StatusLabel1Click(TObject *Sender);
    void __fastcall StatusLabel2Click(TObject *Sender);
    void __fastcall StatusLabel3Click(TObject *Sender);
    void __fastcall StatusLabel4Click(TObject *Sender);
    void __fastcall StatusLabel5Click(TObject *Sender);
    void __fastcall StatusLabel6Click(TObject *Sender);
    void __fastcall StatusLabel7Click(TObject *Sender);
    void __fastcall StatusLabel8Click(TObject *Sender);
    void __fastcall StatusLabel9Click(TObject *Sender);
    void __fastcall StatusLabel10Click(TObject *Sender);
    void __fastcall StatusLabel11Click(TObject *Sender);
    void __fastcall StatusLabel12Click(TObject *Sender);
    void __fastcall StatusLabel13Click(TObject *Sender);
    void __fastcall StatusLabel14Click(TObject *Sender);
    void __fastcall StatusLabel15Click(TObject *Sender);
    void __fastcall StatusLabel16Click(TObject *Sender);
    void __fastcall StatusLabel17Click(TObject *Sender);
    void __fastcall StatusLabel0DblClick(TObject *Sender);
    void __fastcall StatusLabel1DblClick(TObject *Sender);
    void __fastcall StatusLabel2DblClick(TObject *Sender);
    void __fastcall StatusLabel3DblClick(TObject *Sender);
    void __fastcall StatusLabel4DblClick(TObject *Sender);
    void __fastcall StatusLabel5DblClick(TObject *Sender);
    void __fastcall StatusLabel6DblClick(TObject *Sender);
    void __fastcall StatusLabel7DblClick(TObject *Sender);
    void __fastcall StatusLabel8DblClick(TObject *Sender);
    void __fastcall StatusLabel9DblClick(TObject *Sender);
    void __fastcall StatusLabel10DblClick(TObject *Sender);
    void __fastcall StatusLabel11DblClick(TObject *Sender);
    void __fastcall StatusLabel12DblClick(TObject *Sender);
    void __fastcall StatusLabel13DblClick(TObject *Sender);
    void __fastcall StatusLabel14DblClick(TObject *Sender);
    void __fastcall StatusLabel15DblClick(TObject *Sender);
    void __fastcall StatusLabel16DblClick(TObject *Sender);
    void __fastcall StatusLabel17DblClick(TObject *Sender);
    void __fastcall StatusLabel0MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel1MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel2MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel3MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel4MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel5MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel6MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel7MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel8MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel9MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel10MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel11MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel12MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel13MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel14MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel15MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel16MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall StatusLabel17MouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall DoctorListButtonClick(TObject *Sender);
    void __fastcall RejectedAcceptCheckBoxClick(TObject *Sender);
    void __fastcall ReducedAcceptCheckBoxClick(TObject *Sender);
    void __fastcall PatientEditChange(TObject *Sender);
    void __fastcall PatientEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall LatestDateEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall EarliestDateEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall SubmitDateEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ReplyDateEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PatientEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall ClaimDrawGridMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ManuallyReconcile1Click(TObject *Sender);

private:	// User declarations

    void __fastcall         ClaimListGet( MbSQL *sqlIn_p );
    void __fastcall         ClaimListFree( void );
    void __fastcall         ClaimArrayGet( void );
    void __fastcall         PatientEditUpdate( Int32u_t patientId );
    void __fastcall         UpdateTotals( void );
    Boolean_t __fastcall    ClaimSortClaimNumberAscending ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortClaimNumberDescending( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortServiceDateAscending ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortServiceDateDescending( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortNameAscending        ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortNameDescending       ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortPhnAscending         ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortPhnDescending        ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortExpAscending         ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortExpDescending        ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortFeeCodeAscending     ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortFeeCodeDescending    ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortIdAscending          ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );
    Boolean_t __fastcall    ClaimSortIdDescending         ( pmcClaimStruct_p claim1_p, pmcClaimStruct_p claim2_p );

    Int32s_t __fastcall     ClaimHeaderWrite
    (
        FILE               *fp,
        Int32u_t            providerId,
        Int32u_p            recordsWritten_p,
        Int32u_t            tempClinicNumber,
        MbSQL              *sqp_p
    );
    Int32u_t __fastcall     ClaimServiceRecordWrite
    (
        FILE               *fp,
        pmcClaimStruct_p    claim_p,
        Int32u_t            doctorNumber,
        Int32u_p            sequenceNumber_p,
        Int32u_p            recordsWritten_p,
        Int32u_p            feeClaimed_p
    );
    Int32u_t __fastcall     ClaimCommentRecordWrite
    (
        FILE               *fp,
        pmcClaimStruct_p    claim_p,
        Int32u_t            doctorNumber,
        Int32u_t            sequenceNumber,
        Int32u_t            outOfProvince
    );
    Int32u_t __fastcall     ClaimTrailerRecordWrite
    (
        FILE               *fp,
        Int32u_t            doctorNumber,
        Int32u_t            totalFeeClaimed,
        Int32u_t            serviceRecordCount,
        Int32u_t            totalRecordCount
    );

    Int32u_t __fastcall     ClaimReciprocalRecordWrite
    (
        FILE               *fp
    );

    void __fastcall PrintClaims( Int32u_t longFlag );
    void __fastcall ValidateClaim( pmcClaimStruct_p claim_p );
    void __fastcall ClaimCheckNotReady( void );
    void __fastcall ClaimEdit( void );
    void __fastcall UpdateStatusLabels( void );
    void __fastcall StatusLabelClick( Ints_t index );
    void __fastcall StatusLabelDblClick( Ints_t index );
    void __fastcall StatusLabelMouseDown( Ints_t index );

    void __fastcall PatientSelect( Int32u_t key );

    pmcClaimStruct_p   *ClaimArray_pp;

    pmcClaimListInfo_p  ClaimListInfo_p;

    Int32u_t            EarliestDate;
    Int32u_t            LatestDate;
    Int32u_t            ProviderId;
    Int32u_t            ProviderIndex;
    Int32u_t            ProviderAllFlag;
    Int32u_t            PatientId;
    Int32u_t            PatientAllFlag;
    Int32s_t            ClaimArraySize;

    Int32u_t            ReadyFeeClaimed;
    Int32u_t            NotReadyFeeClaimed;
    Int32u_t            SubmittedFeeClaimed;
    Int32u_t            RejectedFeeClaimed;
    Int32u_t            RejectedAcceptFeeClaimed;
    Int32u_t            PaidFeeClaimed;
    Int32u_t            ReducedFeeClaimed;
    Int32u_t            ReducedAcceptFeeClaimed;

    Int32u_t            PaidFeePaid;
    Int32u_t            ReducedFeePaid;
    Int32u_t            ReducedAcceptFeePaid;

    Int32u_t            ReadyCount;
    Int32u_t            NotReadyCount;
    Int32u_t            SubmittedCount;
    Int32u_t            PaidCount;
    Int32u_t            RejectedCount;
    Int32u_t            ReducedCount;
    Int32u_t            ReducedAcceptCount;
    Int32u_t            RejectedAcceptCount;
    Int32u_t            TotalClaimsCount;

    Int32u_t            SearchClaimNumber;
    Int32u_t            SubmitDate;
    Int32u_t            ReplyDate;

    Boolean_t           SkipClaimListDraw;
    Boolean_t           SkipClaimListGet;
    Boolean_t           ProviderComboBoxSkipUpdate;
    Boolean_t           Active;
    Boolean_t           MouseDownExp;

    qHead_t             ClaimQueueHead;
    qHead_p             Claim_q;
    mbLock_t            ClaimQueueLock;
    Int32u_t            SortCode;
    Int32u_t            PreviousSortCode;
    Int32u_t            PreviousClaimNumber;
    Int32u_t            PreviousSequenceNumber;
    Int32u_t            ReciprocalRequired;
    pmcClaimStruct_p    ReciprocalClaim_p;
    Int32u_t            ReciprocalDoctorNumber;

    Int32u_t            SelectedClaimId;
    Int32s_t            MouseInRow;
    Int32s_t            MouseInCol;

    Boolean_t           PaidCheckBoxStore;
    Boolean_t           RejectedCheckBoxStore;
    Boolean_t           SubmittedCheckBoxStore;
    Boolean_t           ReadyCheckBoxStore;
    Boolean_t           NotReadyCheckBoxStore;
    Boolean_t           ReducedCheckBoxStore;
    Boolean_t           ReducedAcceptCheckBoxStore;
    Boolean_t           RejectedAcceptCheckBoxStore;

    TLabel             *StatusLabel[PMC_CLAIM_LIST_LINES];

public:		// User declarations
    __fastcall TClaimListForm(TComponent* Owner);
    __fastcall TClaimListForm(TComponent* Owner, pmcClaimListInfo_p claimListInfo_p );
};

#define PMC_CLAIM_LIST_POPUP_EDIT                   "ClaimGridPopupEdit"
#define PMC_CLAIM_LIST_POPUP_NEW                    "ClaimGridPopupNew"
#define PMC_CLAIM_LIST_POPUP_VIEW                   "ClaimGridPopupView"
#define PMC_CLAIM_LIST_POPUP_VIEW_EXP               "ClaimGridPopupViewExp"
#define PMC_CLAIM_LIST_POPUP_LIST_PATIENT           "ClaimGridPopupListPatient"
#define PMC_CLAIM_LIST_POPUP_LIST_APPOINTMENTS      "ClaimGridPopupListAppointments"

Int32s_t pmcClaimsFilePrint
(
    Char_p      fileName_p,
    Char_p      archiveFileName_p
);

Int32s_t pmcClaimsFilePrintProvider
(
    Char_p      fileName_p,
    Char_p      archiveFileName_p,
    qHead_p     record_q
);

Int32s_t pmcClaimsFilePrintHeader
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p
);

Int32s_t    pmcClaimsFilePrintService
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintReciprocal
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintComment
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintHospital
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintTrailer
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                commentCount
);


Int32s_t    pmcMspFilesLocate
(
    Boolean_t    *claimsin_p,
    Boolean_t    *info_p,
    Boolean_t    *validrpt_p,
    Boolean_t    *returns_p,
    Boolean_t    *notice_p,
    Boolean_t    logFlag
);

Int32s_t pmcSubmit( void );

#endif
