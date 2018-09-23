//---------------------------------------------------------------------------
// File:    mbDebug.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Simple debug dialog box
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbDebug.h"
#include "mbLog.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

//---------------------------------------------------------------------------

Char_p mbFormatString
(
    Char_p  string_p,
    ...
)
{
    va_list             arg;
    Char_p              str_p;

    str_p = (Char_p)malloc( 5000 );
    if( str_p == NULL ) return str_p;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );
    return str_p;
}

//---------------------------------------------------------------------------

void mbDebugBoxFunc
(
    Char_p  file_p,
    Ints_t  line,
    Char_p  string_p
)
{
    Char_t  str[256];

    sprintf( str, "%s : %d ", file_p, line );
    Application->MessageBox( string_p, str, MB_OK );
    mbLog( "%s:%s\n", str, string_p );
    return;
}

