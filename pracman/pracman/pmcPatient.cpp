//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// Patient object
//---------------------------------------------------------------------------

// Platform include files
#include <stdio.h>
#pragma hdrstop

// Library include files
#include "mbUtils.h"

// Project include files
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcPatient.h"
#include "pmcTables.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Class: PmcPatientHistory
//---------------------------------------------------------------------------
__fastcall PmcPatientHistory::PmcPatientHistory( Int32u_t id )
{
    mbStrList_p             item_p;
    mbStrList_p             entry_p;
    MbString                logString;

    mbObjectCountInc();

    this->section_q = qInitialize( &this->sectionQueue );

    mbLockAcquire( gPatHistoryType_q->lock );
    qWalk( item_p, gPatHistoryType_q, mbStrList_p )
    {
        mbLog( "Want to get patient history for type: %s %d\n", item_p->str_p, item_p->handle );

        mbCalloc( entry_p, sizeof(mbStrList_t ) );
        entry_p->str_p = pmcSqlPatHistoryGet( id, item_p->handle, &entry_p->handle, &entry_p->handle2 );

        if( entry_p->str_p == NIL )
        {
            mbFree( entry_p );
        }
        else
        {
            mbMallocStr( entry_p->str2_p, item_p->str2_p );

            qInsertLast( this->section_q, entry_p );

            logString.set( entry_p->str_p );
            mbLog( "Got history: '%s' subKey: %s\n", logString.truncate( 30 ), entry_p->str2_p );
        }
    }
    mbLockRelease( gPatHistoryType_q->lock );
}

//---------------------------------------------------------------------------
// Class: PmcPatientHistory
//---------------------------------------------------------------------------
__fastcall PmcPatientHistory::~PmcPatientHistory( void )
{
    mbObjectCountDec();

    // Free the list of histories
    mbStrListFree( this->section_q );
}

//---------------------------------------------------------------------------
// Class: PmcPatient
//---------------------------------------------------------------------------
__fastcall PmcPatient::PmcPatient( Int32u_t id )
{
    this->init( );
    this->idSet( id );
}

//---------------------------------------------------------------------------
// Set the patient ID
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::idSet( Int32u_t id )
{
    MbString        cmd = MbString( 256 );
    MbDateTime      dateTime = MbDateTime( );
    MbSQL           sql;
    Ints_t          i;
    Int64u_t        timeCreated = 0;
    Int64u_t        timeModified = 0;
    Int32s_t        result = MB_RET_ERR;

    if( this->idSetFlag == TRUE )
    {
        mbDlgError( "This patient object is already inititalized" );
        goto exit;
    }

    this->idSetFlag = TRUE;

    // Allow ID of 0 for new patients
    if( id > 0 )
    {
        this->myId = id;

        // Format the command
        mbSprintf( &cmd, "select * from %s where %s=%ld",
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_ID, id );

        // Issue the SQL command to get the patient details
        sql.Query( cmd.get() );

        if( sql.RowCount() != 1 )
        {
            mbDlgError( "Error retrieving patient details from the database" );
            goto exit;
        }

        while( sql.RowGet() )
        {
            // Read the strings from the database
            for( i = 0 ; i < PMC_PAT_STRING_COUNT ; i++ )
            {
                this->stringIn_p[i]->set( sql.NmString( this->dbStr( i ) )  );
            }

            // Read the ints from the database
            for( i = 0 ; i < PMC_PAT_INT_COUNT ; i++ )
            {
                this->valIntIn[i] = sql.NmInt32u( this->dbInt( i ) );
            }

            timeCreated = sql.NmDateTimeInt64u( PMC_SQL_FIELD_CREATED );
            timeModified = sql.NmDateTimeInt64u( PMC_SQL_FIELD_MODIFIED );
        }
    }

    // Must format some strings for display
    this->phnSet( this->stringIn_p[PMC_PAT_STRING_PHN]->get( ) );
    this->phnProvinceSet( this->stringIn_p[PMC_PAT_STRING_PHN_PROVINCE]->get( ) );
    this->phoneHomeSet( this->stringIn_p[PMC_PAT_STRING_PHONE_HOME]->get( ) );
    this->phoneWorkSet( this->stringIn_p[PMC_PAT_STRING_PHONE_WORK]->get( ) );
    this->phoneCellSet( this->stringIn_p[PMC_PAT_STRING_PHONE_CELL]->get( ) );

    // Record the original Referring Dr. ID
    this->referringIdOrig = this->referringId();

    if( timeModified )
    {
        dateTime.SetDateTime64( timeModified );
        sprintf( this->timeModifiedStr, "%s %s", dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );
    }

    if( timeCreated )
    {
        dateTime.SetDateTime64( timeCreated );
        sprintf( this->timeCreatedStr, "%s %s", dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );
    }

    // Now copy the string In values to the string values
    for( i = 0 ; i < PMC_PAT_STRING_COUNT ; i++ )
    {
        this->string_p[i]->set( this->stringIn_p[i]->get( ) );
    }

    for( i = 0 ; i < PMC_PAT_INT_COUNT ; i++ )
    {
        this->valInt[i] = this->valIntIn[i];
    }

    result = MB_RET_OK;

exit:
    return result;
}

