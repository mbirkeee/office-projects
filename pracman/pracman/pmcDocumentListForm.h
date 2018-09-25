//---------------------------------------------------------------------------
// File:    pmcDocumentListForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    August 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcDocumentListFormH
#define pmcDocumentListFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Graphics.hpp>

#include "pmcDocumentUtils.h"

//---------------------------------------------------------------------------

typedef struct pmcDocumentListInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    patientId;
    Int32u_t    providerId;
} pmcDocumentListInfo_t, *pmcDocumentListInfo_p;

class TDocumentListForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CancelButton;
    TListView *DocListView;
    TPopupMenu *DocListPopup;
    TMenuItem *DocListPopupEdit;
    TMenuItem *DocListPopupExtDel;
    TMenuItem *DocListPopupView;
    TButton *ImportButton;
    TLabel *Label2;
    TButton *NewWordDocumentButton;
    TGroupBox *GroupBox_SelectPatient;
    TEdit *PatientEdit;
    TBitBtn *PatientListButton;
    TBevel *Bevel2;
    TLabel *Label1;
    TLabel *PhnLabel;
    TSaveDialog *SaveDialog;
    TMenuItem *N1;
    TMenuItem *DocListPopupDelete;
    TMenuItem *DocListPopupExtract;
    TMenuItem *DocListPopupListPat;
    TMenuItem *N2;
    TGroupBox *GroupBox_ShowDocuments;
    TCheckBox *CheckBox_ShowFiled;
    TCheckBox *CheckBox_ShowPending;
    TCheckBox *CheckBox_ShowActive;
    TCheckBox *CheckBox_ShowUnknown;
    TImage *Image1;
    TMenuItem *DocListPopupListAll;
    TMenuItem *DocListPopupSubmit;
    TMenuItem *DocListPopupApprove;
    TMenuItem *DocListPopupRevise;
    TMenuItem *N3;
    TMenuItem *DocListPopupEditDoc;
    TMenuItem *N4;
    TMenuItem *DocListPopupListPatApp;
    TMenuItem *PatListPopupPatListEchos;
    TImage *Image2;
    TImage *Image3;
    TImage *Image4;
    TButton *ButtonPatNone;
    TButton *ButtonPatAll;
    TButton *Button_BatchImport;
    TButton *Button_ScannedImport;
    TMenuItem *N5;
    TMenuItem *DocListPopupUnlock;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall PatientListButtonClick(TObject *Sender);
    void __fastcall PatientEditMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall DocListViewDblClick(TObject *Sender);
    void __fastcall ImportButtonClick(TObject *Sender);
    void __fastcall NewWordDocumentButtonClick(TObject *Sender);
    void __fastcall PatientEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall DocListViewColumnClick(TObject *Sender,
          TListColumn *Column);

    void __fastcall DocListPopupPopup(TObject *Sender);
    void __fastcall DocListPopupEditClick(TObject *Sender);
    void __fastcall DocListPopupViewClick(TObject *Sender);
    void __fastcall DocListPopupDeleteClick(TObject *Sender);
    void __fastcall DocListPopupExtDelClick(TObject *Sender);
    void __fastcall DocListPopupExtractClick(TObject *Sender);
    void __fastcall DocListViewSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
    void __fastcall DocListViewData(TObject *Sender, TListItem *Item);
    void __fastcall DocListPopupListPatClick(TObject *Sender);
    void __fastcall DocListPopupListAllClick(TObject *Sender);
    void __fastcall CheckBox_ShowFiledClick(TObject *Sender);
    void __fastcall CheckBox_ShowPendingClick(TObject *Sender);
    void __fastcall CheckBox_ShowActiveClick(TObject *Sender);
    void __fastcall CheckBox_ShowUnknownClick(TObject *Sender);
    void __fastcall DocListPopupListPatAppClick(TObject *Sender);
    void __fastcall ButtonPatNoneClick(TObject *Sender);
    void __fastcall ButtonPatAllClick(TObject *Sender);
    void __fastcall DocListPopupEditDocClick(TObject *Sender);
    void __fastcall DocListPopupSubmitClick(TObject *Sender);
    void __fastcall DocListPopupReviseClick(TObject *Sender);
    void __fastcall DocListPopupApproveClick(TObject *Sender);
    void __fastcall Button_BatchImportClick(TObject *Sender);
    void __fastcall Button_ScannedImportClick(TObject *Sender);
    void __fastcall DocListPopupUnlockClick(TObject *Sender);

