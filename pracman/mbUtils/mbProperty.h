//---------------------------------------------------------------------------
// Function:    mbProperty.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Feb. 2, 2003
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef mbProperty_h
#define mbProperty_h

#include "mbTypes.h"
#include "mbQueue.h"

#ifdef  External
#undef  External
#endif

#ifdef  MB_INIT_GLOBALS
#define External
#else
#define External extern
#endif

enum
{
     MB_PROPERTY_TYPE_INVALID
    ,MB_PROPERTY_TYPE_WIN
    ,MB_PROPERTY_TYPE_INT
    ,MB_PROPERTY_TYPE_STRING
};

//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------

typedef struct mbPropertyWin_s
{
    qLinkage_t      linkage;
    Int32u_t        winId;
    Ints_t          height;
    Ints_t          width;
    Ints_t          top;
    Ints_t          left;
} mbPropertyWin_t, *mbPropertyWin_p;

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

External Int32u_t   mbPropertyInitialized
#ifdef MB_INIT_GLOBALS
= FALSE
#endif
;

External Char_p     mbPropertyFile_p
#ifdef MB_INIT_GLOBALS
= NIL
#endif
;

External qHead_t     mbPropertyWinQueue;
External qHead_p     mbPropertyWin_q;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t    mbPropertyInit( Char_p fileName_p );
Int32s_t    mbPropertyLoad( Void_t );
Int32s_t    mbPropertySave( Void_t );
Int32s_t    mbPropertyClose( Void_t );
Int32s_t    mbPropertyWinSave( Int32u_t winId, Ints_t height, Ints_t width, Ints_t top, Ints_t left );
Int32s_t    mbPropertyWinGet(  Int32u_t winId, Ints_p height, Ints_p width, Ints_p top, Ints_p left );
Int32u_t    mbPropertyLoadTypeGet( Char_p buf_p );
Int32u_t    mbPropertyLoadWin( Char_p buf_p );

mbPropertyWin_p mbPropertyWinFind( Int32u_t winId );

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

#define MB_PROPERTY_INITIALIZED( )\
{\
    if( mbPropertyInitialized == FALSE )\
    {\
        mbDlgError( "Property subsystem not initialized." );\
        goto exit;\
    }\
}

#endif // mbProperty_h

