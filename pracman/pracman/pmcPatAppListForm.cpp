//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "pmcPatAppListForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPatAppListForm *PatAppListForm;
//---------------------------------------------------------------------------
__fastcall TPatAppListForm::TPatAppListForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
