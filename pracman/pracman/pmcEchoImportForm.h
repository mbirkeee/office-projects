//---------------------------------------------------------------------------
// File:    pmcEchoImportForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    October 26, 2002
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

#ifndef pmcEchoImportFormH
#define pmcEchoImportFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include "CGAUGES.h"
#include <Buttons.hpp>
#include <Grids.hpp>
#include <Graphics.hpp>

#define PMC_DISK_DETECTED_UNKNOWN   (0)
#define PMC_DISK_DETECTED_YES       (1)
#define PMC_DISK_DETECTED_NO        (2)

// These enums must correspond to the color squares image list on the
// main form
enum
{
     PMC_ECHO_IMPORT_STATUS_INVALID = 0
    ,PMC_ECHO_IMPORT_STATUS_READY
    ,PMC_ECHO_IMPORT_STATUS_NOT_READY
    ,PMC_ECHO_IMPORT_STATUS_IMPORTING
    ,PMC_ECHO_IMPORT_STATUS_IN_DATABASE
};

// Possible echo file types
enum
{
     PMC_ECHO_FILE_TYPE_HPSONOS_DB_ROOT
    ,PMC_ECHO_FILE_TYPE_HPSONOS_DB
    ,PMC_ECHO_FILE_TYPE_HPSONOS_DSR
};

typedef struct pmcEcho_s
{
    qLinkage_t          linkage;
    Char_p              name_p;
    Int32u_t            status;
    Int32u_t            id;
    Int32u_t            date;
    Int32u_t            time;
    Int32u_t            size;
    Char_p              date_p;
    Char_p              time_p;
    mbFileListStruct_p  dir_p;
    mbFileListStruct_p  dbFile_p;
    TListItem          *item_p;
} pmcEcho_t, *pmcEcho_p;

typedef struct pmcEchoFile_s
{
    qLinkage_t          linkage;
    Char_p              name_p;
    Char_p              path_p;
    Char_p              orig_p;
    Char_p              source_p;
    Char_p              target_p;
    Int32u_t            id;
    Int32u_t            size;
    Int32u_t            crc;
} pmcEchoFile_t, *pmcEchoFile_p;

typedef struct pmcEchoIgnoreNotReady_s
{
    qLinkage_t          linkage;
    Int32u_t            crc;
} pmcEchoIgnoreNotReady_t, *pmcEchoIgnoreNotReady_p;

//---------------------------------------------------------------------------
class TEchoImportForm : public TForm
{
__published:	// IDE-managed Components
    TGroupBox *GroupBox1;
    TLabel *SourceLabel;
    TLabel *DatabaseFreeSpaceLabel;
    TLabel *TotalBytesToCopyLabel;
    TGroupBox *GroupBox2;
    TLabel *StatusLabel;
    TBevel *Bevel2;
    TTimer *DiskPollTimer;
    TListView *ListView;
    TGroupBox *GroupBox3;
    TImage *Image1;
    TImage *Image2;
    TImage *Image3;
    TLabel *Label1;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *TotalBytesCopiedLabel;
    TCGauge *TotalProgressGauge;
    TImage *Image4;
    TLabel *Label6;
    TGroupBox *GroupBox4;
    TButton *StartImportButton;
    TButton *CloseButton;
    TLabel *Label7;
    TCGauge *CurrentFileGauge;
    TCGauge *CurrentStudyGauge;
    TLabel *Label9;
    TBevel *Bevel3;
    TBevel *Bevel4;
    TBevel *Bevel5;
    TLabel *Label2;
    TLabel *CurrentFileLabel;
    TLabel *CurrentStudyLabel;
    TLabel *BackupFreeSpaceLabel;
    TBevel *Bevel6;
    TLabel *Label3;
    TCGauge *BackupGauge;
    TLabel *Label8;
    TCheckBox *BackupCheckBox;
    TCheckBox *BackupSkipCheckBox;
    TGroupBox *GroupBox5;
    TButton *CDEjectButton;
    TButton *CDLoadButton;
    TBevel *Bevel7;
    TLabel *CDLabel;
    TSpeedButton *CancelButton;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall RefreshButtonClick(TObject *Sender);
    void __fastcall StartImportButtonClick(TObject *Sender);
    void __fastcall DiskPollTimerTimer(TObject *Sender);
    void __fastcall CloseButtonClick(TObject *Sender);
    void __fastcall BackupCheckBoxClick(TObject *Sender);
    void __fastcall BackupSkipCheckBoxClick(TObject *Sender);
    void __fastcall CDEjectButtonClick(TObject *Sender);
    void __fastcall CDLoadButtonClick(TObject *Sender);

private:	// User declarations

