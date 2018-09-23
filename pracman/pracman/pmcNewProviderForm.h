//---------------------------------------------------------------------------
#ifndef pmcNewProviderFormH
#define pmcNewProviderFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Db.hpp>
#include <DBTables.hpp>
#include <DB.hpp>
//---------------------------------------------------------------------------
class TProviderDetailsForm : public TForm
{
__published:	// IDE-managed Components
    TEdit *FirstNameEdit;
    TEdit *LastNameEdit;
    TEdit *DescriptionEdit;
    TLabel *FirstNameLable;
    TBevel *Bevel1;
    TEdit *CreatedEdit;
    TButton *OkButton;
    TButton *CancelButton;
    TLabel *DescriptionLabel;
    TLabel *ModifiedLabel;
    TBevel *Bevel3;
    TLabel *LastNameLable;
    TEdit *ModifiedEdit;
    TLabel *CreatedLabel;
    TQuery *Query1;
    TDataSource *DataSource1;
    TEdit *BillingCodeEdit;
    TLabel *BillingCodeLabel;
    TEdit *ClinicCodeEdit;
    TLabel *ClinicCodeLabel;
    TBevel *Bevel2;
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);

private:	// User declarations
public:		// User declarations
    __fastcall TProviderDetailsForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TProviderDetailsForm *ProviderDetailsForm;
//---------------------------------------------------------------------------
#endif
