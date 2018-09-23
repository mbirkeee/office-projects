//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// PmcPatient class definition
//---------------------------------------------------------------------------

#ifndef pmcPatientH
#define pmcPatientH

#include "pmcTables.h"

// String values in the patient record (test)
enum
{
     PMC_PAT_STRING_FIRST_NAME
    ,PMC_PAT_STRING_LAST_NAME
    ,PMC_PAT_STRING_MIDDLE_NAME
    ,PMC_PAT_STRING_TITLE

    ,PMC_PAT_STRING_ADDRESS1
    ,PMC_PAT_STRING_ADDRESS2
    ,PMC_PAT_STRING_CITY

    ,PMC_PAT_STRING_PROVINCE
    ,PMC_PAT_STRING_COUNTRY
    ,PMC_PAT_STRING_POSTAL_CODE
    ,PMC_PAT_STRING_PHN
    ,PMC_PAT_STRING_PHN_PROVINCE
    ,PMC_PAT_STRING_CHART_STRING
    ,PMC_PAT_STRING_EMAIL_ADDRESS
    ,PMC_PAT_STRING_CONTACT_NAME
    ,PMC_PAT_STRING_CONTACT_DESC
    ,PMC_PAT_STRING_CONTACT_PHONE
    ,PMC_PAT_STRING_COMMENT
    ,PMC_PAT_STRING_WORK_DESC

    ,PMC_PAT_STRING_PHONE_HOME
    ,PMC_PAT_STRING_PHONE_WORK
    ,PMC_PAT_STRING_PHONE_CELL

    ,PMC_PAT_STRING_COUNT

};

// Int values in the patient record
enum
{
     PMC_PAT_INT_DATE_OF_BIRTH
    ,PMC_PAT_INT_DATE_DECEASED
    ,PMC_PAT_INT_PROVIDER_ID
    ,PMC_PAT_INT_FAMILY_DR_ID
    ,PMC_PAT_INT_REFERRING_ID
    ,PMC_PAT_INT_GENDER
    ,PMC_PAT_INT_COUNT
};

//---------------------------------------------------------------------------
// PmcPatientHistory
//---------------------------------------------------------------------------
class PmcPatientHistory
{
public:
    __fastcall  PmcPatientHistory( Int32u_t myId );
    __fastcall ~PmcPatientHistory( void );

    qHead_p     __fastcall subQueue( void ) { return this->section_q; }

private:
    qHead_t     sectionQueue;
    qHead_p     section_q;
};

//---------------------------------------------------------------------------
// PmcPatient
//---------------------------------------------------------------------------
class PmcPatient
{
public:
    __fastcall  PmcPatient( void );
    __fastcall  PmcPatient( Int32u_t myId );
    __fastcall ~PmcPatient( void );

    Char_p  __fastcall firstName( void )    { return this->string_p[PMC_PAT_STRING_FIRST_NAME]->get(); }
    Char_p  __fastcall lastName( void )     { return this->string_p[PMC_PAT_STRING_LAST_NAME]->get(); }
    Char_p  __fastcall middleName( void )   { return this->string_p[PMC_PAT_STRING_MIDDLE_NAME]->get(); }
    Char_p  __fastcall title( void )        { return this->string_p[PMC_PAT_STRING_TITLE]->get(); }

    Char_p  __fastcall address1( void )     { return this->string_p[PMC_PAT_STRING_ADDRESS1]->get(); }
    Char_p  __fastcall address2( void )     { return this->string_p[PMC_PAT_STRING_ADDRESS2]->get(); }
    Char_p  __fastcall city( void )         { return this->string_p[PMC_PAT_STRING_CITY]->get(); }

    Char_p  __fastcall province( void )     { return this->string_p[PMC_PAT_STRING_PROVINCE]->get(); }
    Char_p  __fastcall country( void )      { return this->string_p[PMC_PAT_STRING_COUNTRY]->get(); }
    Char_p  __fastcall postalCode( void )   { return this->string_p[PMC_PAT_STRING_POSTAL_CODE]->get(); }

    Char_p  __fastcall phnProvince( void )  { return this->string_p[PMC_PAT_STRING_PHN_PROVINCE]->get(); }
    Char_p  __fastcall chartString( void )  { return this->string_p[PMC_PAT_STRING_CHART_STRING]->get(); }
    Char_p  __fastcall emailAddress( void ) { return this->string_p[PMC_PAT_STRING_EMAIL_ADDRESS]->get(); }
    Char_p  __fastcall contactName( void )  { return this->string_p[PMC_PAT_STRING_CONTACT_NAME]->get(); }
    Char_p  __fastcall contactDesc( void )  { return this->string_p[PMC_PAT_STRING_CONTACT_DESC]->get(); }
    Char_p  __fastcall contactPhone( void ) { return this->string_p[PMC_PAT_STRING_CONTACT_PHONE]->get(); }
    Char_p  __fastcall comment( void )      { return this->string_p[PMC_PAT_STRING_COMMENT]->get(); }
    Char_p  __fastcall workDesc( void )     { return this->string_p[PMC_PAT_STRING_WORK_DESC]->get(); }

