/******************************************************************************
 * File:   pmcdesplMain.h
 *-----------------------------------------------------------------------------
 * Copyright (c) 2001, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Author: Michael Bree
 * Date:   March 6, 2001
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/filecomp/inc/filecompMain.h,v 1.2 2006/12/05 13:47:07 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: filecompMain.h,v $
 * Revision 1.2  2006/12/05 13:47:07  mikeb
 * Update queue macros
 * Add totals to end of display
 * dos2unix files
 *
 * Revision 1.1.1.1  2006/05/23 01:51:24  mikeb
 * Initial Import
 *
 *
 *-----------------------------------------------------------------------------
 */



#ifndef H_filecompMain
#define H_filecompMain

#ifdef  External
#undef  External
#endif

#ifdef INIT_GLOBALS
#define External
#else
#ifdef __cplusplus
#define External extern "C"
#else
#define External extern
#endif
#endif

#define DS_PROG_NAME        "filecomp"

// CRC Polynomial
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
#define MB_CRC_POLYNOMIAL 0x04C11DB7L

#define DS_TYPE_FILE        (1)
#define DS_TYPE_DIR         (2)

typedef struct  dsFileStruct_s
{
    qEntry_t    linkage;
    Char_p      name_p;
    Char_p      path_p;
    Char_p      pathName_p;
    ino_t       inode;
    Int32u_t    type;
    Int32u_t    crc;
    off_t       size;
} dsFileStruct_t, *dsFileStruct_p;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

void            mbCrcInit
(
    void
);

Int32u_t        mbCrcUpdate
(
    Int32u_t    crc,
    Int8u_p     data_p,
    Int32u_t    dataLen
);

Int32u_t        mbCrcFile
(
    dsFileStruct_p  file_p
);

dsFileStruct_p fileFree
(
    dsFileStruct_p file_p
);

dsFileStruct_p fileAdd
(
    Char_p      name_p,
    Char_p      path_p
);

Int32s_t strPos
(
    Char_p      s1,
    Char_p      s2
);

Int32s_t        getFileList
(
    Char_p      dir_p,
    qHead_p     q_p,
    Boolean_t   recurseFlag
);

#if 0
Int32s_t        dsFileCompare
(
    Char_p      fileName1_p,
    Char_p      fileName2_p
);
#endif
#endif /* H_filecompMain */