//---------------------------------------------------------------------------
// Class: PmcPatient
//---------------------------------------------------------------------------
__fastcall PmcPatient::PmcPatient( void )
{
    this->init();
}

//---------------------------------------------------------------------------
// Class: PmcPatient
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::init( void )
{
    mbObjectCountInc();

    this->lockedFlag = FALSE;
    this->idSetFlag = FALSE;
    
    this->dateTime_p = new MbDateTime( );

    this->formattedPhn[0] = 0;
    this->formattedPhoneHome[0] = 0;
    this->formattedPhoneCell[0] = 0;
    this->formattedPhoneWork[0] = 0;
    this->timeCreatedStr[0] = 0;
    this->timeModifiedStr[0] = 0;
    this->ageStr[0] = 0;

    // Record the patient ID
    this->myId = 0;

    for( int i = 0 ; i < PMC_PAT_STRING_COUNT ; i++ )
    {
        this->stringIn_p[i] = new MbString( );
        this->string_p[i] = new MbString( );
    }

    for( int i = 0 ; i < PMC_PAT_INT_COUNT ; i++ )
    {
        this->valIntIn[i] = 0;
        this->valInt[i] = 0;
    }
    return MB_RET_OK;
}


//---------------------------------------------------------------------------
// Setter functions
//---------------------------------------------------------------------------

Int32s_t __fastcall PmcPatient::firstNameSet( Char_p str_p ) { return this->stringSet( PMC_PAT_STRING_FIRST_NAME, str_p ); }
Int32s_t __fastcall PmcPatient::lastNameSet( Char_p str_p )  { return this->stringSet( PMC_PAT_STRING_LAST_NAME, str_p ); }
Int32s_t __fastcall PmcPatient::middleNameSet( Char_p str_p ){ return this->stringSet( PMC_PAT_STRING_MIDDLE_NAME, str_p ); }
Int32s_t __fastcall PmcPatient::address1Set( Char_p str_p )  { return this->stringSet( PMC_PAT_STRING_ADDRESS1, str_p ); }
Int32s_t __fastcall PmcPatient::address2Set( Char_p str_p )  { return this->stringSet( PMC_PAT_STRING_ADDRESS2, str_p ); }
Int32s_t __fastcall PmcPatient::citySet( Char_p str_p )      { return this->stringSet( PMC_PAT_STRING_CITY, str_p ); }
Int32s_t __fastcall PmcPatient::titleSet( Char_p str_p )     { return this->stringSet( PMC_PAT_STRING_TITLE, str_p ); }
Int32s_t __fastcall PmcPatient::provinceSet( Char_p str_p )  { return this->stringSet( PMC_PAT_STRING_PROVINCE, str_p ); }
Int32s_t __fastcall PmcPatient::phnProvinceSet(Char_p str_p ){ return this->stringSet( PMC_PAT_STRING_PHN_PROVINCE, str_p ); }
Int32s_t __fastcall PmcPatient::countrySet( Char_p str_p )   { return this->stringSet( PMC_PAT_STRING_COUNTRY, str_p ); }
Int32s_t __fastcall PmcPatient::emailAddressSet(Char_p str_p){ return this->stringSet( PMC_PAT_STRING_EMAIL_ADDRESS, str_p ); }
Int32s_t __fastcall PmcPatient::workDescSet( Char_p str_p )  { return this->stringSet( PMC_PAT_STRING_WORK_DESC, str_p ); }
Int32s_t __fastcall PmcPatient::contactSet( Char_p str_p )   { return this->stringSet( PMC_PAT_STRING_CONTACT_NAME, str_p ); }
Int32s_t __fastcall PmcPatient::contactPhoneSet(Char_p str_p){ return this->stringSet( PMC_PAT_STRING_CONTACT_PHONE, str_p ); }
Int32s_t __fastcall PmcPatient::contactDescSet( Char_p str_p){ return this->stringSet( PMC_PAT_STRING_CONTACT_DESC, str_p ); }
Int32s_t __fastcall PmcPatient::commentSet( Char_p str_p )   { return this->stringSet( PMC_PAT_STRING_COMMENT, str_p ); }
Int32s_t __fastcall PmcPatient::postalCodeSet( Char_p str_p )
{
    Char_t  buf1[128];
    Char_t  buf2[128];

    strcpy( buf1, str_p );
    mbStrClean( mbStrAlphaNumericOnly( buf1 ), buf2, TRUE );
    pmcPostalCodeFix( buf2, NIL );
    return this->stringSet( PMC_PAT_STRING_POSTAL_CODE, buf2 );
}

