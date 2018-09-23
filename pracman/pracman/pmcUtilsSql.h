//---------------------------------------------------------------------------
// MySQL utility functions
//---------------------------------------------------------------------------
// (c) 2001-2008 Michael A. Bree
//---------------------------------------------------------------------------
#ifndef H_pmcUtilsSql
#define H_pmcUtilsSql

// Platform includes
#include <stdlib.h>

// Library includes
#include "mbTypes.h"
#include "mbSqlLib.h"

// Program includes
#include "pmcDefines.h"

#ifdef  External
#undef  External
#endif

// Appointment structure
typedef struct PmcSqlApp_s
{
    Int32u_t    patientId;
    Int32u_t    startTime;
    Int32u_t    duration;
    Int32u_t    providerId;
    Int32u_t    date;
    Int32u_t    type;
    Int32u_t    confPhoneDate;
    Int32u_t    confPhoneTime;
    Int32u_t    confPhoneId;
    Int32u_t    confLetterDate;
    Int32u_t    confLetterTime;
    Int32u_t    confLetterId;
    Int32u_t    completed;
    Int32u_t    referringDrId;
    Int32u_t    deleted;
    Char_t      commentIn[ PMC_MAX_COMMENT_LEN + 1 ];
    Char_t      commentOut[ PMC_MAX_COMMENT_LEN + 1 ];
#if PMC_APP_HISTORY_CREATE
    Int32u_t    createDate;
    Int32u_t    createTime;
#endif
} PmcSqlApp_t, *PmcSqlApp_p;

// NOTE: This is being replaced with the "Patient" object

typedef struct PmcSqlPatient_s
{
    Char_t                  firstName[ PMC_MAX_NAME_LEN + 1 ];
    Char_t                  lastName[ PMC_MAX_NAME_LEN + 1 ];
    Char_t                  title[ PMC_MAX_TITLE_LEN + 1 ];
    Char_t                  phn[ PMC_MAX_PHN_LEN + 1 ];
    Char_t                  phnProv[ PMC_MAX_PROV_LEN + 1 ];
    Char_t                  phoneDay[PMC_MAX_PHONE_LEN + 1];
    Char_t                  phoneHome[ PMC_MAX_PHONE_LEN + 1];
    Char_t                  areaDay[8];
    Char_t                  areaHome[8];
    Char_t                  formattedPhoneDay[64];
    Char_t                  formattedPhoneHome[64];
    Int32u_t                birthDate;
    Int32u_t                deceasedDate;
    Int32u_t                gender;
    Int32u_t                refDrId;
    Int32u_t                id;
} PmcSqlPatient_t, *PmcSqlPatient_p;

// NOTE: This is being replaced by the "Doctor" object

typedef struct PmcSqlDoctor_s
{
    Char_t                  firstName[ PMC_MAX_NAME_LEN + 1 ];
    Char_t                  lastName[ PMC_MAX_NAME_LEN + 1 ];
    Char_t                  province[ PMC_MAX_PROV_LEN + 1 ];
    Char_t                  otherNumber[ PMC_MAX_OTHER_NUMBER_LEN + 1 ];
    Char_t                  title[ PMC_MAX_TITLE_LEN + 1 ];
    Char_t                  phone[ PMC_MAX_PHONE_LEN + 1 ];
    Char_t                  fax[ PMC_MAX_PHONE_LEN + 1 ];
    Char_t                  formattedPhone[64];
    Char_t                  formattedFax[64];
    Char_t                  address1[ PMC_MAX_ADDRESS_LEN + 1 ];
    Char_t                  address2[ PMC_MAX_ADDRESS_LEN + 1 ];
    Char_t                  address3[ PMC_MAX_ADDRESS_LEN + 1 ];
    Char_t                  city[ PMC_MAX_CITY_LEN + 1 ];
    Char_t                  country[ PMC_MAX_COUNTRY_LEN + 1 ];
    Char_t                  postalCode[ PMC_MAX_POSTAL_CODE_LEN + 1 ];
    Int32u_t                mspNumber;
    Int32u_t                mspActive;
    Int32u_t                cancerClinic;
    Int32u_t                id;
} PmcSqlDoctor_t, *PmcSqlDoctor_p;

typedef struct PmcSqlProvider_s
{
    Char_t                  firstName[ PMC_MAX_NAME_LEN + 1 ];
    Char_t                  lastName[ PMC_MAX_NAME_LEN + 1 ];
    Char_t                  description[ PMC_MAX_DESC_LEN + 1 ];
    Int32u_t                billingNumber;
    Int32u_t                clinicNumber;
    Int32u_t                id;
} PmcSqlProvider_t, *PmcSqlProvider_p;

typedef struct PmcSqlSonographer_s
{
    Char_t                  firstName[ PMC_MAX_NAME_LEN + 1 ];
    Char_t                  lastName[ PMC_MAX_NAME_LEN + 1 ];
    Int32u_t                id;
} PmcSqlSonographer_t, *PmcSqlSonographer_p;

