//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// PmcDocument class definition
//---------------------------------------------------------------------------

#ifndef pmcDocumentH
#define pmcDocumentH

#include "pmcTables.h"

#define PMC_DOCUMENT_STATUS_UNKNOWN         0
#define PMC_DOCUMENT_STATUS_FILED           1
#define PMC_DOCUMENT_STATUS_ACTIVE          2
#define PMC_DOCUMENT_STATUS_PENDING         3
#define PMC_DOCUMENT_STATUS_NEW             4

// String values in the document record
enum
{
     PMC_DOCUMENT_STRING_NAME
    ,PMC_DOCUMENT_STRING_ORIG_NAME
    ,PMC_DOCUMENT_STRING_DESCRIPTION

    ,PMC_DOCUMENT_STRING_COUNT
};

// Int values in the document record
enum
{
     PMC_DOCUMENT_INT_PATIENT_ID
    ,PMC_DOCUMENT_INT_PROVIDER_ID
    ,PMC_DOCUMENT_INT_DOCTOR_ID
    ,PMC_DOCUMENT_INT_CRC
    ,PMC_DOCUMENT_INT_TYPE
    ,PMC_DOCUMENT_INT_SIZE
    ,PMC_DOCUMENT_INT_DATE
    ,PMC_DOCUMENT_INT_APP_DATE
    ,PMC_DOCUMENT_INT_STATUS
    ,PMC_DOCUMENT_INT_NOT_DELETED

    ,PMC_DOCUMENT_INT_COUNT
};

//---------------------------------------------------------------------------
// PmcDocument Class
//---------------------------------------------------------------------------

class PmcDocument
{
public:
    __fastcall  PmcDocument( void );
    __fastcall  PmcDocument( Int32u_t myId );
    __fastcall ~PmcDocument( void );

    // String getter functions
    Char_p      __fastcall name( void )         { return this->string_p[PMC_DOCUMENT_STRING_NAME]->get(); }
    Char_p      __fastcall nameOrig( void )     { return this->string_p[PMC_DOCUMENT_STRING_ORIG_NAME]->get(); }
    Char_p      __fastcall description( void )  { return this->string_p[PMC_DOCUMENT_STRING_DESCRIPTION]->get(); }

    // Int getter functions
    Int32u_t    __fastcall id( void )           { return this->myId; }
    Int32u_t    __fastcall type( void )         { return this->valInt[PMC_DOCUMENT_INT_TYPE]; }
    Int32u_t    __fastcall date( void )         { return this->valInt[PMC_DOCUMENT_INT_DATE]; }
    Int32u_t    __fastcall appDate( void )      { return this->valInt[PMC_DOCUMENT_INT_APP_DATE]; }
    Int32u_t    __fastcall drId( void )         { return this->valInt[PMC_DOCUMENT_INT_DOCTOR_ID]; }
    Int32u_t    __fastcall status( void )       { return this->valInt[PMC_DOCUMENT_INT_STATUS]; }
    Int32u_t    __fastcall patientId( void )    { return this->valInt[PMC_DOCUMENT_INT_PATIENT_ID]; }
    Int32u_t    __fastcall providerId( void )   { return this->valInt[PMC_DOCUMENT_INT_PROVIDER_ID]; }

    Int32u_t    __fastcall modifiedTime( void ) { return this->modifiedTimeInt; }
    Int32u_t    __fastcall modifiedDate( void ) { return this->modifiedDateInt; }
    Int32u_t    __fastcall createdTime( void )  { return this->createdTimeInt; }
    Int32u_t    __fastcall createdDate( void )  { return this->createdDateInt; }

    // Setter functions
    Int32s_t    __fastcall idSet( Int32u_t id );
    Int32s_t    __fastcall descriptionSet( Char_p str_p );

    void        __fastcall nameSet( Char_p str_p )        { this->string_p[PMC_DOCUMENT_STRING_NAME]->set( str_p ); }
    void        __fastcall nameOrigSet( Char_p str_p )    { this->string_p[PMC_DOCUMENT_STRING_ORIG_NAME]->set( str_p ); }

