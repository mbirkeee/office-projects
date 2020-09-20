//----------------------------------------------------------------------------
// File:    pmcDatabaseMaint.h
//----------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//----------------------------------------------------------------------------
// Date:    Feb. 15, 2001
//----------------------------------------------------------------------------
// Description:
//
//----------------------------------------------------------------------------

#ifndef pmcDatabaseMaintH
#define pmcDatabaseMaintH

typedef struct pmcMspFileStruct_s
{
    Int32u_t    linkage;
    Int32u_t    size;
    Int32u_t    type;
} pmcMspFileStruct_t, pmcMspFileStruct_p;

typedef struct pmcDocumentMaintStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    patientId;
    Char_p      origName_p;
    Int32u_t    type;
    Int32u_t    crc;
    Int32u_t    size;
    Char_p      name_p;
    Char_p      desc_p;
} pmcDocumentMaintStruct_t, *pmcDocumentMaintStruct_p;

typedef struct claimCheckStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    providerId;
    Int32u_t    headerId;
    Int32u_t    claimNumber;
    Int32u_t    serviceDate;
    Int32u_t    notDeleted;
    Int32u_t    status;
    Int32u_t    statusChecked;
    Char_t      feeCode[8];
} claimCheckStruct_t, *claimCheckStruct_p;

typedef struct claimHeaderStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    claimId[PMC_CLAIM_COUNT];
} claimHeaderStruct_t, *claimHeaderStruct_p;

typedef struct patCheck_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    gender;
    Int32u_t    refDrId;
    Int32u_t    providerId;
    Int32u_t    dob;
    Char_p      firstName_p;
    Char_p      lastName_p;
    Char_p      title_p;
    Char_p      phn_p;
    Char_p      phnProv_p;
} patCheck_t, *patCheck_p;

typedef struct pat_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    refDr;
    Int32u_t    famDr;
    Int32u_t    code;
    Char_p      postalCode_p;
    Char_p      workPhone_p;
    Char_p      homePhone_p;
} patTemp_t, *patTemp_p;

typedef struct PmcEchoCheck_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    date;
    Int32u_t    providerId;
    Char_p      name_p;
} PmcEchoCheck_t, *PmcEchoCheck_p;

typedef struct docMaintStruct_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
    Int32u_t    doctorNumber;
    Char_p      city_p;
    Char_p      specialty_p;
    Char_p      lastName_p;
    Char_p      firstName_p;
    Char_p      fax_p;
    Char_p      phone_p;
} docMaintStruct_t,   *docMaintStruct_p;

typedef struct  PmcIdCheck_s
{
    qLinkage_t  linkage;
    Int32u_t    id;
} PmcIdCheck_t, PmcIdCheck_p;

//typedef struct appTemp_s
//{
//    qLinkage_t  linkage;
//    Int32u_t    id;
//    Int32u_t    referringId;
//    Int32u_t    patientId;
//    Int32u_t    providerId;
//    Int32u_t    date;
//} appTemp_t, *appTemp_p;

void        pmcDatabaseCheckDocuments( void );
void        pmcDatabaseCleanFields( void );
void        pmcCheckPatientsUnreferenced( void );
void        pmcDatabaseEchosAssignPatients( void );
void        pmcDatabaseCheckTimeslots( void );
void        pmcCheckPatientRecords( void );
void        pmcCheckAppointmentRecords( void );
void        pmcReassignPatients( void );
void        pmcStolenPatientLetters( void );
void        pmcImportMspDrListExcel( void );
void        pmcDatabaseCheckActiveDocuments( void );

Int32u_t    pmcDrNumToId( Int32u_t  drNum, qHead_p  q_p );

#define PMC_SMA_DR_LIST_FIELDS      12

#define PMC_SMA_FIELD_DOCTOR_NUMBER 0
#define PMC_SMA_FIELD_LAST_NAME     1
#define PMC_SMA_FIELD_FIRST_NAME    2
#define PMC_SMA_FIELD_ADDRESS1      3
#define PMC_SMA_FIELD_ADDRESS2      4
#define PMC_SMA_FIELD_ADDRESS3      5
#define PMC_SMA_FIELD_ADDRESS4      6
#define PMC_SMA_FIELD_CITY          7
#define PMC_SMA_FIELD_PROVINCE      8
#define PMC_SMA_FIELD_POSTAL_CODE   9
#define PMC_SMA_FIELD_PHONE         10
#define PMC_SMA_FIELD_FAX           11

void pmcDatabaseCheckClaimRecords( void );
void pmcDatabaseIncrementClaimNumbers( void );

void pmcSmaDoctorRecordUpdate
(
    Int32u_t     doctorId,
    Char_p      *field_pp,
    bool         createFlag,
    FILE        *fp
);

void pmcSmaDoctorListImport
(
    void
);

void pmcSmaCleanFirstName
(
    Char_p nameIn_p,
    Char_p firstNameOut_p,
    Char_p middleNameOut_p
);

void pmcMspCleanNames
(
    Char_p  in_p,
    Char_p  firstName_p,
    Char_p  lastName_p
);

void pmcMspFixCity( Char_p in_p, Char_p prov_p );

void pmcScrambleNames( void );

void pmcExtractDocuments( void );

void pmcImportMspFiles( void );
#endif //pmcDatabaseMaintH
