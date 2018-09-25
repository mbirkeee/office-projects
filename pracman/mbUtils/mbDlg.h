//---------------------------------------------------------------------------
// Function:    mbDlg.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// Dialog functions and macros
//---------------------------------------------------------------------------

#ifndef mbDlg_h
#define mbDlg_h

#ifdef  External
#undef  External
#endif

#ifdef  MB_INIT_GLOBALS
#define External
#else
#define External extern
#endif  // MB_INIT_GLOBALS

enum
{
     MB_BUTTON_OK   = 1
    ,MB_BUTTON_CANCEL
    ,MB_BUTTON_FAILED
    ,MB_BUTTON_YES
    ,MB_BUTTON_NO
};

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

External Boolean_t  mbDlgInitFlag
#ifdef MB_INIT_GLOBALS
= FALSE
#endif
;

External Boolean_t  mbDlgLogFlag
#ifdef MB_INIT_GLOBALS
= FALSE
#endif
;

External Char_p     mbDlgName_p
#ifdef MB_INIT_GLOBALS
= NIL
#endif
;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Ints_t mbDlgInit( Char_p name_p, Boolean_t   logFlag );
Ints_t mbDlgTerminate( void );

Ints_t mbDlgInfo(     Char_p  string_p, ... );
Ints_t mbDlgExclaim(  Char_p  string_p, ... );
Ints_t mbDlgError(    Char_p  string_p, ... );
Ints_t mbDlgOkCancel( Char_p  string_p, ... );
Ints_t mbDlgYesNo(    Char_p  string_p, ... );

#endif // pmcDlg_h

