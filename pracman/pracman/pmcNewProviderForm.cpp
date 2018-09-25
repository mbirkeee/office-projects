//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcNewProviderForm.h"
#include "pmcTables.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProviderDetailsForm *ProviderDetailsForm;
//---------------------------------------------------------------------------
__fastcall TProviderDetailsForm::TProviderDetailsForm(TComponent* Owner)
    : TForm(Owner)
{
    TDataSet   *dataSet_p;
    AnsiString  str = PMC_SQL_CMD_PROVIDERS_MAX_ID;
    Int32u_t    id;

    dataSet_p = DataSource1->DataSet;

    // Ensure SQL command is clear
    Query1->SQL->Clear( );
    Query1->SQL->Add( str );

    // Send the SQL command
    Query1->Close( );
    Query1->Open( );

    dataSet_p->DisableControls( );

    try
    {
        dataSet_p->First( );
        while( !dataSet_p->Eof )
        {
            id = dataSet_p->FieldValues[ PMC_SQL_FIELD_ID ];

            mbDlgDebug(( "Got provider id: %d\n", id ));
            dataSet_p->Next( );
        }
    }
    __finally
    {
        dataSet_p->EnableControls( );
    }
    mbDlgDebug(( "TProviderDetailsForm constructor called" ));

}
//---------------------------------------------------------------------------
void __fastcall TProviderDetailsForm::OkButtonClick(TObject *Sender)
{
    Release( );    
}
//---------------------------------------------------------------------------
void __fastcall TProviderDetailsForm::CancelButtonClick(TObject *Sender)
{
    Release( );    
}

