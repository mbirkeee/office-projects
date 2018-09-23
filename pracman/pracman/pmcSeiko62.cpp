//---------------------------------------------------------------------------
// File:    pmcSeiko62.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 21, 2001
//---------------------------------------------------------------------------
// Description:
//
// Functions for managing the main Practice Manager form.
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcTables.h"
#include "pmcUtils.h"
#include "pmcGlobals.h"
#include "pmcSeiko62.h"
#include "pmcMainForm.h"
#include "pmcPatient.h"

#include "seiko\stdafx.h"
#include "seiko\SlpSdk62.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcLabelPrintAddress
(
    Char_p      add1_p,
    Char_p      add2_p,
    Char_p      add3_p,
    Char_p      add4_p,
    Char_p      add5_p,
    Char_p      add6_p,
    Char_p      add7_p,
    Char_p      add8_p
)
{
    HFONT       hFont;
    Ints_t      line;
    Ints_t      i;
    Ints_t      labelWidth, maxWidth, lineWidth;
    Char_p      print_p[4];

    SlpDebugMode( pmcCfg[CFG_SLP_DEBUG].value );

    if( SlpOpenPrinter( pmcCfg[CFG_SLP_NAME].str_p, SIZE_LARGE, FALSE ) == FALSE )
    {
        mbDlgError( "Seiko Label Printer error: %ld", SlpGetErrorCode( ) );
        goto exit;
    }

    hFont = SlpCreateFont( "Arial", 14, NORMAL );

    labelWidth = SlpGetLabelWidth( );

    print_p[0] = add1_p;
    print_p[1] = add2_p;
    print_p[2] = add3_p;
    print_p[3] = add4_p;

    for( maxWidth = 0, i = 0 ; i < 4 ; i++ )
    {
        if( ( lineWidth = SlpGetTextWidth( hFont, print_p[i] ) ) > maxWidth ) maxWidth = lineWidth;
    }

    if( maxWidth < labelWidth ) goto print;

    SlpDeleteFont( hFont );
    hFont = SlpCreateFont( "Arial", 12, NORMAL );

    for( maxWidth = 0, i = 0 ; i < 4 ; i++ )
    {
        if( ( lineWidth = SlpGetTextWidth( hFont, print_p[i] ) ) > maxWidth ) maxWidth = lineWidth;
    }

    if( maxWidth < labelWidth ) goto print;

    print_p[0] = add5_p;
    print_p[1] = add6_p;
    print_p[2] = add7_p;
    print_p[3] = add8_p;

print:

    SlpStartLabel();
    for( line = 0 ; line < 4 ; line++ )
    {
        SlpDrawTextXY( PMC_SLP_ADD_OFFSET, PMC_SLP_ADD_LINE( line ), hFont, print_p[line] );
    }
    SlpEndLabel( );

    SlpDeleteFont( hFont );

    SlpClosePrinter();

exit:

    return 0;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcLabelPrintPatAddress
(
    Int32u_t        id
)
{
    Int32u_t        recordCount;
    Char_p          cmd_p;
    Char_p          firstName_p;
    Char_p          lastName_p;
    Char_p          address1_p;
    Char_p          address2_p;
    Char_p          add1_p;
    Char_p          add2_p;
    Char_p          add3_p;
    Char_p          add4_p;
    Char_p          add5_p;
    Char_p          add6_p;
    Char_p          add7_p;
    Char_p          add8_p;
    Char_t          title[16];
    Char_t          city[40];
    Char_t          postalCode[16];
    Char_t          province[8];
    MbSQL           sql;

    mbMalloc( cmd_p, 512 );
    mbMalloc( firstName_p, 64 );
    mbMalloc( lastName_p, 64 );
    mbMalloc( address1_p, 64 );
    mbMalloc( address2_p, 64 );
    mbMalloc( add1_p, 128 );
    mbMalloc( add2_p, 128 );
    mbMalloc( add3_p, 128 );
    mbMalloc( add4_p, 128 );
    mbMalloc( add5_p, 128 );
    mbMalloc( add6_p, 128 );
    mbMalloc( add7_p, 128 );
    mbMalloc( add8_p, 128 );

    sprintf( cmd_p, "select %s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_FIRST_NAME,       // 0
        PMC_SQL_FIELD_LAST_NAME,        // 1
        PMC_SQL_FIELD_TITLE,            // 2
        PMC_SQL_FIELD_ADDRESS1,         // 3
        PMC_SQL_FIELD_ADDRESS2,         // 4
        PMC_SQL_FIELD_CITY,             // 5
        PMC_SQL_FIELD_PROVINCE,         // 6
        PMC_SQL_FIELD_POSTAL_CODE,      // 7
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID, id );

    recordCount = 0;

    sql.Query( cmd_p );
    while( sql.RowGet( ) )
    {
        strncpy( firstName_p,   sql.String( 0 ), 64 );
        strncpy( lastName_p,    sql.String( 1 ), 64 );
        strncpy( title,         sql.String( 2 ), 16 );
        strncpy( address1_p,    sql.String( 3 ), 40 );
        strncpy( address2_p,    sql.String( 4 ), 40 );
        strncpy( city,          sql.String( 5 ), 40 );
        strncpy( province,      sql.String( 6 ), 8 );
        strncpy( postalCode,    sql.String( 7 ), 16 );
        recordCount++;
    }

    // Sanity Check
    if( recordCount != 1 ) mbDlgDebug(( "Error: recordCount: %ld", recordCount ));

    pmcMakeMailingAddress( title, firstName_p, lastName_p,
                           address1_p,
                           address2_p,
                           city, province, postalCode,
                           add1_p, add2_p, add3_p, add4_p, TRUE );

    pmcMakeMailingAddress( title, firstName_p, lastName_p,
                           address1_p,
                           address2_p,
                           city, province, postalCode,
                           add5_p, add6_p, add7_p, add8_p, FALSE );

    sprintf( cmd_p, "Print address label for %s %s?\n\n\n%s\n%s\n%s\n%s",
        firstName_p, lastName_p,
        add1_p, add2_p, add3_p, add4_p );

    if( mbDlgOkCancel( cmd_p ) == MB_BUTTON_OK )
    {
        mbLog( "Printed patient address label for '%s %s' (id: %d)\n", firstName_p, lastName_p, id );
        pmcLabelPrintAddress( add1_p, add2_p, add3_p, add4_p, add5_p, add6_p, add7_p, add8_p );
    }

    mbFree( cmd_p );
    mbFree( firstName_p );
    mbFree( lastName_p );
    mbFree( address1_p );
    mbFree( address2_p );
    mbFree( add1_p );
    mbFree( add2_p );
    mbFree( add3_p );
    mbFree( add4_p );
    mbFree( add5_p );
    mbFree( add6_p );
    mbFree( add7_p );
    mbFree( add8_p );

    return 0;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcLabelPrintDrAddress
(
    Int32u_t            id
)
{
    Int32u_t        recordCount;
    Char_p          cmd_p;
    Char_p          firstName_p = NIL;
    Char_p          lastName_p = NIL;
    Char_p          address1_p = NIL;
    Char_p          address2_p = NIL;;
    Char_p          add1_p;
    Char_p          add2_p;
    Char_p          add3_p;
    Char_p          add4_p;
    Char_p          add5_p;
    Char_p          add6_p;
    Char_p          add7_p;
    Char_p          add8_p;
    Char_p          city_p = NIL;
    Char_t          postalCode[16];
    Char_t          province[8];
    MbSQL           sql;

    mbMalloc( cmd_p, 512 );
    mbMalloc( add1_p, 256 );
    mbMalloc( add2_p, 256 );
    mbMalloc( add3_p, 256 );
    mbMalloc( add4_p, 256 );
    mbMalloc( add5_p, 256 );
    mbMalloc( add6_p, 256 );
    mbMalloc( add7_p, 256 );
    mbMalloc( add8_p, 256 );

    sprintf( cmd_p, "select %s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_FIRST_NAME,   // 0
        PMC_SQL_FIELD_LAST_NAME,    // 1
        PMC_SQL_FIELD_ADDRESS1,     // 2
        PMC_SQL_FIELD_ADDRESS2,     // 3
        PMC_SQL_FIELD_CITY,         // 4
        PMC_SQL_FIELD_PROVINCE,     // 5
        PMC_SQL_FIELD_POSTAL_CODE,  // 6
        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_FIELD_ID, id );

    recordCount = 0;

    sql.Query( cmd_p );
    while( sql.RowGet( ) )
    {
        mbMallocStr( firstName_p,   sql.String( 0 ) );
        mbMallocStr( lastName_p,    sql.String( 1 ) );
        mbMallocStr( address1_p,    sql.String( 2 ) );
        mbMallocStr( address2_p,    sql.String( 3 ) );
        mbMallocStr( city_p,        sql.String( 4 ) );
        strncpy( postalCode,        sql.String( 6 ), 16 );
        strncpy( province,          sql.String( 5 ),  8 );

        recordCount++;
    }

    // Sanity Check
    if( recordCount != 1 ) mbDlgDebug(( "Error: recordCount: %ld", recordCount ));

    pmcMakeMailingAddress( "Dr", firstName_p, lastName_p,
                           address1_p,
                           address2_p,
                           city_p, province, postalCode,
                           add1_p, add2_p, add3_p, add4_p, TRUE );
    pmcMakeMailingAddress( "Dr", firstName_p, lastName_p,
                           address1_p,
                           address2_p,
                           city_p, province, postalCode,
                           add5_p, add6_p, add7_p, add8_p, FALSE );

    sprintf( cmd_p, "Print address label for %s %s?\n\n\n%s\n%s\n%s\n%s",
        firstName_p, lastName_p,
        add1_p, add2_p, add3_p, add4_p );

    if( mbDlgOkCancel( cmd_p ) == MB_BUTTON_OK )
    {
        mbLog( "Printed doctor address label for '%s %s' (id: %d)\n", firstName_p, lastName_p, id );
        pmcLabelPrintAddress( add1_p, add2_p, add3_p, add4_p, add5_p, add6_p, add7_p, add8_p );
    }

    mbFree( cmd_p );
    mbFree( add1_p );
    mbFree( add2_p );
    mbFree( add3_p );
    mbFree( add4_p );
    mbFree( add5_p );
    mbFree( add6_p );
    mbFree( add7_p );
    mbFree( add8_p );


    if( city_p )        mbFree( city_p );
    if( firstName_p)    mbFree( firstName_p );
    if( lastName_p )    mbFree( lastName_p );
    if( address1_p )    mbFree( address1_p );
    if( address2_p )    mbFree( address2_p );

    return 0;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcLabelPrintPatReq
(
    Int32u_t            appointIdIn,
    Int32u_t            providerIdIn,
    Int32u_t            patientIdIn
)
{
    Int32u_t            providerId;
    Int32u_t            patientId;
    Int32u_t            referringId = 0;
    Int32u_t            referringIdApp = 0;
    Char_t              curDate[32];
    Char_t              curTime[32];
    Char_t              buf[256];
    HFONT               hFont1;
    HFONT               hFont2;
    HFONT               hFont3;
    HFONT               hFont4;
    Ints_t              line;
    Ints_t              labelWidth, textWidth;
    MbSQL               sql;
    MbDateTime          dateTime;
    PmcPatient          patient;
    MbString            phoneStr;
    MbString            nameStr;
    MbString            addressStr1;
    MbString            addressStr2;
    MbString            cmd;
    MbString            dobStr;
    MbString            providerStr;
    MbString            referringStr;

    if( appointIdIn == 0 && patientIdIn == 0 ) return 0;

    // Get current data and time to place on the label
    dateTime.SetDateTime( mbToday(), mbTime() );
    sprintf( curDate, "%s", dateTime.MDY_DateString( ) );
    sprintf( curTime, "%s", dateTime.HM_TimeString( ) );

    if( appointIdIn )
    {
        // Function called from appointment view... must read
        // patient Id from database to be safe
        mbSprintf( &cmd, "select %s from %s where %s=%ld and %s=%d",
                 PMC_SQL_FIELD_PATIENT_ID,
                 PMC_SQL_TABLE_APPS,
                 PMC_SQL_FIELD_ID, appointIdIn,
                 PMC_SQL_FIELD_NOT_DELETED,
                 PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

        patientId = pmcSqlSelectInt( cmd.get(), NIL );

        // Also get the referring Dr ID from the appointment.  We will use this
        // as the "Family Dr. on the label
        mbSprintf( &cmd, "select %s from %s where %s=%ld and %s=%d",
                 PMC_SQL_APPS_FIELD_REFERRING_DR_ID,
                 PMC_SQL_TABLE_APPS,
                 PMC_SQL_FIELD_ID, appointIdIn,
                 PMC_SQL_FIELD_NOT_DELETED,
                 PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

         referringIdApp = pmcSqlSelectInt( cmd.get(), NIL );
    }
    else
    {
        patientId = patientIdIn;
    }

    if( patientId == 0 )
    {
        mbDlgError( "Could not determine patient." );
        goto exit;
    }

    // Get the patient details
    if( patient.idSet( patientId ) != MB_RET_OK )
    {
        mbDlgError( "Error getting patient details for patient ID %lu", patientId );
        goto exit;
    }

    // Determine the provider (referring Dr. on label)
    if( patient.providerId() ) providerId = patient.providerId();

    // Passed in provider (i.e. app provider) takes precedence over patient record provider
    if( providerIdIn ) providerId = providerIdIn;

    if( providerId == 0 )
    {
        if( mbDlgOkCancel( "No provider. Continue printing label?" ) == MB_BUTTON_CANCEL )
        {
            goto exit;
        }
        sprintf( buf, "" );
    }
    else
    {
        pmcProviderDescGet( providerId, buf );
    }
    providerStr.set( buf );

    // Determine the referring Dr. (CC on the label) - lowest priority is family Dr.
    if( patient.familyDrId() ) referringId = patient.familyDrId();

    // Override with referring Dr, if specified
    if( patient.referringId() ) referringId = patient.referringId();

    // Finally, appointment referring Dr. takes precedence over patient record
    if( referringIdApp ) referringId = referringIdApp;

    if( referringId == 0 )
    {
       sprintf( buf, "" );
    }
    else
    {
        // Must get referring Dr info from database
        PmcSqlDoctor_p doctor_p;
        mbMalloc( doctor_p, sizeof(PmcSqlDoctor_t) );
        if( pmcSqlDoctorDetailsGet( referringId, doctor_p ) )
        {
            sprintf( buf, "Dr. %s %s",  doctor_p->firstName, doctor_p->lastName );
        }
        else
        {
            sprintf( buf, "Unknown (id: %ld)", referringId );
        }
        mbFree( doctor_p );
    }
    referringStr.set( buf );

    mbSprintf( &nameStr, "%s, %s (%s)", patient.lastName( ), patient.firstName( ),  patient.title( ) );
    mbSprintf( &addressStr1, "%s  %s", patient.address1( ), patient.address2( ) );
    mbSprintf( &addressStr2, "%s %s  %s",  patient.city(), patient.province(), patient.postalCode() );
    mbSprintf( &phoneStr, "H: %s  W: %s", patient.phoneHome(),  patient.phoneWork() );

    if( patient.dateOfBirth() )
    {
        mbSprintf( &dobStr, "%s", patient.dateString( patient.dateOfBirth() ));
    }
    else
    {
        mbSprintf( &dobStr, "Unknown" );
    }

    // Prompt user before going ahead with the print.  Attempt to display
    // all information here that will go on the label
    mbSprintf( &cmd, "Print requisition label for %s %s?\n\nPHN:\t%s\nDOB:\t%s\nNAME:\t%s\nADDR:\t%s\n\t%s\n\t%s\nREF:\t%s\nCC:\t%s",
        patient.firstName(), patient.lastName(),
        patient.phn(),
        dobStr.get(),
        nameStr.get(),
        addressStr1.get(),
        addressStr2.get(),
        phoneStr.get(),
        providerStr.get(),
        referringStr.get() );

    if( mbDlgOkCancel( cmd.get() ) == MB_BUTTON_CANCEL ) goto exit;

    SlpDebugMode( pmcCfg[CFG_SLP_DEBUG].value );

    mbLog( "SLP printer name: '%s'\n", pmcCfg[CFG_SLP_NAME].str_p );
    if( SlpOpenPrinter( (LPSTR)pmcCfg[CFG_SLP_NAME].str_p, SIZE_LARGE, FALSE ) == FALSE )
    {
        mbDlgError( "Seiko Label Printer error: %ld", SlpGetErrorCode( ) );
        goto exit;
    }

    mbLog( "Printed patient requisition label for '%s %s' (id: %d)\n",
        patient.firstName(), patient.lastName(), patientId );

    hFont1 = SlpCreateFont( "Arial", 12, BOLD  );
    hFont2 = SlpCreateFont( "Arial", 12, NORMAL);
    hFont3 = SlpCreateFont( "Arial", 10, NORMAL );
    hFont4 = SlpCreateFont( "Arial",  8, NORMAL );

    labelWidth = SlpGetLabelWidth( );
    //labelHeight = SlpGetLabelHeight( );

    SlpStartLabel();
    line = 0;
    SlpDrawTextXY( 1, PMC_SLP_REQ_LINE( line ), hFont2, "PHN:" );
    SlpDrawTextXY( PMC_SLP_REQ_OFFSET, PMC_SLP_REQ_LINE( line ), hFont1, patient.phn() );
    line++;

    SlpDrawTextXY( 1, PMC_SLP_REQ_LINE( line ), hFont2, "NAME:" );
    SlpDrawTextXY( PMC_SLP_REQ_OFFSET, PMC_SLP_REQ_LINE( line ), hFont2, nameStr.get() );
    line++;

    if( addressStr1.length() )
    {
        SlpDrawTextXY( PMC_SLP_REQ_OFFSET, PMC_SLP_REQ_LINE( line ), hFont2, addressStr1.get() );
        line++;
    }

    if( addressStr2.length() )
    {
        SlpDrawTextXY( PMC_SLP_REQ_OFFSET, PMC_SLP_REQ_LINE( line ), hFont2, addressStr2.get() );
        line++;
    }

    if( phoneStr.length() )
    {
        SlpDrawTextXY( PMC_SLP_REQ_OFFSET, PMC_SLP_REQ_LINE( line ), hFont2, phoneStr.get() );
        line++;
    }

    SlpDrawTextXY( 1, PMC_SLP_REQ_LINE( line ), hFont2, "REF:" );
    SlpDrawTextXY( PMC_SLP_REQ_OFFSET, PMC_SLP_REQ_LINE( line ), hFont2, providerStr.get() );
    line++;

    SlpDrawTextXY( 1, PMC_SLP_REQ_LINE( line ), hFont3, "CC:" );
    SlpDrawTextXY( PMC_SLP_REQ_OFFSET, PMC_SLP_REQ_LINE( line ), hFont3, referringStr.get() );

    // Right justify DOB in upper corner
    textWidth = SlpGetTextWidth( hFont2, dobStr.get() );
    SlpDrawTextXY( labelWidth - textWidth , PMC_SLP_REQ_LINE( 0 ), hFont2, dobStr.get() );
    textWidth += SlpGetTextWidth( hFont2, "DOB:" );
    SlpDrawTextXY( labelWidth - textWidth - 20 , PMC_SLP_REQ_LINE( 0 ), hFont2, "DOB:" );

    // Right justify printed timestamp
    mbSprintf( &cmd, "%s %s", curTime, curDate );
    textWidth = SlpGetTextWidth( hFont4, cmd.get() );
    SlpDrawTextXY( labelWidth - textWidth , 312, hFont4, cmd.get() );

    SlpDrawLine( 1, 352, labelWidth, 352, 3 );

    // Footer string
    mbSprintf( &cmd, "402 Queen Street - Phone: 975-0600 Fax: 978-5254" );
    textWidth = SlpGetTextWidth( hFont4, cmd.get() );

    // Center footer
    SlpDrawTextXY( ( ( labelWidth - textWidth ) / 2 ), 355, hFont4, cmd.get() );

#if 0
    // Draw some test lines on the label
    for( i = 0 ; i < 30 ; i++ )
    {
        sprintf( cmd_p, "Line: %d (1, %d)", i, i*10 );
        SlpDrawTextXY(1, i*10 , hFont3, cmd_p );
        SlpDrawLine( 150, i*10, 600, i*10, 2 );
    }
#endif


    SlpEndLabel();

    SlpClosePrinter();
    SlpDeleteFont( hFont1 );
    SlpDeleteFont( hFont2 );
    SlpDeleteFont( hFont3 );
    SlpDeleteFont( hFont4 );

exit:

    return 0;
}

#if 0

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcTestLabelPrinter( void )
{
    bool    returnCode;
    Ints_t  errorCode;
   // LPSTR Port =  "COM1";
    returnCode = SlpSetPrinter( SLP_200, "COM1" );
    mbDlgDebug(( "SlpSetPrinter(): %d", returnCode ));

    if( returnCode == FALSE )
    {
        errorCode = SlpGetErrorCode( );
        mbDlgDebug(( "Error: %d", errorCode ));
    }
    else
    {
        HFONT hFont1;
        HFONT hFont2;
        HFONT hFont3;

        hFont1 = SlpCreateFont("Arial", 14, NORMAL);
        hFont2 = SlpCreateFont("Arial", 12, BOLD | ITALIC);
        hFont3 = SlpCreateFont("Times New Roman", 12, BOLD);
        SlpSetLabelType(SIZE_STANDARD, LANDSCAPE);
        SlpNewLabel();
        SlpDrawTextXY(1, 10, hFont1, "This is Arial 14pt Normal");
        SlpDrawTextXY(1, 50, hFont2, "This is Arial 12pt Bold Italic");
        SlpDrawTextXY(1, 85, hFont3, "This is Times New Roman 12pt Bold");
        SlpDeleteFont(hFont1);
        SlpDeleteFont(hFont2);
        SlpDeleteFont(hFont3);
        SlpPrintLabel(1);
    }
}
//---------------------------------------------------------------------------
#endif
