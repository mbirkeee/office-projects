//---------------------------------------------------------------------------
// File:    pmcIcdForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    April 21, 2002
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcEchoDetailsFormH
#define pmcEchoDetailsFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>

typedef struct pmcEchoDetailsInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    echoId;
    Boolean_t   abortFlag;
} pmcEchoDetailsInfo_t, *pmcEchoDetailsInfo_p;

//---------------------------------------------------------------------------
class TEchoDetailsForm : public TForm
{
__published:	// IDE-managed Components
    TButton *Button_Cancel;
    TPageControl *PageControl1;
    TTabSheet *TabSheet_Measurements;
    TTabSheet *TabSheet_Notes;
    TGroupBox *GroupBox_AorticValve;
    TGroupBox *GroupBox_MitralValve;
    TEdit *Edit_av_apht;
    TEdit *Edit_av_ajl;
    TEdit *Edit_av_mv;
    TEdit *Edit_av_pg;
    TEdit *Edit_av_mg;
    TEdit *Edit_av_ld;
    TEdit *Edit_av_lv;
    TEdit *Edit_av_vti;
    TEdit *Edit_av_va;
    TEdit *Edit_mv_mrja;
    TEdit *Edit_mv_va;
    TEdit *Edit_mv_pev;
    TEdit *Edit_mv_pav;
    TEdit *Edit_mv_mg;
    TEdit *Edit_mv_ivrt;
    TEdit *Edit_mv_edt;
    TEdit *Edit_Comment;
    TButton *Button_Approve;
    TGroupBox *PatientGroupBox;
    TBevel *Bevel24;
    TLabel *PatientDobLabel;
    TBevel *Bevel23;
    TLabel *PatientPhnLabel;
    TBevel *Bevel22;
    TLabel *Label18;
    TLabel *Label23;
    TEdit *Edit_Patient;
    TBitBtn *Button_PatientSelect;
    TGroupBox *ReferringDrGroupBox;
    TEdit *Edit_ReferringDr;
    TBitBtn *Button_ReferringDrSelect;
    TGroupBox *ProviderGroupBox;
    TLabel *ProviderBillingNumberLabel;
    TComboBox *ComboBox_Provider;
    TGroupBox *GroupBoxStudyDetails;
    TLabel *Label3;
    TLabel *Label5;
    TLabel *Label4;
    TEdit *Edit_StudyName;
    TLabel *Label6;
    TBevel *Bevel1;
    TBevel *Bevel2;
    TBevel *Bevel3;
    TLabel *LabelEchoId;
    TLabel *LabelStudyDate;
    TLabel *LabelReadDate;
    TButton *Button_OK;
    TMemo *Memo1;
    TButton *Button_Preview;
    TButton *Button_Submit;
    TButton *Button_Print;
    TLabel *LabelChangeCount;
    TButton *Button_Save;
    TLabel *LabelPatientGender;
    TGroupBox *GroupBox_Sonographer;
    TLabel *Label2;
    TLabel *LabelSonographer;
    TBevel *Bevel4;
    TLabel *Label7;
    TLabel *Label10;
    TBevel *Bevel5;
    TBevel *Bevel6;
    TLabel *LabelDrPhone;
    TLabel *LabelDrFax;
    TGroupBox *GroupBox_CardiacDimensions;
    TLabel *Label1;
    TLabel *Label8;
    TLabel *Label9;
    TLabel *Label11;
    TLabel *Label12;
    TLabel *Label13;
    TLabel *Label14;
    TLabel *Label15;
    TLabel *Label16;
    TLabel *Label17;
    TEdit *Edit_cd_rv;
    TEdit *Edit_cd_aa;
    TEdit *Edit_cd_lved;
    TEdit *Edit_cd_la;
    TEdit *Edit_cd_lves;
    TEdit *Edit_cd_sept;
    TEdit *Edit_cd_pw;
    TEdit *Edit_cd_mi;
    TEdit *Edit_cd_lvef;
    TLabel *Label19;
    TLabel *Label20;
    TLabel *Label21;
    TLabel *Label22;
    TLabel *Label24;
    TLabel *Label25;
    TLabel *Label26;
    TLabel *Label27;
    TLabel *Label28;
    TLabel *Label29;
    TLabel *Label30;
    TLabel *Label31;
    TLabel *Label33;
    TLabel *Label34;
    TLabel *Label35;
    TLabel *Label36;
    TLabel *Label37;
    TLabel *Label38;
    TComboBox *ComboBox_av_reg;
    TComboBox *ComboBox_av_sten;
    TComboBox *ComboBox_mv_reg;
    TComboBox *ComboBox_mv_sten;
    TLabel *Label39;
    TLabel *Label40;
    TLabel *Label41;
    TLabel *Label42;
    TLabel *Label43;
    TLabel *Label44;
    TLabel *Label45;
    TLabel *Label46;
    TLabel *Label47;
    TLabel *Label48;
    TLabel *Label49;
    TLabel *Label50;
    TLabel *Label51;
    TLabel *Label52;
    TLabel *Label53;
    TLabel *Label54;
    TLabel *Label55;
    TLabel *Label56;
    TLabel *Label57;
    TLabel *Label58;
    TLabel *Label59;
    TLabel *Label60;
    TLabel *Label61;
    TLabel *Label62;
    TLabel *Label63;
    TLabel *Label64;
    TLabel *Label65;
    TLabel *Label66;
    TLabel *Label67;
    TLabel *Label68;
    TGroupBox *GroupBox_PulmonicValve;
    TComboBox *ComboBox_pv_reg;
    TLabel *Label69;
    TEdit *Edit_pv_vel;
    TEdit *Edit_pv_pat;
    TEdit *Edit_pv_grad;
    TGroupBox *GroupBox_PulmonicVeinFlow;
    TEdit *Edit_pvf_laa;
    TEdit *Edit_pvf_ar;
    TEdit *Edit_pvf_dia;
    TEdit *Edit_pvf_sys;
    TGroupBox *GroupBox_TricuspidValve;
    TComboBox *ComboBox_tv_reg;
    TLabel *Label70;
    TEdit *Edit_tv_trja;
    TEdit *Edit_tv_rvsp;
    TEdit *Edit_tv_ev;
    TComboBox *ComboBox_tv_sten;
    TEdit *Edit_tv_va;
    TGroupBox *GroupBox_RateRhythm;
    TComboBox *ComboBox_rhythm;
    TEdit *Edit_Indication;
    TComboBox *ComboBox_ImageQuality;
    TLabel *Label71;
    TLabel *Label32;
    TLabel *Label72;
    TLabel *Label73;
    TLabel *Label74;
    TLabel *Label75;
    TLabel *Label76;
    TLabel *Label77;
    TLabel *Label78;
    TLabel *Label79;
    TLabel *Label80;
    TLabel *Label81;
    TLabel *Label82;
    TLabel *Label83;
    TLabel *Label84;
    TLabel *Label85;
    TLabel *Label86;
    TLabel *Label87;
    TLabel *Label88;
    TLabel *Label89;
    TLabel *Label90;
    TLabel *Label91;
    TLabel *Label92;
    TLabel *Label93;
    TLabel *Label94;
    TLabel *Label95;
    TLabel *Label96;
    TLabel *Label97;
    TLabel *Label98;
    TLabel *Label99;
    TLabel *Label100;
    TLabel *Label101;
    TLabel *Label102;
    TLabel *Label103;
    TLabel *Label104;
    TLabel *Label105;
    TLabel *Label108;
    TGroupBox *GroupBox_Rate;
    TEdit *Edit_rate;
    TLabel *Label106;
    TLabel *Label107;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall Button_CancelClick(TObject *Sender);
    void __fastcall EchoDetailsChange(TObject *Sender);
    void __fastcall Button_OKClick(TObject *Sender);
    void __fastcall Button_ReferringDrSelectClick(TObject *Sender);
    void __fastcall Button_SaveClick(TObject *Sender);
    void __fastcall Memo1Change(TObject *Sender);
    void __fastcall Button_PatientSelectClick(TObject *Sender);
    void __fastcall ComboBox_ProviderChange(TObject *Sender);
    void __fastcall Button_PreviewClick(TObject *Sender);
    void __fastcall PageControl1Change(TObject *Sender);
    void __fastcall Button_SubmitClick(TObject *Sender);
    void __fastcall Button_ApproveClick(TObject *Sender);
 
private:	// User declarations

