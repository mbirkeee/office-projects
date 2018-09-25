//---------------------------------------------------------------------------
// File:    pmcMspClaimsin.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcMspClaimsinH
#define pmcMspClaimsinH

#define PMC_CLAIM_FILE_PRINT_MODE_SUBMIT    1
#define PMC_CLAIM_FILE_PRINT_MODE_TEST      2
#define PMC_CLAIM_FILE_PRINT_MODE_ARCHIVE   3

#define PMC_CLAIM_FILE_STATE_WAIT_HEADER    1
#define PMC_CLAIM_FILE_STATE_GOT_HEADER     2
#define PMC_CLAIM_FILE_STATE_GOT_SERVICE    3
#define PMC_CLAIM_FILE_STATE_GOT_TRAILER    4
#define PMC_CLAIM_FILE_STATE_GOT_COMMENT    5

typedef struct pmcClaimsInfoStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    clinicNumber;
    Int32u_t    providerNumber;
    Int32u_t    claimCount;
    Int32u_t    recordCount;
    Int32u_t    serviceRecordCount;
    Int32u_t    reciprocalRecordCount;
    Int32u_t    commentRecordCount;
    Int32u_t    totalClaimed;
    pmcClaimsInfoStruct_s *sub_p;
} pmcClaimsInfoStruct_t, *pmcClaimsInfoStruct_p;

Int32s_t pmcClaimsFileInfoGetString
(
    Char_p      fileName_p,
    Char_p      bufOut_p
);

Int32s_t    pmcClaimsFileTrailerString
(
    pmcClaimRecordStruct_p  record_p,
    Char_p                  bufOut_p
);

Int32s_t    pmcClaimsFilePrint
(
    Char_p      fileName_p,
    Char_p      archiveFileName_p,
    Int32u_t    printMode
);

Int32s_t    pmcClaimsFilePrintProvider
(
    Char_p      fileName_p,
    Char_p      archiveFileName_p,
    qHead_p     record_q,
    Int32u_t    printMode
);

Int32s_t    pmcClaimsFilePrintHeader
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p
);

Int32s_t    pmcClaimsFilePrintService
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintReciprocal
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintComment
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintHospital
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                index
);

Int32s_t    pmcClaimsFilePrintTrailer
(
    FILE                   *fp,
    pmcClaimRecordStruct_p  record_p,
    Int32u_t                commentCount
);

Int32s_t    pmcClaimsFileInfoGet
(
    Char_p                  fileName_p,
    qHead_p                 provider_q
);

Int32s_t pmcClaimsFileProviderInfo
(
    qHead_p                 provider_q,
    qHead_p                 record_q
);

Int32s_t pmcNoticeFilePrint
(
    Char_p                  fileName_p
);

#endif
