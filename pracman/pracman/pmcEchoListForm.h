//---------------------------------------------------------------------------
// File:    pmcEchoListForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    January 12, 2003
//---------------------------------------------------------------------------
// Description:
//
// List of echos
//---------------------------------------------------------------------------

#ifndef pmcEchoListFormH
#define pmcEchoListFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Graphics.hpp>
#include <Menus.hpp>
#include "CGAUGES.h"
//---------------------------------------------------------------------------

enum
{
     PMC_ECHO_ACTION_NONE
    ,PMC_ECHO_ACTION_BACKUP
    ,PMC_ECHO_ACTION_VERIFY
};

enum
{
     PMC_ECHO_BACK_MONTH_1
    ,PMC_ECHO_BACK_MONTH_3
    ,PMC_ECHO_BACK_MONTH_6
    ,PMC_ECHO_BACK_YEARS_1
    ,PMC_ECHO_BACK_YEARS_2
    ,PMC_ECHO_BACK_YEARS_5
    ,PMC_ECHO_BACK_ALL
};

enum
{
     PMC_ECHO_LIST_SUBITEM_INDEX_STUDY_NAME
    ,PMC_ECHO_LIST_SUBITEM_INDEX_PATIENT
    ,PMC_ECHO_LIST_SUBITEM_INDEX_STUDY_DATE
    ,PMC_ECHO_LIST_SUBITEM_INDEX_READER
    ,PMC_ECHO_LIST_SUBITEM_INDEX_ONLINE
    ,PMC_ECHO_LIST_SUBITEM_INDEX_BACKUP
    ,PMC_ECHO_LIST_SUBITEM_INDEX_ID
    ,PMC_ECHO_LIST_SUBITEM_INDEX_SONOGRAPHER
};

enum
{
     PMC_ECHO_CHECK_BOX_READ
    ,PMC_ECHO_CHECK_BOX_NOT_READ
    ,PMC_ECHO_CHECK_BOX_ONLINE
    ,PMC_ECHO_CHECK_BOX_OFFLINE
    ,PMC_ECHO_CHECK_BOX_BACKED_UP
    ,PMC_ECHO_CHECK_BOX_NOT_BACKED_UP
};

enum
{
     PMC_ECHO_LIST_STATUS_READY
    ,PMC_ECHO_LIST_STATUS_READ
    ,PMC_ECHO_LIST_STATUS_MISSING_DATA
    ,PMC_ECHO_LIST_STATUS_NO_BACKUP
    ,PMC_ECHO_LIST_STATUS_MAX
};

typedef struct pmcEchoListInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    patientId;
    Int32u_t    providerId;
    Ints_t      readBack;
} pmcEchoListInfo_t, *pmcEchoListInfo_p;

typedef struct pmcEchoListStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    patientId;
    Char_p      name_p;
    Char_p      sonographer_p;
    Char_p      reader_p;
    Char_p      patientName_p;
    Char_p      patSortName_p;
    Char_p      studySortName_p;
    Char_p      comment_p;
    Char_p      commentSort_p;
    Int32u_t    backupFlag;
    Int32u_t    onlineFlag;
    Int32u_t    date;
    Int32u_t    time;
    Int32u_t    size;
    Int32u_t    readTime;
    Int32u_t    readDate;
    Int32u_t    sonographerId;
    Int32u_t    providerId;
    Int32u_t    notDeleted;
    TListItem  *item_p;
} pmcEchoListStruct_t, *pmcEchoListStruct_p;

class TEchoListForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CloseButton;
    TTimer *DatabaseCheckTimer;
    TPopupMenu *EchoListPopup;
    TMenuItem *EchoListPopupPatientSet;
    TMenuItem *EchoListPopupPatientClear;
    TMenuItem *N1;
    TMenuItem *EchoListPopupMarkAsRead;
    TMenuItem *EchoListPopupBackup;
    TMenuItem *EchoListPopupBackupCDListGet;
    TMenuItem *EchoListPopupMarkAsNotRead;
    TMenuItem *N2;
    TMenuItem *EchoListPopupBackupVerify;
    TMenuItem *N3;
    TMenuItem *EchoListPopupMakeOffline;
    TMenuItem *EchoListPopupMakeOnline;
    TLabel *SelectedLabel;
    TBevel *Bevel1;
    TGroupBox *DisplayEchosGroupBox;
    TCheckBox *NotReadCheckBox;
    TCheckBox *ReadCheckBox;
    TCheckBox *OnlineCheckBox;
    TCheckBox *OfflineCheckBox;
    TCheckBox *BackedUpCheckBox;
    TCheckBox *NotBackedUpCheckBox;
    TLabel *ReadCountLabel;
    TLabel *NotReadCountLabel;
    TLabel *OnlineCountLabel;
    TLabel *OfflineCountLabel;
    TLabel *BackedUpCountLabel;
    TLabel *NotBackedUpCountLabel;
    TMenuItem *EchoListPopupRename;
    TMenuItem *EchoListPopupBackupDatabase;
    TMenuItem *EchoListPopupBackupNonDatabase;
    TPanel *EchoListPanel;
    TListView *EchoListView;
    TPageControl *PageControl;
    TTabSheet *TabSheetAll;
    TTabSheet *TabSheetCD;
    TRadioGroup *PatientSelectRadioGroup;
    TEdit *PatientEdit;
    TBitBtn *PatientListButton;
    TGroupBox *ReadEchosBackGroupBox;
    TComboBox *ReadBackComboBox;
    TGroupBox *SearchForGroupBox;
    TEdit *SearchEdit;
    TButton *SearchClearButton;
    TEdit *CDEdit;
    TButton *DisplayCDButton;
    TLabel *Label7;
    TButton *SuggestButton;
    TMenuItem *N5;
    TMenuItem *EchoListPopupComment;
    TButton *OfflineButton;
    TMenuItem *N6;
    TMenuItem *EchoListPopupDetails;
    TMenuItem *N7;
    TMenuItem *Popup_ViewEcho;
    TMenuItem *N8;
    TMenuItem *Popup_PreviewReport;
    TLabel *Label8;
    TPageControl *PageControl1;
    TTabSheet *TabSheet1;
    TGroupBox *BackupGroupBox;
    TLabel *BytesFreeLabel;
    TLabel *CDIdLabel;
    TCGauge *ProgressGauge;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *EchoSizeLabel;
    TLabel *BytesDoneLabel;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *ActionLabel;
    TLabel *CDLabelLabel;
    TBevel *Bevel2;
    TButton *EjectButton;
    TButton *LoadButton;
    TButton *CheckButton;
    TButton *CancelButton;
    TTabSheet *TabSheet2;
    TButton *Button1;
    TButton *Button2;
    TMenuItem *N4;
    TMenuItem *Popup_Delete;
    TButton *Button_PatientAssign;
    TButton *Button3;
    void __fastcall CloseButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall DatabaseCheckTimerTimer(TObject *Sender);
    void __fastcall EchoListPopupPopup(TObject *Sender);
    void __fastcall EchoListPopupPatientSetClick(TObject *Sender);
    void __fastcall EchoListPopupPatientClearClick(TObject *Sender);
    void __fastcall EchoListViewChange(TObject *Sender, TListItem *Item,
          TItemChange Change);
    void __fastcall EchoListPopupMarkAsReadClick(TObject *Sender);
    void __fastcall EchoListPopupMarkAsNotReadClick(TObject *Sender);
    void __fastcall EchoListViewDblClick(TObject *Sender);
    void __fastcall EchoListViewColumnClick(TObject *Sender,
          TListColumn *Column);
    void __fastcall CheckBoxClick( Int32u_t boxCode );
    void __fastcall SearchEditChange(TObject *Sender);
    void __fastcall SearchClearButtonClick(TObject *Sender);
    void __fastcall ReadBackComboBoxChange(TObject *Sender);
    void __fastcall PatientSelectRadioGroupClick(TObject *Sender);
    void __fastcall PatientListButtonClick(TObject *Sender);
    void __fastcall PatientEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall ReadCheckBoxClick(TObject *Sender);
    void __fastcall NotReadCheckBoxClick(TObject *Sender);
    void __fastcall OnlineCheckBoxClick(TObject *Sender);
    void __fastcall OfflineCheckBoxClick(TObject *Sender);
    void __fastcall BackedUpCheckBoxClick(TObject *Sender);
    void __fastcall NotBackedUpCheckBoxClick(TObject *Sender);
    void __fastcall EjectButtonClick(TObject *Sender);
    void __fastcall LoadButtonClick(TObject *Sender);
    void __fastcall EchoListPopupBackupCDListGetClick(TObject *Sender);
    void __fastcall EchoListPopupRenameClick(TObject *Sender);
    void __fastcall EchoListPopupBackupDatabaseClick(TObject *Sender);
    void __fastcall EchoListPopupBackupNonDatabaseClick(TObject *Sender);
    void __fastcall CheckButtonClick(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall EchoListPopupBackupVerifyClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall EchoListPopupMakeOfflineClick(TObject *Sender);
    void __fastcall EchoListPopupMakeOnlineClick(TObject *Sender);
    void __fastcall PageControlChange(TObject *Sender);
    void __fastcall DisplayCDButtonClick(TObject *Sender);
    void __fastcall CDEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall SuggestButtonClick(TObject *Sender);
    void __fastcall EditCommentClick(TObject *Sender);
    void __fastcall EchoListViewMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall OfflineButtonClick(TObject *Sender);
    void __fastcall EchoListPopupDetailsClick(TObject *Sender);
    void __fastcall Popup_ViewEchoClick(TObject *Sender);
    void __fastcall Popup_PreviewReportClick(TObject *Sender);
    void __fastcall Button2Click(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall Popup_DeleteClick(TObject *Sender);
    void __fastcall Button_PatientAssignClick(TObject *Sender);
    void __fastcall Button3Click(TObject *Sender);
 
private:	// User declarations
    void __fastcall ListRead( qHead_p echo_q, Boolean_t allFlag );
    void __fastcall ListSort( qHead_p echo_q );
    void __fastcall ListFilter( qHead_p echo_q );
    void __fastcall ListFree( qHead_p echo_q );
    void __fastcall ListFreeEntry( pmcEchoListStruct_p echo_p );
    void __fastcall ListUpdate( Boolean_t timerFlag );
    void __fastcall ListGet( void );
    Int32s_t  __fastcall ListEchosDetectNew( qHead_p echo_q );
    Int32s_t  __fastcall ListEchosDetectDeleted( qHead_p echo_q );
    Boolean_t __fastcall ListEchosModifiy( qHead_p echo_q, Boolean_t sortFlag );

    void __fastcall ListItemUpdate( pmcEchoListStruct_p echo_p );

    void __fastcall EchoListPopupPatientEdit( Boolean_t patPromptFlag );
    void __fastcall EchoListPopupReadEdit( Boolean_t readFlag );
    void __fastcall EchoListPopupBackupClick( Int32u_t databaseFlag );
    void __fastcall EditComment( void );
    void __fastcall ViewEcho( Boolean_t dblClickFlag );

    bool __fastcall ListSortPatName
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcEchoListStruct_p         echo_p,
        pmcEchoListStruct_p         tempEcho_p
    );
    bool __fastcall ListSortId
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcEchoListStruct_p         echo_p,
        pmcEchoListStruct_p         tempEcho_p
    );
    bool __fastcall ListSortReadDate
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcEchoListStruct_p         echo_p,
        pmcEchoListStruct_p         tempEcho_p
    );
    bool __fastcall ListSortStudyDate
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcEchoListStruct_p         echo_p,
        pmcEchoListStruct_p         tempEcho_p
    );
    bool __fastcall ListSortStudyName
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcEchoListStruct_p         echo_p,
        pmcEchoListStruct_p         tempEcho_p
    );
    bool __fastcall ListSortComment
    (
        Int32u_t                    sortMode,
        qHead_p                     temp_q,
        pmcEchoListStruct_p         echo_p,
        pmcEchoListStruct_p         tempEcho_p
    );

    void __fastcall CountsUpdate( void );
    void __fastcall ActionLabelUpdate( Int32u_t code );
    void __fastcall PatientList( Int32u_t key );
    void __fastcall PatientUpdate( Int32u_t patientId );
    void __fastcall CDLabelUpdate( Char_p str_p, Int32u_t bytesFree );
    void __fastcall CancelButtonEnable( Boolean_t enableFlag );
    void __fastcall ControlsStore( void );
    void __fastcall ControlsRestore( void );
    void __fastcall ControlsDisable( void );
    void __fastcall DisplayCD( void );

    Int32s_t __fastcall BytesProcessedCallbackReal( Int32u_t bytes );
    Int32s_t __fastcall CancelCheckCallbackReal( );

    pmcEchoListStruct_p __fastcall TEchoListForm::GetSelectedEcho( void );

    Boolean_t           CancelButtonClicked;
    Boolean_t           Active;
    Boolean_t           FilterRun;

    Boolean_t           FilterRequired;

    Boolean_t           GetInProgress;
    Boolean_t           UpdateInProgress;
    Int32u_t            PopupInProgress;
    Boolean_t           SortInProgress;
    Boolean_t           BackupInProgress;
    Boolean_t           AutoOfflineFlag;
    Boolean_t           OfflineCanceled;

    Int32u_t            ActinCode;

    Int32u_t            ReadCount;
    Int32u_t            NotReadCount;
    Int32u_t            OnlineCount;
    Int32u_t            OfflineCount;
    Int32u_t            BackedUpCount;
    Int32u_t            NotBackedUpCount;

    qHead_t             EchoQueue;
    qHead_p             Echo_q;
    pmcEchoListInfo_p   FormInfo_p;

    qHead_t             IgnoreCDQueue;
    qHead_p             Ignore_q;

    Int64u_t            TableReadTime;
    Int32u_t            TableReadSize;

    Int32u_t            SelectedEchoId;
    Int32u_t            SortMode;
    Int32u_t            SortReadDateMode;
    Int32u_t            SortStudyNameMode;
    Int32u_t            SortPatNameMode;
    Int32u_t            SortStudyDateMode;
    Int32u_t            SortIdMode;
    Int32u_t            SortCommentMode;

    Int32u_t            CheckRecorded;
    Int32u_t            SaveCheckState;

    Int32u_t            ReadCheck;
    Int32u_t            NotReadCheck;
    Int32u_t            OnlineCheck;
    Int32u_t            OfflineCheck;
    Int32u_t            BackedUpCheck;
    Int32u_t            NotBackedUpCheck;

    Int32s_t            ReadBackItemIndexOld;
    Int32u_t            ReadBackDate;

    Int32u_t            PatientIdOld;
    Int32s_t            EchoReadBackItemIndex;

    Int32u_t            BytesProcessed;
    Int32u_t            BytesToProcess;
    Int32u_t            BytesFree;

