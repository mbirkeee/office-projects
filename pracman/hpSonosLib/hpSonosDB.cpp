//---------------------------------------------------------------------------
// File:    hpSonosDB.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Nov. 6, 2002
//---------------------------------------------------------------------------
// Description:
//
// Utilities for processing the HP SONOS 5500 database files.
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#define HPS_INIT_GLOBALS
#include "hpSonosDB.h"
#undef  HPS_INIT_GLOBALS

//---------------------------------------------------------------------------
// hpsDBIDQueueGet
//---------------------------------------------------------------------------
// Returns a queue of patient IDs detected in the database file
//---------------------------------------------------------------------------

qHead_p hpsDBIDQueueGet( Char_p name_p, qHead_p q_p )
{
    HpsDBHandle_t   handle = NIL;
    HpsDBRecord_t   record;
    HpsDBPatId_p    id_p;
    Boolean_t       found;

    if( q_p == NIL ) goto exit;

    if( ( handle = hpsDBOpen( name_p ) ) == NIL ) goto exit;

    while( hpsDBNextRecordGet( handle, &record ) )
    {
        // hpsDBRecordDisplay( &record );
        found = FALSE;
        qWalk( id_p, q_p, HpsDBPatId_p )
        {
            //if( strcmp( record.patId.lastName, id_p->lastName ) == 0 )
            if( strcmp( record.patId.id, id_p->id ) == 0 )
            {
                found = TRUE;
                break;
            }
        }

        if( found == FALSE )
        {
            mbMalloc( id_p, sizeof(HpsDBPatId_t) );
            memcpy( id_p, &record.patId, sizeof(HpsDBPatId_t) );
            qInsertLast( q_p, id_p );
        }
    }

exit:
    hpsDBClose( handle );

    return q_p;
}

//---------------------------------------------------------------------------
// hpsDBRecordTypeString
//---------------------------------------------------------------------------
Char_p hpsDBRecordTypeString( Int32u_t  type )
{
    switch( type )
    {
        case HPS_DB_RECORD_TYPE_HEADER:    return "HPS_DB_RECORD_TYPE_HEADER";
        case HPS_DB_RECORD_TYPE_HEADER_EX: return "HPS_DB_RECORD_TYPE_HEADER_EX";
        case HPS_DB_RECORD_TYPE_ENTRY:     return "HPS_DB_RECORD_TYPE_ENTRY";
        default: break;
    }
    return "HPS_DB_RECORD_TYPE_INVALID";
}

//---------------------------------------------------------------------------
// hpsDBRecordDisplay
//---------------------------------------------------------------------------
// Return the database file type
//
//typedef struct HpsDBRecord_s
//{
//    Char_t              fileName[16];
//    HpsDBPatId_t        patId;
//    HpsDBDateTime_t     date;
//    Int32u_t            type;
//} HpsDBRecord_t, *HpsDBRecord_p;

//---------------------------------------------------------------------------

