//---------------------------------------------------------------------------
// File:    pmcClaimEditForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 19, 2001
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

#ifndef pmcClaimEditFormH
#define pmcClaimEditFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

#include "mbTypes.h"
#include <Mask.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>

#define PMC_CLAIM_COUNT         9

typedef struct pmcClaimEditInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    patientId;
    Int32u_t    providerId;
    Int32u_t    claimNumber;
    Int32u_t    claimHeaderId;
    Int32u_t    claimId;
    Int32u_t    mode;
    Int32u_t    updateClaims;
} pmcClaimEditInfo_t, *pmcClaimEditInfo_p;

//---------------------------------------------------------------------------

class TClaimEditForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CancelButton;
    TButton *OkButton;
    TBevel *Bevel1;
    TComboBox *FeeList0;
    TComboBox *FeeList1;
    TComboBox *FeeList2;
    TComboBox *FeeList3;
    TComboBox *FeeList4;
    TComboBox *FeeList5;
    TComboBox *FeeList6;
    TComboBox *FeeList7;
    TComboBox *FeeList8;
    TEdit *DateEdit0;
    TEdit *UnitsEdit0;
    TEdit *UnitsEdit1;
    TEdit *UnitsEdit2;
    TEdit *UnitsEdit3;
    TEdit *UnitsEdit4;
    TEdit *UnitsEdit5;
    TEdit *UnitsEdit6;
    TEdit *UnitsEdit7;
    TEdit *UnitsEdit8;
    TEdit *FeeChargedEdit0;
    TEdit *FeeChargedEdit1;
    TEdit *FeeChargedEdit2;
    TEdit *FeeChargedEdit3;
    TEdit *FeeChargedEdit4;
    TEdit *FeeChargedEdit5;
    TEdit *FeeChargedEdit6;
    TEdit *FeeChargedEdit7;
    TEdit *FeeChargedEdit8;
    TGroupBox *PatientGroupBox;
    TGroupBox *ProviderGroupBox;
    TGroupBox *ReferringDrGroupBox;
    TEdit *PatientEdit;
    TEdit *ReferringDrEdit;
    TComboBox *LocationListBox;
    TComboBox *ProviderListBox;
    TComboBox *IcdComboBox;
    TEdit *DiagnosisEdit;
    TEdit *DateEdit1;
    TEdit *DateEdit2;
    TEdit *DateEdit3;
    TEdit *DateEdit4;
    TEdit *DateEdit5;
    TEdit *DateEdit6;
    TEdit *DateEdit7;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Label7;
    TLabel *Label8;
    TLabel *Label9;
    TLabel *Label10;
    TLabel *Label11;
    TEdit *DateEdit9;
    TEdit *DateEdit10;
    TEdit *DateEdit8;
    TLabel *Label13;
    TBitBtn *DateButton0;
    TLabel *Label14;
    TLabel *Label15;
    TLabel *Label16;
    TLabel *Label17;
    TLabel *Label1;
    TLabel *Label18;
    TPageControl *PageControl;
    TTabSheet *CommentsSheet;
    TTabSheet *SubmitStatusSheet;
    TEdit *CommentEdit0;
    TEdit *CommentEdit1;
    TEdit *CommentEdit2;
    TEdit *CommentEdit3;
    TEdit *CommentEdit4;
    TEdit *CommentEdit5;
    TEdit *CommentEdit6;
    TEdit *CommentEdit7;
    TEdit *CommentEdit8;
    TLabel *Label2;
    TLabel *Label12;
    TLabel *Label19;
    TLabel *Label20;
    TGroupBox *GroupBox4;
    TBitBtn *DateButton1;
    TBitBtn *DateButton2;
    TBitBtn *DateButton3;
    TBitBtn *DateButton4;
    TBitBtn *DateButton5;
    TBitBtn *DateButton6;
    TBitBtn *DateButton7;
    TBitBtn *DateButton8;
    TBitBtn *DateButton9;
    TBitBtn *DateButton10;
    TBitBtn *PatientSelectButton;
    TBitBtn *ReferringDrSelectButton;
    TLabel *Label21;
    TLabel *Label22;
    TButton *PatientEditButton;
    TButton *ReferringDrEditButton;
    TLabel *SeqLabelDisplay;
    TLabel *Label23;
    TComboBox *ReferringDrTypeListBox;
    TGroupBox *GroupBox5;
    TLabel *Label24;
    TTabSheet *EditStatusSheet;
    TBevel *Bevel2;
    TBevel *Bevel3;
    TBevel *Bevel4;
    TBevel *Bevel5;
    TBevel *Bevel6;
    TBevel *Bevel7;
    TBevel *Bevel8;
    TBevel *Bevel9;
    TBevel *Bevel10;
    TLabel *StatusLabel0;
    TLabel *StatusLabel1;
    TLabel *StatusLabel2;
    TLabel *StatusLabel3;
    TLabel *StatusLabel4;
    TLabel *StatusLabel5;
    TLabel *StatusLabel6;
    TLabel *StatusLabel7;
    TLabel *StatusLabel8;
    TBevel *Bevel12;
    TBevel *Bevel13;
    TBevel *Bevel14;
    TBevel *Bevel15;
    TBevel *Bevel16;
    TBevel *Bevel17;
    TBevel *Bevel18;
    TBevel *Bevel19;
    TBevel *Bevel20;
    TLabel *EditStatusLabel0;
    TLabel *EditStatusLabel1;
    TLabel *EditStatusLabel2;
    TLabel *EditStatusLabel3;
    TLabel *EditStatusLabel4;
    TLabel *EditStatusLabel5;
    TLabel *EditStatusLabel6;
    TLabel *EditStatusLabel7;
    TLabel *EditStatusLabel8;
    TLabel *TotalFeeChargedLabel;
    TBevel *Bevel22;
    TBevel *Bevel23;
    TLabel *PatientPhnLabel;
    TBevel *Bevel24;
    TLabel *Label25;
    TLabel *PatientDobLabel;
    TLabel *PatientGenderLabel;
    TBevel *Bevel25;
    TBevel *Bevel26;
    TLabel *ProviderClinicNumberLabel;
    TLabel *ProviderBillingNumberLabel;
    TBevel *Bevel27;
    TBevel *Bevel28;
    TBevel *Bevel29;
    TBevel *Bevel30;
    TLabel *ClaimNumberLabel;
    TLabel *ClaimHeaderIdLabel;
    TLabel *ClaimCreatedLabel;
    TLabel *Label26;
    TLabel *ClaimModifiedLabel;
    TLabel *Label27;
    TLabel *Label28;
    TLabel *Label29;
    TBevel *Bevel31;
    TLabel *ReferringDrNumberLabel;
    TButton *ReferringViewButton;
    TButton *PatientViewButton;
    TButton *ReferringDrClearButton;
    TBevel *Bevel32;
    TBevel *Bevel33;
    TBevel *Bevel34;
    TBevel *Bevel35;
    TBevel *Bevel36;
    TBevel *Bevel37;
    TBevel *Bevel38;
    TBevel *Bevel39;
    TBevel *Bevel40;
    TLabel *SubmitLabel0;
    TLabel *SubmitLabel1;
    TLabel *SubmitLabel2;
    TLabel *SubmitLabel3;
    TLabel *SubmitLabel4;
    TLabel *SubmitLabel5;
    TLabel *SubmitLabel6;
    TLabel *SubmitLabel7;
    TLabel *SubmitLabel8;
    TBevel *Bevel41;
    TBevel *Bevel42;
    TBevel *Bevel43;
    TBevel *Bevel44;
    TBevel *Bevel46;
    TBevel *Bevel47;
    TBevel *Bevel48;
    TBevel *Bevel49;
    TBevel *Bevel50;
    TLabel *SeqLabel0;
    TLabel *SeqLabel1;
    TLabel *SeqLabel2;
    TLabel *SeqLabel3;
    TLabel *SeqLabel4;
    TLabel *SeqLabel5;
    TLabel *SeqLabel6;
    TLabel *SeqLabel7;
    TLabel *SeqLabel8;
    TBevel *Bevel51;
    TBevel *Bevel52;
    TBevel *Bevel53;
    TBevel *Bevel54;
    TBevel *Bevel55;
    TBevel *Bevel56;
    TBevel *Bevel57;
    TBevel *Bevel58;
    TBevel *Bevel59;
    TLabel *ReplyLabel0;
    TLabel *ReplyLabel1;
    TLabel *ReplyLabel2;
    TLabel *ReplyLabel3;
    TLabel *ReplyLabel4;
    TLabel *ReplyLabel5;
    TLabel *ReplyLabel6;
    TLabel *ReplyLabel7;
    TLabel *ReplyLabel8;
    TBevel *Bevel60;
    TBevel *Bevel61;
    TBevel *Bevel62;
    TBevel *Bevel63;
    TBevel *Bevel64;
    TBevel *Bevel65;
    TBevel *Bevel66;
    TBevel *Bevel67;
    TBevel *Bevel68;
    TLabel *PaidLabel0;
    TLabel *PaidLabel1;
    TLabel *PaidLabel2;
    TLabel *PaidLabel3;
    TLabel *PaidLabel4;
    TLabel *PaidLabel5;
    TLabel *PaidLabel6;
    TLabel *PaidLabel7;
    TLabel *PaidLabel8;
    TBevel *Bevel69;
    TBevel *Bevel70;
    TBevel *Bevel71;
    TBevel *Bevel72;
    TBevel *Bevel73;
    TBevel *Bevel74;
    TBevel *Bevel75;
    TBevel *Bevel76;
    TBevel *Bevel77;
    TLabel *ExpLabel0;
    TLabel *ExpLabel1;
    TLabel *ExpLabel2;
    TLabel *ExpLabel3;
    TLabel *ExpLabel4;
    TLabel *ExpLabel5;
    TLabel *ExpLabel6;
    TLabel *ExpLabel7;
    TLabel *ExpLabel8;
    TPopupMenu *ClaimEditPopup;
    TMenuItem *ClaimEditPopupClear;
    TMenuItem *ClaimEditPopupAccept;
    TMenuItem *ClaimEditPopupResubmit;
    TEdit *IcdDescEdit;
    TLabel *Label30;
    TBevel *Bevel11;
    TBevel *Bevel78;
    TBevel *Bevel79;
    TBevel *Bevel80;
    TBevel *Bevel81;
    TBevel *Bevel82;
    TBevel *Bevel83;
    TBevel *Bevel84;
    TBevel *Bevel85;
    TLabel *CodeLabel0;
    TLabel *CodeLabel1;
    TLabel *CodeLabel2;
    TLabel *CodeLabel3;
    TLabel *CodeLabel4;
    TLabel *CodeLabel5;
    TLabel *CodeLabel6;
    TLabel *CodeLabel7;
    TLabel *CodeLabel8;
    TLabel *Label31;
    TMenuItem *N1;
    TMenuItem *ClaimEditPopupExp;
    TTabSheet *MspCommentSheet;
    TBevel *Bevel86;
    TBevel *Bevel87;
    TBevel *Bevel88;
    TBevel *Bevel89;
    TBevel *Bevel90;
    TBevel *Bevel91;
    TBevel *Bevel92;
    TBevel *Bevel93;
    TBevel *Bevel94;
    TLabel *MspCommentLabel0;
    TLabel *MspCommentLabel1;
    TLabel *MspCommentLabel2;
    TLabel *MspCommentLabel3;
    TLabel *MspCommentLabel4;
    TLabel *MspCommentLabel5;
    TLabel *MspCommentLabel6;
    TLabel *MspCommentLabel7;
    TLabel *MspCommentLabel8;
    TGroupBox *GroupBox1;
    TButton *PatientListButton;
    TButton *DoctorListButton;
    TLabel *Label32;
    TComboBox *PremiumComboBox0;
    TComboBox *PremiumComboBox1;
    TComboBox *PremiumComboBox2;
    TComboBox *PremiumComboBox3;
    TComboBox *PremiumComboBox4;
    TComboBox *PremiumComboBox5;
    TComboBox *PremiumComboBox6;
    TLabel *Label33;
    TComboBox *PremiumComboBox8;
    TComboBox *PremiumComboBox7;
    TLabel *Label34;
    TLabel *Label35;
    TLabel *Label36;
    TLabel *Label37;
    TBevel *Bevel95;
    TBevel *Bevel96;
    TBevel *Bevel97;
    TBevel *Bevel98;
    TBevel *Bevel99;
    TBevel *Bevel100;
    TBevel *Bevel101;
    TBevel *Bevel102;
    TBevel *Bevel103;
    TLabel *Label38;
    TLabel *PremPaid0;
    TLabel *PremPaid1;
    TLabel *PremPaid2;
    TLabel *PremPaid3;
    TLabel *PremPaid4;
    TLabel *PremPaid5;
    TLabel *PremPaid6;
    TLabel *PremPaid7;
    TLabel *PremPaid8;
    TLabel *Label39;
    TLabel *Label40;
    TLabel *Label41;
    TLabel *TotalPaidLabel;
    TLabel *TotalPremPaidLabel;
    TLabel *GrandTotalLabel;
    TButton *IcdListButton;
    TBevel *Bevel21;
    TBevel *Bevel104;
    TBevel *Bevel105;
    TBevel *Bevel106;
    TBevel *Bevel107;
    TBevel *Bevel108;
    TBevel *Bevel109;
    TBevel *Bevel110;
    TBevel *Bevel111;
    TLabel *UnitsPaidLabel0;
    TLabel *UnitsPaidLabel1;
    TLabel *UnitsPaidLabel2;
    TLabel *UnitsPaidLabel3;
    TLabel *UnitsPaidLabel4;
    TLabel *UnitsPaidLabel5;
    TLabel *UnitsPaidLabel6;
    TLabel *UnitsPaidLabel7;
    TLabel *UnitsPaidLabel8;
    TLabel *Label42;
    TPopupMenu *FeeCodePopup;
    TMenuItem *Editfeecodepicklist1;
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall IcdComboBoxChange(TObject *Sender);
    void __fastcall IcdComboBoxExit(TObject *Sender);
    void __fastcall FeeList0Change(TObject *Sender);
    void __fastcall FeeList1Change(TObject *Sender);
    void __fastcall FeeList2Change(TObject *Sender);
    void __fastcall FeeList3Change(TObject *Sender);
    void __fastcall FeeList4Change(TObject *Sender);
    void __fastcall FeeList5Change(TObject *Sender);
    void __fastcall FeeList6Change(TObject *Sender);
    void __fastcall FeeList7Change(TObject *Sender);
    void __fastcall FeeList8Change(TObject *Sender);
    void __fastcall FeeList0Exit(TObject *Sender);
    void __fastcall FeeList1Exit(TObject *Sender);
    void __fastcall FeeList2Exit(TObject *Sender);
    void __fastcall FeeList3Exit(TObject *Sender);
    void __fastcall FeeList4Exit(TObject *Sender);
    void __fastcall FeeList5Exit(TObject *Sender);
    void __fastcall FeeList6Exit(TObject *Sender);
    void __fastcall FeeList7Exit(TObject *Sender);
    void __fastcall FeeList8Exit(TObject *Sender);
    void __fastcall DateButton0Click(TObject *Sender);
    void __fastcall UnitsEdit0Change(TObject *Sender);
    void __fastcall UnitsEdit1Change(TObject *Sender);
    void __fastcall UnitsEdit2Change(TObject *Sender);
    void __fastcall UnitsEdit3Change(TObject *Sender);
    void __fastcall UnitsEdit4Change(TObject *Sender);
    void __fastcall UnitsEdit5Change(TObject *Sender);
    void __fastcall UnitsEdit6Change(TObject *Sender);
    void __fastcall UnitsEdit7Change(TObject *Sender);
    void __fastcall UnitsEdit8Change(TObject *Sender);
    void __fastcall UnitsEdit0Exit(TObject *Sender);
    void __fastcall UnitsEdit1Exit(TObject *Sender);
    void __fastcall UnitsEdit2Exit(TObject *Sender);
    void __fastcall UnitsEdit3Exit(TObject *Sender);
    void __fastcall UnitsEdit4Exit(TObject *Sender);
    void __fastcall UnitsEdit5Exit(TObject *Sender);
    void __fastcall UnitsEdit6Exit(TObject *Sender);
    void __fastcall UnitsEdit7Exit(TObject *Sender);
    void __fastcall UnitsEdit8Exit(TObject *Sender);
    void __fastcall DateButton1Click(TObject *Sender);
    void __fastcall DateButton2Click(TObject *Sender);
    void __fastcall DateButton3Click(TObject *Sender);
    void __fastcall DateButton4Click(TObject *Sender);
    void __fastcall DateButton5Click(TObject *Sender);
    void __fastcall DateButton6Click(TObject *Sender);
    void __fastcall DateButton7Click(TObject *Sender);
    void __fastcall DateButton8Click(TObject *Sender);
    void __fastcall DateButton9Click(TObject *Sender);
    void __fastcall DateButton10Click(TObject *Sender);
    void __fastcall PatientSelectButtonClick(TObject *Sender);
    void __fastcall LocationListBoxChange(TObject *Sender);
    void __fastcall ReferringDrSelectButtonClick(TObject *Sender);
    void __fastcall FeeChargedEdit0Exit(TObject *Sender);
    void __fastcall FeeChargedEdit1Exit(TObject *Sender);
    void __fastcall FeeChargedEdit2Exit(TObject *Sender);
    void __fastcall FeeChargedEdit3Exit(TObject *Sender);
    void __fastcall FeeChargedEdit4Exit(TObject *Sender);
    void __fastcall FeeChargedEdit5Exit(TObject *Sender);
    void __fastcall FeeChargedEdit6Exit(TObject *Sender);
    void __fastcall FeeChargedEdit7Exit(TObject *Sender);
    void __fastcall FeeChargedEdit8Exit(TObject *Sender);
    void __fastcall ReferringDrTypeListBoxChange(TObject *Sender);
    void __fastcall ProviderListBoxChange(TObject *Sender);
    void __fastcall ReferringDrClearButtonClick(TObject *Sender);
    void __fastcall PatientViewButtonClick(TObject *Sender);
    void __fastcall PatientEditButtonClick(TObject *Sender);
    void __fastcall ReferringViewButtonClick(TObject *Sender);
    void __fastcall ReferringDrEditButtonClick(TObject *Sender);
    void __fastcall ExpLabel0Click(TObject *Sender);
    void __fastcall ExpLabel1Click(TObject *Sender);
    void __fastcall ExpLabel2Click(TObject *Sender);
    void __fastcall ExpLabel3Click(TObject *Sender);
    void __fastcall ExpLabel4Click(TObject *Sender);
    void __fastcall ExpLabel5Click(TObject *Sender);
    void __fastcall ExpLabel6Click(TObject *Sender);
    void __fastcall ExpLabel7Click(TObject *Sender);
    void __fastcall ExpLabel8Click(TObject *Sender);
    void __fastcall ClaimEditPopupPopup(TObject *Sender);
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
    void __fastcall ClaimEditPopupExpClick(TObject *Sender);
    void __fastcall ClaimEditPopupClearClick(TObject *Sender);
    void __fastcall ClaimEditPopupAcceptClick(TObject *Sender);
    void __fastcall ClaimEditPopupResubmitClick(TObject *Sender);
    void __fastcall LocationListBoxExit(TObject *Sender);
    void __fastcall StatusLabel0Click(TObject *Sender);
    void __fastcall StatusLabel1Click(TObject *Sender);
    void __fastcall StatusLabel2Click(TObject *Sender);
    void __fastcall StatusLabel3Click(TObject *Sender);
    void __fastcall StatusLabel4Click(TObject *Sender);
    void __fastcall StatusLabel5Click(TObject *Sender);
    void __fastcall StatusLabel6Click(TObject *Sender);
    void __fastcall StatusLabel7Click(TObject *Sender);
    void __fastcall StatusLabel8Click(TObject *Sender);
    void __fastcall PatientListButtonClick(TObject *Sender);
    void __fastcall DoctorListButtonClick(TObject *Sender);
    void __fastcall PremiumComboBox0Change(TObject *Sender);
    void __fastcall PremiumComboBox1Change(TObject *Sender);
    void __fastcall PremiumComboBox2Change(TObject *Sender);
    void __fastcall PremiumComboBox3Change(TObject *Sender);
    void __fastcall PremiumComboBox4Change(TObject *Sender);
    void __fastcall PremiumComboBox5Change(TObject *Sender);
    void __fastcall PremiumComboBox6Change(TObject *Sender);
    void __fastcall PremiumComboBox6Exit(TObject *Sender);
    void __fastcall PremiumComboBox5Exit(TObject *Sender);
    void __fastcall PremiumComboBox4Exit(TObject *Sender);
    void __fastcall PremiumComboBox3Exit(TObject *Sender);
    void __fastcall PremiumComboBox2Exit(TObject *Sender);
    void __fastcall PremiumComboBox1Exit(TObject *Sender);
    void __fastcall PremiumComboBox0Exit(TObject *Sender);
    void __fastcall PatientEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall IcdListButtonClick(TObject *Sender);
    void __fastcall FeeCodePopupPopup(TObject *Sender);

