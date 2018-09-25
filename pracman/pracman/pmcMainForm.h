//---------------------------------------------------------------------------
// File:    pmcMainForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcMainFormH
#define pmcMainFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include "CCALENDR.h"
#include <Grids.hpp>
#include <DBCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <StdActns.hpp>
#include <Db.hpp>
#include <DBTables.hpp>
#include <DBGrids.hpp>
#include <Dialogs.hpp>

#include "mbTypes.h"
#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcColors.h"
#include "pmcSplashScreen.h"
#include "pmcPollTableThread.h"
#include <Mask.hpp>

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
    TCCalendar *Calendar;
    TButton *TodayButton;
    TLabel *CalendarTitleLabel;
    TActionList *ActionList1;
    TEditCopy *EditCopy1;
    TEditCut *EditCut1;
    TEditPaste *EditPaste1;
    TMainMenu *MainMenu1;
    TMenuItem *File1;
    TMenuItem *Tools1;
    TMenuItem *Help;
    TMenuItem *Quit;
    TMenuItem *CheckDatabase1;
    TMenuItem *Reports1;
    TMenuItem *HelpAbout;
    TComboBox *ProvidersListBox;
    TDrawGrid *DayViewGrid;
    TDrawGrid *WeekViewGrid;
    TTabSheet *MonthViewTab;
    TDrawGrid *MonthViewGrid;
    TBevel *Bevel2;
    TDrawGrid *WeekViewTimeGrid;
    TDrawGrid *MonthViewTimeGrid;
    TDrawGrid *DayViewTimeGrid;
    TBevel *Bevel3;
    TBevel *Bevel4;
    TLabel *ProviderLabel;
    TColorDialog *ColorDialog1;
    TPageControl *AppointmentTabs;
    TPopupMenu *DayViewPopup;
    TMenuItem *DayViewPopupNew;
    TMenuItem *DayViewPopupCut;
    TMenuItem *DayViewPopupConfirm;
    TMenuItem *N1;
    TMenuItem *DayViewPopupArrived;
    TMenuItem *DayViewPopupCompleted;
    TMenuItem *DayViewPopupPaste;
    TCCalendar *HiddenCalendar;
    TGroupBox *GroupBox1;
    TTimer *Timer1;
    TMenuItem *N2;
    TMenuItem *DayViewPopupPatPrintLabelReq;
    TMenuItem *N3;
    TMenuItem *DayViewPopupPat;
    TMenuItem *DayViewPopupPatAppList;
    TMenuItem *DayViewPopupPrintDaySheet;
    TMenuItem *DayViewPopupPatPrintRecord;
    TMenuItem *DayViewPopupPatEdit;
    TScrollBar *YearScrollBar;
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
    TLabel *Label1;
    TLabel *Label2;
    TMenuItem *AppointmentLetters1;
    TMenuItem *DayViewPopupDr;
    TMenuItem *DayViewPopupDrEdit;
    TMenuItem *DayViewPopupDrPrintRecord;
    TMenuItem *DayViewPopupConfirmPhone;
    TMenuItem *DayViewPopupConfirmLetter;
    TMenuItem *DayViewPopupPatPrintLetter;
    TMenuItem *DayViewPopupPatChange;
    TMenuItem *DayViewPopupDrChange;
    TMenuItem *DayViewPopupAppType;
    TMenuItem *DayViewPopupComment;
    TMenuItem *DayViewPopupAppTypeNew;
    TMenuItem *N7;
    TMenuItem *DayViewPopupAppTypeClear;
    TMenuItem *DayViewPopupPatClear;
    TMenuItem *DayViewPopupDrClear;
    TMenuItem *N5;
    TMenuItem *N8;
    TMenuItem *DayViewPopupCommentEdit;
    TMenuItem *DayViewPopupCommentView;
    TMenuItem *DayViewPopupPatView;
    TMenuItem *DayViewPopupDrView;
    TMenuItem *N6;
    TMenuItem *SystemStatus;
    TMenuItem *DayViewPopupDrPrintLetter;
    TMenuItem *DayViewPopupDrPrintLabelAddr;
    TMenuItem *DayViewPopupPatPrintLabelAddr;
    TMenuItem *Routine1;
    TMenuItem *Urgent1;
    TMenuItem *DayViewPopupAppTypeReview;
    TMenuItem *DayViewPopupAppTypeUrgent;
    TMenuItem *MainMenuDatabaseCleanFields;
    TButton *TestLabelPrinter;
    TMenuItem *DayViewPopupConfirmCancelPhone;
    TMenuItem *DayViewPopupConfirmCancelLetter;
    TMenuItem *DayViewPopupConfirmQuery;
    TMenuItem *N9;
    TMenuItem *N10;
    TGroupBox *GroupBox2;
    TOpenDialog *OpenDialog;
    TLabel *AppointCutBufLabel;
    TMenuItem *PrintArchivedMSPClaimsFile;
    TButton *ReportButton;
    TPageControl *AppInfo;
    TTabSheet *AppInfoInfo;
    TTabSheet *AppInfoLegend;
    TDrawGrid *LegendDrawGrid;
    TTimer *Timer2;
    TLabel *AppInfoNameLabel;
    TMenuItem *DayViewPopupPatClaimList;
    TMenuItem *Import2;
    TMenuItem *MainMenuImportSmaDrList;
    TMenuItem *MainMenuImportMSPDrList1;
    TMenuItem *MainMenuImportMSPDrListExcel;
    TMenuItem *MainMenuCheckPatientRecords;
    TMenuItem *MainMenuCheckAppointmentRecords;
    TMenuItem *CheckDoctorRecords;
    TLabel *AppInfoPhnLabel;
    TLabel *Label3;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Label7;
    TLabel *Label8;
    TBevel *Bevel9;
    TBevel *Bevel10;
    TLabel *AppInfoPhoneLabel;
    TLabel *AppInfoDobLabel;
    TLabel *AppInfoTypeLabel;
    TLabel *AppInfoRefLabel;
    TLabel *AppInfoAppCommentLabel;
    TLabel *AppInfoPatCommentLabel;
    TBevel *Bevel11;
    TLabel *CutBufferSizeLabel;
    TTabSheet *DayViewTab;
    TTabSheet *WeekViewTab;
    TMenuItem *PrintArchivedValidrptFile;
    TMenuItem *CheckClaimRecords;
    TMenuItem *PrintArchivedReturnsFile;
    TMenuItem *PrintArchivedNoticeFile;
    TMenuItem *N13;
    TMenuItem *N4;
    TMenuItem *DayViewPopupAvailable;
    TMenuItem *DayViewPopupUnavailable;
    TMenuItem *Documents2;
    TMenuItem *ImportDocumentsBatch;
    TMenuItem *ImportDocumentsSingle;
    TButton *TestCrcButton;
    TButton *PatientListButton;
    TButton *DoctorListButton;
    TButton *ClaimListButton;
    TButton *DocumentListButton;
    TMenuItem *N14;
    TMenuItem *CreateWord;
    TButton *TestSelectButton;
    TTabSheet *ProviderViewTab;
    TDrawGrid *ProviderViewTimeGrid;
    TDrawGrid *ProviderViewGrid;
    TMenuItem *N15;
    TMenuItem *Administrator1;
    TMenuItem *ScramblePatientNames;
    TMenuItem *N12;
    TMenuItem *ExtractDocuments2;
    TMenuItem *CheckDocumentDatabase;
    TMenuItem *N16;
    TMenuItem *CheckDocumentRecords1;
    TButton *Appointments;
    TButton *TestPatientFormModal;
    TButton *TestPatientFormNonModal;
    TGroupBox *GroupBox3;
    TLabel *Label10;
    TLabel *Label9;
    TLabel *PatientCountLbl;
    TLabel *DoctorCountLabel;
    TLabel *Label12;
    TLabel *Label11;
    TLabel *DocumentsCountLabel;
    TLabel *AppointmentsCountLabel;
    TButton *LockTestButton;
    TMenuItem *ImportMSPFiles;
    TMenuItem *N17;
    TMenuItem *ICDList;
    TMenuItem *DrFaxPhoneList;
    TSaveDialog *SaveDialog;
    TMenuItem *DayViewPopupAppTypeClinic;
    TMenuItem *DayViewPopupCancel;
    TPopupMenu *PopupMenu1;
    TMenuItem *Test1;
    TTabSheet *TabSheet1;
    TMenuItem *N18;
    TMenuItem *DayViewPopupHistory;
    TMenuItem *CreateAppHistory;
    TMenuItem *DayViewPopupDelete;
    TMenuItem *Echos1;
    TMenuItem *ImportEchos;
    TImageList *ImageListColorSquares;
    TMenuItem *CDEject;
    TMenuItem *CDLoad;
    TMenuItem *DisplayEchoCDContents;
    TButton *EchosListButton;
    TMenuItem *DayViewPopupPatDocList;
    TMenuItem *DayViewPopupPatEchoList;
    TButton *Test;
    TMenuItem *N11;
    TMenuItem *CheckArchivedRETURNSFileContents1;
    TMenuItem *DayViewPopupAppTypeHospital;
    TMenuItem *N19;
    TMenuItem *MallocTest1;
    TMenuItem *CheckPatientsUnreferenced;
    TMenuItem *N20;
    TMenuItem *AssignPatientstoEchos;
    TMenuItem *DatabaseCheckTimeslots;
    TMenuItem *MedList;
    TMenuItem *Formulary;
    TMenuItem *ImportScannedDocuments;
    TMenuItem *FileWriteTest;
    TMenuItem *FileReadTest;
    TMenuItem *Write50Files;
    TMenuItem *Read50Files;
    TMenuItem *DayViewPopupDaySheet;
    TButton *ButtonDaySheet;
    TMenuItem *CheckActiveDocuments;
    TMenuItem *IncrementClaimNumbers;

    void __fastcall TodayButtonClick(TObject *Sender);
    void __fastcall PatientListButtonClick(TObject *Sender);
    void __fastcall MonthViewTimeGridDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
    void __fastcall MonthViewGridDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
    void __fastcall WeekViewTimeGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall DayViewTimeGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall WeekViewGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall DayViewGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall ProvidersListBoxChange(TObject *Sender);
    void __fastcall ProvidersListBoxDropDown(TObject *Sender);
    void __fastcall LegendDrawGridClick(TObject *Sender);
    void __fastcall LegendDrawGridDrawCell(TObject *Sender, int ACol, int ARow,
          TRect &Rect, TGridDrawState State);
    void __fastcall DateButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall DayViewGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall DayViewPopupPopup(TObject *Sender);
    void __fastcall DayViewGridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);
    void __fastcall DayViewGridStartDrag(TObject *Sender,
          TDragObject *&DragObject);
    void __fastcall DayViewGridEndDrag(TObject *Sender, TObject *Target,
          int X, int Y);
    void __fastcall DayViewGridDragOver(TObject *Sender, TObject *Source,
          int X, int Y, TDragState State, bool &Accept);
    void __fastcall Timer1Tick(TObject *Sender);
    void __fastcall DoctorListButtonClick(TObject *Sender);
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
    void __fastcall YearScrollBarChange(TObject *Sender);
    void __fastcall CalendarChange(TObject *Sender);
    void __fastcall AppointmentLetters1Click(TObject *Sender);
    void __fastcall DayViewPopupNewClick(TObject *Sender);
    void __fastcall DayViewPopupCutClick(TObject *Sender);
    void __fastcall DayViewPopupPasteClick(TObject *Sender);
    void __fastcall DayViewPopupPatChangeClick(TObject *Sender);
    void __fastcall DayViewPopupDrChangeClick(TObject *Sender);
    void __fastcall DayViewPopupPatEditClick(TObject *Sender);
    void __fastcall DayViewPopupDrEditClick(TObject *Sender);
    void __fastcall DayViewPopupPatViewClick(TObject *Sender);
    void __fastcall DayViewPopupPatClearClick(TObject *Sender);
    void __fastcall DayViewPopupAppTypeClearClick(TObject *Sender);
    void __fastcall DayViewPopupAppTypeNewClick(TObject *Sender);
    void __fastcall DayViewPopupDrViewClick(TObject *Sender);
    void __fastcall DayViewPopupDrClearClick(TObject *Sender);
    void __fastcall DayViewPopupCommentEditClick(TObject *Sender);
    void __fastcall DayViewPopupCommentViewClick(TObject *Sender);
    void __fastcall DayViewPopupConfirmPhoneClick(TObject *Sender);
    void __fastcall DayViewPopupConfirmLetterClick(TObject *Sender);
    void __fastcall DayViewPopupAppTypeReviewClick(TObject *Sender);
    void __fastcall DayViewPopupAppTypeUrgentClick(TObject *Sender);
    void __fastcall DayViewPopupAppTypeSet( Int32u_t appType );
    void __fastcall DayViewPopupPatPrintRecordClick(TObject *Sender);
    void __fastcall DayViewPopupPatPrintLabelReqClick(TObject *Sender);
    void __fastcall DayViewPopupConfirmCancelPhoneClick(TObject *Sender);
    void __fastcall DayViewPopupConfirmCancelLetterClick(TObject *Sender);
    void __fastcall DayViewPopupConfirmQueryClick(TObject *Sender);
    void __fastcall DayViewPopupPatPrintLabelAddrClick(TObject *Sender);
    void __fastcall DayViewPopupDrPrintLabelAddrClick(TObject *Sender);
    void __fastcall DayViewPopupCompletedClick(TObject *Sender);
    void __fastcall DayViewPopupPatAppListClick(TObject *Sender);
    void __fastcall DayViewPopupPrintDaySheetClick(TObject *Sender);
    void __fastcall DayViewPopupPurgeClick(TObject *Sender);
    void __fastcall HelpAboutClick(TObject *Sender);
    void __fastcall ClaimListButtonClick(TObject *Sender);
    void __fastcall SystemStatusClick(TObject *Sender);
    void __fastcall PrintArchivedMSPClaimsFileClick(TObject *Sender);
    void __fastcall ReportButtonClick(TObject *Sender);
    void __fastcall QuitClick(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    void __fastcall TestLabelPrinterClick(TObject *Sender);
    void __fastcall CalendarKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall CalendarKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall Timer2Timer(TObject *Sender);
    void __fastcall MainMenuImportSmaDrListClick(TObject *Sender);
    void __fastcall MainMenuImportMspDrListExcelClick(TObject *Sender);
    void __fastcall MainMenuDatabaseCleanFieldsClick(TObject *Sender);
    void __fastcall MainMenuCheckPatientRecordsClick(TObject *Sender);
    void __fastcall MainMenuCheckDoctorRecordsClick(TObject *Sender);
    void __fastcall MainMenuCheckAppointmentRecordsClick(TObject *Sender);
    void __fastcall WeekViewGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall MonthViewGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall WeekViewGridEndDrag(TObject *Sender, TObject *Target,
          int X, int Y);
    void __fastcall WeekViewGridDragOver(TObject *Sender, TObject *Source,
          int X, int Y, TDragState State, bool &Accept);
    void __fastcall WeekViewGridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);
    void __fastcall WeekViewGridStartDrag(TObject *Sender,
          TDragObject *&DragObject);
    void __fastcall MonthViewGridDragOver(TObject *Sender, TObject *Source,
          int X, int Y, TDragState State, bool &Accept);
    void __fastcall MonthViewGridEndDrag(TObject *Sender, TObject *Target,
          int X, int Y);
    void __fastcall MonthViewGridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);
    void __fastcall MonthViewGridStartDrag(TObject *Sender,
          TDragObject *&DragObject);
    void __fastcall AppointmentTabsChange(TObject *Sender);
    void __fastcall PrintArchivedValidrptFileClick(TObject *Sender);
    void __fastcall CheckClaimRecordsClick(TObject *Sender);
    void __fastcall PrintArchivedReturnsFileClick(TObject *Sender);
    void __fastcall PrintArchivedNoticeFileClick(TObject *Sender);
    void __fastcall DayViewPopupAvailableClick(TObject *Sender);
    void __fastcall DayViewPopupUnavailableClick(TObject *Sender);
    void __fastcall ImportDocumentsBatchClick(TObject *Sender);
    void __fastcall ImportDocumentsSingleClick(TObject *Sender);
    void __fastcall TestCrcButtonClick(TObject *Sender);
    void __fastcall DocumentListButtonClick(TObject *Sender);
    void __fastcall CreateWordClick(TObject *Sender);
    void __fastcall TestSelectButtonClick(TObject *Sender);
    void __fastcall ExtractDocumentsClick(TObject *Sender);
    void __fastcall ProviderViewGridDragOver(TObject *Sender,
          TObject *Source, int X, int Y, TDragState State, bool &Accept);
    void __fastcall ProviderViewGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall ProviderViewGridEndDrag(TObject *Sender,
          TObject *Target, int X, int Y);
    void __fastcall ProviderViewGridMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall ProviderViewGridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect);
    void __fastcall ProviderViewGridStartDrag(TObject *Sender,
          TDragObject *&DragObject);
    void __fastcall ProviderViewTimeGridDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
    void __fastcall ScramblePatientNamesClick(TObject *Sender);
    void __fastcall CheckDocumentRecordsClick(TObject *Sender);
    void __fastcall CheckDocumentDatabaseClick(TObject *Sender);
    void __fastcall AppointmentsClick(TObject *Sender);
    void __fastcall TestPatientFormModalClick(TObject *Sender);
    void __fastcall TestPatientFormNonModalClick(TObject *Sender);
    void __fastcall LockTestButtonClick(TObject *Sender);
    void __fastcall ImportMSPFilesClick(TObject *Sender);
    void __fastcall ICDListClick(TObject *Sender);
    void __fastcall DrFaxPhoneListClick(TObject *Sender);
    void __fastcall DayViewPopupAppTypeClinicClick(TObject *Sender);
    void __fastcall DayViewPopupCancelClick(TObject *Sender);
    void __fastcall DayViewPopupHistoryClick(TObject *Sender);
    void __fastcall CreateAppHistoryClick(TObject *Sender);
    void __fastcall DayViewPopupPatClaimListClick(TObject *Sender);
    void __fastcall DayViewPopupDeleteClick(TObject *Sender);
    void __fastcall ImportEchosClick(TObject *Sender);
    void __fastcall CheckDoctorRecordsClick(TObject *Sender);
    void __fastcall CDEjectClick(TObject *Sender);
    void __fastcall CDLoadClick(TObject *Sender);
    void __fastcall DisplayEchoCDContentsClick(TObject *Sender);
    void __fastcall EchosListButtonClick(TObject *Sender);
    void __fastcall DayViewPopupPatEchoListClick(TObject *Sender);
    void __fastcall DayViewPopupPatDocListClick(TObject *Sender);
    void __fastcall CheckArchivedRETURNSFileContents1Click( TObject *Sender);
    void __fastcall DayViewPopupAppTypeHospitalClick(TObject *Sender);
    void __fastcall MallocTest1Click(TObject *Sender);
    void __fastcall CheckPatientsUnreferencedClick(TObject *Sender);
    void __fastcall AssignPatientstoEchosClick(TObject *Sender);
    void __fastcall DatabaseCheckTimeslotsClick(TObject *Sender);
    void __fastcall MedListClick(TObject *Sender);
    void __fastcall FormularyClick(TObject *Sender);
    void __fastcall TestClick(TObject *Sender);
    void __fastcall ImportScannedDocumentsClick(TObject *Sender);
    void __fastcall FileWriteTestClick(TObject *Sender);
    void __fastcall FileReadTestClick(TObject *Sender);
    void __fastcall Write50FilesClick(TObject *Sender);
    void __fastcall DayViewPopupDaySheetClick(TObject *Sender);
    void __fastcall DayViewTabShow(TObject *Sender);
    void __fastcall WeekViewTabShow(TObject *Sender);
    void __fastcall MonthViewTabShow(TObject *Sender);
    void __fastcall ProviderViewTabShow(TObject *Sender);
    void __fastcall ButtonDaySheetClick(TObject *Sender);
    void __fastcall CheckActiveDocumentsClick(TObject *Sender);
    void __fastcall IncrementClaimNumbersClick(TObject *Sender);