    Int64u_t        DatabaseFreeSize;
    Int64u_t        BackupFreeSize;

    Int64u_t        BytesToCopyFile;
    Int64u_t        BytesToCopyStudy;
    Int64u_t        BytesToCopyTotal;

    Int64u_t        TotalProgressTotal;
    Int64u_t        TotalProgressCurrent;

    Int64u_t        BytesCopiedFile;
    Int64u_t        BytesCopiedStudy;
    Int64u_t        BytesCopiedTotal;

    Int64u_t        BytesBackupStudy;
    Int64u_t        BytesBackupStudyDone;

    Int32u_t        LastUpdateTime;
    Int32u_t        LastRemainingTimeUpdate;
    Int32u_t        CopyStartTime;

    Int32u_t        EchoCountReady;
    Int32u_t        EchoCountNotReady;
    Int32u_t        EchoCountInDatabase;
    Int32u_t        EchoCount;

    Int32u_t        DiskDetected;
    Boolean_t       CancelButtonDown;
    Boolean_t       Valid;

    Boolean_t       BackupFlag;
    Boolean_t       BackupSkip;
    Boolean_t       BackupInProgress;

    Int32u_t        StatusImageIndex[8];

    // List of files as read from the disk
    qHead_t         FileQueue;
    qHead_p         File_q;

    qHead_t         EchoQueue;
    qHead_p         Echo_q;

    qHead_t         DoneQueue;
    qHead_p         Done_q;

    qHead_t         IgnoreNotReadyQueue;
    qHead_p         IgnoreNotReady_q;

    FILE           *Fp;
    TCursor         ScreenCursorOrig;
    TCursor         CursorOrig;

    void     __fastcall UpdateProgress( TObject *Sender, Boolean_t reset );
    void     __fastcall UpdateThermometers( TObject *Sender );
    void     __fastcall CDLabelUpdate( Char_p str_p, Boolean_t progressFlag );

    void     __fastcall EchoFilesListGet( TObject *Sender );
    void     __fastcall EchoFilesValidate( TObject *Sender );
    void     __fastcall EchoFilesSizeCompute( qHead_p echo_q );
    void     __fastcall EchoFilesCheckDatabase( qHead_p echo_q  );

    Int32s_t __fastcall EchoFilesCheck( qHead_p file_q, qHead_p echo_q );
    Int32s_t __fastcall EchoFilesStatusCount( qHead_p echo_q );

    Int32s_t __fastcall EchoFilesListFlatten
    (
        qHead_p     file_q
    );
    Int32s_t __fastcall EchoFilesListFlattenRecurse
    (
        qHead_p     file_q,
        qHead_p     temp_q
    );
    Int32s_t __fastcall EchoFilesDBCheck
    (
        mbFileListStruct_p  fileIn_p,
        qHead_p             echo_q,
        Boolean_p           addedFlag_p
    );
    Int32s_t  __fastcall EchoImportAddFile
    (
        Int32s_t    echoId,
        Char_p      source_p,
        Char_p      path_p,
        Int32u_t    size,
        Int32u_t    crc,
        Int32u_t    date,
        Int32u_t    time,
        Int32u_t    type,
        qHead_p     dirCache_q
    );
    Int32s_t __fastcall EchoListFree( qHead_p echo_q );
    void     __fastcall EchoFilesDelete( void );
    Int32s_t __fastcall CancelCheckCallbackReal( );
    Int32s_t __fastcall FileNameCallbackReal( Char_p fileName_p );
    Int32s_t __fastcall BytesCopiedCallbackReal( Int32u_t bytes );

public:		// User declarations
    __fastcall TEchoImportForm(TComponent* Owner);

    static Int32s_t     CancelCheckCallback( Void_p this_p );
    static Int32s_t     FileNameCallback( Void_p this_p, Char_p fileName_p );
    static Int32s_t     BytesCopiedCallback( Void_p this_p, Int32u_t bytes );
    static Int32s_t     CDLabelCallback( Void_p this_p, Char_p str_p, Int32u_t bytesFree );
};

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

void            pmcEchoImport( TCursor *cursor );

#endif // pmcEchoImportFormH
