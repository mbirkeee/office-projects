//---------------------------------------------------------------------------
// File:    mbDlg.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 9, 2002
//---------------------------------------------------------------------------
// Description:
//
// Dialog utilities
//---------------------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbDebug.h"
#include "mbLock.h"
#include "mbQueue.h"
#include "mbMalloc.h"
#include "mbLog.h"

#define MB_INIT_GLOBALS
#include "mbDlg.h"
#undef  MB_INIT_GLOBALS

//---------------------------------------------------------------------------
// Function:    mbDlgInit
//---------------------------------------------------------------------------
// Description: Initialize the dialog system
//---------------------------------------------------------------------------

Ints_t          mbDlgInit
(
    Char_p      name_p,
    Boolean_t   logFlag
)
{
    Int32s_t    returnCode = FALSE;

    if( mbDlgInitFlag == FALSE )
    {
        mbDlgLogFlag = logFlag;
        if( name_p )
        {
            mbDlgName_p = (Char_p)malloc( strlen( name_p ) + 1 );
            strcpy( mbDlgName_p, name_p );
        }
        else
        {
            mbDlgName_p = NIL;
        }
        mbDlgInitFlag = TRUE;
        returnCode = TRUE;
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbDlgTerminate
//---------------------------------------------------------------------------
// Description: Terminate the dialog subsystem
//---------------------------------------------------------------------------

Ints_t      mbDlgTerminate
(
    void
)
{
    if( mbDlgInitFlag == FALSE )
    {
        return FALSE;
    }
    else
    {
        if( mbDlgName_p ) free( mbDlgName_p );
        mbDlgInitFlag = FALSE;
        return TRUE;
    }
}

//---------------------------------------------------------------------------
// Function:    mbDlgInfo
//---------------------------------------------------------------------------
// Description: "info" dialog box
//---------------------------------------------------------------------------

Ints_t mbDlgInfo
(
    Char_p  string_p,
    ...
)
{
    va_list             arg;
    Char_p              str_p;
    Int32u_t            result;
    Int32u_t            returnCode;
    Ints_t              len;
    TCursor             cursorOrig;

    str_p = (Char_p)malloc( 5000 );
    if( str_p == NULL ) return 0;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    Application->BringToFront();

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crArrow;

    result = (Int32u_t)Application->MessageBox( str_p, mbDlgName_p, MB_OK | MB_ICONINFORMATION );
    Screen->Cursor = cursorOrig;

    if( result & IDOK )
    {
       returnCode = MB_BUTTON_OK;
    }
    if( result & IDCANCEL )
    {
       returnCode = MB_BUTTON_CANCEL;
    }
    len = strlen( str_p );
    if( len )
    {
        // Get rid of trailing newline
        if( *( str_p + len - 1 ) == '\n' ) *( str_p + len - 1 ) = 0;
    }
    if( mbDlgLogFlag ) mbLog( "%s : %s\n", str_p, returnCode == MB_BUTTON_OK ? "OK" : "CANCEL" );
    free( str_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbDlgExclaim
//---------------------------------------------------------------------------
// Description: "exclaim" dislaog box
//---------------------------------------------------------------------------

Ints_t mbDlgExclaim
(
    Char_p      string_p,
    ...
)
{
    va_list     arg;
    Char_p      str_p;
    Int32u_t    result;
    Int32u_t    returnCode;
    Ints_t      len;
    TCursor     cursorOrig;

    str_p = (Char_p)malloc( 5000 );
    if( str_p == NULL ) return 0;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crArrow;
    result = (Int32u_t)Application->MessageBox( str_p, mbDlgName_p, MB_OK | MB_ICONEXCLAMATION );
    Screen->Cursor = cursorOrig;

    if( result & IDOK )
    {
       returnCode = MB_BUTTON_OK;
    }
    if( result & IDCANCEL )
    {
       returnCode = MB_BUTTON_CANCEL;
    }
    len = strlen( str_p );
    if( len )
    {
        // Get rid of trailing newline
        if( *( str_p + len - 1 ) == '\n' ) *( str_p + len - 1 ) = 0;
    }
    if( mbDlgLogFlag ) mbLog( "%s : %s\n", str_p, returnCode == MB_BUTTON_OK ? "OK" : "CANCEL" );
    free( str_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbDlgError
//---------------------------------------------------------------------------
// Description: "error" dialog box
//---------------------------------------------------------------------------

Ints_t mbDlgError
(
    Char_p      string_p,
    ...
)
{
    va_list     arg;
    Char_p      str_p;
    Int32u_t    result;
    Int32u_t    returnCode;
    Ints_t      len;
    TCursor     cursorOrig;

    str_p = (Char_p)malloc( 5000 );
    if( str_p == NULL ) return 0;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crArrow;
    result = (Int32u_t)Application->MessageBox( str_p, mbDlgName_p, MB_OK|MB_ICONERROR );
    Screen->Cursor = cursorOrig;


    if( result & IDOK )
    {
       returnCode = MB_BUTTON_OK;
    }
    if( result & IDCANCEL )
    {
       returnCode = MB_BUTTON_CANCEL;
    }
    len = strlen( str_p );
    if( len )
    {
        // Get rid of trailing newline
        if( *( str_p + len - 1 ) == '\n' ) *( str_p + len - 1 ) = 0;
    }
    if( mbDlgLogFlag ) mbLog( "%s : %s\n", str_p, returnCode == MB_BUTTON_OK ? "OK" : "CANCEL" );
    free( str_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbDlgOkCancel
//---------------------------------------------------------------------------
// Description: "OK/Cancel" dialog box
//---------------------------------------------------------------------------

Ints_t mbDlgOkCancel
(
    Char_p      string_p,
    ...
)
{
    va_list     arg;
    Char_p      str_p;
    Int32u_t    result;
    Int32u_t    returnCode;
    Ints_t      len;
    TCursor     cursorOrig;

    str_p = (Char_p)malloc( 5000 );
    if( str_p == NULL ) return 0;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crArrow;
    result = (Int32u_t)Application->MessageBox( str_p, mbDlgName_p, MB_OKCANCEL | MB_ICONINFORMATION );
    Screen->Cursor = cursorOrig;

    if( result & IDOK )
    {
       returnCode = MB_BUTTON_OK;
    }
    if( result & IDCANCEL )
    {
       returnCode = MB_BUTTON_CANCEL;
    }
    len = strlen( str_p );
    if( len )
    {
        // Get rid of trailing newline
        if( *( str_p + len - 1 ) == '\n' ) *( str_p + len - 1 ) = 0;
    }
    if( mbDlgLogFlag ) mbLog( "%s : %s\n", str_p, returnCode == MB_BUTTON_OK ? "OK" : "CANCEL" );
    free( str_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    mbDlgYesNo
//---------------------------------------------------------------------------
// Description: "Yes/No" dialog box
//---------------------------------------------------------------------------

Ints_t mbDlgYesNo
(
    Char_p      string_p,
    ...
)
{
    va_list     arg;
    Char_p      str_p;
    Int32u_t    result;
    Int32u_t    returnCode;
    Ints_t      len;
    TCursor     cursorOrig;

    str_p = (Char_p)malloc( 5000 );
    if( str_p == NULL ) return 0;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );

    cursorOrig = Screen->Cursor;
    Screen->Cursor = crArrow;
    result = (Int32u_t)Application->MessageBox( str_p, mbDlgName_p , MB_YESNO|MB_ICONQUESTION );
    Screen->Cursor = cursorOrig;

    if( result == IDYES )
    {
       returnCode = MB_BUTTON_YES;
    }
    else if( result == IDNO )
    {
       returnCode = MB_BUTTON_NO;
    }
    len = strlen( str_p );
    if( len )
    {
        // Get rid of trailing newline
        if( *( str_p + len - 1 ) == '\n' ) *( str_p + len - 1 ) = 0;
    }
    if( mbDlgLogFlag ) mbLog( "%s : %s\n", str_p, returnCode == MB_BUTTON_YES ? "YES" : "NO" );
    free( str_p );
    return returnCode;
}