Int32s_t    hpsDBRecordDisplay( HpsDBRecord_p record_p )
{
    Int32s_t        returnCode = MB_RET_ERR;
    if( record_p == NIL ) goto exit;

    mbLog( "RECORD type:             %s\n",  hpsDBRecordTypeString( record_p->type ) );
    mbLog( "RECORD filename:        '%s'\n", record_p->fileName );
    mbLog( "RECORD pat.Id.raw:      '%s'\n", record_p->patId.raw );
    mbLog( "RECORD pat.Id.fullName: '%s'\n", record_p->patId.fullName );

    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// hpsDBNextRecordGet
//---------------------------------------------------------------------------
// Return the database file type
//---------------------------------------------------------------------------

Int32s_t  hpsDBNextRecordGet
(
    HpsDBHandle_t   handle,
    HpsDBRecord_p   record_p
)
{
    Int32s_t        returnCode = MB_RET_ERR;
    Int32s_t        result;
    Ints_t          i, j;
    HpsDBFile_p     handle_p = handle;
    Char_p          buf_p;
    Char_p          buf2_p;
    Int32u_t        type;
    Int32u_t        length;
    Ints_t          recLength;

    mbCalloc( buf_p, HPS_SONOS_BUF_SIZE );
    mbCalloc( buf2_p, 128 );

    HPS_VALIDATE_HANDLE( handle_p );

    for( ; ; )
    {
        // Read the rext record.  It is possible to get an OK return
        // code but the record is invalid (length < 0)
        result = hpsDBNextRecordGetRaw( handle_p, &type, &length, (Int8u_p)buf_p, HPS_SONOS_BUF_SIZE );

        if( result != MB_RET_OK ) goto exit;

        // Get the length from the record itself
        for( i = 0 ; i < 4 ; i++ )
        {
            *( buf2_p + i ) = *( buf_p + i );
        }
        *( buf2_p + i ) = 0;
        sscanf( buf2_p, "%X", &recLength );

        if( recLength != (Ints_t)hpsDBRecordLen[HPS_DB_RECORD_TYPE_ENTRY] )
        {
            mbDlgDebug(( "Got unexpected record length %ld\n", recLength ));
            continue;
        }

        // Sanity Check
        if( type != HPS_DB_RECORD_TYPE_ENTRY )
        {
            mbDlgDebug(( "Got unexpected record type %ld\n", type ));
            goto exit;
        }

        // At this point, the record must be valid
        break;
    }

    record_p->type = type;
    
    // Get the filename
    if( handle_p->header_p->type == HPS_DB_FILE_TYPE_ECHO )
    {
        for( i = 0 , j = HPS_DB_FILENAME_OFFSET_ENTRY ; i < HPS_DB_FILENAME_LEN + 1 ; i++, j++ )
        {
            *( buf2_p + i ) = *( buf_p + j );
            if( i == 7 )
            {
                *( buf2_p + ++i ) = '.';
            }
        }
        *( buf2_p + i ) = 0;
        mbStrClean( buf2_p, record_p->fileName, FALSE );
    }
    else
    {
        mbLog( "This is a root file - not getting fileName\n" );
    }

    // Get patient id
    for( i = 0 , j = HPS_DB_PATIENTID_OFFSET_ENTRY ;
         i < HPS_DB_PATIENTID_LEN ; i++, j++ )
    {
        *( buf2_p + i ) = *( buf_p + j );
    }
    *( buf2_p + i ) = 0;
    mbStrClean( buf2_p, record_p->patId.raw, FALSE );

    //mbLog( "Got patientIdRaw:    '%s'\n", record_p->patId.raw );
    //mbLog( "Header patientIDRaw: '%s'\n", handle_p->header_p->patId.raw );

    hpsDBPatientIdClean( &record_p->patId );


    returnCode = MB_RET_OK;
exit:
    mbFree( buf_p );
    mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// hpsDBTypeGet
//---------------------------------------------------------------------------
// Return the database file type
//---------------------------------------------------------------------------

Int32u_t  hpsDBTypeGet
(
    HpsDBHandle_t   handle
)
{
    Int32u_t        result = HPS_DB_FILE_TYPE_INVALID;
    HpsDBFile_p     handle_p = handle;

    HPS_VALIDATE_HANDLE( handle_p );
    result = handle_p->header_p->type;
exit:
    return result;
}

//---------------------------------------------------------------------------
// hpsDBDateGet
//---------------------------------------------------------------------------
// Return the database file type
//---------------------------------------------------------------------------

Int32u_t  hpsDBDateGet
(
    HpsDBHandle_t   handle
)
{
    Int32u_t        result = HPS_DB_FILE_TYPE_INVALID;
    HpsDBFile_p     handle_p = handle;

    HPS_VALIDATE_HANDLE( handle_p );
    result = handle_p->header_p->date.date;
exit:
    return result;
}
//---------------------------------------------------------------------------
// hpsDBTimeGet
//---------------------------------------------------------------------------
// Return the database file type
//---------------------------------------------------------------------------

Int32u_t  hpsDBTimeGet
(
    HpsDBHandle_t   handle
)
{
    Int32u_t        result = HPS_DB_FILE_TYPE_INVALID;
    HpsDBFile_p     handle_p = handle;

    HPS_VALIDATE_HANDLE( handle_p );
    result = handle_p->header_p->date.time;
exit:
    return result;
}

//---------------------------------------------------------------------------
// hpsDBPatNameGet
//---------------------------------------------------------------------------
// Return the database file type
//---------------------------------------------------------------------------

Char_p  hpsDBPatNameGet
(
    HpsDBHandle_t   handle
)
{
    Char_p          result_p = NIL;
    HpsDBFile_p     handle_p = handle;

    HPS_VALIDATE_HANDLE( handle_p );
    if( strlen( handle_p->header_p->patId.fullName ) )
    {
        result_p = handle_p->header_p->patId.fullName;
    }

exit:
    return result_p;
}

//---------------------------------------------------------------------------
// hpsDBDateGet
//---------------------------------------------------------------------------
// Return the database file type
//---------------------------------------------------------------------------

Char_p  hpsDBDateStringGet
(
    HpsDBHandle_t   handle
)
{
    Char_p          result_p = NIL;
    HpsDBFile_p     handle_p = handle;

    HPS_VALIDATE_HANDLE( handle_p );
    if( strlen( handle_p->header_p->date.dateString ) )
    {
        result_p = handle_p->header_p->date.dateString;
    }

exit:
    return result_p;
}

//---------------------------------------------------------------------------
// hpsDBPatNameGet
//---------------------------------------------------------------------------
// Return the database file type
//---------------------------------------------------------------------------

Char_p  hpsDBTimeStringGet
(
    HpsDBHandle_t   handle
)
{
    Char_p          result_p = NIL;
    HpsDBFile_p     handle_p = handle;

    HPS_VALIDATE_HANDLE( handle_p );
    if( strlen( handle_p->header_p->date.timeString ) )
    {
        result_p = handle_p->header_p->date.timeString;
    }

exit:
    return result_p;
}

//---------------------------------------------------------------------------
// hpsDBHeaderFree
//---------------------------------------------------------------------------
// Free a header structure
//---------------------------------------------------------------------------

Int32u_t  hpsDBHeaderFree
(
    HpsDBFileHeader_p   header_p
)
{
    Int32s_t    returnCode = MB_RET_ERR;

    HPS_VALIDATE_HEADER( header_p );
    mbFree( header_p );
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// hpsBDOpen
//---------------------------------------------------------------------------
// This function opens a HPSONOS.DB file and returns a handle to the calling
// code.
//---------------------------------------------------------------------------

HpsDBHandle_t   hpsDBOpen
(
    Char_p      name_p
)
{
    HpsDBFile_p     handle_p = 0;
    Int32s_t        result = MB_RET_ERR;
    Int8u_p         buf_p;
    Int32u_t        type;
    Int32u_t        length;

    if( name_p == NIL ) goto exit;

    mbCalloc( handle_p, sizeof(HpsDBFile_t) );
    mbCalloc( buf_p, HPS_SONOS_BUF_SIZE );

    if( handle_p == NIL || buf_p == NIL ) goto exit;

    // Allocate space for the header structure
    mbCalloc( handle_p->header_p, sizeof( HpsDBFileHeader_t ) );

    if( handle_p->header_p == NIL ) goto exit;

    if( ( handle_p->fp = fopen( name_p, "rb" ) ) == NIL ) goto exit;

#if HPS_DEBUG
    handle_p->magic = HPS_DB_MAGIC;
#endif

    // Now lets read the first record and determine the file type
    result = hpsDBNextRecordGetRaw( handle_p, &type, &length, buf_p, HPS_SONOS_BUF_SIZE );

    if( result != MB_RET_OK ) goto exit;

    // Sanity check
    if( type != HPS_DB_RECORD_TYPE_HEADER )
    {
        mbDlgDebug(( "Error: expected header (type %d); got type %d", HPS_DB_RECORD_TYPE_HEADER, type ));
        goto exit;
    }

    // Extract the header information
    if( ( result = hpsDBHeaderGet( buf_p, length, handle_p->header_p ) ) != MB_RET_OK )
    {
        mbDlgDebug(( "Error extracting header information" ));
        goto exit;
    }

    if( handle_p->header_p->type == HPS_DB_FILE_TYPE_ROOT )
    {
        result = MB_RET_OK;
        goto exit;
    }

    // The echo database files have an extended header record
    result = hpsDBNextRecordGetRaw( handle_p, &type, &length, buf_p, HPS_SONOS_BUF_SIZE );

    if( result != MB_RET_OK ) goto exit;

    // Sanity check
    if( type != HPS_DB_RECORD_TYPE_HEADER_EX )
    {
        mbDlgDebug(( "Error: expected header (type %d); got type %d", HPS_DB_RECORD_TYPE_HEADER_EX, type ));
        goto exit;
    }

    // Extract the extended header information
    if( ( result = hpsDBHeaderExGet( buf_p, length, handle_p->header_p ) ) != MB_RET_OK )
    {
        mbDlgDebug(( "Error extracting extended header information" ));
        goto exit;
    }

    result = MB_RET_OK;
exit:

    if( result != MB_RET_OK )
    {
        if( handle_p )
        {
            if( handle_p->fp ) fclose( handle_p->fp );

            hpsDBHeaderFree( handle_p->header_p );
            mbFree( handle_p );
            handle_p = NIL;
        }
    }

    if( buf_p ) mbFree( buf_p );

    return (HpsDBHandle_t)handle_p;
}

//---------------------------------------------------------------------------
// hpsDBHeaderGet
//---------------------------------------------------------------------------
// This function extracts the data from a raw header record into a struct
//---------------------------------------------------------------------------

Int32s_t hpsDBHeaderGet
(
    Int8u_p             buf_p,
    Int32u_t            length,
    HpsDBFileHeader_p   header_p
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    if( buf_p == NIL || header_p == NIL ) goto exit;

    sscanf( (Char_p)buf_p, "%X", &header_p->size );

    switch( header_p->size )
    {
        case HPS_DB_FILE_HEADER_SIZE_ROOT:
            header_p->type = HPS_DB_FILE_TYPE_ROOT;
            break;

        case HPS_DB_FILE_HEADER_SIZE_ECHO:
            header_p->type = HPS_DB_FILE_TYPE_ECHO;
            break;

        default:
            mbDlgDebug(( "Unexpected header size: %d\n", header_p->size ));
            goto exit;
    }

    returnCode = MB_RET_OK;

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// hpsDBHeaderExGet
//---------------------------------------------------------------------------
// This function extracts the data from a raw header record into a struct
//---------------------------------------------------------------------------

Int32s_t hpsDBHeaderExGet
(
    Int8u_p             in_p,
    Int32u_t            length,
    HpsDBFileHeader_p   header_p
)
{
    Int8u_p     buf_p;
    Int32s_t    returnCode = MB_RET_ERR;
    Ints_t      i, j;

    mbMalloc( buf_p, 512 );
    if( in_p == NIL || header_p == NIL ) goto exit;

    // Get directory name
    for( i = 0 , j = HPS_DB_FILENAME_OFFSET_HEADER ;
         i < HPS_DB_FILENAME_LEN ; i++, j++ )
    {
        *( buf_p + i ) = *( in_p + j );
    }
    *( buf_p + i ) = 0;
    mbStrClean( (Char_p)buf_p, header_p->fileName, FALSE );

    // mbLog( "Got filename '%s'\n", header_p->fileName );

    // Get patient id
    for( i = 0 , j = HPS_DB_PATIENTID_OFFSET_HEADER ;
         i < HPS_DB_PATIENTID_LEN ; i++, j++ )
    {
        *( buf_p + i ) = *( in_p + j );
    }
    *( buf_p + i ) = 0;
    mbStrClean( (Char_p)buf_p, header_p->patId.raw, TRUE );

    //mbLog( "Got patientIdRaw '%s'\n", header_p->patId.raw );
    hpsDBPatientIdClean( &header_p->patId );

    // Get date and time
    for( i = 0 , j = HPS_DB_DATETIME_OFFSET_HEADER ;
         i < HPS_DB_DATETIME_LEN ; i++, j++ )
    {
        *( buf_p + i ) = *( in_p + j );
    }
    *( buf_p + i ) = 0;
    mbStrClean( (Char_p)buf_p, header_p->date.raw, TRUE );
    hpsDBDateTimeClean( &header_p->date );
    returnCode = MB_RET_OK;

exit:
    mbFree( buf_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// hpsDBPatientIdClean
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t hpsDBDateTimeClean
(
    HpsDBDateTime_p     date_p
)
{
    Int32u_t            year = 0;
    Int32u_t            month = 0;
    Int32u_t            day = 0;
    Int32u_t            hour = 0;
    Int32u_t            minute = 0;
    Int32u_t            second = 0;
    Int32s_t            returnCode = MB_RET_ERR;

    if( date_p != NIL )
    {
        sscanf ( date_p->raw, "%ld:%ld:%ld %ld:%ld:%ld", &year, &month, &day, &hour, &minute, &second );

        MbDateTime dateTime = MbDateTime( year, month, day, hour, minute, second );

        // Sanity checks
        if( year < 2000 || year > 2100 ) goto exit;
        if( month < 1 || month > 12 )    goto exit;
        if( day < 1 || day > 31 )        goto exit;
        if( hour > 23 )                  goto exit;
        if( minute > 59 )                goto exit;
        if( second > 59 )                goto exit;

        date_p->date = dateTime.Date();
        date_p->time = dateTime.Time();

        sprintf( date_p->timeString, dateTime.HM_TimeString( ) );
        sprintf( date_p->dateString, dateTime.MDY_DateString( ) );

        returnCode = MB_RET_OK;
    }
exit:

    if( returnCode != MB_RET_OK )
    {
        mbDlgDebug(( "Got invalid time string '%s'\n", date_p->raw ));
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// hpsDBPatientIdClean
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32s_t hpsDBPatientIdClean
(
    HpsDBPatId_p    id_p
)
{
    Int32s_t        returnCode = MB_RET_ERR;
    Ints_t          i;
    Ints_t          j;
    Char_p          buf_p;
    Char_p          temp_p;

    if( id_p == NIL ) goto exit;

    temp_p = &id_p->raw[0];

    mbMalloc( buf_p, 32 );

    for( i = 0 , j = 0 ;  i < 15 ; i++, j++ )
    {
        *( buf_p + i ) = *( temp_p + j );
    }
    *( buf_p + i ) = 0;
    mbStrClean( buf_p, id_p->lastName, TRUE );

    for( i = 0 , j = 15 ; i < 15 ; i++, j++ )
    {
        *( buf_p + i ) = *( temp_p + j );
    }
    *( buf_p + i ) = 0;
    mbStrClean( buf_p, id_p->firstName, TRUE );

    for( i = 0 , j = 30 ;  i < 15 ; i++, j++ )
    {
        *( buf_p + i ) = *( temp_p + j );
    }
    *( buf_p + i ) = 0;
    mbStrClean( buf_p, id_p->id, TRUE );

    for( i = 0 , j = 45 ;  i < 15 ; i++, j++ )
    {
        *( buf_p + i ) = *( temp_p + j );
    }
    *( buf_p + i ) = 0;
    mbStrClean( buf_p, id_p->misc, TRUE );

    //mbLog( "first name '%s'\n", id_p->firstName );
    //mbLog( "last name  '%s'\n", id_p->lastName );
    //mbLog( "id         '%s'\n", id_p->id );
    //mbLog( "misc       '%s'\n", id_p->misc );

    id_p->fullName[0] = 0;

    if( strlen( id_p->firstName ) ) strcat( id_p->fullName, id_p->firstName );

    if( strlen( id_p->lastName ) )
    {
        strcat( id_p->fullName, " " );
        strcat( id_p->fullName, id_p->lastName );
    }

    returnCode = MB_RET_OK;
exit:
    mbFree( buf_p );

    return returnCode;
}
//---------------------------------------------------------------------------
// hpsDBNextRecordGetRaw
//---------------------------------------------------------------------------
// This function looks for the end of record marker.  It is possible there
// could be large empty spaces in the file between valid records, and I
// am not sure if those spaces would be filled with zeros, or invalid data,
// or what.  It is not specified and I have no example files.  The spec
// does say that the absolute value of a negative length indidates the
// offset to the next record, but again, I have no data to test against.
// It may be that this function does not properly process some files.
//---------------------------------------------------------------------------

Int32s_t            hpsDBNextRecordGetRaw
(
    HpsDBHandle_t   handle,
    Int32u_p        type_p,
    Int32u_p        len_p,
    Int8u_p         buf_p,
    Int32u_t        bufLen
)
{
    HpsDBFile_p     handle_p = handle;
    Int32s_t        returnCode = MB_RET_ERR;
    Ints_t          c;
    Int32s_t        i;
    Int8u_p         buf1_p;
    Int8u_p         buf2_p;
    Int8u_p         start_p;
    Int32u_t        type = HPS_DB_RECORD_TYPE_INVALID;
    Int32u_t        length = 0;

    mbCalloc( buf1_p, HPS_SONOS_BUF_SIZE );
    mbCalloc( buf2_p, HPS_SONOS_BUF_SIZE );

    // Sanity check
    if( handle_p == NIL || buf1_p == NIL || buf2_p == NIL ) goto exit;

    HPS_VALIDATE_HANDLE( handle_p );

    for( i = 0 ; ;  )
    {
        if( ( c = fgetc( handle_p->fp ) ) == EOF ) goto exit;

        *(buf1_p + i) = (Int8u_t)c;

        if( ++i == HPS_SONOS_BUF_SIZE ) i = 0;

        if( c == MB_CHAR_FF )
        {
            // mbLog( "found end of record at count %d. i: %d\n", count, i );

            // Align last characters to the end of the buffer
            memcpy( ( buf2_p ), ( buf1_p + i ), HPS_SONOS_BUF_SIZE - i );
            memcpy( ( buf2_p + HPS_SONOS_BUF_SIZE - i  ), buf1_p, i );

            if( hpsDBRecordSigCheck( buf2_p, &hpsDBLFSigHeader[0] ) )
            {
                type = HPS_DB_RECORD_TYPE_HEADER;

            }
            else if( hpsDBRecordSigCheck( buf2_p, &hpsDBLFSigHeaderEx[0] ) )
            {
                type = HPS_DB_RECORD_TYPE_HEADER_EX;
            }
            else if( hpsDBRecordSigCheck( buf2_p, &hpsDBLFSigEntry[0] ) )
            {
                type = HPS_DB_RECORD_TYPE_ENTRY;
            }
            else
            {
                mbLog( "Could not determine record type\n" );
                goto exit;
            }
            break;
        }
    }

    if( type == HPS_DB_RECORD_TYPE_INVALID ) goto exit;

    length = hpsDBRecordLen[ type ];

    start_p = buf2_p + HPS_SONOS_BUF_SIZE - length;
    returnCode = MB_RET_OK;

exit:

    // Return detected type regardless of result
    if( type_p ) *type_p = type;

    if( returnCode == MB_RET_OK )
    {
        if( bufLen >= length )
        {
            // Copy the record into the supplied buffer.
            memcpy( buf_p, start_p, length );
        }
        else
        {
            length = 0;
            returnCode = MB_RET_ERR;
        }
    }

    // Return length
    if( len_p ) *len_p = length;

    if( buf1_p ) mbFree( buf1_p );
    if( buf2_p ) mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// hpsDBRecordSigCheck
//---------------------------------------------------------------------------
// Examine the record, which is aligned to the *end* of the buffer.  Look
// for the matching pattern of LFs to identify the record type.
//
//---------------------------------------------------------------------------

Int32s_t  hpsDBRecordSigCheck
(
    Int8u_p     buf_p,
    Int32u_p    sig_p
)
{
    Int32s_t    i;
    Int32u_t    k;
    Int32s_t    returnCode = MB_RET_ERR;

    for( i = HPS_SONOS_BUF_SIZE - 1, k = 0 ; i >= 0 ; i--, k++ )
    {
        // if( *( buf_p + i ) == MB_CHAR_CR ) mbLog( "got CR at position %d\n", k );
        if( *( buf_p + i ) == MB_CHAR_LF )
        {
            //mbLog( "got LF at position %d\n", k );
            if( k == *sig_p )
            {
                // There was an expected spot for an LF
                if( *sig_p != HSP_SIG_INVALID ) sig_p++;
            }
            else
            {
                // This LF was not expected, ignore.
            }
        }
    }

    // Ensure that all expected LFs were detected
    if( *sig_p != HSP_SIG_INVALID ) goto exit;
    returnCode = MB_RET_OK;

exit:
    // mbLog( "Match: %s\n", ( returnCode == MB_RET_OK ) ? "TRUE" : "FALSE" );
    return returnCode;
}

//---------------------------------------------------------------------------
// hpsBDClose
//---------------------------------------------------------------------------
// This function opens a HPSONOS.DB file and returns a handle to the calling
// code.
//---------------------------------------------------------------------------

Int32s_t            hpsDBClose
(
    HpsDBHandle_t   handle
)
{
    HpsDBFile_p     handle_p = handle;
    Int32s_t        returnCode = MB_RET_ERR;

    HPS_VALIDATE_HANDLE( handle_p );
    // Sanity check

    if( handle_p )
    {
        if( handle_p->fp ) fclose( handle_p->fp );
        hpsDBHeaderFree( handle_p->header_p );
        mbFree( handle_p );
    }
    returnCode = MB_RET_OK;
exit:

    return returnCode;
}