private:	// User declarations
    PollTableThread    *PollThread_p;

    pmcTableStatus_t    tableStatus[PMC_TABLE_COUNT];
    Int32s_t            TestColor;
    Ints_t              MouseInRow;
    Ints_t              MouseInCol;
    bool                MouseInRowTop;
    bool                CheckTables;

    Int32u_t            SelectedProviderId;
    Int32u_t            SelectedDate;
    Int32u_t            SelectedDayOfWeek;

    // Range of dates for week view
    Int32u_t            SelectedWeekViewYear[7];
    Int32u_t            SelectedWeekViewMonth[7];
    Int32u_t            SelectedWeekViewDay[7];
    Int32u_t            SelectedWeekViewDate[7];

    Int32u_t            SelectedMonthViewDay[31];
    Int32u_t            SelectedMonthViewDate[31];
    Int32u_t            SelectedMonthViewDateEnd;

    Int32u_t            KeyDownDate;
    Int32u_t            KeyDownKey;

    pmcAppViewInfo_t    DayViewInfo;
    pmcAppViewInfo_t    WeekViewInfo;
    pmcAppViewInfo_t    MonthViewInfo;
    pmcAppViewInfo_t    ProviderViewInfo;

    Int32u_t __fastcall     AppViewExtractAppointId
    (
        Int32s_t            row,
        Int32s_t            col,
        pmcAppViewInfo_p    appViewInfo
    );

    Int32s_t __fastcall     AppViewExtractAvailable
    (
        Int32s_t            row,
        Int32s_t            col,
        pmcAppViewInfo_p    appViewInfo_p
    );

    void __fastcall         DayViewPopupAvailableChange
    (
        TObject            *Sender,
        Int32u_t            availFlag,
        Int32u_t            startSlot,
        Int32u_t            endSlot,
        bool                ingnoreNotReadyFlag
    );

    void __fastcall         AppViewGridEndDrag
    (
        TDrawGrid          *AppViewGrid,
        pmcAppViewInfo_p    AppViewInfo_p,
        int                 X,
        int                 Y
    );

    void __fastcall         AppViewInfoUpdate
    (
        Int32s_t            row,
        Int32s_t            col,
        pmcAppViewInfo_p    appViewInfo_p,
        Int32u_t            appointIn
    );

    pmcAppViewInfo_p __fastcall AppViewPopupGet
    (
        void
    );

    void __fastcall         AppListGet
    (
        qHead_p             app_q,
        Int32u_t            startDate,
        Int32u_t            endDate
    );

    Int32s_t __fastcall     TimeSlotsGet
    (
        pmcAppViewInfo_p    appView_p,
        qHead_p             providerView_q
    );

    Int32s_t __fastcall     FindConflicts
    (
        qHead_p             app_q,
        pmcAppViewInfo_p    appView_p
    );

    Int32s_t __fastcall     CullProviders
    (
        qHead_p             app_q,
        pmcAppViewInfo_p    appView_p,
        qHead_p             providerView_q
    );

    Int32u_t __fastcall     TimeSlotRecordCreate
    (
        Int32u_t            providerId,
        Int32u_t            date,
        Char_p              string_p
    );
    
    Int32s_t __fastcall     DayViewPopupChangePat
    (
        Int32u_t            appointId,
        pmcAppViewInfo_p    appView_p
    );

    void        __fastcall UpdateProviderLabel( void );
    void        __fastcall DayViewPopupCommentClick( Int32u_t mode );

