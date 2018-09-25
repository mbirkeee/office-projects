//---------------------------------------------------------------------------
// File:    pmcMspReturns.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcMspReturnsH
#define pmcMspReturnsH

//---------------------------------------------------------------------------
// Types and Defines
//---------------------------------------------------------------------------

//#define PMC_RETURNS_RECORD_LENGTH     (100)
#define PMC_RETURNS_RECORD_LENGTH       (257)

// Structure describing each returned record
typedef struct pmcMspReturnsRecordStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    type;
    Int32u_t    doctorNumber;
    Int32u_t    referringNumber;
    Int32u_t    clinicNumber;
    Int32u_t    claimNumber;
    Int32s_t    seqNumber;
    Int32u_t    serviceDate;
    Int32u_t    phn;

    Int8u_t     day;
    Int8u_t     month;
    Int16u_t    year;

    Int8u_t     lastDay;
    Int8u_t     lastMonth;
    Int16u_t    lastYear;

    Int16u_t    units;
    Int16u_t    formType;

    Int32s_t    feeeApproved;
    Int32s_t    feeeSubmitted;
    Int32s_t    feeePremium;

    Char_t      name[32];
    Char_t      firstName[12];
    Char_t      feeCodeApproved[8];
    Char_t      feeCodeSubmitted[8];
    Char_t      expCode[4];
    Char_t      runCode[4];
    Char_t      prov[4];
    Char_t      healthNum[16];

    Char_t      dob[6];
    Char_t      gender[2];

    Char_t      location[2];
    Char_t      adjCode[2];

    Char_t      icdCode[4];

    Char_p      str_p;
    Char_p      comment_p;

} pmcMspReturnsRecordStruct_t, *pmcMspReturnsRecordStruct_p;

typedef struct pmcMspRunCodeList_s
{
    qLinkage_t  linkage;
    Int32u_t    count;
    Char_t      runCode[4];
} pmcMspRunCodeList_t, *pmcMspRunCodeList_p;

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32s_t            pmcMspRunCodesCheck
(
    qHead_p                         q_p,
    Boolean_t                       ignoreFlag
);

Int32s_t            pmcMspReturnsFilePrint
(
    Char_p                          fileName_p
);

Int32s_t            pmcMspReturnsFileContentsDialog
(
    Char_p                          fileName_p
);

Int32s_t            pmcMspReturnsFileRecordsGet
(
    Char_p                          fileName_p,
    qHead_p                         record_q
);

Int32s_t            pmcMspReturnsRecordPaidGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
);

Int32s_t            pmcMspReturnsRecordTotalGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
);

Int32s_t            pmcMspReturnsRecordMessageGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
);

Int32s_t            pmcMspReturnsRecordHospitalGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
);

Int32s_t            pmcMspReturnsRecordVisitGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
);

Int32s_t            pmcMspReturnsRecordCommentGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
);

Int32s_t            pmcMspReturnsRecordReciprocalGet
(
    Char_p                          buf_p,
    pmcMspReturnsRecordStruct_p     record_p
);

Int32s_t            pmcMspReturnsFileProvidersGet
(
    qHead_p                         record_q,
    qHead_p                         provider_q
);

Int32s_t            pmcMspRunCodeAdd
(
    qHead_p                         q_p,
    Char_p                          runCode_p
);

Int32s_t            pmcMspReturnsFileProviderPrint
(
    pmcProviderList_p               provider_p,
    qHead_p                         record_q,
    Char_p                          fileName_p
);

Int32s_t            pmcMspReturnsQueueClean
(
    qHead_p                         queue_p
);

Int32s_t            pmcMspReturnsRecordsPrint
(
    FILE                           *fp,
    qHead_p                         queue_p,
    Int32s_p                        claimed_p,
    Int32s_p                        paid_p
);

Int32s_t pmcMspReturnsRecordPaidPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    claimed_p,
    Int32s_p                    paid_p
);

Int32s_t pmcMspReturnsRecordMessagePrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p
);

Int32s_t pmcMspReturnsRecordTotalPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    paid_p
);

Int32s_t pmcMspReturnsRecordVisitPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    claimed_p,
    Int32s_p                    paid_p
);

Int32s_t pmcMspReturnsRecordHospitalPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p,
    Int32s_p                    claimed_p,
    Int32s_p                    paid_p
);

Int32s_t pmcMspReturnsRecordCommentPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p
);

Int32s_t pmcMspReturnsRecordReciprocalPrint
(
    FILE                       *fp,
    Int32u_t                    count,
    pmcMspReturnsRecordStruct_p record_p
);

Int32s_t pmcMspReturnsFileProcess
(
    Char_p                          fileName_p
);

Int32s_t pmcMspReturnsRecordProcess
(
    pmcMspReturnsRecordStruct_p     record_p,
    MbSQL                          *sql_p
);

Int32u_t pmcMspReturnsRecordIdGet
(
    pmcMspReturnsRecordStruct_p     record_p,
    Int32s_p                        feeSubmitted_p,
    Int32u_p                        submitDate_p,
    Int32u_p                        replyDate_p,
    Int32u_p                        status_p,
    Int32u_p                        unitsPaid_p,
    Int32u_p                        unitsClaimed_p,
    Int32s_p                        feePaid_p,
    Int32s_p                        premiumPaid_p,
    Int32u_p                        newFlag_p,
    MbSQL                          *sql_p
);

Int32s_t pmcReturnsRecordCheckPhn
(
    Int32u_t                        claimId,
    Int32u_t                        claimPhn,
    MbSQL                          *sql_p
);

Int32s_t pmcMspReturnsFileHandle( void );

Int32s_t    pmcMspReturnsBugFix1
(
    qHead_p         record_q
);

Int32u_t    pmcMspFileDatabaseCheck
(
    Int32u_t        crc,
    Int32u_t        size,
    Int32u_t        type
);

Int32u_t    pmcMspFileDatabaseAdd
(
    Char_p          fileName_p,
    Int32u_t        type
);

Int32u_t    pmcMspAddToClaim
(
    Int32u_t                        providerId,
    pmcMspReturnsRecordStruct_p     record_p,
    MbSQL                          *sql_p
);

Int32s_t    pmcMspReturnsFeeGet
(
    Char_p          buf_p
);

#endif
