//---------------------------------------------------------------------------
// File:    pmcPatientListForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Form for displaying list of patients
//---------------------------------------------------------------------------

#ifndef pmcPatientListFormH
#define pmcPatientListFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Db.hpp>
#include <DBGrids.hpp>
#include <DBTables.hpp>
#include <Grids.hpp>
#include <Menus.hpp>

typedef struct PmcPatListInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    patientId;
    Int32u_t    providerId;
    Int32u_t    mode;
    Int32u_t    allowGoto;
    Int32u_t    gotoProviderId;
    Int32u_t    gotoDate;
    Int32u_t    character;
} PmcPatListInfo_t, *PmcPatListInfo_p;

//---------------------------------------------------------------------------

typedef struct pmcPatFormStruct_s
{
    Char_p      lastNameSearch_p;
    Char_p      lastName_p;
    Char_p      firstName_p;
    Char_p      homePhoneSearch_p;
    Char_p      homePhone_p;
    Char_p      phnSearch_p;
    Char_p      phn_p;
    Char_t      title[PMC_TITLE_LEN+1];
    Char_t      areaCode[PMC_AREA_CODE_LEN+1];
    Int32u_t    patientId;
} pmcPatFormStruct_t, *pmcPatFormStruct_p;

//---------------------------------------------------------------------------

typedef struct pmcPatFormOffset_s
{
    Int32u_t    name;
    Int32u_t    phone;
    Int32u_t    phn;
    Int32u_t    current;
} pmcPatFormOffset_t, *pmcPatFormOffset_p;

//---------------------------------------------------------------------------

class TPatientListForm : public TForm
{
__published:	// IDE-managed Components
    TButton *NewPatientButton;
    TButton *ShowAllButton;
    TEdit *SearchEdit;
    TStringGrid *PatientStringGrid;
    TButton *OkButton;
    TGroupBox *GroupBox1;
    TBevel *Bevel1;
    TBevel *Bevel5;
    TButton *CancelButton;
    TLabel *LastNameLabel;
    TLabel *PhnLabel;
    TPopupMenu *PatientGridPopup;
    TMenuItem *PatListPopupView;
    TMenuItem *PatListPopupEdit;
    TMenuItem *PatListPopupDelete;
    TMenuItem *PatListPopupPrint;
    TMenuItem *PatListPopupListApp;
    TMenuItem *ViewRecords1;
    TMenuItem *PatListPopupClaimsNew;
    TMenuItem *PatListPopupClaimsList;
    TMenuItem *N1;
    TMenuItem *N2;
    TMenuItem *N3;
    TMenuItem *N4;
    TButton *RefreshButton;
    TMenuItem *PatListPopupPrintAddrLabel;
    TMenuItem *PatListPopupPrintRecord;
    TMenuItem *PatListPopupPrintReqLabel;
    TMenuItem *PatListPopupListDoc;
    TGroupBox *GroupBox2;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *RecordsTotalLabel;
    TLabel *RecordsShownLabel;
    TMenuItem *PatListPopupEchoList;
    TButton *ButtonClear;
    void __fastcall PatientListGridCellClick(TColumn *Column);
    void __fastcall ShowAllButtonClick(TObject *Sender);
    void __fastcall PatientStringGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall PatientStringGridClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall SearchEditChange(TObject *Sender);
    void __fastcall NewPatientButtonClick(TObject *Sender);
    void __fastcall PatientGridPopupPopup(TObject *Sender);
    void __fastcall PatientStringGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PatListPopupViewClick(TObject *Sender);
    void __fastcall RefreshButtonClick(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall PatListPopupEditClick(TObject *Sender);
    void __fastcall DeleteClick(TObject *Sender);
    void __fastcall PatListPopupListAppClick(TObject *Sender);
    void __fastcall PatListPopupPrintRecordClick(TObject *Sender);
    void __fastcall PatListPopupClaimsNewClick(TObject *Sender);
    void __fastcall PatListPopupClaimsListClick(TObject *Sender);
    void __fastcall PatListPopupPrintAddrLabelClick(TObject *Sender);
    void __fastcall PatListPopupPrintReqLabelClick(TObject *Sender);
    void __fastcall PatientStringGridDblClick(TObject *Sender);
    void __fastcall PatListPopupListDocClick(TObject *Sender);
    void __fastcall PatientStringGridMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PatListPopupEchoListClick(TObject *Sender);
    void __fastcall SearchEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall PatientStringGridKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall ButtonClearClick(TObject *Sender);

private:	// User declarations

    void __fastcall BuildCurrentList( Ints_t index, Char_p searchString_p );
    void __fastcall BuildMasterList( );
    void __fastcall FreeMasterList( void );
    void __fastcall UpdateSelectedPatient( void );
    void __fastcall PatListEditPatient( void );
    void __fastcall ClearPatient( void );
    void __fastcall CreateNewPatient( void );

    // Pointer to array of patient list structures
    pmcPatFormStruct_p PatForm_p;

    // Pointer to array of patient list struct offsets (sorted by name, phone, phn);
    pmcPatFormOffset_p PatFormOffset_p;

    Int32s_t    TotalPatientRecords;
    Int32s_t    CurrentPatientRecords;
    Ints_t      MouseInRow;
    Ints_t      MouseInCol;
    Int32u_t    SelectedPatientId;
    Int32u_t    Ready;
    Boolean_t   UpdateSelectedPatientIdFlag;
    Boolean_t   SkipCurrentListBuild;
    Boolean_t   DoubleClickFlag;
    Boolean_t   ClearFlag;
    Int32u_t    SortIndex;

    // Store whatever was typed for the search string here.  We may be able
    // to use it when the new patient button is clicked; e.g., to set PHN
    Char_t      SearchString[1024];

    // Pointer to structure to return info to calling code
    PmcPatListInfo_p    PatListInfo_p;

public:		// User declarations
    __fastcall TPatientListForm(TComponent* Owner);
    __fastcall TPatientListForm(TComponent* Owner, PmcPatListInfo_p patListInfo_p );
    __fastcall ~TPatientListForm( void );
};

#define PMC_CURRENT_LIST_NAME       0
#define PMC_CURRENT_LIST_PHONE      1
#define PMC_CURRENT_LIST_PHN        2

#define PMC_PAT_LIST_POPUP_VIEW             "PatListPopupView"
#define PMC_PAT_LIST_POPUP_EDIT             "PatListPopupEdit"
#define PMC_PAT_LIST_POPUP_DELETE           "PatListPopupDelete"
#define PMC_PAT_LIST_POPUP_LIST_APP         "PatListPopupListApp"
#define PMC_PAT_LIST_POPUP_LIST_DOC         "PatListPopupListDoc"
#define PMC_PAT_LIST_POPUP_CLAIMS_NEW       "PatListPopupClaimsNew"
#define PMC_PAT_LIST_POPUP_CLAIMS_LIST      "PatListPopupClaimsList"
#define PMC_PAT_LIST_POPUP_ECHO_LIST        "PatListPopupEchoList"

#define PMC_PAT_LIST_POPUP_PRINT            "PatListPopupPrint"
#define PMC_PAT_LIST_POPUP_PRINT_RECORD     "PatListPopupPrintRecord"
#define PMC_PAT_LIST_POPUP_PRINT_LBL_ADDR   "PatListPopupPrintAddrLabel"
#define PMC_PAT_LIST_POPUP_PRINT_LBL_REQ    "PatListPopupPrintReqLabel"

#endif