//-----------------------------------------------------------------------------
// This method looks at the passed in search string for the patient.  If 9
// digits are found, then it is assumed to be a phn
//-----------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::phnInit( Char_p str_p )
{
    Char_t  buf1[128];
    Char_t  buf2[128];

    mbLog("phnInit() called with '%s'\n", str_p );

    strcpy( buf1, str_p );
    mbStrClean( mbStrAlphaNumericOnly( buf1 ), buf2, TRUE );
    
    if( strlen( buf2 ) == 9 )
    {
        // Put the formatted phn display into this string
        pmcFormatPhnDisplay( buf2, NIL, this->formattedPhn );
        return this->stringSet( PMC_PAT_STRING_PHN, buf2 );
    }
    return FALSE;
}

//-----------------------------------------------------------------------------

Int32s_t __fastcall PmcPatient::phnSet( Char_p str_p )
{
    Char_t  buf1[128];
    Char_t  buf2[128];

    strcpy( buf1, str_p );
    mbStrClean( mbStrAlphaNumericOnly( buf1 ), buf2, TRUE );

    // Put the formatted phn display into this string
    pmcFormatPhnDisplay( buf2, NIL, this->formattedPhn );
    return this->stringSet( PMC_PAT_STRING_PHN, buf2 );
}

//---------------------------------------------------------------------------
// Phone setter.  This really needs to be cleaned up
// (e.g., can't put extension in phone)
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::phoneHomeSet( Char_p str_p )
{
    Char_t      buf[128];
    Char_t      buf2[128];
    Char_t      phone[128];
    Char_t      area[16];
    Boolean_t   localFlag;

    strcpy( buf2, str_p );
    mbStrClean( buf2, buf, TRUE );
    if( ( this->phoneHomeValid = pmcPhoneFix( buf, NIL ) ) == TRUE )
    {
        // This is the string that goes into the database
        this->stringSet( PMC_PAT_STRING_PHONE_HOME, buf );

        // Now get the string for display
        if( strlen( buf ) > 0 )
        {
            pmcPhoneFormat( buf, area, phone, &localFlag );
            if( localFlag )
            {
                sprintf( this->formattedPhoneHome, "%s", phone );
            }
            else
            {
                sprintf( this->formattedPhoneHome, "(%s) %s", area, phone );

            }
        }
    }
    else
    {
        sprintf( this->formattedPhoneHome, "%s", buf2 );
        this->stringSet( PMC_PAT_STRING_PHONE_HOME, buf2 );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::phoneWorkSet( Char_p str_p )
{
    Char_t      buf[128];
    Char_t      buf2[128];
    Char_t      phone[128];
    Char_t      area[16];
    Boolean_t   localFlag;

    strcpy( buf2, str_p );
    mbStrClean( buf2, buf, TRUE );
    if( ( this->phoneWorkValid = pmcPhoneFix( buf, NIL ) ) == TRUE )
    {
        // This is the string that goes into the database
        this->stringSet( PMC_PAT_STRING_PHONE_WORK, buf );

        // Now get the string for display
        if( strlen( buf ) > 0 )
        {
            pmcPhoneFormat( buf, area, phone, &localFlag );
            if( localFlag )
            {
                sprintf( this->formattedPhoneWork, "%s", phone );
            }
            else
            {
                sprintf( this->formattedPhoneWork, "(%s) %s", area, phone );
            }
        }
    }
    else
    {
        sprintf( this->formattedPhoneWork, "%s", buf2 );
        this->stringSet( PMC_PAT_STRING_PHONE_WORK, buf2 );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::phoneCellSet( Char_p str_p )
{
    Char_t      buf[128];
    Char_t      buf2[128];
    Char_t      phone[128];
    Char_t      area[16];
    Boolean_t   localFlag;

    strcpy( buf2, str_p );
    mbStrClean( buf2, buf, TRUE );
    if( ( this->phoneCellValid = pmcPhoneFix( buf, NIL ) ) == TRUE )
    {
        // This is the string that goes into the database
        this->stringSet( PMC_PAT_STRING_PHONE_CELL, buf );

        // Now get the string for display
        if( strlen( buf ) > 0 )
        {
            pmcPhoneFormat( buf, area, phone, &localFlag );
            if( localFlag )
            {
                sprintf( this->formattedPhoneCell, "%s", phone );
            }
            else
            {
                sprintf( this->formattedPhoneCell, "(%s) %s", area, phone );
            }
        }
    }
    else
    {
        sprintf( this->formattedPhoneCell, "%s", buf2 );
        this->stringSet( PMC_PAT_STRING_PHONE_CELL, buf2 );
    }
    return TRUE;
}
//---------------------------------------------------------------------------
// Helper funtion cleans the string so that it can be safely put into
// the database.
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::stringSet( Ints_t index, Char_p str_p )
{
    this->string_p[index]->set( str_p );
    this->string_p[index]->clean( );

    if(     index == PMC_PAT_STRING_PROVINCE
         || index == PMC_PAT_STRING_PHN_PROVINCE )
    {
        // Upper case the province strings
        this->string_p[index]->upper( );
    }

    // mbDlgInfo( "string index %d set to: %s\n", index, this->string_p[index]->get() );
    return TRUE;
}

//---------------------------------------------------------------------------
// Detect any changes in the patient details
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::changed( void )
{
    Ints_t      i;
    Int32s_t    change = FALSE;

    for( i = 0 ; i < PMC_PAT_STRING_COUNT ; i++ )
    {
        if( strcmp( this->stringIn_p[i]->get(), this->string_p[i]->get() ) != 0 )
        {
            change = TRUE;
            //mbLog( "Patient string %d changed from '%s' to '%s'", i, this->stringIn_p[i]->get(), this->string_p[i]->get() );
            goto exit;
        }
    }

    for( i = 0 ; i < PMC_PAT_INT_COUNT ; i++ )
    {
        if( this->valInt[i] != this->valIntIn[i] )
        {
            change = TRUE;
            //mbLog( "Int %d changed from %u to %u", i, this->valIntIn[i], this->valInt[i] );
            goto exit;
        }
    }

exit:
    return change;
}

//---------------------------------------------------------------------------
// Save any detected changes
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::save( void )
{
    Ints_t      i;
    Int32u_t    suspect = FALSE;

    if( strlen( this->firstName( ) ) == 0 )
    {
        mbDlgExclaim( "Cannot save patient record without first name set." );
        return FALSE;
    }

    if( strlen( this->lastName( ) ) == 0 )
    {
        mbDlgExclaim( "Cannot save patient record without last name set." );
        return FALSE;
    }

    if( strlen( this->title( ) ) == 0 )
    {
        mbDlgExclaim( "A title must be entered for this patient." );
        return FALSE;
    }
    else
    {
        if( this->gender( ) == PMC_SQL_GENDER_MALE )
        {
            if( mbStrPos( this->title( ), "Mrs"  ) >= 0 ||
                mbStrPos( this->title( ), "Ms"   ) >= 0 ||
                mbStrPos( this->title( ), "Miss" ) >= 0  )
            {
                suspect = TRUE;
            }
        }
        else if( this->gender( ) == PMC_SQL_GENDER_FEMALE )
        {
            if( mbStrPos( this->title( ), "Mr."  ) >= 0 )
            {
                suspect = TRUE;
            }
        }
        if( suspect == TRUE )
        {
            mbDlgExclaim( "Check patient's title and gender." );
            return FALSE;
        }
    }

    // Check and reformat the postal code
    if( strlen( this->postalCode( ) ) > 0 )
    {
        if( pmcPostalCodeFix( this->postalCode( ), NIL ) == FALSE )
        {
            mbDlgExclaim( "Invalid postal code." );
            return FALSE;
        }
    }

    // Check the deceased data
    if( this->dateDeceased( ) > 0 )
    {
        if( this->dateDeceased( ) > mbToday( ) )
        {
            mbDlgExclaim( "Deceased date is in the future.\n" );
            return FALSE;
        }
    }

    if( this->phoneHomeValidate() == FALSE )
    {
        mbDlgExclaim( "Invalid home phone number.\n" );
        return FALSE;
    }

    // Check that PHN is unique.  Allow blank PHN to be entered
    {
        Char_t      buf1[128];
        Char_t      buf2[128];
        Char_t      buf3[128];
        Int32u_t    result;
        Int32u_t    existingPatientId;
        Int32u_t    close = TRUE;

        strcpy( buf1, this->phn( )  );
        mbStrClean( mbStrAlphaNumericOnly( buf1 ), buf2, TRUE );

        strcpy( buf1, this->phnProvince( ) );
        mbStrToUpper( mbStrClean( buf1, buf3, TRUE ) );

        if( strcmp( buf3, PMC_PHN_DEFAULT_PROVINCE ) == 0 && strlen( buf2 ) > 0 )
        {
            if( pmcPhnVerifyString( buf2 ) == FALSE )
            {
                mbDlgExclaim( "Invalid PHN\n" );
                return FALSE;
            }
        }

        if( strlen( buf2 ) > 0 )
        {
            sprintf( buf1, "select %s from %s where %s=\"%s\" and %s=\"%s\" and %s = %d",
                PMC_SQL_FIELD_ID,
                PMC_SQL_TABLE_PATIENTS,
                PMC_SQL_PATIENTS_FIELD_PHN,             buf2,
                PMC_SQL_PATIENTS_FIELD_PHN_PROVINCE,    buf3,
                PMC_SQL_FIELD_NOT_DELETED,              PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

            existingPatientId = pmcSqlSelectInt( buf1, &result );
            if( result != 0 )
            {
                // Sanity check
                if( result != 1 )
                {
                    mbDlgDebug(( "Found multiple matching patient records with phn '%s'", this->phn( ) ));
                    close = FALSE;
                }
                else
                {
                    if( this->myId == 0 ) close = FALSE;

                    if( ( this->myId != 0 ) && ( this->myId != existingPatientId ) ) close = FALSE;
                }
            }
            if( close == FALSE )
            {
                mbDlgExclaim( "There is another patient in the database with PHN '%s (%s)'",
                    this->phn( ), this->phnProvince( ) );
                return FALSE;
            }
        }
    }

    // If the patient ID is 0, then this must be a new record
    if( this->myId == 0 )
    {
        this->myId = pmcSqlRecordCreate( PMC_SQL_TABLE_PATIENTS, NIL );
        if( this->myId == 0 )
        {
            mbDlgDebug(( "Error creating patient record" ));
            goto exit;
        }
        mbLog( "Created new patient record with id %ld\n", this->myId );

        pmcSqlRecordLock( PMC_SQL_TABLE_PATIENTS, this->id( ), FALSE );
        pmcSqlRecordUndelete( PMC_SQL_TABLE_PATIENTS, this->id( ) );
        this->lockedFlag = TRUE;
    }

    // Loop through the strings, saving them to the database
    for( i = 0 ; i < PMC_PAT_STRING_COUNT ; i++ )
    {
        if( strcmp( this->stringIn_p[i]->get(), this->string_p[i]->get() ) != 0 )
        {
            // First, make the change in the database
            pmcSqlExecString( PMC_SQL_TABLE_PATIENTS,
                              this->dbStr(i),
                              this->string_p[i]->get(),
                              this->myId);

            // Next record the change internally
            this->stringIn_p[i]->set( this->string_p[i]->get() );
        }
    }

    // Loop through the ints, saving them to the database
    for( i = 0 ; i < PMC_PAT_INT_COUNT ; i++ )
    {
        if( this->valInt[i] != this->valIntIn[i] )
        {
            // First, make the change in the database
            pmcSqlExecInt( PMC_SQL_TABLE_PATIENTS,
                           this->dbInt(i),
                           this->valInt[i],
                           this->myId );

            // Next record the change internally
            this->valIntIn[i] = this->valInt[i];
        }
    }

    // MAB:20020424: Check to see if patient is marked as deceased
    if( this->dateDeceased( ) != 0 )
    {
        pmcCheckDeceasedApps( this->myId, this->dateDeceased( ) );
    }

    // MAB:20020413: Prompt to see if any appointments should be updated.
    if( this->referringId() != this->referringIdOrig )
    {
        pmcCheckReferringDrApps( this->myId, this->referringId() );
    }

exit:
    return TRUE;
}

//---------------------------------------------------------------------------
// Utility that returns a date string
//---------------------------------------------------------------------------
Char_p __fastcall PmcPatient::dateString( Int32u_t date )
{
    if( date == 0 ) return "";

    this->dateTime_p->SetDate( date );
    return this->dateTime_p->MDY_DateString( );
}

//---------------------------------------------------------------------------
// Utility that returns a time string
//---------------------------------------------------------------------------
Char_p __fastcall PmcPatient::timeString( Int32u_t time )
{
    if( time == 0 ) return "";

    this->dateTime_p->SetTime( time );
    return this->dateTime_p->HM_TimeString( );
}


//---------------------------------------------------------------------------
// Returns string containing current age of patient, or, if deceased,
// age at death.
//---------------------------------------------------------------------------
Char_p __fastcall PmcPatient::ageString( void )
{
    if( this->dateOfBirth() == 0 ) return "";

    this->dateTime_p->SetDate( this->dateOfBirth() );
    if( this->dateDeceased() == 0 )
    {
        sprintf( this->ageStr, "%s", this->dateTime_p->AgeString( mbToday() ));
    }
    else
    {
        sprintf( this->ageStr, "%s", this->dateTime_p->AgeString( this->dateDeceased() ));
    }
    return this->ageStr;
}

//---------------------------------------------------------------------------
// Lock the record
//---------------------------------------------------------------------------
Int32s_t __fastcall PmcPatient::lock( Boolean_t forceFlag )
{
    Int32s_t    result = MB_RET_ERR;

    if( this->lockedFlag == TRUE )
    {
        mbDlgInfo( "Patient record is already locked" );
        return MB_RET_OK;
    }

    if( pmcSqlRecordLock( PMC_SQL_TABLE_PATIENTS, this->id(), TRUE ) == FALSE )
    {
        if( forceFlag == TRUE )
        {
            if( mbDlgYesNo( "Patient record for %s %s locked for editing by another user.\n\n"
                            "To unlock this record, click Yes.\n"
                            "To leave this record locked, click No.\n\n"
                            "WARNING:\n\nUnlocking a locked record can cause data corruption.\n"
                            "Unlock only if you're sure it is safe to do so.",
                            this->firstName(), this->lastName() ) == MB_BUTTON_YES )
            {
                pmcSqlRecordUnlockForce( PMC_SQL_TABLE_PATIENTS, this->id( ) );
                if( pmcSqlRecordLock( PMC_SQL_TABLE_PATIENTS, this->id( ), TRUE ) == TRUE )
                {
                    result = MB_RET_OK;
                }
                else
                {
                    mbDlgExclaim( "Patient record unlock failed." );
                }
            }
        }
    }
    else
    {
        result = MB_RET_OK;
    }

    if( result == MB_RET_OK )
    {
        mbLog( "Locked patient record id: %lu\n", this->id( ) );
        this->lockedFlag = TRUE;
    }
    return result;
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
__fastcall PmcPatient::~PmcPatient( void )
{
    Ints_t i;

    for( i = 0 ; i < PMC_PAT_STRING_COUNT ; i++ )
    {
        delete this->stringIn_p[i];
        delete this->string_p[i];
    }

    delete this->dateTime_p;

    if( this->lockedFlag == TRUE )
    {
        mbLog( "Unlocking patient record that is going out of scope" );
        pmcSqlRecordUnlock( PMC_SQL_TABLE_PATIENTS, this->id() );
    }

    mbObjectCountDec();
}






