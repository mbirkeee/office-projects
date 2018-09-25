//---------------------------------------------------------------------------
// File:    pmcDoctorEditForm.ch
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcDoctorEditFormH
#define pmcDoctorEditFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>

// Used to pass parameters to the doctor edit form
typedef struct pmcDocEditInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    mode;
    Int32u_t    id;
    Char_t      caption[256];
} pmcDocEditInfo_t, *pmcDocEditInfo_p;

class TDoctorEditForm : public TForm
{
__published:	// IDE-managed Components
    TMaskEdit *LastNameEdit;
    TBevel *Bevel1;
    TMaskEdit *FirstNameEdit;
    TMaskEdit *PostalCodeEdit;
    TMaskEdit *Address1Edit;
    TMaskEdit *Address2Edit;
    TMaskEdit *CityEdit;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TMaskEdit *CommentEdit;
    TLabel *Label5;
    TComboBox *SpecialtyCombo;
    TComboBox *ProvList;
    TLabel *Label7;
    TLabel *Label8;
    TMaskEdit *CountryEdit;
    TLabel *Label9;
    TMaskEdit *PhoneEdit;
    TLabel *Label10;
    TLabel *Label11;
    TLabel *Label12;
    TMaskEdit *MspNumberEdit;
    TLabel *Label15;
    TLabel *Label16;
    TLabel *Label17;
    TLabel *Label18;
    TButton *CancelButton;
    TButton *OkButton;
    TMaskEdit *IdEdit;
    TMaskEdit *ModifiedEdit;
    TMaskEdit *FaxEdit;
    TMaskEdit *EmailEdit;
    TMaskEdit *CreatedEdit;
    TMaskEdit *PhoneAreaEdit;
    TMaskEdit *FaxAreaEdit;
    TButton *AddressLabelButton;
    TGroupBox *GroupBox1;
    TMaskEdit *OtherDoctorNumberEdit;
    TCheckBox *CancerClinicCheckBox;
    TGroupBox *GroupBox2;
    TLabel *Label6;
    TLabel *Label13;
    TCheckBox *MspNumberActiveCheckBox;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall AddressLabelButtonClick(TObject *Sender);
    void __fastcall MspNumberEditExit(TObject *Sender);
 
private:	// User declarations

    pmcDocEditInfo_p    DocEditInfo_p;
    Int32u_t            DoctorId;
    Int32u_t            MspNumber;

public:		// User declarations
    __fastcall ~TDoctorEditForm( void );
    __fastcall TDoctorEditForm(TComponent* Owner);
    __fastcall TDoctorEditForm
    (
        TComponent         *Owner,
        pmcDocEditInfo_p   docEditInfo_p
    );
};

#endif
