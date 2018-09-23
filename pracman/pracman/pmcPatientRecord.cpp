//---------------------------------------------------------------------------
// Patient record utilities, primarily printing
//---------------------------------------------------------------------------
// Copyright Michael A. Bree, Nov. 23, 2008
//---------------------------------------------------------------------------

// Platform includes
#include <stdio.h>
#pragma hdrstop

// Library includes
#include "mbUtils.h"

// Program includes
#include "pmcUtils.h"
#include "pmcPDF.h"
#include "pmcPatient.h"
#include "pmcPatientRecord.h"
#include "pmcDocumentUtils.h"
#include "pmcGlobals.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Callback function returns true to Latex formatter if patient deceased,
// false otherwise.  The deceased date is passed into this function via
// the handle
//---------------------------------------------------------------------------

Int32s_t pmcPatRecordDeceasedCheck( Int32u_t handle, FILE *fp, Char_p key_p )
{
    // mbLog( "deceased callback called, key: %s", key_p );

    if( handle == 0 ) return FALSE;

    return TRUE;
}

//---------------------------------------------------------------------------
// Callback function
//---------------------------------------------------------------------------
Int32s_t pmcPatRecordDocFlag( Int32u_t handle, FILE *fp, Char_p key_p )
{
    Int32s_t            result = FALSE;
    Int32u_t            myCount = 0;
    Int32u_t            count, i;
    pmcDocumentItem_p  *doc_pp;
    pmcDocumentItem_p   doc_p;

    // Get list of documents
    doc_pp = pmcDocumentListGet( &count );

    for( i = 0 ; i < count ; i++ )
    {
        doc_p = doc_pp[i];
        if( doc_p->patientId == handle )
        {
            if( doc_p->notDeleted != 0 )
            {
                myCount++;
            }
        }
    }

    // Free List of documents
    mbFree( doc_pp );

    //mbLog( "pmcPatRecordDocFlag called with handle: %lu key: %s docCount: %lu myCount: %lu\n",
    //    handle, key_p, count, myCount );

    if( myCount > 0 ) result = TRUE;
    return result;
}

//---------------------------------------------------------------------------
// Callback function - Adds items to the list of documents in patient record
//---------------------------------------------------------------------------
Int32s_t pmcPatRecordDocList( Int32u_t handle, FILE *fp, Char_p key_p )
{
    Int32s_t            result = FALSE;
    Int32u_t            myCount = 0;
    Int32u_t            count, i;
    pmcDocumentItem_p  *doc_pp;
    pmcDocumentItem_p   doc_p;
    MbString            providerName;
    MbString            docName;
    MbString            docDescription;
    MbDateTime          dateTime;

    // Get list of documents
    doc_pp = pmcDocumentListGet( &count );

    for( i = 0 ; i < count ; i++ )
    {
        doc_p = doc_pp[i];
        if( doc_p->patientId == handle )
        {
            if( doc_p->notDeleted != 0 )
            {
                pmcProviderDescGet( doc_p->providerId, providerName.get() );
                dateTime.SetDate( doc_p->date );

                mbStrTex( doc_p->description_p, docDescription.get(), 30 );
                mbStrTex( doc_p->origName_p, docName.get(), 30 );

                fprintf( fp, "%s & %s & %s & %s & %s\\\\\n",
                    dateTime.MDY_DateString(),
                    docDescription.get(),
                    providerName.get(),
                    pmcDocumentStatusString( doc_p->status ),
                    docName.get() );

                myCount++;
            }
        }
    }

    // Free List of documents
    mbFree( doc_pp );

    //mbLog( "pmcPatRecordDocList called with handle: %lu key: %s docCount: %lu myCount: %lu\n",
    //    handle, key_p, count, myCount );

    if( myCount > 0 ) result = TRUE;
    return result;
}

