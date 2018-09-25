//---------------------------------------------------------------------------
// File:    pmcDoctorEditForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for managing the doctor list form
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcUtils.h"
#include "pmcDoctorEditForm.h"
#include "pmcSeiko240.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
__fastcall TDoctorEditForm::~TDoctorEditForm( void )
{
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
__fastcall TDoctorEditForm::TDoctorEditForm(TComponent* Owner)
    : TForm(Owner)
{
    mbDlgDebug(( "DoctorEditForm default Constructor called" ));
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
__fastcall TDoctorEditForm::TDoctorEditForm
(
    TComponent         *Owner,
    pmcDocEditInfo_p   docEditInfo_p
) : TForm(Owner)
{
    Char_p              buf_p;
    Char_p              phone_p;
    Char_p              buf2_p;
    Char_p              buf3_p;
    Char_t              area[8];
    Boolean_t           localFlag;
    Int32u_t            i;
    Int32u_t            cancerClinic;
    Int32u_t            mspActive;
    MbDateTime          dateTime;
    MbSQL               sql;
    Boolean_t           result = FALSE;

    mbMalloc( buf_p, 512 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf3_p, 256 );
    mbMalloc( phone_p, 64 );

    if( docEditInfo_p )
    {
        DocEditInfo_p = docEditInfo_p;
    }
    else
    {
        mbDlgDebug(( "Error: Called without a doctor edit info struct" ));
        return;
    }
    DocEditInfo_p->returnCode = MB_BUTTON_CANCEL;

    Caption = DocEditInfo_p->caption;
    DoctorId = DocEditInfo_p->id;

    sprintf( buf_p, "select * from %s where %s=%ld",
        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_FIELD_ID, DoctorId );

    // Default - Set all controls disabled
    // DegreesEdit->Enabled = FALSE;
    SpecialtyCombo->Enabled = FALSE;
    MspNumberEdit->Enabled = FALSE;
    OtherDoctorNumberEdit->Enabled = FALSE;
    FirstNameEdit->Enabled = FALSE;
    LastNameEdit->Enabled = FALSE;
    Address1Edit->Enabled = FALSE;
    Address2Edit->Enabled = FALSE;
    CityEdit->Enabled = FALSE;
    CountryEdit->Enabled = FALSE;
    PostalCodeEdit->Enabled = FALSE;
    ProvList->Enabled = FALSE;
    EmailEdit->Enabled = FALSE;
    CommentEdit->Enabled = FALSE;
    IdEdit->Enabled = FALSE;
    CreatedEdit->Enabled = FALSE;
    ModifiedEdit->Enabled = FALSE;
    PhoneEdit->Enabled = FALSE;
    FaxEdit->Enabled = FALSE;
    PhoneAreaEdit->Enabled = FALSE;
    FaxAreaEdit->Enabled = FALSE;
    AddressLabelButton->Enabled = FALSE;
    CancerClinicCheckBox->Enabled = FALSE;
    MspNumberActiveCheckBox->Enabled = FALSE;

    i = 0;

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        // Get the strings
        FirstNameEdit->Text = sql.NmString( PMC_SQL_FIELD_FIRST_NAME );
        LastNameEdit->Text  = sql.NmString( PMC_SQL_FIELD_LAST_NAME );
        Address1Edit->Text  = sql.NmString( PMC_SQL_FIELD_ADDRESS1 );
        Address2Edit->Text  = sql.NmString( PMC_SQL_FIELD_ADDRESS2 );
        CityEdit->Text      = sql.NmString( PMC_SQL_FIELD_CITY );
        CountryEdit->Text   = sql.NmString( PMC_SQL_FIELD_COUNTRY );
        PostalCodeEdit->Text= sql.NmString( PMC_SQL_FIELD_POSTAL_CODE );
        ProvList->Text      = sql.NmString( PMC_SQL_FIELD_PROVINCE );
        EmailEdit->Text     = sql.NmString( PMC_SQL_DOCTORS_FIELD_EMAIL );
        CommentEdit->Text   = sql.NmString( PMC_SQL_DOCTORS_FIELD_COMMENT );
        SpecialtyCombo->Text= sql.NmString( PMC_SQL_DOCTORS_FIELD_SPECIALTY );
        OtherDoctorNumberEdit->Text = sql.NmString( PMC_SQL_DOCTORS_FIELD_OTHER_NUMBER );
        MspNumber           = sql.NmInt32u( PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER );
        cancerClinic        = sql.NmInt32u( PMC_SQL_DOCTORS_FIELD_CANCER_CLINIC );
        mspActive           = sql.NmInt32u( PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST );

        // Set the cancer clinic check box
        CancerClinicCheckBox->Checked = ( cancerClinic == PMC_SQL_TRUE_VALUE ) ? TRUE : FALSE;

        // Set MSP number active check box
        MspNumberActiveCheckBox->Checked = ( mspActive == PMC_SQL_TRUE_VALUE ) ? TRUE : FALSE;

        // Creation time
        dateTime.SetDateTime64( sql.NmDateTimeInt64u( PMC_SQL_FIELD_CREATED ) );

        if( dateTime.Date() && DoctorId != 0)
        {
            sprintf( buf_p, "%s %s", dateTime.YMD_DateString( ), dateTime.HMS_TimeString( ) );
        }
        else
        {
            *buf_p = 0;
        }
        CreatedEdit->Text = buf_p;

        // Modify time
        dateTime.SetDateTime64( sql.NmDateTimeInt64u( PMC_SQL_FIELD_MODIFIED ) );
        if( dateTime.Date() && DoctorId != 0 )
        {
            sprintf( buf_p, "%s %s", dateTime.YMD_DateString( ), dateTime.HMS_TimeString( ) );
        }
        else
        {
            *buf_p = 0;
        }
        ModifiedEdit->Text = buf_p;

        // Phone
        strcpy( buf_p, sql.NmString( PMC_SQL_DOCTORS_FIELD_WORK_PHONE ) );
        pmcPhoneFix( buf_p, NIL );
        if( strlen( buf_p ) > 0 )
        {
            pmcPhoneFormat( buf_p, area, phone_p, &localFlag );
            PhoneEdit->Text = phone_p;
            PhoneAreaEdit->Text = area;
        }
        else
        {
            PhoneAreaEdit->Text = pmcCfg[CFG_AREA_CODE].str_p;
        }

        // Fax
        strcpy( buf_p,  sql.NmString( PMC_SQL_DOCTORS_FIELD_WORK_FAX )  );
        if( strlen( buf_p ) > 0 )
        {
            pmcPhoneFormat( buf_p, area, phone_p, &localFlag );
            FaxEdit->Text = phone_p;
            FaxAreaEdit->Text = area;
        }
        else
        {
            FaxAreaEdit->Text = pmcCfg[CFG_AREA_CODE].str_p;
        }

        i++;
    }

    // Sanity check
    if( i != 1 )  mbDlgDebug(( "Error reading doctor id: %ld", DoctorId));

    // Convert doctor number to text
    sprintf( buf_p, "%ld", MspNumber );
    MspNumberEdit->Text = buf_p;

    // Convert doctor ID to text
    sprintf( buf_p, "%ld", DoctorId );
    IdEdit->Text = buf_p;

    // Enable selected controls
    if( DocEditInfo_p->mode == PMC_EDIT_MODE_VIEW )
    {
        if( pmcCfg[CFG_SLP_NAME].str_p )
        {
            AddressLabelButton->Enabled = TRUE;
        }
        OkButton->Visible = FALSE;
        CancelButton->Caption = "Close";
    }

    if( DocEditInfo_p->mode == PMC_EDIT_MODE_EDIT ||
        DocEditInfo_p->mode == PMC_EDIT_MODE_NEW )
    {
        FirstNameEdit->Enabled = TRUE;
        LastNameEdit->Enabled = TRUE;
        MspNumberEdit->Enabled = TRUE;
        OtherDoctorNumberEdit->Enabled = TRUE;
        Address1Edit->Enabled = TRUE;
        Address2Edit->Enabled = TRUE;
        CityEdit->Enabled = TRUE;
        CountryEdit->Enabled = TRUE;
        PostalCodeEdit->Enabled = TRUE;
        ProvList->Enabled = TRUE;
        EmailEdit->Enabled = TRUE;
        CommentEdit->Enabled = TRUE;
        PhoneEdit->Enabled = TRUE;
        FaxEdit->Enabled = TRUE;
        PhoneAreaEdit->Enabled = TRUE;
        FaxAreaEdit->Enabled = TRUE;
        //DegreesEdit->Enabled = TRUE;
        CancerClinicCheckBox->Enabled = TRUE;
        SpecialtyCombo->Enabled = TRUE;
        MspNumberActiveCheckBox->Enabled = TRUE;
    }
    result = TRUE;

exit:
    if( result == FALSE )
    {
        mbDlgDebug(( "error" ));
    }
    mbFree( buf_p );
    mbFree( phone_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void __fastcall TDoctorEditForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Action = caFree;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TDoctorEditForm::OkButtonClick(TObject *Sender)
{
    Char_p      buf_p;
    Char_p      buf2_p;
    Int32u_t    newMspNumber;
    bool        close = FALSE;

    mbMalloc( buf_p, 1024 );
    mbMalloc( buf2_p, 512 );

    DocEditInfo_p->returnCode = MB_BUTTON_OK;

    // Before proceeding with any kind of update, check the doctor number
    strcpy( buf2_p, MspNumberEdit->Text.c_str() );
    mbStrDigitsOnly( buf2_p );
    newMspNumber = atol( buf2_p );

    mbStrClean( ProvList->Text.c_str(), buf_p, TRUE );
    pmcProvinceFix( buf_p, NIL );

    // Check the doctor number
    // 20020316: Allow a doctor number of 0 to be saved.
    if( newMspNumber == 0 )
    {
        if( mbDlgOkCancel( "MSP number is 0. Continue saving record?" ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }

        // Force the MSP active indicator to 0
        MspNumberActiveCheckBox->Checked = FALSE;
    }

     if( newMspNumber > PMC_MAX_SASK_DR_NUMBER )
    {
        if( strcmp( buf_p, PMC_DEFAULT_PROVINCE ) == 0 )
        {
            mbDlgExclaim( "Cannot create a doctor record with MSP number > %ld.", PMC_MAX_SASK_DR_NUMBER );
            goto exit;
        }
    }

    // Check the first name
    strcpy( buf_p, LastNameEdit->Text.c_str() );
    mbStrClean( buf_p, buf2_p, TRUE );
    {
        if( strlen( buf2_p ) == 0 )
        {
            mbDlgExclaim( "A last name must be entered for this doctor." );
            goto exit;
        }
    }

    // Check the first name
    strcpy( buf_p, FirstNameEdit->Text.c_str() );
    mbStrClean( buf_p, buf2_p, TRUE );
    {
        if( strlen( buf2_p ) == 0 )
        {
            mbDlgExclaim( "A first name must be entered for this doctor." );
            goto exit;
        }
    }

    strcpy( buf_p, PostalCodeEdit->Text.c_str() );
    mbStrClean( mbStrAlphaNumericOnly( buf_p ), buf2_p, TRUE );
    if( strlen( buf2_p ) > 0 )
    {
        if( pmcPostalCodeFix( buf2_p, NIL ) == FALSE )
        {
            mbDlgExclaim( "Invalid postal code." );
            goto exit;
        }
    }

    if( strlen( PhoneEdit->Text.c_str() ) )
    {
        sprintf( buf_p, "%s%s", PhoneAreaEdit->Text.c_str(), PhoneEdit->Text.c_str() );
        if( pmcPhoneFix( buf_p, NIL ) == FALSE )
        {
            mbDlgExclaim( "Invalid phone number." );
            goto exit;
        }
    }

    if( strlen( FaxEdit->Text.c_str() ) )
    {
        sprintf( buf_p, "%s%s", FaxAreaEdit->Text.c_str(), FaxEdit->Text.c_str() );
        if( pmcPhoneFix( buf_p, NIL ) == FALSE )
        {
            mbDlgExclaim( "Invalid fax number." );
            close = FALSE;
            goto exit;
        }
    }

    // Examine changes to the doctor number
    if( ( newMspNumber != MspNumber ) )
    {
        Int32u_t    count;

        if( newMspNumber != 0 )
        {
            sprintf( buf_p, "select %s from %s where %s=%ld",
                PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,
                PMC_SQL_TABLE_DOCTORS,
                PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,
                newMspNumber );

            count = pmcSqlMatchCount( buf_p );

            if( count > 0 )
            {
                mbDlgExclaim( "A doctor record with doctor number %ld already exists in the database.", newMspNumber );
                goto exit;
            }
        }

        if( MspNumber != 0 )
        {
            // Check to see if user really wants to do this
            if( mbDlgOkCancel( "Change the MSP number from %ld to %ld?\n",
                    MspNumber, newMspNumber ) == MB_BUTTON_CANCEL )
            {
                goto exit;
            }
        }
    }

    // At this point, proceed with updating the database
    if( DocEditInfo_p->mode == PMC_EDIT_MODE_NEW )
    {
        nbDlgDebug(( "Must create a new record" ));
        DoctorId = pmcSqlRecordCreate( PMC_SQL_TABLE_DOCTORS, NIL );
        if( DoctorId == 0 )
        {
            mbDlgDebug(( "Error creating doctor record" ));
            goto exit;
        }
        DocEditInfo_p->id = DoctorId;
    }

    if( DocEditInfo_p->mode == PMC_EDIT_MODE_NEW  ||
        DocEditInfo_p->mode == PMC_EDIT_MODE_EDIT  )
    {
        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_DOCTORS_FIELD_OTHER_NUMBER,
                          mbStrClean( OtherDoctorNumberEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_FIRST_NAME,
                          mbStrClean( FirstNameEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_LAST_NAME,
                          mbStrClean( LastNameEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_ADDRESS1,
                          mbStrClean( Address1Edit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_ADDRESS2,
                          mbStrClean( Address2Edit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_CITY,
                          mbStrClean( CityEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        mbStrClean( ProvList->Text.c_str(), buf_p, TRUE );
        pmcProvinceFix( buf_p, NIL );
        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_PROVINCE,
                          buf_p,
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_POSTAL_CODE,
                          mbStrClean( PostalCodeEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_FIELD_COUNTRY,
                          mbStrClean( CountryEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_DOCTORS_FIELD_EMAIL,
                          mbStrClean( EmailEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        sprintf( buf_p, "" );
        if( strlen( PhoneEdit->Text.c_str() ) )
        {
            sprintf( buf_p, "%s%s", PhoneAreaEdit->Text.c_str(), PhoneEdit->Text.c_str() );
            pmcPhoneFix( buf_p, NIL );
        }
        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_DOCTORS_FIELD_WORK_PHONE,
                          buf_p,
                          DoctorId );

        sprintf( buf_p, "" );
        if( strlen( FaxEdit->Text.c_str() ) )
        {
            sprintf( buf_p, "%s%s", FaxAreaEdit->Text.c_str(), FaxEdit->Text.c_str() );
            pmcPhoneFix( buf_p, NIL );
        }
        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_DOCTORS_FIELD_WORK_FAX,
                          buf_p,
                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_DOCTORS_FIELD_COMMENT,
                          mbStrClean( CommentEdit->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

//        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
//                          PMC_SQL_DOCTORS_FIELD_DEGREES,
//                          mbStrClean( DegreesEdit->Text.c_str(), buf_p, TRUE ),
//                          DoctorId );

        pmcSqlExecString( PMC_SQL_TABLE_DOCTORS,
                          PMC_SQL_DOCTORS_FIELD_SPECIALTY,
                          mbStrClean( SpecialtyCombo->Text.c_str(), buf_p, TRUE ),
                          DoctorId );

        pmcSqlExecInt( PMC_SQL_TABLE_DOCTORS,
                       PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,
                       newMspNumber,
                       DoctorId );

        pmcSqlExecInt( PMC_SQL_TABLE_DOCTORS,
                       PMC_SQL_DOCTORS_FIELD_CANCER_CLINIC,
                       ( CancerClinicCheckBox->Checked ) ? PMC_SQL_TRUE_VALUE : PMC_SQL_FALSE_VALUE,
                       DoctorId );

        pmcSqlExecInt( PMC_SQL_TABLE_DOCTORS,
                       PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST,
                       ( MspNumberActiveCheckBox->Checked ) ? PMC_SQL_TRUE_VALUE : PMC_SQL_FALSE_VALUE,
                       DoctorId );

        pmcSqlRecordUndelete( PMC_SQL_TABLE_DOCTORS, DoctorId );
    }

    close = TRUE;

exit:

    if( buf_p )  mbFree( buf_p );
    if( buf2_p ) mbFree( buf2_p );

    if( close == TRUE)  Close( );

    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
void __fastcall TDoctorEditForm::CancelButtonClick(TObject *Sender)
{
    DocEditInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}

void __fastcall TDoctorEditForm::AddressLabelButtonClick(TObject *Sender)
{
    pmcLabelPrintDrAddress( DoctorId );
}
//---------------------------------------------------------------------------



void __fastcall TDoctorEditForm::MspNumberEditExit(TObject *Sender)
{
    Char_t      buf[128];
    Int32u_t    newMspNumber;

    strcpy( buf, MspNumberEdit->Text.c_str() );
    mbStrDigitsOnly( buf );
    newMspNumber = atol( buf );
    if( ( newMspNumber != 0 ) && ( newMspNumber != MspNumber ) )
    {
        MspNumberActiveCheckBox->Checked = TRUE;
    }
    else
    {
        MspNumberActiveCheckBox->Checked = FALSE;
    }
}
//---------------------------------------------------------------------------

