/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/inc/mbFileUtils.h,v 1.6 2007/12/17 01:00:07 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbFileUtils.h,v $
 * Revision 1.6  2007/12/17 01:00:07  mikeb
 * Update file utilities
 *
 * Revision 1.5  2007/11/12 05:05:53  mikeb
 * Updates to mbFileListDisplay.  Display name_p if fileName_p is NULL
 *
 * Revision 1.4  2007/11/10 23:29:32  mikeb
 * Add md5fast to MbFile_t
 *
 * Revision 1.3  2007/11/10 17:39:41  mikeb
 * Updates
 *
 * Revision 1.2  2007/07/16 04:16:21  mikeb
 * Add a few utility functions
 *
 * Revision 1.1.1.1  2007/07/10 05:06:02  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

#ifndef MB_FILEUTILS_H
#define MB_FILEUTILS_H

#include "mbQueue.h"

/******************************************************************************
 * Defines and Typedefs
 *-----------------------------------------------------------------------------
 */

typedef mbStrList_t mbNamePart_t;
typedef mbStrList_p mbNamePart_p;


#define MB_FILE_TYPE_FILE        (1)
#define MB_FILE_TYPE_DIR         (2)

typedef struct MbFile_s
{
    qLinkage_t  linkage;
    Char_p      name_p;
    Char_p      path_p;
    Char_p      pathName_p;
    Char_t      md5str[68];
    Int8u_t     md5[16];
    Int32u_t    md5fast[4];
    ino_t       inode;
    Int32u_t    type;
    Int32u_t    crc;
    Int32u_t    date;
    off_t       size;
} MbFile_t, *MbFile_p;

/******************************************************************************
 * Function Prototypes
 *-----------------------------------------------------------------------------
 */

Int32s_t        mbFileNameCompare
(
    Char_p      fileName1_p,
    Char_p      fileName2_p
);

Int32s_t mbFileListGet
(
    Char_p      dir_p,
    qHead_p     q_p,
    Boolean_t   recurseFlag,
    Boolean_t   symFlag,
    Boolean_t   dirFlag
);

MbFile_p mbFileMalloc
(
    Char_p      name_p,
    Char_p      path_p
);

MbFile_p mbFileFree
(
    MbFile_p    file_p
);

void mbFileListFree
(
    qHead_p     q_p
);

Int32s_t mbFileExists
(
    Char_p      name_p
);

Int32s_t mbFileListDisplay
(
    qHead_p     q_p
);

void  mbFileListLogValueSet
(
    Int32u_t    value
);

qHead_p mbFileListSort
(
    qHead_p     in_p,
    Boolean_t   dateFlag
);

Char_p          mbFileNamePathStrip
(
    Char_p          name_p,
    Char_p          out_p
);

Int32s_t    mbFileNamePartsGet
(
    Char_p          name_p,
    qHead_p         name_q
);

Int32s_t    mbFileNamePartsFree
(
    qHead_p         name_q
);

Int32s_t    mbFileNamePartsFreeElement
(
    mbStrList_p     entry_p
);

Int32s_t    mbFileNamePartsDisplay
(
    qHead_p         name_q
);

#endif /* MB_FILEUTILS_H */
