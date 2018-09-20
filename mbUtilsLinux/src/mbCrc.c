/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/src/mbCrc.c,v 1.1.1.1 2007/07/10 05:06:03 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbCrc.c,v $
 * Revision 1.1.1.1  2007/07/10 05:06:03  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Project include files */
#include "mbTypes.h"
#include "mbMalloc.h"
#include "mbCrc.h"
#define PROTO(ARGS) ARGS
#include "md5.h"

static Int32u_t     mbCrcTable[256];
static Boolean_t    mbCrcInitFlag = FALSE;

/*-----------------------------------------------------------------------------
 * Procedure: mbCrcInit( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Generates a 256-word table containing all CRC remainders for every
 * possible 8-bit byte.
 *-----------------------------------------------------------------------------
 */

static void mbCrcInit( void )
{
    Int32u_t    i, j, crcTableEntry;

    for( i = 0 ; i < 256 ; i++ )
    {
        crcTableEntry = (Int32u_t)( i << 24 );
        for( j = 0 ; j < 8 ; j++ )
        {
            if( crcTableEntry & 0x80000000L )
            {
                crcTableEntry = ( crcTableEntry << 1 ) ^ MB_CRC_POLYNOMIAL;
            }
            else
            {
                crcTableEntry = ( crcTableEntry << 1 );
            }
        }
        mbCrcTable[i] = crcTableEntry;
    }
    mbCrcInitFlag = TRUE;
    return;
}


/******************************************************************************
 * Function: mbCrcUpdate( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

static Int32u_t        mbCrcUpdate
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

/******************************************************************************
 * Function: mbCrcFile( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * Compute the CRC and length of the specified file
 *-----------------------------------------------------------------------------
 */

Int32u_t mbCrcFile( Char_p fileName_p, Int32u_p size_p )
{
    FILE       *fp;
    Int8u_p     buf_p;
    size_t      bytes;
    Int32u_t    crc = 0;
    Int32u_t    size = 0;
    Int64u_t    testSize = 0;

    if( mbCrcInitFlag == FALSE ) mbCrcInit( );

    buf_p = malloc( 5000 );

    if( ( fp = fopen( fileName_p, "rb" ) ) == NIL ) goto exit;

    for( ; ; )
    {
        bytes = fread( (Void_p)buf_p, 1, 4096, fp );
        size += bytes;
        testSize += (Int64u_t)bytes;

        if( testSize > 0x00000000FFFFFFFF )
        {
            break;
        }
        crc = mbCrcUpdate( crc, buf_p, bytes );
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

    if( fp ) fclose( fp );
    free( buf_p );

    return crc;
}

/******************************************************************************
 * Function: mbMD5File( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Char_p mbMD5File( Char_p fileName_p, Int8u_p result_p, Char_p str_p, Int32u_p size_p )
{
    FILE                   *fp = NIL;
    Int8u_p                 buf_p = NIL;
    size_t                  bytes;
    Int32u_t                size = 0;
    Int64u_t                testSize = 0;
    struct cvs_MD5Context   context;
    Int8u_t                 checksum[16+2];

    buf_p = mbMalloc( 5000 );

    if( ( fp = fopen( fileName_p, "rb" ) ) == NIL ) goto exit;

    cvs_MD5Init( &context );

    for( ; ; )
    {
        bytes = fread( (Void_p)buf_p, 1, 4096, fp );
        size += bytes;
        testSize += (Int64u_t)bytes;

        cvs_MD5Update( &context, buf_p, bytes );
        if( testSize > 0x00000000FFFFFFFF )
        {
            break;
        }
        if( bytes < 4096 ) break;
    }

    cvs_MD5Final( checksum, &context );

exit:

    if( size_p ) *size_p = size;

    if( fp ) fclose( fp );
    mbFree( buf_p );

    if( result_p )
    {
        memcpy( result_p, checksum, 16 );
        *(result_p + 16) = 0;
    }

    if( str_p )
    {
        Char_t      buf[8];
        Int32s_t    i;

        *str_p = 0;
        for( i = 0 ; i < 16 ; i++ )
        {
            sprintf( buf, "%02x", checksum[i] );
            strcat( str_p, buf );
        }
    }
    return str_p;
}

