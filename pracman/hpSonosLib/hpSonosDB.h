//---------------------------------------------------------------------------
// Function:    hpSonosDB.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Nov. 6 2002
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef hpSonosDB_h
#define hpSonosDB_h

#ifdef  External
#undef  External
#endif

#ifdef  HPS_INIT_GLOBALS
#define External
#else
#define External extern
#endif

//---------------------------------------------------------------------------
// Defines and Typedefs
//---------------------------------------------------------------------------

#define HPS_SONOS_BUF_SIZE  1024
#define HSP_SIG_INVALID     0xFFFFFFFF

#define HPS_DB_FILENAME     "HPSONOS.DB"

#define HPS_DEBUG           (1)
#define HPS_DB_MAGIC        (0xE608F321)

#if HPS_DEBUG
#define HPS_VALIDATE_HANDLE( _parm )\
{\
    if( _parm == NIL ) goto exit;\
    if( _parm->magic != HPS_DB_MAGIC ) goto exit;\
}
#else
#define HPS_VALIDATE_HANDLE( _parm )\
{\
    if( _parm == NIL ) goto exit;\
}
#endif

#define HPS_VALIDATE_HEADER( _parm )\
{\
    if( _parm == NIL ) goto exit;\
}

typedef struct HpsDBDateTime_s
{
    Char_t              raw[32];
    Char_t              timeString[32];
    Char_t              dateString[32];
    Int32u_t            date;
    Int32u_t            time;
} HpsDBDateTime_t, *HpsDBDateTime_p;

typedef struct HpsDBPatId_s
{
    qLinkage_t          linkage;
    Char_t              raw[64];
    Char_t              lastName[16];
    Char_t              firstName[16];
    Char_t              fullName[32];
    Char_t              id[16];
    Char_t              misc[16];
} HpsDBPatId_t, *HpsDBPatId_p;

typedef struct HpsDBRecord_s
{
    Char_t              fileName[16];
    HpsDBPatId_t        patId;
    HpsDBDateTime_t     date;
    Int32u_t            type;
} HpsDBRecord_t, *HpsDBRecord_p;

typedef struct HpsDBFileHeader_s
{
    Int32u_t            size;
    Int32u_t            type;
    Char_t              fileName[16];
    HpsDBPatId_t        patId;
    HpsDBDateTime_t     date;
#if HPS_DEBUG
    Void_p              file_p;
#endif
} HpsDBFileHeader_t, *HpsDBFileHeader_p;

typedef struct HpsDBFile_s
{
    FILE               *fp;
    HpsDBFileHeader_p   header_p;
#if HPS_DEBUG
    Int32u_t            magic;
#endif
} HpsDBFile_t, *HpsDBFile_p;

// For now, the handle is just a pointer to a structure
typedef HpsDBFile_p     HpsDBHandle_t;
typedef HpsDBHandle_t  *HpsDBHandle_p;

// Record Types
// ============
enum
{
     HPS_DB_RECORD_TYPE_HEADER = 0
    ,HPS_DB_RECORD_TYPE_HEADER_EX
    ,HPS_DB_RECORD_TYPE_ENTRY
    ,HPS_DB_RECORD_TYPE_INVALID
};

// File Types
// ==========
enum
{
     HPS_DB_FILE_TYPE_INVALID = 0
    ,HPS_DB_FILE_TYPE_ROOT
    ,HPS_DB_FILE_TYPE_ECHO
};

#define HPS_DB_FILE_HEADER_SIZE_ROOT    (34)
#define HPS_DB_FILE_HEADER_SIZE_ECHO    (266)

#define HPS_DB_PATIENTID_LEN            (61)
#define HPS_DB_FILENAME_LEN             (11)
#define HPS_DB_DATETIME_LEN             (19)

#define HPS_DB_PATIENTID_OFFSET_HEADER  (13)
#define HPS_DB_PATIENTID_OFFSET_ENTRY   (29)
#define HPS_DB_FILENAME_OFFSET_HEADER   (0)
#define HPS_DB_FILENAME_OFFSET_ENTRY    (16)
#define HPS_DB_DATETIME_OFFSET_HEADER   (76)
#define HPS_DB_DATETIME_OFFSET_ENTRY    (92)

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

// Database Record Signatures
// ==========================
// Since I do not have a complete description of the database file format,
// I am detecting records by looking at the LF positions as measured
// backwards from the end-of-record marker

External Int32u_t   hpsDBLFSigHeader[]
#ifdef HPS_INIT_GLOBALS
= { 1, 7, 13, 23, HSP_SIG_INVALID }
#endif
;

External Int32u_t   hpsDBLFSigHeaderEx[]
#ifdef HPS_INIT_GLOBALS
= { 1, 7, 13, 135, 156, 219, HSP_SIG_INVALID }
#endif
;

External Int32u_t   hpsDBLFSigEntry[]
#ifdef HPS_INIT_GLOBALS
= { 1, 7, 13, 19, 25, 31, 153, 174, 237, 250, 260, HSP_SIG_INVALID }
#endif
;

// These values are the offset (from the end of the record) to the
// start of the record (i.e., the record length).  The order must
// match the record type enumeration (above).

External Int32u_t   hpsDBRecordLen[]
#ifdef HPS_INIT_GLOBALS
=
{
     33
    ,232
    ,266
    ,0
}
#endif
;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

qHead_p hpsDBIDQueueGet
(
    Char_p          name_p,
    qHead_p         qIn_p
);

Char_p              hpsDBRecordTypeString
(
    HpsDBRecord_p   record_p
);

Int32s_t            hpsDBRecordDisplay
(
    HpsDBRecord_p   record_p
);

Char_p              hpsDBPatNameGet
(
    HpsDBHandle_t   handle
);

Int32s_t            hpsDBHeaderGet
(
    Int8u_p         buf_p,
    Int32u_t        length,
    HpsDBFileHeader_p header_p
);

Int32s_t            hpsDBNextRecordGet
(
    HpsDBHandle_t   handle,
    HpsDBRecord_p   record_p
);

Int32u_t            hpsDBDateGet
(
    HpsDBHandle_t   handle
);

Int32u_t            hpsDBTimeGet
(
    HpsDBHandle_t   handle
);

Char_p              hpsDBDateStringGet
(
    HpsDBHandle_t   handle
);

Char_p              hpsDBTimeStringGet
(
    HpsDBHandle_t   handle
);

Int32s_t            hpsDBHeaderExGet
(
    Int8u_p         buf_p,
    Int32u_t        length,
    HpsDBFileHeader_p header_p
);

Int32s_t            hpsDBPatientIdClean
(
    HpsDBPatId_p    in_p
);


Int32s_t            hpsDBDateTimeClean
(
    HpsDBDateTime_p date_p
);

Int32u_t            hpsDBHeaderFree
(
    HpsDBFileHeader_p header_p
);

Int32u_t            hpsDBTypeGet
(
    HpsDBHandle_t   handle
);

HpsDBHandle_t       hpsDBOpen
(
    Char_p          name_p
);

Int32s_t            hpsDBClose
(
    HpsDBHandle_t   handle
);

Int32s_t            hpsDBNextRecordGetRaw
(
    HpsDBHandle_t   handle,
    Int32u_p        type_p,
    Int32u_p        len_p,
    Int8u_p         buf_p,
    Int32u_t        bufLen
);

Int32s_t            hpsDBRecordSigCheck
(
    Int8u_p         buf_p,
    Int32u_p        sig_p
);

#endif // hpSononsDB_h

