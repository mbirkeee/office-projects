//---------------------------------------------------------------------------
// Function:    mbStrUtils.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// String functions and macros
//---------------------------------------------------------------------------

#ifndef mbStrUtils_h
#define mbStrUtils_h

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

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define MB_CHAR_DASH            (0x2D)
#define MB_CHAR_SPACE           (0x20)
#define MB_CHAR_TAB             (0x09)
#define MB_CHAR_COLON           (0x3A)
#define MB_CHAR_PERIOD          (0x2E)
#define MB_CHAR_BACKSLASH       (0x5C)
#define MB_CHAR_SLASH           (0x2F)
#define MB_CHAR_LF              (0x0A)
#define MB_CHAR_CR              (0x0D)
#define MB_CHAR_FF              (0x0C)
#define MB_CHAR_QUOTE_DOUBLE    (0x22)
#define MB_CHAR_QUOTE_SINGLE    (0x27)
#define MB_CHAR_DEGREES         (0xB0)
#define MB_CHAR_ALPHA           (0xE1)
#define MB_CHAR_BETA            (0xE2)

//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------

typedef struct mbStrList_s
{
    qLinkage_t      linkage;
    Char_p          str_p;
    Char_p          str2_p;
    Int32u_t        handle;
    Int32u_t        handle2;
} mbStrList_t, *mbStrList_p;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Char_p          mbStrAlphaNumericOnly
(
    Char_p      string_p
);

Char_p          mbStrAlphaOnly
(
    Char_p      string_p
);

Char_p          mbStrToFileName
(
    Char_p      str_p
);

Char_p          mbStrDigitsOnly
(
    Char_p      string_p
);

Char_p          mbStrRemoveSlashTrailing
(
    Char_p      str_p
);

Char_p          mbStrRemoveSlashLeading
(
    Char_p      str_p
);

Ints_t          mbStrPos
(
    Char_p      s1,
    Char_p      s2
);

Ints_t          mbStrPosIgnoreCase
(
    Char_p      s1,
    Char_p      s2
);

Ints_t          mbStrPosLen
(
    Char_p      s1,
    Char_p      s2
);

Char_p          mbStrClean
(
    Char_p      in_p,
    Char_p      out_p,
    Boolean_t   backslahFlag
);

Char_p          mbStrToLatin
(
    Char_p      in_p
);

Char_p          mbStrStrip
(
    Char_p      in_p
);

Char_p          mbStrBackslash
(
    Char_p      in_p,
    Char_p      out_p,
    Int32u_t    flag
);

Char_p          mbStrUpdateRoot
(
    Char_p      string_p,
    Char_p      source_p,
    Char_p      target_p,
    Char_p     *result_pp
);

Int32s_t        mbStrListAdd
(
    qHead_p     list_q,
    Char_p      str_p
);

Int32s_t        mbStrListTake
(
    qHead_p     list_q,
    Char_p      str_p
);

Int32s_t        mbStrListAddPair
(
    qHead_p     list_q,
    Char_p      str_p,
    Char_p      str2_p
);

Int32s_t        mbStrListAddNew
(
    qHead_p     list_q,
    Char_p      str_p,
    Char_p      str2_p,
    Int32u_t    handle,
    Int32u_t    handle2
);

Int32s_t        mbStrListAddInternal
(
    qHead_p     list_q,
    Char_p      str_p,
    Char_p      str2_p,
    Int32u_t    handle,
    Int32u_t    handle2,
    Boolean_t   takeFlag
);

Int32s_t        mbStrListAddEntry
(
    qHead_p     list_q,
    mbStrList_p entry_p
);

Int32u_t        mbStrListFree
(
    qHead_p     list_q
);

mbStrList_p     mbStrListItemAlloc
(
    Char_p      str_p,
    Char_p      str2_p,
    Int32u_t    handle,
    Int32u_t    handle2
);

Int32s_t        mbStrListItemFree
(
    mbStrList_p entry_p
);

Int32s_t        mbStrListCheck
(
    qHead_p     list_q,
    Char_p      str_p
);

Int32s_t        mbStrListCheckCache
(
    qHead_p     list_q,
    Char_p      str_p,
    Boolean_t   cacheFlag
);

Int32s_t        mbStrListLog
(
    qHead_p     list_q
);

Char_p          mbStrInt64u
(
    Int64u_t    number,
    Char_p      result_p
);

Char_p          mbStrInt32u
(
    Int32u_t    number,
    Char_p      result_p
);

Char_p          mbDollarStrInt32u
(
    Int32u_t    number,
    Char_p      result_p
);

Char_p          mbStrToUpper
(
    Char_p      str_p
);

Char_p          mbStrToLower
(
    Char_p      str_p
);

#endif // mbStrUtils_h

