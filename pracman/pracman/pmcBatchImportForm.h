//---------------------------------------------------------------------------
// File:    pmcBatchImportForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    August 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Imports documents into the database, either one at a time or in
// batch mode
//---------------------------------------------------------------------------

#ifndef pmcBatchImportFormH
#define pmcBatchImportFormH

#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcDocumentEditForm.h"

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <FileCtrl.hpp>
#include "cdiroutl.h"
#include <Grids.hpp>
#include <Outline.hpp>
#include <Buttons.hpp>

//---------------------------------------------------------------------------

typedef struct pmcBatchImportInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    providerId;
    Int32u_t    mode;
} pmcBatchImportInfoo_t, *pmcBatchImportInfoo_p;


class TBatchImportForm : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TButton *CancelButton;
    TButton *StartButton;
    TListView *ListViewIn;
    TPageControl *PageControl1;
    TTabSheet *SuccessTabSheet;
    TTabSheet *FailedTabSheet;
    TListView *ListViewSucceeded;
    TGroupBox *GroupBox1;
    TComboBox *DocumentTypeComboBox;
    TListView *ListViewFailed;
    TRadioGroup *PhnRadioGroup;
    TGroupBox *GroupBox2;
    TCheckBox *FailDateCheckBox;
    TCheckBox *FailProviderCheckBox;
    TCheckBox *FailPhnCheckBox;
    TRadioGroup *PromptRadioGroup;
    TRadioGroup *ProviderRadioGroup;
    TRadioGroup *DateRadioGroup;
    TEdit *DateEdit;
    TGroupBox *GroupBox3;
    TEdit *ImportEdit;
    TEdit *FailedEdit;
    TLabel *Label1;
    TLabel *Label2;
    TCheckBox *MoveFailedCheckBox;
    TBitBtn *DateSelectButton;
    TBitBtn *ImportDirSelectButton;
    TBitBtn *FailedDirSelectButton;
    TComboBox *ProviderComboBox;
    TGroupBox *GroupBox4;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *TotalCountLabel;
    TLabel *ImportedCountLabel;
    TLabel *FailedCountLabel;
    TLabel *RemainingCountLabel;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall StartButtonClick(TObject *Sender);
    void __fastcall ListViewInDblClick(TObject *Sender);
    void __fastcall DocumentTypeComboBoxChange(TObject *Sender);
    void __fastcall DateSelectButtonClick(TObject *Sender);
    void __fastcall ImportDirSelectButtonClick(TObject *Sender);
    void __fastcall FailedDirSelectButtonClick(TObject *Sender);
    void __fastcall ImportEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall FailedEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ProviderComboBoxChange(TObject *Sender);
    void __fastcall DateEditMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall MoveFailedCheckBoxClick(TObject *Sender);
    void __fastcall ListViewSucceededDblClick(TObject *Sender);
    void __fastcall ListViewFailedDblClick(TObject *Sender);


private:	// User declarations

    void __fastcall UpdateFileListIn( void );
    void __fastcall UpdateCounts( void );
    void __fastcall UpdateDirectory( Char_p dir_p );

    pmcBatchImportInfoo_p BatchInfo_p;
    Int32s_t    DocumentTypeComboBoxIndex;

    qHead_t     FileListInHead;
    qHead_p     FileListIn_q ;
    qHead_t     FileListDoneHead;
    qHead_p     FileListDone_q;

    bool        Initialized;

    Int32u_t    TotalCount;
    Int32u_t    ImportedCount;
    Int32u_t    FailedCount;
    Int32u_t    RemainingCount;

    Char_p      ImportDir_p;
    Char_p      ImportFailedDir_p;
    Int32u_t    ProviderIndex;
    Int32u_t    Date;

public:		// User declarations
    __fastcall TBatchImportForm(TComponent* Owner);
    __fastcall TBatchImportForm(TComponent* Owner, pmcBatchImportInfoo_p  importInfo_p );
};

enum
{
     PMC_IMPORT_DIALOG_ALWAYS = 0
    ,PMC_IMPORT_DIALOG_IF_REQUIRED
    ,PMC_IMPORT_DIALOG_NEVER
};

#define PMC_IMPORT_MODE_DOCS            0
#define PMC_IMPORT_MODE_SCANS           1

#define PMC_IMPORT_PHN_DOCUMENT         0
#define PMC_IMPORT_PHN_FILENAME         1
#define PMC_IMPORT_PHN_BOTH             2
#define PMC_IMPORT_PHN_NONE             3

#define PMC_IMPORT_PROVIDER_DETERMINE   0
#define PMC_IMPORT_PROVIDER_SELECTED    1
#define PMC_IMPORT_PROVIDER_NONE        2

#define PMC_IMPORT_DATE_DETERMINE       0
#define PMC_IMPORT_DATE_SELECTED        1
#define PMC_IMPORT_DATE_NONE            2

#define PMC_IMPORT_FAIL_BIT_PHN         0x00000001
#define PMC_IMPORT_FAIL_BIT_DATE        0x00000002
#define PMC_IMPORT_FAIL_BIT_PROVIDER    0x00000004

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t pmcDocProcess
(
    mbFileListStruct_p      file_p,
    Int32u_p                documentId_p,
    Int32u_t                dialogMode,
    Int32u_t                providerMode,
    Int32u_t                phnMode,
    Int32u_t                dateMode,
    Int32u_t                moveFailedFlag,
    Int32u_t                providerId,
    Int32u_t                patientIdIn,
    Int32u_t                date,
    Int32u_t                failMask,
    Int32u_p                autoTerminateFlag_p,
    Int32u_p                formDisplayedFlag_p,
    Char_p                  failedDir_p,
    Char_p                  description_p,
    Int32u_t                importFlag
);


Int32u_t    pmcDocImportSingle
(
    Int32u_t                patientId
);

Int32s_t    pmcDocInDatabase
(
    void
);

Int32u_t    pmcDocExistingRecordCheck
(
    mbFileListStruct_p      file_p,
    Char_p                  phn_p,
    pmcDocumentEditInfo_p   editInfo_p,
    Int32u_p                imported_p
);

Int32s_t pmcDocSearchTemplate
(
    mbFileListStruct_p      file_p,
    Char_p                 *result_pp
);

Int32s_t pmcDocSearchDate
(
    mbFileListStruct_p  file_p,
    Int32u_p            date_p
);

#endif
