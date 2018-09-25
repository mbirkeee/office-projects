//---------------------------------------------------------------------------
// File:    mbProprty.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Oct. 31, 2002
//---------------------------------------------------------------------------
// Description:
//
// Functions for saving dynamic properties
//---------------------------------------------------------------------------

#include <stdio.h>
#include <dir.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbLock.h"
#include "mbMalloc.h"
#include "mbDebug.h"
#include "mbDlg.h"
#include "mbLog.h"
#include "mbStrUtils.h"
#include "mbFileUtils.h"

#define MB_INIT_GLOBALS
#include "mbProperty.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function:    mbPropertyInit
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

Int32s_t    mbPropertyInit
(
    Char_p      fileName_p
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    if( fileName_p == NIL ) goto exit;

    if( mbPropertyFile_p != NIL )
    {
        mbDlgError( "Property file '%s' already specified\n", mbPropertyFile_p );
        goto exit;
    }
    mbMallocStr( mbPropertyFile_p, fileName_p );

    mbPropertyWin_q = qInitialize( &mbPropertyWinQueue );

    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbPropertyClose
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

Int32s_t    mbPropertyClose
(
    Void_t
)
{
    Int32s_t            returnCode = MB_RET_ERR;
    mbPropertyWin_p     win_p;

    MB_PROPERTY_INITIALIZED( );

    while( !qEmpty( mbPropertyWin_q ) )
    {
        win_p = (mbPropertyWin_p)qRemoveFirst( mbPropertyWin_q );
        mbFree( win_p );
    }
    mbFree( mbPropertyFile_p );

    mbPropertyInitialized = FALSE;
    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbPropertyLoad
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

Int32s_t    mbPropertyLoad
(
    Void_t
)
{
    Int32s_t    returnCode = MB_RET_ERR;
    FILE        *fp = NIL;
    Char_p      buf_p = NIL;
    Int32u_t    type;

    mbMalloc( buf_p, 512 );

    if( mbPropertyFile_p == NIL )
    {
        mbDlgError( "Properties file not specified\n" );
        goto exit;
    }

    mbPropertyInitialized = TRUE;

    fp = fopen( mbPropertyFile_p, "r" );
    if( fp == NIL )
    {
        // There may not ba a file, and this is not an error condition.
        // Want to indicate subsystem is initialized so that a file can
        // be created/
        goto exit;
    }

    while( fgets( buf_p, 512, fp ) != 0 )
    {
        type = mbPropertyLoadTypeGet( buf_p );
        switch( type )
        {
            case MB_PROPERTY_TYPE_WIN:
                mbPropertyLoadWin( buf_p );
                break;
            case MB_PROPERTY_TYPE_INT:
                break;
            case MB_PROPERTY_TYPE_STRING:
                break;

            default:
                mbDlgError( "Unknown property type %lu encountered\n", type );
                break;
        }
    }

    returnCode = MB_RET_OK;
exit:
    if( fp ) fclose( fp );
    mbFree( buf_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbPropertyLoadWin
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32u_t    mbPropertyLoadWin
(
    Char_p  buf_p
)
{
    Int32u_t    type = MB_PROPERTY_TYPE_INVALID;
    mbPropertyWin_p     win_p;

    if( buf_p == NIL ) goto exit;

    mbMalloc( win_p, sizeof( mbPropertyWin_t ) );
    qInsertLast( mbPropertyWin_q, win_p );

    sscanf( buf_p, "%d %d %d %d %d %d",
        &type, &win_p->winId, &win_p->height, &win_p->width, &win_p->top, &win_p->left );

exit:

    return type;
}

//---------------------------------------------------------------------------
// Function:    mbPropertyLoadTypeGet
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

Int32u_t    mbPropertyLoadTypeGet
(
    Char_p  buf_p
)
{
    Int32u_t    type = MB_PROPERTY_TYPE_INVALID;

    if( buf_p == NIL ) goto exit;
    sscanf( buf_p, "%d", &type );

exit:

    return type;
}

//---------------------------------------------------------------------------
// Function:    mbPropertySave
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

Int32s_t    mbPropertySave
(
    Void_t
)
{
    Int32s_t            returnCode = MB_RET_ERR;
    FILE               *fp = NIL;
    mbPropertyWin_p     win_p;

    MB_PROPERTY_INITIALIZED( );

    // Erase old file - in the future I should make new file before erase
    unlink( mbPropertyFile_p );

    // Open new file
    fp = fopen( mbPropertyFile_p, "w" );
    if( fp == NIL )
    {
        mbDlgError( "Failed to open property file '%s'", mbPropertyFile_p );
        goto exit;
    }

    // First lets save the windows parameters
    while( !qEmpty( mbPropertyWin_q ) )
    {
        win_p = (mbPropertyWin_p)qRemoveFirst( mbPropertyWin_q );
        fprintf( fp, "%d %d %d %d %d %d\n", MB_PROPERTY_TYPE_WIN,
            win_p->winId,
            win_p->height,
            win_p->width,
            win_p->top,
            win_p->left );

        mbFree( win_p );
    }

    returnCode = MB_RET_OK;
exit:
    if( fp ) fclose( fp );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbPropertyWinFind
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

mbPropertyWin_p mbPropertyWinFind
(
    Int32u_t    winId
)
{
    mbPropertyWin_p win_p;
    Boolean_t       found = FALSE;

    MB_PROPERTY_INITIALIZED( );

    qWalk( win_p, mbPropertyWin_q, mbPropertyWin_p )
    {
        if( win_p->winId == winId )
        {
            found = TRUE;
            break;
        }
    }
    if( found == FALSE ) win_p = NIL;
exit:
    return win_p;
}

//---------------------------------------------------------------------------
// Function:    mbPropertyWinSave
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

Int32s_t    mbPropertyWinSave
(
    Int32u_t    winId,
    Ints_t      height,
    Ints_t      width,
    Ints_t      top,
    Ints_t      left
)
{
    Int32s_t            returnCode = MB_RET_ERR;
    mbPropertyWin_p     win_p;

    MB_PROPERTY_INITIALIZED( );

    // Find entry
    win_p = mbPropertyWinFind( winId );

    // Create new entry if entry not found
    if( win_p == NIL )
    {
        mbMalloc( win_p , sizeof(  mbPropertyWin_t ) );
        qInsertLast( mbPropertyWin_q, win_p );
    }

    // Update the parameters
    win_p->winId    = winId;
    win_p->height   = height;
    win_p->width    = width;
    win_p->top      = top;
    win_p->left     = left;

    returnCode = MB_RET_OK;
exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbPropertyWinGet
//---------------------------------------------------------------------------
// Get the location for the properties file, and load it.
//---------------------------------------------------------------------------

Int32s_t    mbPropertyWinGet
(
    Int32u_t    winId,
    Ints_p      height_p,
    Ints_p      width_p,
    Ints_p      top_p,
    Ints_p      left_p
)
{
    Int32s_t            returnCode = MB_RET_ERR;
    mbPropertyWin_p     win_p;

    MB_PROPERTY_INITIALIZED( );

    // Find entry
    win_p = mbPropertyWinFind( winId );

    // If entry not found... goto exit.  No error message
    if( win_p == NIL ) goto exit;

    // Update the parameters
    if( win_p->height )
    {
        if( height_p ) *height_p = win_p->height;
    }

    if( win_p->width )
    {
        if( width_p ) *width_p =  win_p->width;
    }

    if( win_p->top )
    {
        if( top_p ) *top_p =  win_p->top;
    }

    if( win_p->left )
    {
        if( left_p ) *left_p =  win_p->left;
    }

    returnCode = MB_RET_OK;
exit:
    return returnCode;
}



