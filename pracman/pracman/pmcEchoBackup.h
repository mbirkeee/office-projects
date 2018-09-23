//---------------------------------------------------------------------------
// File:    pmcEchoBackup.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2003, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    February 15, 2003
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcEchoBackupH
#define pmcEchoBackupH

#define PMC_ECHO_CD_ID_FILE         ".echoID.txt"
#define PMC_ECHO_CD_LABEL           "ECHO-%04lu"

#define PMC_DISK_DETECTED_UNKNOWN   (0)
#define PMC_DISK_DETECTED_YES       (1)
#define PMC_DISK_DETECTED_NO        (2)

typedef struct pmcCDList_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
} pmcCDList_t, *pmcCDList_p;
         
//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t pmcEchoCDListGet( qHead_p cd_q, Int32u_t echoId );
Int32s_t pmcEchoCDListFree( qHead_p cd_q );
Int32s_t pmcEchoCDIDSet( Int32u_t id );
Int32u_t pmcEchoCDIDGet( Int64u_p spaceAvailable_p );
Char_p   pmcEchoNameGet( Int32u_t echoId, Char_p echoName_p, Ints_t length );

Int32s_t        pmcEchoBackupVerifyRestore
(
    Int32u_t    echoId,
    Int32u_t    cdId,
    Void_p      this_p,
    Int32s_t    (*cancelCallback)( Void_p this_p ),
    Int32s_t    (*bytesCallback)( Void_p this_p, Int32u_t bytes ),
    Int32s_t    (*cdLabelCallback)( Void_p this_p, Char_p str_p, Int32u_t bytesFree ),
    Int32u_p    cancelFlag_p,
    Int32u_t    restoreFlag
);

Int32u_t        pmcEchoCDCheck
(
    Int64u_p    spaceAvailable_p,
    Int64u_t    spaceRequired,
    Boolean_p   promptedFlag_p,
    Void_p      this_p,
    Int32s_t   (*cdLabelCallback)( Void_p this_p, Char_p str_p, Int32u_t bytesFree ),
    Int32u_t    thisFlag,
    Int32u_t    databaseCDFlag
);

Int32s_t        pmcEchoBackup
(
    Int32u_t    echoId,
    Int32u_t    backupSkip,
    Int64u_p    backupFreeSpace_p,
    Void_p      this_p,
    Int32s_t    (*cancelCallback)( Void_p this_p ),
    Int32s_t    (*bytesCallback)( Void_p this_p, Int32u_t bytes ),
    Int32s_t    (*cdLabelCallback)( Void_p this_p, Char_p str_p, Int32u_t bytesFree ),
    Int32u_t    thisFlag,
    Int32u_t    databaseCDFlag
);

Int32s_t        pmcEchoFileListGet
(
    Int32u_t    echoId,
    qHead_p     file_q,
    Int32u_p    totalSize_p
);

Int32s_t        pmcEchoFileListFree
(
    qHead_p     file_q
);

Int32s_t        pmcEchoBackupDeleteFiles
(
    Int32u_t    echoId
);

Int32s_t        pmcEchoBackupDeleteEntries
(
    Int32u_t    echoId,
    Int32u_t    cdId
);

Int32s_t        pmcEchoBackupCount
(
    Int32u_t    echoId
);

Int32s_t        pmcEchoDatabaseSpaceCheck
(
    void
);

#endif // pmcEchoBackup
