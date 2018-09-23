//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"
#include "pmcTables.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcEchoPatSelectForm.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TEchoPatSelectForm::TEchoPatSelectForm(TComponent* Owner)
    : TForm(Owner)
{
}

__fastcall TEchoPatSelectForm::TEchoPatSelectForm
(
    TComponent                 *Owner,
    pmcEchoPatSelectFormInfo_p  info_p
) : TForm(Owner)
{
    MbDateTime                  dateTime;
    Char_p                      buf_p;
    Char_p                      buf2_p;
    MbSQL                       sql;
    Int32u_t                    count, i;
    Int32u_t                    foundCount = 0;

    count = 0;

    Caption = "Assign Echo to Patient";

    for( i = 0 ; i < PMC_PAT_BUTTON_COUNT ; i++ )
    {
        PatId[i] = 0;
    }

    mbMalloc( buf_p, 1024 );
    mbMalloc( buf2_p, 512 );

    PatButton_p[0] = Button0;
    PatButton_p[1] = Button1;
    PatButton_p[2] = Button2;
    PatButton_p[3] = Button3;
    PatButton_p[4] = Button4;
    PatButton_p[5] = Button5;
    PatButton_p[6] = Button6;
    PatButton_p[7] = Button7;
    PatButton_p[8] = Button8;
    PatButton_p[9] = Button9;

    dateTime.SetDate( info_p->date );

    Info_p = info_p;
    Info_p->returnCode = MB_BUTTON_CANCEL;

    if( Info_p->cancelButton == FALSE )
    {
        ButtonCancel->Visible = FALSE;
        sprintf( buf_p, "Cannot determine patient for this echo.\n"
                    "Possible reasons include:\n\n"
                    "- No patient names match the echo name.\n"
                    "- Multiple patient names match the echo name.\n\n"
                    "The following patients had echo appointments on %s.\n"
                    "Choose the patient to which this echo belongs (if shown),\n"
                    "or click 'No Match'.",
                    dateTime.MDY_DateString( ) );
    }
    else
    {
        sprintf( buf_p, "Cannot determine patient for this echo.\n"
                    "Possible reasons include:\n\n"
                    "- No patient names match the echo name.\n"
                    "- Multiple patient names match the echo name.\n\n"
                    "The following patients had echo appointments on %s.\n"
                    "Choose the patient to which this echo belongs (if shown),\n"
                    "or click 'No Match'.  Click 'Cancel' to stop processing echos.",
                    dateTime.MDY_DateString( ) );
    }

    LabelEchoName->Caption = info_p->name_p;
    LabelEchoDate->Caption = dateTime.MDY_DateString( );
    LabelExplain->Caption = buf_p;

    sprintf( buf_p, "select appointments.id,patients.id,patients.first_name,patients.last_name from appointments,patients where "
                     "appointments.provider_id=%lu and appointments.date=%lu and appointments.not_deleted=1 "
                     "and patients.id=appointments.patient_id",
                     info_p->providerId, info_p->date );

    mbLog( buf_p );

    sql.Query( buf_p );
    while( sql.RowGet( ) )
    {
        if( sql.Int32u( 1 ) == 0 ) continue;
        sprintf( buf2_p, "%s %s", sql.String( 2 ), sql.String( 3 ) );
        PatButton_p[count]->Caption = buf2_p;
        PatId[count] = sql.Int32u( 1 );

        count++;
        foundCount++;
        if( count >= PMC_PAT_BUTTON_COUNT ) break;
    }

    for( ; count < PMC_PAT_BUTTON_COUNT ; count++ )
    {
        PatButton_p[count]->Caption = "";
        PatButton_p[count]->Visible = FALSE;
    }

    info_p->abortFlag = FALSE;
    if( foundCount == 0 )
    {
        mbDlgInfo( "Study Name: %s\n\n"
                   "Unable to determine patient to which this echo belongs.\n"
                   "No echo appointments were found on %s.\n",
                        ( info_p->name_p == NIL ) ? "NIL" : info_p->name_p,
                        dateTime.MDY_DateString( ) );
        info_p->abortFlag = TRUE;
        Info_p->returnCode = MB_BUTTON_OK;
    }

    mbFree( buf_p );
    mbFree( buf2_p );
}

//---------------------------------------------------------------------------

void __fastcall TEchoPatSelectForm::ButtonCancelClick(TObject *Sender)
{
    Info_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TEchoPatSelectForm::ButtonNoMatchClick(TObject *Sender)
{
    Info_p->patientId = 0;
    Info_p->returnCode = MB_BUTTON_OK;
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TEchoPatSelectForm::ButtonClick( Int32u_t index )
{
    Info_p->patientId = PatId[index];
    Info_p->returnCode = MB_BUTTON_OK;
    Close( );
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TEchoPatSelectForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TEchoPatSelectForm::Button0Click(TObject *Sender) { ButtonClick( 0 ); }
void __fastcall TEchoPatSelectForm::Button1Click(TObject *Sender) { ButtonClick( 1 ); }
void __fastcall TEchoPatSelectForm::Button2Click(TObject *Sender) { ButtonClick( 2 ); }
void __fastcall TEchoPatSelectForm::Button3Click(TObject *Sender) { ButtonClick( 3 ); }
void __fastcall TEchoPatSelectForm::Button4Click(TObject *Sender) { ButtonClick( 4 ); }
void __fastcall TEchoPatSelectForm::Button5Click(TObject *Sender) { ButtonClick( 5 ); }
void __fastcall TEchoPatSelectForm::Button6Click(TObject *Sender) { ButtonClick( 6 ); }
void __fastcall TEchoPatSelectForm::Button7Click(TObject *Sender) { ButtonClick( 7 ); }
void __fastcall TEchoPatSelectForm::Button8Click(TObject *Sender) { ButtonClick( 8 ); }
void __fastcall TEchoPatSelectForm::Button9Click(TObject *Sender) { ButtonClick( 9 ); }

