//---------------------------------------------------------------------------
// Utilities for PDF file generation
//---------------------------------------------------------------------------

// System Include Files
#include <stdio.h>
//#include <dos.h>
//#include <dir.h>
//#include <process.h>

#pragma hdrstop

// Library Include Files
#include "mbUtils.h"

// Project Include Files
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcDoctor.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
// PmcDoctor Constuctor
//---------------------------------------------------------------------------
__fastcall PmcDoctor::PmcDoctor( Int32u_t id )
{
#if 0
    Char_p          buf_p = NIL;
    Int32s_t        returnCode = FALSE;
    MbSQL           sql;

    if( dr_p == NIL ) goto exit;
    memset( dr_p, 0, sizeof( PmcSqlDoctor_t ) );
    if( id == 0 ) goto exit;

    if( mbMalloc( buf_p, 1024 ) == NIL ) goto exit;

    // Format SQL command
    //                       0  1  2  3  4  5  6  7  8
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_LAST_NAME,                // 0
        PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_FIELD_PROVINCE,                 // 2
        PMC_SQL_DOCTORS_FIELD_OTHER_NUMBER,     // 3
        PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,    // 4
        PMC_SQL_DOCTORS_FIELD_CANCER_CLINIC,    // 5
        PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST,      // 6
        PMC_SQL_DOCTORS_FIELD_WORK_PHONE,       // 7
        PMC_SQL_DOCTORS_FIELD_WORK_FAX,         // 8

        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_FIELD_ID, id );

    if( sql.Query( buf_p ) == FALSE ) goto exit;
    if( sql.RowCount( ) != 1 ) goto exit;
    if( sql.RowGet( ) == FALSE ) goto exit;

    strncpy( dr_p->lastName,    sql.String( 0 ), PMC_MAX_NAME_LEN );
    strncpy( dr_p->firstName,   sql.String( 1 ), PMC_MAX_NAME_LEN );
    strncpy( dr_p->province,    sql.String( 2 ), PMC_MAX_PROV_LEN );
    strncpy( dr_p->otherNumber, sql.String( 3 ), PMC_MAX_OTHER_NUMBER_LEN );
    strncpy( dr_p->phone,       sql.String( 7 ), PMC_MAX_PHONE_LEN );
    strncpy( dr_p->fax,         sql.String( 8 ), PMC_MAX_PHONE_LEN );

    dr_p->mspNumber     = sql.Int32u( 4 );
    dr_p->cancerClinic  = ( sql.Int32u( 5 ) > 0 ) ? TRUE : FALSE;
    dr_p->mspActive     = sql.Int32u( 6 );
    dr_p->id            = id;

exit:
#endif
}

//---------------------------------------------------------------------------
// PmcDoctor Destructor
//---------------------------------------------------------------------------
__fastcall PmcDoctor::~PmcDoctor(  )
{
}
