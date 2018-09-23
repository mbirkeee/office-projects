//---------------------------------------------------------------------------
// Function:    mbFileList.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// File functions and macros
//---------------------------------------------------------------------------

#ifndef mbFileList_h
#define mbFileList_h

#include "mbTypes.h"
#include "mbQueue.h"
#include "mbThermometer.h"

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

typedef struct mbFileListStruct_s
{
    qLinkage_t  linkage;
    Char_p      name_p;
    Char_p      fullName_p;
    Char_p      target_p;
    Char_p      path_p;
    Char_p      crcStr_p;
    Char_p      sizeStr_p;
    Char_p      dateStr_p;
    Char_p      timeStr_p;
    Int32u_t    attrib;
    Int32u_t    crc;
    Int32u_t    byteSum;
    Int64u_t    size64;
    Int32s_t    type;
    Int32u_t    data;
    Int16u_t    date;
    Int16u_t    time;
    qHead_t     subQueue;
    qHead_p     sub_q;
} mbFileListStruct_t, *mbFileListStruct_p;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t        mbFileListFreeElement
(
    mbFileListStruct_p file_p
);

Int32s_t        mbFileListFree
(
    qHead_p     q_p
);

Int64u_t        mbFileListGet
(
    Char_p      dir_p,
    Char_p      filter_p,
    qHead_p     q_p,
    Boolean_t   recurse
);

Int64u_t        mbFileListGetNew
(
    Char_p      dir_p,
    Char_p      filter_p,
    qHead_p     q_p,
    Boolean_t   recurse,
    Boolean_t   dirOnlyFlag,
    Boolean_t   firstFlag,
    Int32u_t    depthMax
);

Int32s_t        mbFileListLog
(
    qHead_p     file_q
);

Int32s_t        mbFileListPurgeDuplicates
(
    qHead_p     file_q
);

Int32s_t        mbFileListSearch
(
    qHead_p     file_q,
    Char_p      search_p
);

Int32s_t        mbFileListTargetMake
(
    Char_p      source_p,
    Char_p      target_p,
    qHead_p     file_q
);

Int32s_t        mbFileListTargetMakeElement
(
    Char_p      source_p,
    Char_p      target_p,
    mbFileListStruct_p  file_p
);

Int32s_t        mbFileListCopy
(
    qHead_p         file_q,
    Boolean_t       thermometerFlag,
    Void_p          this_p,
    Int32s_t        (*cancelFunc)( Void_p this_p ),
    Int32s_t        (*fileFunc)( Void_p this_p, Char_p name_p ),
    Int32s_t        (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
);

Int32s_t        mbFileListCopyRecurse
(
    qHead_p         file_q,
    qHead_p         dirCache_q,
    TThermometer   *thermometer_p,
    Void_p          this_p,
    Int32s_t        (*cancelFunc)( Void_p this_p ),
    Int32s_t        (*fileFunc)( Void_p this_p, Char_p name_p ),
    Int32s_t        (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
);

Int32s_t        mbFileListCopyFile
(
    mbFileListStruct_p  file_p,
    qHead_p             dirCache_q,
    Void_p              this_p,
    Int32s_t            (*cancelFunc)( Void_p this_p ),
    Int32s_t            (*bytesFunc)( Void_p this_p, Int32u_t bytes_p )
);

Int32s_t        mbFileListDelete
(
    qHead_p         file_q,
    Boolean_t       deleteDirs
);

Int32s_t        mbFileListFlatten
(
    qHead_p         file_q
);

Int32s_t        mbFileListFlattenRecurse
(
    qHead_p         file_q,
    qHead_p         temp_q
);

Char_p          mbFFTimeToStr
(
    Int16u_t        time,
    Char_p          str_p
);

Char_p          mbFFDateToStr
(
    Int16u_t        date,
    Char_p          str_p
);
#endif // mbFileList_h