public:		// User declarations
    void __fastcall UpdateDate
    (
        Int32u_t    year,
        Int32u_t    month,
        Int32u_t    day
    );
    void        __fastcall PatientCountUpdate( Int32u_t mode );
    void        __fastcall AppointmentCountUpdate( Int32u_t mode );
    void        __fastcall DocumentCountUpdate( Int32u_t mode );

    void        __fastcall UpdateDateInt( Int32u_t dateIn );
    void        __fastcall UpdateProviderListBox(TObject *Sender);
    Int32u_t    __fastcall RefreshTable( Int32u_t tableBitMask  );
    Int32u_t    __fastcall RefreshTableForce( Int32u_t tableBitMask );
    void        __fastcall RefreshTableInit( void );
    void        __fastcall RefreshTableQuery( void );
    void        __fastcall UpdatePatientList( pmcTableStatus_p tableStatus_p, bool forceFlag );
    void        __fastcall UpdatePatientListRecordDelete( pmcPatRecordStruct_p patRecord_p );
    void        __fastcall UpdatePatientListRecordAdd(    pmcPatRecordStruct_p patRecord_p );
    void        __fastcall UpdateDoctorList( pmcTableStatus_p tableStatus_p, bool forceFlag );
    void        __fastcall UpdateDoctorListRecordDelete( pmcDocRecordStruct_p docRecord_p );
    void        __fastcall UpdateDoctorListRecordAdd(    pmcDocRecordStruct_p docRecord_p );

    void        __fastcall PatientRecordFree( pmcPatRecordStruct_p patRecord_p );
    void        __fastcall DoctorRecordFree( pmcDocRecordStruct_p docRecord_p );

    Int32s_t    __fastcall DayViewInfoUpdate      ( pmcAppViewInfo_p day_p   );
    Int32s_t    __fastcall WeekViewInfoUpdate     ( pmcAppViewInfo_p week_p  );
    Int32s_t    __fastcall MonthViewInfoUpdate    ( pmcAppViewInfo_p month_p );
    Int32s_t    __fastcall ProviderViewInfoUpdate ( pmcAppViewInfo_p view_p  );

    void        __fastcall UpdateDateString( void );
    void        __fastcall SelectedWeekViewDateUpdate( void );
    void        __fastcall SelectedMonthViewDateUpdate( void );
 
