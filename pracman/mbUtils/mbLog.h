//---------------------------------------------------------------------------
// Function:    mbLog.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// Logging functions and macros
//---------------------------------------------------------------------------

#ifndef mbLog_h
#define mbLog_h

#ifdef  External
#undef  External
#endif

#ifdef  MB_INIT_GLOBALS
#define External
#else
#define External extern
#endif

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

External Boolean_t  mbLogInitFlag
#ifdef MB_INIT_GLOBALS
= FALSE
#endif
;


External Char_p     mbLogName_p
#ifdef MB_INIT_GLOBALS
= NIL
#endif
;

External Char_p     mbLogDir_p
#ifdef MB_INIT_GLOBALS
= NIL
#endif
;

External Char_p     mbLogFileName_p
#ifdef MB_INIT_GLOBALS
= NIL
#endif
;

External Int32u_t   mbLogPid
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   mbLogYear
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   mbLogMonth
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   mbLogDay
#ifdef MB_INIT_GLOBALS
= 0
#endif
;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Ints_t mbLogInit( Char_p name_p, Char_p directory_p, Int32u_t pid );
Ints_t mbLogTerminate( void );
Ints_t mbLog( Char_p  string_p, ... );

#endif // mbLog_h