    Char_p  __fastcall phn( void )          { return formattedPhn; }
    Char_p  __fastcall phoneHome( void )    { return formattedPhoneHome; }
    Char_p  __fastcall phoneWork( void )    { return formattedPhoneWork; }
    Char_p  __fastcall phoneCell( void )    { return formattedPhoneCell; }
    Char_p  __fastcall timeCreated( void )  { return timeCreatedStr; }
    Char_p  __fastcall timeModified( void ) { return timeModifiedStr; }

    Boolean_t   __fastcall phoneHomeValidate(void) { return this->phoneHomeValid; }
    Boolean_t   __fastcall phoneWorkValidate(void) { return this->phoneWorkValid; }
    Boolean_t   __fastcall phoneCellValidate(void) { return this->phoneCellValid; }

    Int32u_t    __fastcall dateOfBirth( void )    { return this->valInt[PMC_PAT_INT_DATE_OF_BIRTH]; }
    Int32u_t    __fastcall dateDeceased( void )   { return this->valInt[PMC_PAT_INT_DATE_DECEASED]; }
    Int32u_t    __fastcall gender( void )         { return this->valInt[PMC_PAT_INT_GENDER]; }
    Int32u_t    __fastcall providerId( void )     { return this->valInt[PMC_PAT_INT_PROVIDER_ID]; }
    Int32u_t    __fastcall familyDrId( void )     { return this->valInt[PMC_PAT_INT_FAMILY_DR_ID]; }
    Int32u_t    __fastcall referringId( void )    { return this->valInt[PMC_PAT_INT_REFERRING_ID]; }
    Int32u_t    __fastcall id( void )             { return this->myId; }

    void    __fastcall dateOfBirthSet( Int32u_t val )  { this->valInt[PMC_PAT_INT_DATE_OF_BIRTH] = val; }
    void    __fastcall dateDeceasedSet( Int32u_t val ) { this->valInt[PMC_PAT_INT_DATE_DECEASED] = val; }
    void    __fastcall genderSet( Int32u_t val )       { this->valInt[PMC_PAT_INT_GENDER] = val; }
    void    __fastcall providerIdSet( Int32u_t val )   { this->valInt[PMC_PAT_INT_PROVIDER_ID] = val; }
    void    __fastcall familyDrIdSet( Int32u_t val )   { this->valInt[PMC_PAT_INT_FAMILY_DR_ID] = val; }
    void    __fastcall referringIdSet( Int32u_t val )  { this->valInt[PMC_PAT_INT_REFERRING_ID] = val; }

    Int32s_t    __fastcall idSet( Int32u_t id );
    Int32s_t    __fastcall firstNameSet( Char_p str_p );
    Int32s_t    __fastcall lastNameSet( Char_p str_p );
    Int32s_t    __fastcall middleNameSet( Char_p str_p );
    Int32s_t    __fastcall address1Set( Char_p str_p );
    Int32s_t    __fastcall address2Set( Char_p str_p );
    Int32s_t    __fastcall citySet( Char_p str_p );
    Int32s_t    __fastcall titleSet( Char_p str_p );
    Int32s_t    __fastcall provinceSet( Char_p str_p );
    Int32s_t    __fastcall postalCodeSet( Char_p str_p );
    Int32s_t    __fastcall countrySet( Char_p str_p );
    Int32s_t    __fastcall phnSet( Char_p str_p );
    Int32s_t    __fastcall phnInit( Char_p str_p );
    Int32s_t    __fastcall phnProvinceSet( Char_p str_p );
    Int32s_t    __fastcall emailAddressSet( Char_p str_p );
    Int32s_t    __fastcall workDescSet( Char_p str_p );
    Int32s_t    __fastcall contactSet( Char_p str_p );
    Int32s_t    __fastcall contactPhoneSet( Char_p str_p );
    Int32s_t    __fastcall contactDescSet( Char_p str_p );
    Int32s_t    __fastcall commentSet( Char_p str_p );

    Int32s_t    __fastcall phoneHomeSet( Char_p str_p );
    Int32s_t    __fastcall phoneWorkSet( Char_p str_p );
    Int32s_t    __fastcall phoneCellSet( Char_p str_p );

    Int32s_t    __fastcall changed( void );
    Int32s_t    __fastcall save( void );
    Int32s_t    __fastcall lock( Boolean_t forceFlag );
    Char_p      __fastcall dateString( Int32u_t );
    Char_p      __fastcall timeString( Int32u_t );
    Char_p      __fastcall ageString( void );

private:

    Char_t  formattedPhn[64];
    Char_t  formattedPhoneHome[64];
    Char_t  formattedPhoneCell[64];
    Char_t  formattedPhoneWork[64];
    Char_t  timeCreatedStr[64];
    Char_t  timeModifiedStr[64];
    Char_t  ageStr[8];