typedef struct PmcSqlEchoDetails_s
{
    PmcSqlPatient_t         patient;
    PmcSqlSonographer_t     sono;
    PmcSqlDoctor_t          referring;
    PmcSqlProvider_t        provider;
    Int32u_t                echoId;
    Int32u_t                detailsId;
    Int32u_t                readDate;
    Int32u_t                studyDate;
    Char_p                  comment_p;
    Char_p                  studyName_p;
    Char_p                  text_p;
    Char_p                  indication_p;
    Char_p                  imageQuality_p;
    Char_p                  rhythm_p;
    Float_t                 rate;

    Float_t                 cd_rv;
    Float_t                 cd_aa;
    Float_t                 cd_la;
    Float_t                 cd_lved;
    Float_t                 cd_lves;
    Float_t                 cd_sept;
    Float_t                 cd_pw;
    Float_t                 cd_mi;
    Float_t                 cd_lvef;

    Char_p                  mv_reg_p;
    Char_p                  mv_sten_p;
    Float_t                 mv_mrja;
    Float_t                 mv_va;
    Float_t                 mv_pev;
    Float_t                 mv_pav;
    Float_t                 mv_mg;
    Float_t                 mv_ivrt;
    Float_t                 mv_edt;

    Char_p                  av_reg_p;
    Char_p                  av_sten_p;
    Float_t                 av_ajl;
    Float_t                 av_apht;
    Float_t                 av_mv;
    Float_t                 av_pg;
    Float_t                 av_mg;
    Float_t                 av_ld;
    Float_t                 av_lv;
    Float_t                 av_vti;
    Float_t                 av_va;

    Char_p                  pv_reg_p;
    Float_t                 pv_vel;
    Float_t                 pv_pat;
    Float_t                 pv_grad;
    Float_t                 pvf_sys;
    Float_t                 pvf_dia;
    Float_t                 pvf_ar;
    Float_t                 pvf_laa;

    Char_p                  tv_reg_p;
    Char_p                  tv_sten_p;
    Float_t                 tv_trja;
    Float_t                 tv_rvsp;
    Float_t                 tv_ev;
    Float_t                 tv_va;
} PmcSqlEchoDetails_t, *PmcSqlEchoDetails_p;


#define PMC_PAT_HISTORY_TYPE_HISTORY        (0)
#define PMC_PAT_HISTORY_TYPE_ALLERGIES      (1)
#define PMC_PAT_HISTORY_TYPE_MEDICATIONS    (2)

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Int32u_t        pmcSqlExecInt
(
    Char_p      table_p,
    Char_p      field_p,
    Int32u_t    value,
    Int32u_t    id
);

Int32u_t        pmcSqlExecDate
(
    Char_p      table_p,
    Char_p      field_p,
    Int32u_t    value,
    Int32u_t    id
);

Int64u_t        pmcSqlTableUpdateTimeGet
(
    Char_p      requestedTable_p
);

Int32u_t        pmcSqlExecTime
(
    Char_p      table_p,
    Char_p      field_p,
    Int32u_t    value,
    Int32u_t    id
);

Int32u_t        pmcSqlExecString
(
    Char_p      table_p,
    Char_p      field_p,
    Char_p      string_p,
    Int32u_t    id
);

Int32u_t        pmcSqlRecordDelete
(
    Char_p      table_p,
    Int32u_t    id
);

Int32u_t        pmcSqlRecordUndelete
(
    Char_p      table_p,
    Int32u_t    id
);
Int32u_t        pmcSqlRecordDeleteUndelete
(
    Char_p      table_p,
    Int32u_t    id,
    Boolean_t   delFlag
);

Int32u_t        pmcSqlExec
(
    Char_p      cmd_p
);

Int32u_t        pmcSqlRecordCreate
(
    Char_p      table_p,
    MbSQL      *sqp_p
);

Int32u_t        pmcSqlSelectInt
(
    Char_p      cmd_p,
    Int32u_p    count_p
);

Char_p          pmcSqlSelectStr
(
    Char_p      cmd_p,
    Char_p      result_p,
    Ints_t      resultLen,
    Int32u_p    count_p
);

Char_p      pmcSqlPatHistoryGet( Int32u_t patientId, Int32u_t type, Int32u_p date_p, Int32u_p time_p );
Int32u_t    pmcSqlPatHistoryEntryGet( mbStrList_p entry_p );
Int32u_t    pmcSqlPatHistoryListGet( Int32u_t patientId, Int32u_t type, qHead_p history_q, Boolean_t addFlag );
Int32u_t    pmcSqlPatHistoryPut( Int32u_t patientId, Int32u_t type, Char_p history_p );
Int32u_t    pmcSqlRecordDeleted( Char_p table_p, Int32u_t id );
Int32u_t    pmcSqlRecordPurge( Char_p table_p, Int32u_t id );
Int32u_t    pmcSqlEchoLock( Int32u_t id, Char_p name_p );
Int32u_t    pmcSqlRecordLock( Char_p table_p, Int32u_t id, Int32u_t notDeletedFlag );
Int32u_t    pmcSqlRecordUnlock( Char_p table_p, Int32u_t id );
Int32u_t    pmcSqlRecordUnlockForce( Char_p table_p, Int32u_t id );
Int32u_t    pmcSqlMatchCount( Char_p cmd_p );
Int32s_t    pmcSqlAppDetailsGet( Int32u_t id, PmcSqlApp_p pat_p );
Int32s_t    pmcSqlPatientDetailsGet( Int32u_t id, PmcSqlPatient_p pat_p );
Int32s_t    pmcSqlDoctorDetailsGet( Int32u_t id,   PmcSqlDoctor_p dr_p );
Int32s_t    pmcSqlProviderDetailsGet( Int32u_t id, PmcSqlProvider_p provider_p );
Int32s_t    pmcSqlSonographerDetailsGet( Int32u_t id, PmcSqlSonographer_p sonographer_p );
Int32s_t    pmcSqlEchoDetailsFree( PmcSqlEchoDetails_p echo_p );

PmcSqlEchoDetails_p pmcSqlEchoDetailsGet( Int32u_t echoId, Boolean_t subDetailsFlag );

#endif /* H_pmcUtilsSql */

