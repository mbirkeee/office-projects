//---------------------------------------------------------------------------
// Function:    mbDebug.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 29, 2002
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

#ifndef mbDebug_h
#define mbDebug_h

#ifdef  External
#undef  External
#endif

#define MB_DEBUG_DATE( )\
{\
    char    buf[128];\
    sprintf( buf, "%d %d %d", Calendar_p->Day, Calendar_p->Month, Calendar_p->Year );\
    Application->MessageBox( buf, "Debug Date", MB_OK );\
}

Char_p mbFormatString
(
    Char_p  string_p,
    ...
);

void mbDebugBoxFunc
(
    Char_p  file_p,
    Ints_t  line,
    Char_p  string_p
);

#define mbDlgDebug( _parm )\
{\
    Char_p  _str_p = mbFormatString _parm ;\
    if( _str_p ) {\
        mbDebugBoxFunc( __FILE__, __LINE__, _str_p ); free( _str_p );\
    }\
}

#define nbDlgDebug( _parm )

#endif // mbDebug_h