    MbSQL                   Sql;
    Char_p                  Cmd_p;
    PmcSqlEchoDetails_p     Echo_p;
    pmcEchoDetailsInfo_p    Info_p;

    Boolean_t               ControlUpdateInProgress;
    Boolean_t               ReadOnly;
    Boolean_t               MemoChanged;
    Int32u_t                ChangeCount;
    Boolean_t               OkFlag;
    Int32u_t                EchoId;

    void    __fastcall ReferringDrUpdate( void );
    void    __fastcall PatientUpdate( void );
    void    __fastcall ControlsUpdate( void );
    void    __fastcall ControlsGet( PmcSqlEchoDetails_p echo_p );
    void    __fastcall Save( Boolean_t reloadFlag, Boolean_t promptFlag );
    Float_t __fastcall StrToFloat( Char_p str_p );
    void    __fastcall ControlsDisable( void );
    void    __fastcall AssignPatient( PmcSqlEchoDetails_p echo_p );

public:		// User declarations

    __fastcall TEchoDetailsForm(TComponent* Owner);
    __fastcall TEchoDetailsForm(TComponent* Owner, pmcEchoDetailsInfo_p info_p );
};

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t    pmcEchoDetailsForm( Int32u_t patientId );
Int32s_t    pmcEchoReportGenerate( Int32u_t             echoId,
                                   PmcSqlEchoDetails_p  echoIn_p,
                                   Char_p               watermark_p,
                                   Boolean_t            viewFlag,
                                   Boolean_t            importFlag );

#endif //  pmcEchoDetailsFormH