    Boolean_t   phoneHomeValid;
    Boolean_t   phoneWorkValid;
    Boolean_t   phoneCellValid;

    Boolean_t   lockedFlag;
    Boolean_t   idSetFlag;

    Int32u_t    referringIdOrig;

    // The patient ID
    Int32u_t    myId;

    // For date/time utilities
    MbDateTime  *dateTime_p;

    // Array of pointers to string objects - 2 copies to detect changes
    MbString    *stringIn_p[PMC_PAT_STRING_COUNT];
    MbString    *string_p[PMC_PAT_STRING_COUNT];

    // Array of int values - 2 copies to detect changes
    Int32u_t    valInt[PMC_PAT_INT_COUNT];
    Int32u_t    valIntIn[PMC_PAT_INT_COUNT];

    // Init helper shared between constructors
    Int32s_t    __fastcall init( void );

    // Utility function for string setter functions
    Int32s_t    __fastcall stringSet( Ints_t index, Char_p str_p );

    // Return the corresponding database name for the specified index
    Char_p  __fastcall dbStr( Ints_t index )
    {
        switch( index )
        {
            case PMC_PAT_STRING_FIRST_NAME:     return PMC_SQL_FIELD_FIRST_NAME;
            case PMC_PAT_STRING_LAST_NAME:      return PMC_SQL_FIELD_LAST_NAME;
            case PMC_PAT_STRING_MIDDLE_NAME:    return PMC_SQL_FIELD_MIDDLE_NAME;
            case PMC_PAT_STRING_TITLE:          return PMC_SQL_FIELD_TITLE;

            case PMC_PAT_STRING_ADDRESS1:       return PMC_SQL_FIELD_ADDRESS1;
            case PMC_PAT_STRING_ADDRESS2:       return PMC_SQL_FIELD_ADDRESS2;
            case PMC_PAT_STRING_CITY:           return PMC_SQL_FIELD_CITY;

            case PMC_PAT_STRING_PHN:            return PMC_SQL_PATIENTS_FIELD_PHN;
            case PMC_PAT_STRING_PROVINCE:       return PMC_SQL_FIELD_PROVINCE;
            case PMC_PAT_STRING_COUNTRY:        return PMC_SQL_FIELD_COUNTRY;
            case PMC_PAT_STRING_POSTAL_CODE:    return PMC_SQL_FIELD_POSTAL_CODE;

            case PMC_PAT_STRING_PHN_PROVINCE:   return PMC_SQL_PATIENTS_FIELD_PHN_PROVINCE;
            case PMC_PAT_STRING_CHART_STRING:   return PMC_SQL_PATIENTS_FIELD_CHART_STRING;
            case PMC_PAT_STRING_EMAIL_ADDRESS:  return PMC_SQL_PATIENTS_FIELD_EMAIL_ADDRESS;
            case PMC_PAT_STRING_CONTACT_NAME:   return PMC_SQL_PATIENTS_FIELD_CONTACT_NAME;
            case PMC_PAT_STRING_CONTACT_DESC:   return PMC_SQL_PATIENTS_FIELD_CONTACT_DESC;
            case PMC_PAT_STRING_CONTACT_PHONE:  return PMC_SQL_PATIENTS_FIELD_CONTACT_PHONE;
            case PMC_PAT_STRING_COMMENT:        return PMC_SQL_PATIENTS_FIELD_COMMENT;
            case PMC_PAT_STRING_WORK_DESC:      return PMC_SQL_PATIENTS_FIELD_WORK_DESC;

            case PMC_PAT_STRING_PHONE_HOME:     return PMC_SQL_PATIENTS_FIELD_HOME_PHONE;
            case PMC_PAT_STRING_PHONE_WORK:     return PMC_SQL_PATIENTS_FIELD_WORK_PHONE;
            case PMC_PAT_STRING_PHONE_CELL:     return PMC_SQL_PATIENTS_FIELD_CELL_PHONE;

            default:
                return "UNKNOWN";
        }
    }

    // Return the corresponding database name for the specified index
    Char_p  __fastcall dbInt( Ints_t index )
    {
        switch( index )
        {
            case PMC_PAT_INT_DATE_OF_BIRTH:     return PMC_SQL_PATIENTS_FIELD_DATE_OF_BIRTH;
            case PMC_PAT_INT_DATE_DECEASED:     return PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH;
            case PMC_PAT_INT_PROVIDER_ID:       return PMC_SQL_FIELD_PROVIDER_ID;
            case PMC_PAT_INT_FAMILY_DR_ID:      return PMC_SQL_PATIENTS_FIELD_FAMILY_DR_ID;
            case PMC_PAT_INT_REFERRING_ID:      return PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID;
            case PMC_PAT_INT_GENDER:            return PMC_SQL_PATIENTS_FIELD_GENDER;

            default:
                return "UNKNOWN";
        }
    }
};

#endif // pmcPatientH
