//---------------------------------------------------------------------------
// File:    pmcDoctorListForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcDoctorListFormH
#define pmcDoctorListFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Menus.hpp>

#include "mbTypes.h"

typedef struct pmcDocListInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    doctorId;
    Int32u_t    mode;
} pmcDocListInfo_t, *pmcDocListInfo_p;

typedef struct pmcDocFormStruct_s
{
    Char_p      lastNameSearch_p;
    Char_p      lastName_p;
    Char_p      firstName_p;
    Char_p      workPhoneSearch_p;
    Char_p      workPhone_p;
    Char_p      workFax_p;
    Char_p      num_p;
    Char_t      workAreaCode[PMC_AREA_CODE_LEN+1];
    Char_t      faxAreaCode[PMC_AREA_CODE_LEN+1];
    Int32u_t    doctorId;
} pmcDocFormStruct_t, *pmcDocFormStruct_p;

typedef struct pmcDocFormOffset_s
{
    Int32u_t    name;
    Int32u_t    phone;
    Int32u_t    num;
    Int32u_t    current;
} pmcDocFormOffset_t, *pmcDocFormOffset_p;

//---------------------------------------------------------------------------
class TDoctorListForm : public TForm
{
__published:	// IDE-managed Components
    TStringGrid *DoctorStringGrid;
    TButton *OkButton;
    TButton *CancelButton;
    TBevel *Bevel2;
    TBevel *Bevel3;
    TLabel *DoctorNameLabel;
    TLabel *DoctorNumberLabel;
    TGroupBox *SearchForBox;
    TButton *NewDoctorButton;
    TButton *ShowAllButton;
    TButton *RefreshButton;
    TEdit *SearchEdit;
    TPopupMenu *DoctorGridPopup;
    TMenuItem *DrListPopupEdit;
    TMenuItem *DrListPopupDelete;
    TMenuItem *ViewReferredPatients1;
    TMenuItem *DrListPopupView;
    TMenuItem *DrListPopupPrintAddressLabel;
    TButton *ClearButton;
    TGroupBox *GroupBox2;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *RecordsTotalLabel;
    TLabel *RecordsShownLabel;
    void __fastcall DoctorStringGridClick(TObject *Sender);
    void __fastcall DoctorStringGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall DoctorStringGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ShowAllButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall FormActivate(TObject *Sender);
    void __fastcall SearchEditChange(TObject *Sender);
    void __fastcall NewDoctorButtonClick(TObject *Sender);
    void __fastcall DoctorGridPopupPopup(TObject *Sender);
    void __fastcall RefreshButtonClick(TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall DrListPopupEditClick(TObject *Sender);
    void __fastcall DrListPopupViewClick(TObject *Sender);
    void __fastcall DrListPopupDeleteClick(TObject *Sender);
    void __fastcall DrListPopupPrintAddressLabelClick(TObject *Sender);
    void __fastcall DoctorStringGridDblClick(TObject *Sender);
    void __fastcall ClearButtonClick(TObject *Sender);
    void __fastcall DoctorStringGridMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall SearchEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall DoctorStringGridKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);

private:	// User declarations

    void __fastcall BuildCurrentList( Ints_t index, Char_p searchString_p );
    void __fastcall BuildMasterList( );
    void __fastcall FreeMasterList( void );
    void __fastcall UpdateSelectedDoctor( void );
    void __fastcall DoctorEdit( void );
    
    // Pointer to array of doctor list structures
    pmcDocFormStruct_p DocForm_p;

    // Pointer to array of doctor list struct offsets (sorted by name, phone, num);
    pmcDocFormOffset_p DocFormOffset_p;

    Int32s_t    TotalDoctorRecords;
    Int32s_t    CurrentDoctorRecords;
    Ints_t      MouseInRow;
    Ints_t      MouseInCol;
    Int32u_t    SelectedDoctorId;
    Int32u_t    Ready;
    bool        UpdateSelectedDoctorIdFlag;
    bool        SkipCurrentListBuild;
    bool        ClearFlag;
    bool        DoubleClickFlag;
    Int32u_t    SortIndex;
        
    // Pointer to structure to return info to calling code
    pmcDocListInfo_p    FormInfo_p;

public:		// User declarations
    __fastcall TDoctorListForm(TComponent* Owner);
    __fastcall TDoctorListForm::TDoctorListForm
    (
        TComponent*         Owner,
        pmcDocListInfo_p    docListInfo_p
    );
    __fastcall ~TDoctorListForm( void );
};

#define PMC_DR_LIST_POPUP_VIEW             "DrListPopupView"
#define PMC_DR_LIST_POPUP_EDIT             "DrListPopupEdit"
#define PMC_DR_LIST_POPUP_DELETE           "DrListPopupDelete"
#define PMC_DR_LIST_POPUP_PRINT_LBL_ADDR   "DrListPopupPrintAddressLabel"

#endif
