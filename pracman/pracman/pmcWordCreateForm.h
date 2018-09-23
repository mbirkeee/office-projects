//---------------------------------------------------------------------------
// File:    pmcWordCreateForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    September 11, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcWordCreateFormH
#define pmcWordCreateFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------

typedef struct pmcWordCreateFormInfo_s
{
    Int32s_t        returnCode;
    Int32u_t        patientId;
    Int32u_t        doctorId;
    Int32u_t        providerId;
} pmcWordCreateFormInfo_t, *pmcWordCreateFormInfo_p;


class TWordCreateForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CancelButton;
    TButton *OkButton;
    TGroupBox *PatientGroupBox;
    TGroupBox *DoctorGroupBox;
    TEdit *PatientEdit;
    TEdit *DoctorEdit;
    TBitBtn *PatientListButton;
    TBitBtn *DoctorListButton;
    TLabel *Label2;
    TLabel *PhnLabel;
    TLabel *Label4;
    TLabel *BirthDateLabel;
    TLabel *Label6;
    TButton *PatientClearButton;
    TButton *PatientEditButton;
    TButton *DoctorClearButton;
    TButton *DoctorEditButton;
    TLabel *DrAddr1Label;
    TLabel *DrAddr2Label;
    TLabel *DrAddr3Label;
    TLabel *DrFaxLabel;
    TLabel *DrPhoneLabel;
    TLabel *Label1;
    TLabel *Label3;
    TLabel *Label5;
    TLabel *PatAddr1Label;
    TLabel *PatAddr2Label;
    TLabel *PatAddr3Label;
    TLabel *DrNameLabel;
    TLabel *PatNameLabel;
    TLabel *Label7;
    TLabel *Label8;
    TLabel *Label9;
    TLabel *PatPhoneLabel;
    TGroupBox *GroupBox1;
    TButton *AppClearButton;
    TButton *AppSelectButton;
    TLabel *Label10;
    TLabel *Label11;
    TLabel *Label12;
    TLabel *AppDateLabel;
    TLabel *AppTimeLabel;
    TLabel *AppProviderLabel;
    TGroupBox *GroupBox3;
    TComboBox *TemplateComboBox;
    TEdit *DateEdit;
    TBitBtn *DateSelectButton;
    TEdit *CreateDirectoryEdit;
    TBitBtn *CreateDirectoryButton;
    TEdit *DescriptionEdit;
    TLabel *Label13;
    TLabel *Label14;
    TLabel *Label15;
    TLabel *Label16;
    TComboBox *ProviderComboBox;
    TLabel *Label17;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall PatientListButtonClick(TObject *Sender);
    void __fastcall CreateDirectoryButtonClick(TObject *Sender);
    void __fastcall CreateDirectoryEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall DateSelectButtonClick(TObject *Sender);
    void __fastcall DoctorListButtonClick(TObject *Sender);
    void __fastcall DoctorEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall DateEditMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall DateEditChange(TObject *Sender);
    void __fastcall PatientEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PatientClearButtonClick(TObject *Sender);
    void __fastcall DoctorClearButtonClick(TObject *Sender);
    void __fastcall PatientEditButtonClick(TObject *Sender);
    void __fastcall DoctorEditButtonClick(TObject *Sender);
    void __fastcall PatientEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall AppSelectButtonClick(TObject *Sender);
    void __fastcall AppClearButtonClick(TObject *Sender);
    void __fastcall TemplateComboBoxChange(TObject *Sender);

private:	// User declarations
    void __fastcall CreateMergeFile( Char_p  mergeFileName_p );
    void __fastcall AddField( Char_p buf_p, Char_p field_p, Int32u_t commaFlag );
    void __fastcall PatientSelect( Int32u_t key );

    void __fastcall UpdateDirectory( void  );
    void __fastcall UpdateDoctor( Int32u_t doctorId );
    void __fastcall UpdatePatient( Int32u_t patientId );
    void __fastcall UpdateAppointment( Int32u_t appointId );
    void __fastcall UpdateDate( Int32u_t date );

    Char_p                      CreateDir_p;

    pmcWordCreateFormInfo_p     FormInfo_p;
    qHead_t                     FileListHead;
    qHead_p                     File_q;

    Int32u_t                    DateOfBirth;
    Int32u_t                    Date;
    Int32u_t                    SelectedAppointId;
    Int32u_t                    PatGenderMale;

    Char_t                      PatFirstName[128];
    Char_t                      PatLastName[128];
    Char_t                      Phn[32];
    Char_t                      PhnProv[32];
    Char_t                      PhnFormat[32];
    Char_t                      PatCity[128];
    Char_t                      PatProvince[128];
    Char_t                      PatPostalCode[32];
    Char_t                      PatAddress1[128];
    Char_t                      PatAddress2[128];
    Char_t                      PatAddress3[128];
    Char_t                      PatPhone[64];
    Char_t                      PatTitle[32];
    Char_p                      PatAddress_p[4];
    Int32u_t                    PatGender;

    Int32u_t                    AppDate;
    Int32u_t                    AppTime;
    Char_t                      AppProvider[128];

    Char_t                      DrFirstName[128];
    Char_t                      DrLastName[128];
    Char_t                      DrCity[128];
    Char_t                      DrProvince[128];
    Char_t                      DrPostalCode[32];
    Char_t                      DrAddress1[128];
    Char_t                      DrAddress2[128];
    Char_t                      DrAddress3[128];
    Char_t                      DrFax[64];
    Char_t                      DrPhone[64];

    Char_p                      DrAddress_p[4];

    Char_t                      NullString[4];

public:		// User declarations
    __fastcall TWordCreateForm(TComponent* Owner);
    __fastcall TWordCreateForm(TComponent* Owner, pmcWordCreateFormInfo_p formInfo_p);

};

#define PMC_WORD_CREATE_MERGE_FILE "merge.txt"

Int32s_t pmcWordCreate( Int32u_t patientId, Int32u_t providerId );

#endif