#if 0
    Ints_t              InitialTop;
    Ints_t              InitialLeft;
    Ints_t              InitialHeight;
    Ints_t              InitialWidth;
#endif

    Int32u_t            DisplayCDFlag;
    Int32u_t            DisplayCDId;

    Ints_t              MouseX;
    Ints_t              MouseY;

    // These variables are used to store the state of the controls
    Boolean_t           StoreEjectButtonEnabled;
    Boolean_t           StoreLoadButtonEnabled;
    Boolean_t           StoreCheckButtonEnabled;
    Boolean_t           StoreReadCheckBoxEnabled;
    Boolean_t           StoreNotReadCheckBoxEnabled;
    Boolean_t           StoreOnlineCheckBoxEnabled;
    Boolean_t           StoreOfflineCheckBoxEnabled;
    Boolean_t           StoreBackedUpCheckBoxEnabled;
    Boolean_t           StoreNotBackedUpCheckBoxEnabled;
    Boolean_t           StorePatientListButtonEnabled;
    Boolean_t           StoreReadBackComboBoxEnabled;
    Boolean_t           StoreSearchClearButtonEnabled;
    Boolean_t           StoreSearchEditEnabled;
    Boolean_t           StorePatientSelectRadioGroupEnabled;
    Boolean_t           StoreCloseButtonEnabled;
    Boolean_t           StorePatientEditEnabled;
    Boolean_t           StoreEchoListPanelEnabled;
    Boolean_t           StorePageControlEnabled;
    Boolean_t           StoreCDEditEnabled;
    Boolean_t           StoreDisplayCDButtonEnabled;
    Boolean_t           StoreSuggestButtonEnabled;
    Boolean_t           StoreOfflineButtonEnabled;

