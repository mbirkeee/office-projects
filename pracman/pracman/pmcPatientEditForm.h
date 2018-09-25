//---------------------------------------------------------------------------
// pmcPatientEditForm.h
//---------------------------------------------------------------------------
// (c) 2001-2008 Michael A. Bree
//---------------------------------------------------------------------------
// Include file for patient edit form
//---------------------------------------------------------------------------

#ifndef pmcPatientEditFormH
#define pmcPatientEditFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>

#include "pmcPatient.h"
#include "pmcDocument.h"

#define PMC_CONSULT_TYPE_PICKLIST_INDEX_CONSULT     0
#define PMC_CONSULT_TYPE_PICKLIST_INDEX_FOLLOWUP    1

//---------------------------------------------------------------------------
// For list of programatically added sections
//---------------------------------------------------------------------------
typedef struct pmcPatEditSection_s
{
    qLinkage_t      linkage;
    Int32u_t        type;
    TTabSheet      *tabSheet_p;
    TMemo          *memo_p;
    TButton        *buttonHelp_p;
    TSpeedButton   *buttonPrev_p;
    TSpeedButton   *buttonNext_p;
    TMaskEdit      *updateEdit_p;
    TLabel         *updateLabel_p;
    qHead_t         itemQueue;
    qHead_p         item_q;
    mbStrList_p     first_p;
    mbStrList_p     last_p;
    mbStrList_p     current_p;
    Char_p          orig_p;
    Char_p          subKey_p;
    Boolean_t       modified;
} pmcPatEditSection_t, *pmcPatEditSection_p;

//---------------------------------------------------------------------------
// Used to pass parameters to the patient edit form
//---------------------------------------------------------------------------

typedef struct pmcPatEditInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    mode;
    Int32u_t    patientId;
    Int32u_t    providerId;
    Char_t      caption[256];
    Char_t      searchString[1024];
} pmcPatEditInfo_t, *pmcPatEditInfo_p;