private:	// User declarations

    void __fastcall UpdatePatient( Int32u_t    patientId );
    void __fastcall UpdateList( Boolean_t topFlag );
    void __fastcall ListFree( void );
    void __fastcall PatientList( Int32u_t key );
    void __fastcall SortList( void );

#if 0
    bool __fastcall SortDate
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcDocumentListStruct_p     doc_p,
        pmcDocumentListStruct_p     tempDoc_p,
        Int32u_t                    nameSort
    );
    bool __fastcall SortName
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcDocumentListStruct_p     doc_p,
        pmcDocumentListStruct_p     tempDoc_p,
        Int32u_t                    dateSort
    );
    bool __fastcall SortDesc
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcDocumentListStruct_p     doc_p,
        pmcDocumentListStruct_p     tempDoc_p,
        Int32u_t                    dateSort
    );
    bool __fastcall SortProv
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcDocumentListStruct_p     doc_p,
        pmcDocumentListStruct_p     tempDoc_p,
        Int32u_t                    dateSort
    );
    bool __fastcall SortType
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcDocumentListStruct_p     doc_p,
        pmcDocumentListStruct_p     tempDoc_p,
        Int32u_t                    dateSort
    );
    bool __fastcall SortId
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcDocumentListStruct_p     doc_p,
        pmcDocumentListStruct_p     tempDoc_p,
        Int32u_t                    dateSort
    );
#endif


    void __fastcall DeleteExtract( Int32u_t mode );

    pmcDocumentListInfo_p FormInfo_p;

    Int32u_t    SortListActive;
    Int32u_t    Active;

    Int32u_t    TopId;
    Int32u_t    SelectedId;
    Int32u_t    ImportedId;
#if 0
    Int32u_t    SortMode;
    Int32u_t    SortDateMode;
    Int32u_t    SortNameMode;
    Int32u_t    SortDescMode;
    Int32u_t    SortProvMode;
    Int32u_t    SortTypeMode;
    Int32u_t    SortIdMode;
#endif

    pmcDocumentItem_p  *DocumentMaster_pp;
    pmcDocumentItem_p  *Document_pp;

    Int32u_t            ItemCountMaster;
    Int32u_t            ItemCount;
    Boolean_t           ClearingListView;
    Boolean_t           NoPatientFlag;

public:		// User declarations
    __fastcall TDocumentListForm(TComponent* Owner);
    __fastcall TDocumentListForm(TComponent* Owner, pmcDocumentListInfo_p formInfo_p );
};

#define PMC_DOC_LIST_POPUP_EDIT             "DocListPopupEdit"
#define PMC_DOC_LIST_POPUP_EXTRACT          "DocListPopupExtract"
#define PMC_DOC_LIST_POPUP_EXTRACT_DELETE   "DocListPopupExtDel"
#define PMC_DOC_LIST_POPUP_DELETE           "DocListPopupDelete"
#define PMC_DOC_LIST_POPUP_VIEW             "DocListPopupView"
#define PMC_DOC_LIST_POPUP_LIST_PAT         "DocListPopupListPat"
#define PMC_DOC_LIST_POPUP_LIST_ALL         "DocListPopupListAll"
#define PMC_DOC_LIST_POPUP_LIST_PAT_APP     "DocListPopupListPatApp"
#define PMC_DOC_LIST_POPUP_EDIT_DOCUMENT    "DocListPopupEditDoc"
#define PMC_DOC_LIST_POPUP_SUBMIT           "DocListPopupSubmit"
#define PMC_DOC_LIST_POPUP_REVISE           "DocListPopupRevise"
#define PMC_DOC_LIST_POPUP_APPROVE          "DocListPopupApprove"
#define PMC_DOC_LIST_POPUP_UNLOCK           "DocListPopupUnlock"

#define PMC_DOC_LIST_MODE_DELETE    0x00000001
#define PMC_DOC_LIST_MODE_EXTRACT   0x00000002


#endif