//---------------------------------------------------------------------------
// Print PDF version of patient record.  If the data_q is not null, it
// contains the data from the dynaminc tabs in the patient edit form.
// If the data queue is NIL (e.g., not printing the record from the
// patient edit form), then the data should be read directly from the
// database.
//---------------------------------------------------------------------------
Int32s_t pmcPatDocumentPDF
(
    PmcPatient     *patient_p,
    qHead_p         data_q,
    Char_p          watermark_p,
    Char_p          template_p,
    PmcDocument    *consult_p,
    Boolean_t       viewFlag,
    MbString       *name_p
)
{
    MbCursor        cursor = MbCursor( crHourGlass );
    MbDateTime      dateTime = MbDateTime(  );
    PmcSqlDoctor_t  doctor;
    mbStrList_p     entry_p;
    Int32s_t        result = MB_RET_OK;

    // Sanity check
    if( patient_p == NIL )
    {
        mbDlgError( "Got NIL pointer to patient" );
        result = MB_RET_ERROR;
    }

    // Create document object
    MbPDF doc = MbPDF( template_p, patient_p->providerId( ) );

    if( result != MB_RET_OK ) goto exit;

    if( doc.check() != MB_RET_OK )
    {
        mbDlgError( "Error loading template file %s\n", template_p );
        goto exit;
    }

    doc.subString( "_WATERMARK_", ( watermark_p == NIL ) ? "" : watermark_p, 20 );
 
    // Put the data into the sub queue
    qWalk( entry_p, data_q, mbStrList_p )
    {
        if( entry_p->str_p == NIL || entry_p->str2_p == NIL )
        {
            mbLog( "Error detected NULL pointer in sub list: %s %s\n", entry_p->str2_p, entry_p->str_p );
        }
        else
        {
            doc.subString( entry_p->str2_p, entry_p->str_p, 0 );
        }
    }

    dateTime.SetDateTime( mbToday( ), mbTime( ) );
    doc.subString( "_PRINT_DATE_", dateTime.MDYHM_DateStringLong( ), 0 );
    doc.subDate( "_DATE_", dateTime.Date() );

    doc.subString( "_PAT_FIRST_NAME_",  patient_p->firstName(), 20 );
    doc.subString( "_PAT_LAST_NAME_",   patient_p->lastName(), 20 );
    doc.subString( "_PAT_MIDDLE_NAME_", patient_p->middleName(), 20 );
    doc.subString( "_PAT_TITLE_",       patient_p->title(), 20 );
    doc.subString( "_PAT_ADDRESS_1_",   patient_p->address1(), 40 );
    doc.subString( "_PAT_ADDRESS_2_",   patient_p->address2(), 40 );
    doc.subString( "_PAT_CITY_",        patient_p->city(), 40 );
    doc.subString( "_PAT_PROVINCE_",    patient_p->province(), 40 );
    doc.subString( "_PAT_COUNTRY_",     patient_p->country(), 40 );
    doc.subString( "_PAT_POSTAL_CODE_", patient_p->postalCode(), 40 );
    doc.subString( "_PAT_PHN_",         patient_p->phn( ), 20 );
    doc.subString( "_PAT_PHN_PROVINCE_",patient_p->phnProvince(), 40 );
    doc.subString( "_PAT_PHONE_HOME_",  patient_p->phoneHome(), 40 );
    doc.subString( "_PAT_PHONE_WORK_",  patient_p->phoneWork(), 40 );
    doc.subString( "_PAT_PHONE_CELL_",  patient_p->phoneCell(), 40 );
    doc.subString( "_PAT_BIRTH_DATE_",  patient_p->dateString( patient_p->dateOfBirth( )), 20 );
    doc.subString( "_PAT_AGE_",         patient_p->ageString( ), 10 );
    doc.subString( "_CONTACT_NAME_",    patient_p->contactName(), 40 );
    doc.subString( "_CONTACT_DESC_",    patient_p->contactDesc(), 40 );
    doc.subString( "_CONTACT_PHONE_",   patient_p->contactPhone(), 40 );
    doc.subString( "_PAT_WORK_DESC_",   patient_p->workDesc(), 40 );
    doc.subString( "_PAT_EMAIL_",       patient_p->emailAddress(), 40 );

    doc.subString( "_PAT_DECEASED_DATE_",  patient_p->dateString( patient_p->dateDeceased( )), 20 );
    doc.subCallback( "_IF_DECEASED_",      pmcPatRecordDeceasedCheck, patient_p->dateDeceased( ) );
    doc.subCallback( "_ENDIF_DECEASED_",   pmcPatRecordDeceasedCheck, patient_p->dateDeceased( ) );

    pmcSqlDoctorDetailsGet( patient_p->familyDrId(), &doctor );
    doc.subString( "_FAMILY_DR_LAST_NAME_",     doctor.lastName  ? doctor.lastName  : "", 40 );
    doc.subString( "_FAMILY_DR_FIRST_NAME_",    doctor.firstName ? doctor.firstName : "", 40 );
    doc.subString( "_FAMILY_DR_PHONE_",         doctor.formattedPhone, 40 );
    doc.subString( "_FAMILY_DR_FAX_",           doctor.formattedFax, 40 );

    pmcSqlDoctorDetailsGet( patient_p->referringId(), &doctor );
    doc.subString( "_REF_DR_LAST_NAME_",        doctor.lastName  ? doctor.lastName  : "", 40 );
    doc.subString( "_REF_DR_FIRST_NAME_",       doctor.firstName ? doctor.firstName : "", 40 );
    doc.subString( "_REF_DR_PHONE_",            doctor.formattedPhone, 40 );
    doc.subString( "_REF_DR_FAX_",              doctor.formattedFax, 40 );

    // This is a callback that indicates if there are any documents associated with this patient
    doc.subCallback( "_IF_DOCUMENT_FLAG_",      pmcPatRecordDocFlag,  patient_p->id() );
    doc.subCallback( "_DOCUMENT_LIST_",         pmcPatRecordDocList,  patient_p->id() );

    if( consult_p )
    {
        pmcSqlDoctorDetailsGet( consult_p->drId(), &doctor );

        doc.subString( "_CONSULT_DR_LAST_NAME_",    doctor.lastName  ? doctor.lastName  : "", 40 );
        doc.subString( "_CONSULT_DR_FIRST_NAME_",   doctor.firstName ? doctor.firstName : "", 40 );
        doc.subString( "_CONSULT_DR_PHONE_",        doctor.formattedPhone, 40 );
        doc.subString( "_CONSULT_DR_FAX_",          doctor.formattedFax, 40 );

        doc.subString( "_CONSULT_DR_ADDRESS_1_",    doctor.address1, 40 );
        doc.subString( "_CONSULT_DR_ADDRESS_2_",    doctor.address2, 40 );
        doc.subString( "_CONSULT_DR_ADDRESS_3_",    doctor.address3, 40 );

        doc.subString( "_LETTER_DATE_",             patient_p->dateString( consult_p->date( )), 20 );
        doc.subString( "_APP_DATE_",                patient_p->dateString( consult_p->appDate( )), 20 );
    }

    if( doc.templateToTempSub( ) != MB_RET_OK )
    {
        mbDlgError( "Document template processing failed\n" );
        goto exit;
    }

    if( doc.generate( 15 ) != MB_RET_OK ) goto exit;

    if( viewFlag ) doc.view( );

    if( name_p ) name_p->set( doc.nameFinal( ) );

exit:

    return result;
}

