//---------------------------------------------------------------------------
// File:    mbCrc.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 9, 2002
//---------------------------------------------------------------------------
// Description:
//
// Crc utilities
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbLock.h"
#include "mbQueue.h"
#include "mbMalloc.h"
#include "mbDlg.h"
#include "mbDebug.h"

#define MB_INIT_GLOBALS
#include "mbCrc.h"
#undef  MB_INIT_GLOBALS

//-----------------------------------------------------------------------------
// Procedure: mbCrcInit( )
//-----------------------------------------------------------------------------
// Description:
//
// Generates a 256-word table containing all CRC remainders for every
// possible 8-bit byte.
//
// Polynomial: (MB_CRC_POLYNOMIAL)
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
//-----------------------------------------------------------------------------

void mbCrcInit( void )
{
    Int32u_t    i, j, crcTableEntry;

    for( i = 0 ; i < 256 ; i++ )
    {
        crcTableEntry = (Int32u_t)( i << 24 );
        for( j = 0 ; j < 8 ; j++ )
        {
            if ( crcTableEntry & 0x80000000L )
            {
                crcTableEntry = ( crcTableEntry << 1 ) ^ MB_CRC_POLYNOMIAL;
            }
            else
            {
                crcTableEntry = ( crcTableEntry << 1 );
            }
        }
        mbCrcTable[i] = crcTableEntry;
        // mbLog( "CRC Table Entry: %03d  0x%08lX  %011ld\n", i, pmcCrcTable[i], pmcCrcTable[i] );
    }
    return;
}

//-----------------------------------------------------------------------------
// Procedure:   mbCrcFile( )
//-----------------------------------------------------------------------------
// Description:
//
//-----------------------------------------------------------------------------

Int32u_t        mbCrcFile
(
    Char_p      fileName_p,
    Int32u_p    size_p,
    Void_p      this_p,
    Int32s_t    (*cancelFunc)( Void_p this_p ),
    Int32s_t    (*bytesFunc)( Void_p this_p, Int32u_t bytes_p ),
    Int32u_p    cancelFlag_p,
    Char_p      cancelString_p
)
{
    FILE       *fp;
    Int8u_p     buf_p;
    size_t      bytes;
    Int32u_t    crc = 0;
    Int32u_t    size = 0;
    Int32u_t    cancelFlag = FALSE;

    mbMalloc( buf_p, 5000 );

    if( ( fp = fopen( fileName_p, "rb" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        if( cancelFunc )
        {
            if( (*cancelFunc)( this_p ) == TRUE )
            {
                if( mbDlgYesNo( cancelString_p ) == MB_BUTTON_YES )
                {
                    cancelFlag = TRUE;
                    goto exit;
                }
            }
        }

        bytes = fread( (Void_p)buf_p, 1, 4096, fp );
        size += bytes;

        crc = mbCrcUpdate( crc, buf_p, bytes );

        // Call the callback with the number of bytes processed
        if( bytesFunc ) (*bytesFunc)( this_p, (Int32u_t)bytes );

        if( bytes < 4096 ) break;
    }

    bytes = size;

    while( bytes > 0 )
    {
        crc = (crc << 8) ^ mbCrcTable[((crc >> 24) ^ bytes) & 0xFF];
        bytes >>= 8;
    }

    crc = ~crc & 0xFFFFFFFF;

exit:

    if( size_p ) *size_p = size;
    if( cancelFlag_p ) *cancelFlag_p = cancelFlag;

    if( fp ) fclose( fp );
    mbFree( buf_p );

    return crc;
}

//-----------------------------------------------------------------------------
// Procedure: mbCrcUpdate( )
//-----------------------------------------------------------------------------
// Description:
//
//-----------------------------------------------------------------------------

Int32u_t        mbCrcUpdate
(
    Int32u_t    crc,
    Int8u_p     data_p,
    Int32u_t    dataLen
)
{
    Int32u_t    j;

    for( j = 0 ;  j < dataLen ;  j++ )
    {
        crc = ( crc << 8 ) ^ mbCrcTable[ ( ( crc >> 24 ) ^ *data_p++ ) & 0xFF ];
    }
    return crc;
}