private:	// User declarations

    void __fastcall InitializeControls( void );
    void __fastcall ProgramControls( void );
    void __fastcall FeeListChange( Int32u_t index );
    void __fastcall FeeListExit( Int32u_t index);
    void __fastcall FeeChargedEditUpdate( Int32u_t index );
    void __fastcall FeeChargedEditExit( Int32u_t index );
    void __fastcall UnitsEditChange( Int32u_t index );
    void __fastcall UnitsEditExit( Int32u_t index );
    void __fastcall TotalFeeChargedEditUpdate( void );
    void __fastcall DateButtonClick( Int32u_t index );
    void __fastcall PatientUpdate( Int32u_t patientId );
    void __fastcall ProviderUpdate( Int32u_t providerId );
    void __fastcall ReferringDrUpdate( Int32u_t doctorId, Int32s_t typeIndex );
    void __fastcall ButtonClick( Int32u_t index );
    void __fastcall UnitsEditUpdate( Int32u_t index );
    void __fastcall DateEditUpdate( Int32u_t index );
    void __fastcall SubmitStatusUpdate( Int32u_t index );
    void __fastcall ClaimsValidate( void );
    void __fastcall ClaimsRead( void );
    void __fastcall PatientEditView( Int32u_t mode );
    void __fastcall ReferringDrEditView( Int32u_t mode );
    void __fastcall ExpLabelClick( Int32u_t index );
    void __fastcall StatusLabelMouseDown( int X, int Y, Int32u_t index );
    void __fastcall StatusLabelClick( Int32u_t index );
    void __fastcall PremiumComboBoxChange( Int32u_t index );
    void __fastcall PremiumComboBoxExit( Int32u_t index );
    void __fastcall ComputeFeeCharged( Int32u_t index );
    void __fastcall SetIcd( Char_p str_p );

    TComboBox   *FeeList[PMC_CLAIM_COUNT];
    TComboBox   *PremiumComboBox[PMC_CLAIM_COUNT];
    TEdit       *FeeChargedEdit[PMC_CLAIM_COUNT];
    TEdit       *UnitsEdit[PMC_CLAIM_COUNT];
    TLabel      *StatusLabel[PMC_CLAIM_COUNT];
    TEdit       *CommentEdit[PMC_CLAIM_COUNT];
    TLabel      *SubmitLabel[PMC_CLAIM_COUNT];
    TLabel      *ReplyLabel[PMC_CLAIM_COUNT];
    TLabel      *SeqLabel[PMC_CLAIM_COUNT];
    TLabel      *ExpLabel[PMC_CLAIM_COUNT];
    TLabel      *UnitsPaidLabel[PMC_CLAIM_COUNT];
    TLabel      *EditStatusLabel[PMC_CLAIM_COUNT];
    TLabel      *FeePaidLabel[PMC_CLAIM_COUNT];
    TLabel      *PremPaidLabel[PMC_CLAIM_COUNT];
    TLabel      *FeeCodeApprovedLabel[PMC_CLAIM_COUNT];
    TLabel      *MspCommentLabel[PMC_CLAIM_COUNT];

    TEdit       *DateEdit[PMC_CLAIM_COUNT + 2];
    TButton     *DateButton[PMC_CLAIM_COUNT + 2];

    Char_t      OrigFeeCode[PMC_CLAIM_COUNT][8];
    Char_t      ExpCode[PMC_CLAIM_COUNT][4];
    Char_t      FeeCodeApproved[PMC_CLAIM_COUNT][8];
    Int32u_t    FeeOverride[PMC_CLAIM_COUNT];
    Int32u_t    FeeCharged[PMC_CLAIM_COUNT];
    Int32u_t    FeeHigh[PMC_CLAIM_COUNT];
    Int32u_t    FeeLow[PMC_CLAIM_COUNT];
    Int32u_t    FeeDeterminant[PMC_CLAIM_COUNT];
    Int32u_t    FeeIndex[PMC_CLAIM_COUNT];
    Int32u_t    FeeMultiple[PMC_CLAIM_COUNT];
    Int32u_t    FeePaid[PMC_CLAIM_COUNT];
    Int32u_t    PremPaid[PMC_CLAIM_COUNT];
    Int32u_t    Units[PMC_CLAIM_COUNT];
    Int32u_t    ClaimId[PMC_CLAIM_COUNT];
    Int32u_t    Date[PMC_CLAIM_COUNT + 2];
    Int32u_t    SubmittedDate[PMC_CLAIM_COUNT];
    Int32u_t    ReplyDate[PMC_CLAIM_COUNT];
    Int32s_t    SeqNumber[PMC_CLAIM_COUNT];
    Int32u_t    SubmitCount[PMC_CLAIM_COUNT];
    Int32u_t    ClaimType[PMC_CLAIM_COUNT];
    Int32u_t    Status[PMC_CLAIM_COUNT];
    Int32u_t    StatusIn[PMC_CLAIM_COUNT];
    Int32u_t    ReplyStatus[PMC_CLAIM_COUNT];
    Int32u_t    Exists[PMC_CLAIM_COUNT];
    Int32u_t    UnitsPaid[PMC_CLAIM_COUNT];

    Int32u_t    ClaimHeaderLocked;
    Int32u_t    ClaimHeaderId;
    Int32u_t    TotalFeeCharged;
    Int32u_t    ProviderId;
    Int32u_t    NewProviderId;
    Int32u_t    ProviderNumber;
    Int32u_t    ProviderIndex;
    Int32u_t    PatientId;
    Int32u_t    ClaimNumber;
    Int32u_t    PatientDob;
    Int32u_t    LocationCodeIndex;
    Int32u_t    PremiumComboBoxIndex[PMC_CLAIM_COUNT];

    Int32u_t    ReferringDrTypeIndex;
    Int32u_t    ReferringDrId;
    Int32u_t    ReferringDrNumber;
    Int32u_t    ReferringDrType;

    Char_p      PatientPhnProv_p;
    Char_p      PatientPhn_p;

    Int32u_t    ReadyCount;
    Int32u_t    NotReadyCount;
    Int32u_t    SubmittedCount;
    Int32u_t    RejectedCount;
    Int32u_t    PaidCount;
    Int32u_t    ClaimCount;
    Int32u_t    NoneCount;
    Int32u_t    ReducedAcceptCount;
    Int32u_t    RejectedAcceptCount;
    Int32u_t    ReducedCount;

    Int32u_t    MouseDownIndex;
    int         MouseDownX;
    int         MouseDownY;

    Char_t      PatientTitle[16];

    bool        Active;
    bool        UpdateClaimStatus;
    
    pmcClaimEditInfo_p  ClaimEditInfo_p;

public:		// User declarations
    __fastcall TClaimEditForm(TComponent* Owner);
    __fastcall TClaimEditForm(TComponent* Owner, pmcClaimEditInfo_p claimEditInfo_p );
};

#define PMC_CLAIM_EDIT_POPUP_CLEAR          "ClaimEditPopupClear"
#define PMC_CLAIM_EDIT_POPUP_ACCEPT         "ClaimEditPopupAccept"
#define PMC_CLAIM_EDIT_POPUP_RESUBMIT       "ClaimEditPopupResubmit"
#define PMC_CLAIM_EDIT_POPUP_EXP            "ClaimEditPopupExp"

#define PMC_CLAIM_EDIT_MODE_EDIT    0
#define PMC_CLAIM_EDIT_MODE_VIEW    1
#define PMC_CLAIM_EDIT_MODE_NEW     2

#endif