//---------------------------------------------------------------------------
// Prints an HTML version of the patient record
//---------------------------------------------------------------------------

void pmcPatRecordHTML( Int32u_t patientId )
{
    Char_p          buf_p;
    Char_p          buf1_p;
    Char_p          buf2_p;
    Char_p          phone_p;
    Char_t          area[8];
    FILE           *fp;
    Char_p          spoolFileName_p;
    Char_p          flagFileName_p;
    Char_p          fileName_p;
    Int32u_t        birthDate;
    Int32u_t        familyDrId;
    Int32u_t        referringDrId;
    Boolean_t       localFlag;
    Int64u_t        createTime;
    Int64u_t        modifyTime;
    PmcSqlPatient_p patient_p;
    PmcSqlDoctor_p  doctor_p;
    MbSQL           sql;
    MbDateTime      dateTime;

    mbMalloc( phone_p, 64 );
    mbMalloc( buf_p, 256 );
    mbMalloc( buf1_p, 256 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( spoolFileName_p, 128 );
    mbMalloc( flagFileName_p, 128 );
    mbMalloc( fileName_p, 128 );
    mbMalloc( patient_p, sizeof( PmcSqlPatient_t ) );
    mbMalloc( doctor_p, sizeof(  PmcSqlDoctor_t ) );

    if( patientId == 0 )
    {
        mbDlgDebug(( "Called with patient ID 0" ));
        goto exit;
    }

    pmcSqlPatientDetailsGet( patientId, patient_p );
    if( mbDlgOkCancel( "Print patient record for %s %s?",
                       patient_p->firstName, patient_p->lastName ) == MB_BUTTON_CANCEL ) goto exit;

    sprintf( fileName_p, "%s.html", pmcMakeFileName( NIL, buf_p ) );

    sprintf( spoolFileName_p, "%s\\spool\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );
    sprintf( flagFileName_p,  "%s\\flags\\%s", pmcCfg[CFG_PMCDESPL_DIR].str_p, fileName_p );

    fp = fopen( spoolFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Error opening patient record print file" ));
        goto exit;
    }

    sprintf( buf_p, "select * from %s where %s=%ld",
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID, patientId );

    sql.Query( buf_p );
    while( sql.RowGet() )
    {
        // Get the strings
        fprintf( fp, "<HTML><HEAD><Title>Patient Record</TITLE></HEAD><BODY>\n" );
        fprintf( fp, "<H2><CENTER>Patient Record: %s %s</H2></CENTER><HR>\n", patient_p->firstName, patient_p->lastName );

        fprintf( fp, "<PRE WIDTH = 80>\n" );
        fprintf( fp, "First Name:               %s\n\n", patient_p->firstName );
        fprintf( fp, "Middle Name:              %s\n\n", sql.NmString( PMC_SQL_FIELD_MIDDLE_NAME ) );
        fprintf( fp, "LastName:                 %s\n\n", patient_p->lastName );
        fprintf( fp, "Title:                    %s\n\n", sql.NmString( PMC_SQL_FIELD_TITLE ));
        fprintf( fp, "Address Line 1:           %s\n\n", sql.NmString(PMC_SQL_FIELD_ADDRESS1) );
        fprintf( fp, "Address Line 2:           %s\n\n", sql.NmString(PMC_SQL_FIELD_ADDRESS2) );
        fprintf( fp, "City:                     %s  ",   sql.NmString( PMC_SQL_FIELD_CITY ) );
        fprintf( fp, " %s  ", sql.NmString( PMC_SQL_FIELD_PROVINCE ) );
        fprintf( fp, " %s\n\n", sql.NmString( PMC_SQL_FIELD_COUNTRY ) );
        fprintf( fp, "Postal Code:              %s\n\n", sql.NmString( PMC_SQL_FIELD_POSTAL_CODE ) );

        // Home Phone
        strcpy( buf_p, sql.NmString( PMC_SQL_PATIENTS_FIELD_HOME_PHONE  ) );
        pmcPhoneFix( buf_p, NIL );
        pmcPhoneFormat( buf_p, area, phone_p, &localFlag );
        if( localFlag )
        {
            sprintf( buf2_p, "%s", phone_p );
        }
        else
        {
            sprintf( buf2_p,  "(%s) %s", area, phone_p );
        }
        fprintf( fp, "Phone:                    %s\n\n", buf2_p );

        // Birth Date
        fprintf( fp, "Date of Birth:            " );
        birthDate = sql.NmInt32u( PMC_SQL_PATIENTS_FIELD_DATE_OF_BIRTH );
        if( birthDate )
        {
            dateTime.SetDate( birthDate );
            fprintf( fp, "%s\n\n", dateTime.MDY_DateString( ) );
        }
        else
        {
            fprintf( fp, "\n\n" );
        }

        fprintf( fp, "PHN (Health Number):      %s ", pmcFormatPhnDisplay( sql.NmString( PMC_SQL_PATIENTS_FIELD_PHN ), NIL, buf_p ) );
        fprintf( fp, " (%s)\n\n", sql.NmString( PMC_SQL_PATIENTS_FIELD_PHN_PROVINCE ) );
        fprintf( fp, "Contact Name:             %s\n\n", sql.NmString( PMC_SQL_PATIENTS_FIELD_CONTACT_NAME ) );
        fprintf( fp, "Contact Description:      %s\n\n", sql.NmString( PMC_SQL_PATIENTS_FIELD_CONTACT_DESC ) );
        fprintf( fp, "Contact Phone:            %s\n\n", sql.NmString( PMC_SQL_PATIENTS_FIELD_CONTACT_PHONE ));
        fprintf( fp, "Work Description:         %s\n\n", sql.NmString(PMC_SQL_PATIENTS_FIELD_WORK_DESC));

        // Work Phone
        strcpy( buf_p, sql.NmString( PMC_SQL_PATIENTS_FIELD_WORK_PHONE ) );
        pmcPhoneFix( buf_p, NIL );
        pmcPhoneFormat( buf_p, area, phone_p, &localFlag );
        if( localFlag )
        {
            sprintf( buf2_p, "%s", phone_p );
        }
        else
        {
            sprintf( buf2_p,  "(%s) %s", area, phone_p );
        }

        fprintf( fp, "Work Phone:               %s\n\n", buf2_p );
        fprintf( fp, "E-mail Address:           %s\n\n", sql.NmString( PMC_SQL_PATIENTS_FIELD_EMAIL_ADDRESS ) );

        // The following paramters will be handled after the SQL query loop
        familyDrId      = sql.NmInt32u( PMC_SQL_PATIENTS_FIELD_FAMILY_DR_ID );
        referringDrId   = sql.NmInt32u( PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID );
        createTime      = sql.NmDateTimeInt64u( PMC_SQL_FIELD_CREATED );
        modifyTime      = sql.NmDateTimeInt64u( PMC_SQL_FIELD_MODIFIED );
    }

    pmcSqlDoctorDetailsGet( familyDrId, doctor_p );
    if( strlen( doctor_p->lastName ) )
    {
        fprintf( fp, "Family Dr:                %s %s (%ld)\n\n", doctor_p->firstName, doctor_p->lastName, doctor_p->mspNumber );
    }
    else
    {
        fprintf( fp, "Family Dr: \n\n" );
    }

    pmcSqlDoctorDetailsGet( referringDrId, doctor_p );
    if( strlen( doctor_p->lastName ) )
    {
        fprintf( fp, "Referring Dr:             %s %s (%ld)\n\n", doctor_p->firstName, doctor_p->lastName, doctor_p->mspNumber );
    }
    else
    {
        fprintf( fp, "Referring Dr:\n\n" );
    }

    fprintf( fp, "</PRE><HR>\n" );
    fprintf( fp, "<PRE>\n" );

    fprintf( fp, "Patient ID:               %ld\n", patientId );

    dateTime.SetDateTime64( createTime );
    fprintf( fp, "Record Created:           %-15s %s\n", dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );
    dateTime.SetDateTime64( modifyTime );
    fprintf( fp, "Record Modified:          %-15s %s\n", dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );

    dateTime.SetDate( mbToday() );
    dateTime.SetTime( mbTime() );

    fprintf( fp, "Record Printed:           %-15s %s\n", dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );

    fprintf( fp, "</PRE><HR>\n\n" );

    // Footer
    PMC_REPORT_FOOTER( fp );

    // End of document
    fprintf( fp, "</BODY></HTML>\n" );
    fclose( fp );

    // Write flag file to trigger despooler
    fp = fopen( flagFileName_p, "w" );
    if( fp == NIL )
    {
        mbDlgDebug(( "Could not open flag file '%s'", flagFileName_p ));
    }
    else
    {
        fprintf( fp, "%s", flagFileName_p );
        fclose( fp );
    }

    mbDlgInfo( "Patient record for %s %s sent to printer.",
        patient_p->firstName, patient_p->lastName );

exit:

    mbFree( buf_p );
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( spoolFileName_p );
    mbFree( flagFileName_p );
    mbFree( fileName_p );
    mbFree( patient_p );
    mbFree( doctor_p );
    mbFree( phone_p );
    return;
}