#if PMC_AVAIL_TIME
    void        __fastcall AvailTimeListInit( void );
#endif

    void        __fastcall DayViewInfoCellConstruct     ( void );
    void        __fastcall WeekViewInfoCellConstruct    ( void );
    void        __fastcall MonthViewInfoCellConstruct   ( void );
    void        __fastcall ProviderViewInfoCellConstruct( void );

    void        __fastcall AppointmentGoto( Int32u_t providerId, Int32u_t date );

    Int32s_t   static DaySheetContents( Int32u_t handle, FILE *fp, Char_p key_p );

    bool        RefreshTableInitFlag;
    bool        UpdatePatientListDoInit;
    bool        UpdatePatientListDone;

    bool        UpdateDoctorListDoInit;
    bool        UpdateDoctorListDone;
    bool        CalendarSkipRefresh;
    bool        CalendarSkipUpdate;
    bool        CalendarMonthSkipUpdate;

    bool        SkipTimer;
    bool        SkipUpdateProviderLabelInvalidate;

    __fastcall  TMainForm(TComponent* Owner);
    __fastcall ~TMainForm( void );
};

//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------

typedef struct  pmcAppointCutBuf_s
{
    qLinkage_t      linkage;
    Ints_t          appointId;
    Char_t          desc[255];
} pmcAppointCutBuf_t, *pmcAppointCutBuf_p;

#define REFRESH_DAY_GRID()\
{\
    if( DayInfoUpdate( &DayViewInfo ) )\
    {\
        DayInfoCellConstruct( );\
        pmcDayViewUpdateSkip = TRUE;\
        DayViewGrid->Invalidate( );\
        pmcDayViewUpdateSkip = FALSE;\
    }\
}

void  TimeGridDrawCell
(
    TObject        *Sender,
    int             ACol,
    int             ARow,
    TRect           &Rect,
    TGridDrawState  State,
    TDrawGrid      *Grid_p
);

#endif