    void        __fastcall crcSet( Int32u_t val )         { this->valInt[PMC_DOCUMENT_INT_CRC] = val; }
    void        __fastcall sizeSet( Int32u_t val )        { this->valInt[PMC_DOCUMENT_INT_SIZE] = val; }
    void        __fastcall statusSet( Int32u_t val )      { this->valInt[PMC_DOCUMENT_INT_STATUS] = val; }
    void        __fastcall typeSet( Int32u_t val )        { this->valInt[PMC_DOCUMENT_INT_TYPE] = val; }
    void        __fastcall providerIdSet( Int32u_t val )  { this->valInt[PMC_DOCUMENT_INT_PROVIDER_ID] = val; }
    void        __fastcall patientIdSet( Int32u_t val )   { this->valInt[PMC_DOCUMENT_INT_PATIENT_ID] = val; }
    void        __fastcall appDateSet( Int32u_t val )     { this->valInt[PMC_DOCUMENT_INT_APP_DATE] = val; }
    void        __fastcall dateSet( Int32u_t val )        { this->valInt[PMC_DOCUMENT_INT_DATE] = val; }
    void        __fastcall drIdSet( Int32u_t val )        { this->valInt[PMC_DOCUMENT_INT_DOCTOR_ID] = val; }

    // Utility functions
    Int32s_t    __fastcall import( void );
    Int32s_t    __fastcall importLegacy( void );
    Int32s_t    __fastcall importConsult( void );
    Int32s_t    __fastcall view( void );
    Int32s_t    __fastcall save( void );
    Int32s_t    __fastcall changed( void );
    Int32s_t    __fastcall edit( void );
    Int32s_t    __fastcall lock( Boolean_t forceFlag );
    Int32s_t    __fastcall unlock( void );
    Int32s_t    __fastcall extract( void );
    Int32s_t    __fastcall del( void );

    Boolean_t   __fastcall nameOrigExists( void );

private:

    Int32u_t    myId;
    Boolean_t   lockedFlag;
    Boolean_t   alreadyLockedFlag;

    // Init helper shared between constructors
    Int32s_t    __fastcall init( void );

    // Array of pointers to string objects - 2 copies to detect changes
    MbString    *stringIn_p[PMC_DOCUMENT_STRING_COUNT];
    MbString    *string_p[PMC_DOCUMENT_STRING_COUNT];

    // Array of int values - 2 copies to detect changes
    Int32u_t    valInt[PMC_DOCUMENT_INT_COUNT];
    Int32u_t    valIntIn[PMC_DOCUMENT_INT_COUNT];

    Int32u_t    createdDateInt;
    Int32u_t    createdTimeInt;

    Int32u_t    modifiedDateInt;
    Int32u_t    modifiedTimeInt;

    // Return the corresponding database name for the specified index
    Char_p  __fastcall dbStr( Ints_t index )
    {
        switch( index )
        {
            case PMC_DOCUMENT_STRING_NAME:           return PMC_SQL_FIELD_NAME;
            case PMC_DOCUMENT_STRING_ORIG_NAME:      return PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME;
            case PMC_DOCUMENT_STRING_DESCRIPTION:    return PMC_SQL_FIELD_DESC;

            default:
                return "UNKNOWN";
        }
    }

    // Return the corresponding database name for the specified index
    Char_p  __fastcall dbInt( Ints_t index )
    {
        switch( index )
        {
            case PMC_DOCUMENT_INT_PATIENT_ID:       return PMC_SQL_FIELD_PATIENT_ID;
            case PMC_DOCUMENT_INT_PROVIDER_ID:      return PMC_SQL_FIELD_PROVIDER_ID;
            case PMC_DOCUMENT_INT_DOCTOR_ID:        return PMC_SQL_DOCUMENTS_FIELD_DOCTOR_ID;
            case PMC_DOCUMENT_INT_CRC:              return PMC_SQL_FIELD_CRC;
            case PMC_DOCUMENT_INT_TYPE:             return PMC_SQL_FIELD_TYPE;
            case PMC_DOCUMENT_INT_SIZE:             return PMC_SQL_FIELD_SIZE;
            case PMC_DOCUMENT_INT_DATE:             return PMC_SQL_FIELD_DATE;
            case PMC_DOCUMENT_INT_APP_DATE:         return PMC_SQL_DOCUMENTS_FIELD_APP_DATE;
            case PMC_DOCUMENT_INT_STATUS:           return PMC_SQL_DOCUMENTS_FIELD_STATUS;
            case PMC_DOCUMENT_INT_NOT_DELETED:      return PMC_SQL_FIELD_NOT_DELETED;

            default:
                return "UNKNOWN";
        }
    }
};

#endif // pmcDocumentH