public:		// User declarations
    __fastcall TEchoListForm( TComponent *Owner);
    __fastcall TEchoListForm( TComponent *Owner, pmcEchoListInfo_p   formInfo_p );

    Int32s_t  static    CancelCheckCallback( Void_p this_p );
    Int32s_t  static    BytesProcessedCallback( Void_p this_p, Int32u_t bytes );
    Int32s_t  static    CDLabelCallback( Void_p this_p, Char_p str_p, Int32u_t bytesFree );
};

#define PMC_ECHO_LIST_POPUP_COMMENT             "EchoListPopupComment"
#define PMC_ECHO_LIST_POPUP_RENAME              "EchoListPopupRename"
#define PMC_ECHO_LIST_POPUP_PATIENT_SET         "EchoListPopupPatientSet"
#define PMC_ECHO_LIST_POPUP_PATIENT_CLEAR       "EchoListPopupPatientClear"
#define PMC_ECHO_LIST_POPUP_MARK_AS_READ        "EchoListPopupMarkAsRead"
#define PMC_ECHO_LIST_POPUP_MARK_AS_NOT_READ    "EchoListPopupMarkAsNotRead"
#define PMC_ECHO_LIST_POPUP_BACKUP              "EchoListPopupBackup"
#define PMC_ECHO_LIST_POPUP_BACKUP_DATABASE     "EchoListPopupBackupDatabase"
#define PMC_ECHO_LIST_POPUP_BACKUP_NON_DATABASE "EchoListPopupBackupNonDatabase"
#define PMC_ECHO_LIST_POPUP_BACKUP_CD_LIST_GET  "EchoListPopupBackupCDListGet"
#define PMC_ECHO_LIST_POPUP_BACKUP_VERIFY       "EchoListPopupBackupVerify"
#define PMC_ECHO_LIST_POPUP_MAKE_OFFLINE        "EchoListPopupMakeOffline"
#define PMC_ECHO_LIST_POPUP_MAKE_ONLINE         "EchoListPopupMakeOnline"
#define PMC_ECHO_LIST_POPUP_DETAILS             "EchoListPopupDetails"
#define PMC_ECHO_LIST_POPUP_VIEW                "Popup_ViewEcho"
#define PMC_ECHO_LIST_POPUP_PREVIEW             "Popup_PreviewReport"
#define PMC_ECHO_LIST_POPUP_DELETE              "Popup_Delete"

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t pmcEchoListForm
(
    Int32u_t    patientId
);

#endif // pmcEchoListFormH
