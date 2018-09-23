//---------------------------------------------------------------------------
// Function:    mbFileUtils.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// File functions and macros
//---------------------------------------------------------------------------

#ifndef mbFileUtils_h
#define mbFileUtils_h

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
// Typedefs
//---------------------------------------------------------------------------

typedef mbStrList_t mbNamePart_t;
typedef mbStrList_p mbNamePart_p;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Char_p      mbFileNameExtStrip
(
    Char_p          name_p,
    Char_p          out_p
);

Char_p      mbFileNamePathStrip
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

Int32s_t    mbFileCopy
(
    Char_p          srcFileName_p,
    Char_p          destFileName_p
);

Int32s_t    mbFileCopyCall
(
    Char_p          srcFileName_p,
    Char_p          destFileName_p,
    Int32u_p        crc_p,
    Int32u_p        size_p,
    Void_p          this_p,
    Int32s_t        (*cancelFunc)( Void_p this_p ),
    Int32s_t        (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
);

Int32s_t    mbFileDirCheckCreate
(
    Char_p          name_p,
    qHead_p         dirCache_q,
    Boolean_t       fileFlag,
    Boolean_t       createFlag,
    Boolean_t       promptFlag
);

Int32s_t    mbFileNamePartsAssemble
(
    qHead_p         part_q,
    Int32u_t        partCount,
    Char_p          result_p
);

Int32s_t    mbDriveFreeSpace
(
    Char_p          name_p,
    Int64u_p        freeSpace_p
);

Int32s_t    mbDriveOpen
(
    Boolean_t       openFlag,
    Char_p          drive_p
);

Char_p      mbDriveLabel
(
    Char_p          name_p,
    Char_p          out_p
);

Int32u_t    mbFileByteSum
(
    Char_p          fileName_p
);


Int32u_t    mbProcessIdGet
(
    Char_p          processName_p
);

Int32s_t    mbProcessIdWaitIdle
(
    Int32u_t    pid,
    Int32u_t    idleTimeMsec,
    Int32u_t    timeoutMsec
);

#endif // mbFileUtils_h

