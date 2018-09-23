//---------------------------------------------------------------------------
// File:    pmcAppHistoryForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    July 13, 2002
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcAppHistoryFormH
#define pmcAppHistoryFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>

#ifdef  External
#undef  External
#endif

#ifdef  PMC_INIT_GLOBALS
#define External
#else
#define External extern
#endif  /* PMC_INIT_GLOBALS */

//---------------------------------------------------------------------------
// Appointment History Actions
// ===========================
// These values are stored in the database and must not change.
//---------------------------------------------------------------------------

enum
{
     PMC_APP_ACTION_CREATE = 0              // 0
    ,PMC_APP_ACTION_DELETE
    ,PMC_APP_ACTION_CANCEL
    ,PMC_APP_ACTION_CUT
    ,PMC_APP_ACTION_PASTE
    ,PMC_APP_ACTION_MOVE                    // 5
    ,PMC_APP_ACTION_CONFIRM_LETTER
    ,PMC_APP_ACTION_CONFIRM_PHONE
    ,PMC_APP_ACTION_CONFIRM_LETTER_CANCEL
    ,PMC_APP_ACTION_CONFIRM_PHONE_CANCEL
    ,PMC_APP_ACTION_TYPE                    // 10
    ,PMC_APP_ACTION_PATIENT_ID
    ,PMC_APP_ACTION_REF_DR_ID
    ,PMC_APP_ACTION_COMMENT
    ,PMC_APP_ACTION_COMMENT_2TEST
    ,PMC_APP_ACTION_DURATION                // 15
    ,PMC_APP_ACTION_COMPLETE_SET
    ,PMC_APP_ACTION_COMPLETE_CLEAR
};

External Char_p pmcAppHistStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "CREATE"                               // 0
    ,"DELETE"
    ,"CANCEL"
    ,"CUT"
    ,"PASTE"
    ,"MOVE"                                 // 5
    ,"CONFIRM LETTER"
    ,"CONFIRM PHONE"
    ,"CANCEL LETTER CONF."
    ,"CANCEL PHONE CONF."
    ,"SET TYPE"                             // 10
    ,"SET PATIENT"
    ,"SET REFERRING DR"
    ,"SET COMMENT"
    ,"SET COMMENT 2"
    ,"SET DURATION"                         // 15
    ,"COMPLETE"
    ,"UNCOMPLETE"
}
#endif
;

typedef struct HistList_s
{
    qLinkage_t      linkage;
    Int32u_t        id;
    Int32u_t        appId;
    Int32u_t        date;
    Int32u_t        time;
    Int32u_t        action;
    Int32u_t        int_1;
    Int32u_t        int_2;
    Int32u_t        int_3;
    Int32u_t        int_4;
    Int32u_t        int_5;
    Char_p          string_1_p;
    Char_p          details_p;
} HistList_t, *HistList_p;

class TAppHistoryForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CloseButton;
    TBevel *Bevel1;
    TGroupBox *AppointmentGroupBox;
    TLabel *AppDateLabel;
    TLabel *AppTimeLabel;
    TLabel *AppProviderLabel;
    TBevel *Bevel9;
    TBevel *Bevel8;
    TBevel *Bevel7;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Label7;
    TGroupBox *PatientGroupBox;
    TLabel *PatPhnLabel;
    TBevel *Bevel3;
    TLabel *PatPhoneLabel;
    TBevel *Bevel4;
    TLabel *Label1;
    TLabel *Label2;
    TListView *ListView;
    TBevel *Bevel2;
    TLabel *AppIdLabel;
    TBevel *Bevel5;
    TLabel *PatNameLabel;
    TButton *PatientEditButton;
    TButton *PatientViewButton;
    TLabel *Label3;
    TBevel *Bevel6;
    TLabel *AppTypeLabel;
    TBevel *Bevel10;
    TBevel *Bevel11;
    TLabel *AppCommentLabel;
    TLabel *Label8;
    TLabel *Label9;
    TLabel *Label10;
    TLabel *Label11;
    TLabel *Label13;
    TBevel *Bevel12;
    TBevel *Bevel13;
    TBevel *Bevel14;
    TLabel *AppReferringLabel;
    TLabel *AppPhoneConfLabel;
    TLabel *AppLetterConfLabel;
    TLabel *Label14;
    TBevel *Bevel15;
    TLabel *PatIdLabel;
    TLabel *AppDurationLabel;
    TButton *ConfButton;
    void __fastcall CloseButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall ConfButtonClick(TObject *Sender);
    void __fastcall PatientEditButtonClick(TObject *Sender);
    void __fastcall PatientViewButtonClick(TObject *Sender);

private:	// User declarations
    void        __fastcall UpdateInfo( Int32u_t id );
    void        __fastcall UpdatePatient( Int32u_t id );
    void        __fastcall ListGet( Int32u_t appId );
    void        __fastcall ListFree( void );
    void        __fastcall PatientEditView( Int32u_t mode );
    Int32s_t    __fastcall FormatDetails( HistList_p entry_p );

    Int32u_t        AppId;
    Int32u_t        PatientId;
    qHead_t         ListHead;
    qHead_p         List_q;
    mbLock_t       ListLock;

public:		// User declarations
    __fastcall TAppHistoryForm(TComponent* Owner);
    __fastcall TAppHistoryForm(TComponent* Owner, Int32u_t appId );
};

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t    pmcAppHistory
(
    Int32u_t    appId,
    Int32u_t    action,
    Int32u_t    int_1,
    Int32u_t    int_2,
    Int32u_t    int_3,
    Int32u_t    int_4,
    Char_p      string_1_p
);

Int32s_t    pmcAppHistoryTime
(
    Int32u_t    appId,
    Int32u_t    date,
    Int32u_t    time,
    Int32u_t    action,
    Int32u_t    int_1,
    Int32u_t    int_2,
    Int32u_t    int_3,
    Int32u_t    int_4,
    Char_p      string_1_p
);

Int32s_t    pmcAppHistoryForm( Int32u_t appId );

#if PMC_APP_HISTORY_CREATE
void        pmcAppHistoryCreate( void );
#endif

#endif // pmcAppHistoryFormH