//---------------------------------------------------------------------------
// Patient Edit Form class
//---------------------------------------------------------------------------
class TPatientEditForm : public TForm
{
__published:	// IDE-managed Components
    TButton *OkButton;
    TButton *CancelButton;
    TBevel *Bevel1;
    TLabel *LabelPatientName;
    TBevel *Bevel2;
    TLabel *LabelPHN;
    TSpeedButton *HiddenButtonPrev;
    TSpeedButton *HiddenButtonNext;
    TButton *SaveButton;
    TPageControl *PageControl;
    TTabSheet *TabSheetDetails;
    TLabel *Label1;
    TLabel *Label3;
    TLabel *Label2;
    TLabel *Label9;
    TLabel *Label22;
    TLabel *Label8;
    TLabel *Label5;
    TLabel *Label7;
    TLabel *Label10;
    TLabel *Label16;
    TLabel *Label17;
    TLabel *Label28;
    TLabel *Label25;
    TLabel *Label21;
    TLabel *Label26;
    TLabel *Label24;
    TLabel *Label23;
    TLabel *Label20;
    TLabel *Label19;
    TLabel *Label14;
    TLabel *Label18;
    TLabel *Label15;
    TLabel *Label6;
    TLabel *Label11;
    TLabel *Label12;
    TLabel *Label13;
    TLabel *Label27;
    TLabel *Label4;
    TLabel *Label29;
    TMaskEdit *ReferringDrEdit;
    TMaskEdit *FamilyDrEdit;
    TMaskEdit *DateOfBirthEdit;
    TMaskEdit *LastNameEdit;
    TMaskEdit *FirstNameEdit;
    TMaskEdit *MiddleNameEdit;
    TMaskEdit *Address1Edit;
    TMaskEdit *Address2Edit;
    TComboBox *ProviderList;
    TBitBtn *BirthDateButton;
    TMaskEdit *DeceasedDateEdit;
    TMaskEdit *EmailEdit;
    TMaskEdit *HomePhoneEdit;
    TCheckBox *PreferEmailCheck;
    TMaskEdit *WorkPhoneEdit;
    TMaskEdit *WorkDescEdit;
    TMaskEdit *ContactNameEdit;
    TMaskEdit *ContactPhoneEdit;
    TMaskEdit *CommentEdit;
    TBitBtn *ReferringDrButton;
    TBitBtn *FamilyDrButton;
    TMaskEdit *IdEdit;
    TMaskEdit *ContactDescEdit;
    TComboBox *TitleList;
    TRadioGroup *GenderRadio;
    TComboBox *ProvList;
    TMaskEdit *PostalCodeEdit;
    TComboBox *PhnProvList;
    TBitBtn *DeceasedDateButton;
    TMaskEdit *CountryEdit;
    TMaskEdit *PhnEdit;
    TMaskEdit *ChartEdit;
    TMaskEdit *CreatedEdit;
    TMaskEdit *ModifiedEdit;
    TMaskEdit *CellPhoneEdit;
    TComboBox *ComboBoxCity;
    TMaskEdit *AgeEdit;
    TLabel *Label30;
    TGroupBox *GroupBoxConsult;
    TGroupBox *GroupBoxPrint;
    TButton *PrintRecordNewButton;
    TButton *PrintAddressLabelButton;
    TButton *PrintReqLabelButton;
    TGroupBox *GroupBoxList;
    TButton *DocumentsButton;
    TButton *AppointmentsButton;
    TButton *ClaimsButton;
    TButton *EchosButton;
    TGroupBox *GroupBoxTools;
    TButton *NewWordDocButton;
    TButton *NewClaimButton;
    TButton *SubmitConsultButton;
    TButton *ApproveConsultButton;
    TButton *LastConsultButton;
    TEdit *ConsultDrEdit;
    TMaskEdit *ConsultLetterDateEdit;
    TMaskEdit *ConsultAppDateEdit;
    TLabel *Label31;
    TLabel *Label32;
    TLabel *Label33;
    TMaskEdit *ConsultStatusEdit;
    TLabel *Label34;
    TButton *NewConsultButton;
    TButton *PreviewConsultButton;
    TBitBtn *ConsultDrButton;
    TBitBtn *ConsultLetterDateButton;
    TBitBtn *ConsultAppDateButton;
    TLabel *Label35;
    TComboBox *ConsultTypeComboBox;
    
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall BirthDateButtonClick(TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall ProviderListChange(TObject *Sender);
    void __fastcall DeceasedDateButtonClick(TObject *Sender);
    void __fastcall PrintLabelClick(TObject *Sender);
    void __fastcall FamilyDrButtonClick(TObject *Sender);
    void __fastcall ReferringDrButtonClick(TObject *Sender);
    void __fastcall PrintRecordButtonClick(TObject *Sender);
    void __fastcall PrintReqLabelButtonClick(TObject *Sender);
    void __fastcall PrintAddressLabelButtonClick(TObject *Sender);
    void __fastcall TitleListChange(TObject *Sender);
    void __fastcall DateOfBirthEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall DeceasedDateEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ReferringDrEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FamilyDrEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall LastNameEditChange(TObject *Sender);
    void __fastcall FirstNameEditChange(TObject *Sender);
    void __fastcall PhnEditChange(TObject *Sender);
    void __fastcall PhnProvListChange(TObject *Sender);
    void __fastcall PageControlChange(TObject *Sender);
    void __fastcall Memo_medicationsChange(TObject *Sender);
    void __fastcall PrintRecordNewButtonClick(TObject *Sender);
    void __fastcall GenderRadioClick(TObject *Sender);
    void __fastcall SaveButtonClick(TObject *Sender);
    void __fastcall MiddleNameEditChange(TObject *Sender);
    void __fastcall Address1EditChange(TObject *Sender);
    void __fastcall Address2EditChange(TObject *Sender);
    void __fastcall ComboBoxCityChange(TObject *Sender);
    void __fastcall ProvListChange(TObject *Sender);
    void __fastcall PostalCodeEditChange(TObject *Sender);
    void __fastcall CountryEditChange(TObject *Sender);
    void __fastcall HomePhoneEditChange(TObject *Sender);
    void __fastcall HomePhoneEditExit(TObject *Sender);
    void __fastcall WorkPhoneEditChange(TObject *Sender);
    void __fastcall WorkPhoneEditExit(TObject *Sender);
    void __fastcall CellPhoneEditChange(TObject *Sender);
    void __fastcall CellPhoneEditExit(TObject *Sender);
    void __fastcall EmailEditChange(TObject *Sender);
    void __fastcall WorkDescEditChange(TObject *Sender);
    void __fastcall ContactNameEditChange(TObject *Sender);
    void __fastcall ContactPhoneEditChange(TObject *Sender);
    void __fastcall ContactDescEditChange(TObject *Sender);
    void __fastcall CommentEditChange(TObject *Sender);
    void __fastcall PhnEditExit(TObject *Sender);
    void __fastcall PhnProvListExit(TObject *Sender);
    void __fastcall PreviewConsultButtonClick(TObject *Sender);
    void __fastcall DocumentsButtonClick(TObject *Sender);
    void __fastcall LastConsultButtonClick(TObject *Sender);
    void __fastcall AppointmentsButtonClick(TObject *Sender);
    void __fastcall NewWordDocButtonClick(TObject *Sender);
    void __fastcall ClaimsButtonClick(TObject *Sender);
    void __fastcall EchosButtonClick(TObject *Sender);
    void __fastcall PostalCodeEditExit(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall NewConsultButtonClick(TObject *Sender);
    void __fastcall NewClaimButtonClick(TObject *Sender);
    void __fastcall SubmitConsultButtonClick(TObject *Sender);
    void __fastcall FamilyDrEditChange(TObject *Sender);
    void __fastcall ReferringDrEditChange(TObject *Sender);
    void __fastcall ConsultDrButtonClick(TObject *Sender);
    void __fastcall ConsultLetterDateButtonClick(TObject *Sender);
    void __fastcall ConsultAppDateButtonClick(TObject *Sender);
    void __fastcall ConsultAppDateEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ConsultLetterDateEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ConsultDrEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PageControlResize(TObject *Sender);
    void __fastcall ApproveConsultButtonClick(TObject *Sender);
    void __fastcall ConsultTypeComboBoxChange(TObject *Sender);
    void __fastcall DateOfBirthEditClick(TObject *Sender);
    void __fastcall DateOfBirthEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall DeceasedDateEditClick(TObject *Sender);
    void __fastcall DeceasedDateEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
 
private:	// User declarations

    // Patient object
    PmcPatient         *Patient_p;
    PmcDocument        *Consult_p;
    Boolean_t           ForceChangeFlag;

    qHead_t             sectionQueue;
    qHead_p             section_q;
    pmcPatEditSection_p ActiveSection_p;
    pmcPatEditInfo_p    PatEditInfo_p;

    Int32u_t            obtainedMode;

    Int32u_t            consultDrId;
    Int32u_t            consultDateLetter;
    Int32u_t            consultDateApp;

    Boolean_t   __fastcall SectionsModifiedCheck( void );
    void        __fastcall SectionDisplayUpdate( pmcPatEditSection_p section_p );
    void        __fastcall SectionsSave( void );
    void        __fastcall SectionButtonHelpClick( TObject *Sender );
    void        __fastcall SectionButtonPrevClick( TObject *Sender );
    void        __fastcall SectionButtonNextClick( TObject *Sender );
    void        __fastcall SectionMemoChange(TObject *Sender);
    void        __fastcall MemoKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void        __fastcall ControlsUpdate( void );
    void        __fastcall BannerUpdate( void );
    void        __fastcall BuildProviderList( void );
    void        __fastcall UpdateDoctorEdit( Int32u_t doctorId, Int32u_t fieldFlag );
    void        __fastcall ConsultControlsUpdate( void );
    void        __fastcall MemoSize( TMemo *memo_p );

public:		// User declarations

    void        __fastcall PrintDocument( Char_p template_p );

    __fastcall  TPatientEditForm( TComponent* Owner);
    __fastcall  TPatientEditForm( TComponent* Owner, pmcPatEditInfo_p patInfo_p );
    __fastcall ~TPatientEditForm( void );
};

//---------------------------------------------------------------------------

#define PMC_PATEDIT_REF_DR          1
#define PMC_PATEDIT_FAM_DR          2
#define PMC_PATEDIT_CONSULT_DR      3

#endif
