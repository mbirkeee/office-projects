/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/inc/mbStrUtils.h,v 1.5 2008/04/06 22:03:16 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbStrUtils.h,v $
 * Revision 1.5  2008/04/06 22:03:16  mikeb
 * Add ability to subtract day from date integer
 *
 * Revision 1.4  2008/01/02 01:04:37  mikeb
 * Add string formatting of Int64u_t and Int32u_t
 *
 * Revision 1.3  2007/12/17 01:00:08  mikeb
 * Update file utilities
 *
 * Revision 1.2  2007/07/16 04:16:21  mikeb
 * Add a few utility functions
 *
 * Revision 1.1.1.1  2007/07/10 05:06:02  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

#ifndef MB_STRUTILS_H
#define MB_STRUTILS_H

//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------

typedef struct mbStrList_s
{
    qLinkage_t      linkage;
    Char_p          str_p;
    Char_p          str2_p;
    Int32u_t        handle;
} mbStrList_t, *mbStrList_p;

/******************************************************************************
 * Defines
 *-----------------------------------------------------------------------------
 */

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

/******************************************************************************
 * Function Prototypes
 *-----------------------------------------------------------------------------
 */

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

Ints_t mbStrPos
(
    Char_p      s1,
    Char_p      s2
);

Ints_t mbStrPosIgnoreCase
(
    Char_p      s1,
    Char_p      s2
);

Char_p   mbStrRemoveSlashTrailing( Char_p str_p );
Char_p   mbStrRemoveSlashLeading( Char_p str_p );
Int32u_t mbDateIntFromString( Char_p str_p );
Char_p   mbDateStringFromInt( Int32u_t date, Char_p str_p );
Int32u_t mbDateBackOneDay( Int32u_t dateIn );
Int32u_t mbDateIsYearLeapYear( Int32u_t year );
Int32u_t mbDateDaysInMonth( Int32u_t year, Int32u_t month );

Char_p  mbStrCleanWhite
(
    Char_p      in_p,
    Char_p      out_p
);

Int32s_t        mbStrListAdd
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
    Int32u_t    handle
);

Int32u_t        mbStrListFree
(
    qHead_p     list_q
);

Int32s_t        mbStrListFreeElement
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

Char_p          mbDigitsOnly
(
    Char_p      string_p
);

#endif /* MB_STRUTILS_H */
