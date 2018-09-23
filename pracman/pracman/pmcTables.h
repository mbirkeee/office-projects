//---------------------------------------------------------------------------
// File:    pmcTables.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Various definitions for the pracman SQL tables
//---------------------------------------------------------------------------

#ifndef H_pmcTables
#define H_pmcTables

#include <vcl\DBTables.hpp>
#include <vcl\DB.hpp>
#include <Db.hpp>
#include <vcl.h>

#include "mbTypes.h"
#include "pmcDefines.h"

#ifdef  External
#undef  External
#endif

#ifdef  PMC_INIT_GLOBALS
#define External
#else
#define External extern
#endif  // PMC_INIT_GLOBALS

// Table names - Polled for changes
#define PMC_SQL_TABLE_PATIENTS                  "patients"
#define PMC_SQL_TABLE_PROVIDERS                 "providers"
#define PMC_SQL_TABLE_APPS                      "appointments"
#define PMC_SQL_TABLE_DOCTORS                   "doctors"
#define PMC_SQL_TABLE_CLAIMS                    "claims"
#define PMC_SQL_TABLE_CLAIM_HEADERS             "claim_headers"
#define PMC_SQL_TABLE_TIMESLOTS                 "timeslots"
#define PMC_SQL_TABLE_ECHOS                     "echos"
#define PMC_SQL_TABLE_MED_LIST                  "med_list"
#define PMC_SQL_TABLE_CONFIG                    "config"
#define PMC_SQL_TABLE_DOCUMENTS                 "documents"


// Number of tables that are auto polled
#define PMC_TABLE_COUNT     11

// These tables - not polled for updates.
#define PMC_SQL_TABLE_MSP_FILES                 "msp_files"
#define PMC_SQL_TABLE_APP_HIST                  "app_hist"
#define PMC_SQL_TABLE_ECHO_FILES                "echo_files"
#define PMC_SQL_TABLE_ECHO_CDS                  "echo_cds"
#define PMC_SQL_TABLE_ECHO_BACKUPS              "echo_backups"
#define PMC_SQL_TABLE_SONOGRAPHERS              "sonographers"
#define PMC_SQL_TABLE_RUN_CODES                 "run_codes"
#define PMC_SQL_TABLE_ECHO_DETAILS              "echo_details"
#define PMC_SQL_TABLE_PAT_HISTORY               "pat_history"

// Table names that are auto polled
External  Char_p pmcTableNames_p[]
#ifdef PMC_INIT_GLOBALS
=
{
    PMC_SQL_TABLE_PATIENTS
   ,PMC_SQL_TABLE_PROVIDERS
   ,PMC_SQL_TABLE_APPS
   ,PMC_SQL_TABLE_DOCTORS
   ,PMC_SQL_TABLE_CLAIMS
   ,PMC_SQL_TABLE_CLAIM_HEADERS
   ,PMC_SQL_TABLE_TIMESLOTS
   ,PMC_SQL_TABLE_ECHOS
   ,PMC_SQL_TABLE_MED_LIST
   ,PMC_SQL_TABLE_CONFIG
   ,PMC_SQL_TABLE_DOCUMENTS
}
#endif
;

External Int64u_t pmcPollTableModifyTime[]
 #ifdef PMC_INIT_GLOBALS
=
{
     0ui64
    ,0ui64
    ,0ui64
    ,0ui64
    ,0ui64
    ,0ui64
    ,0ui64
    ,0ui64
    ,0ui64
    ,0ui64
    ,0ui64
}
#endif
;

External Int32u_t pmcPollTableSize[]
#ifdef PMC_INIT_GLOBALS
=
{
     0
    ,0
    ,0
    ,0
    ,0
    ,0
    ,0
    ,0
    ,0
    ,0
    ,0
}
#endif
;

#define PMC_TABLE_INDEX_PATIENT         0
#define PMC_TABLE_INDEX_PROVIDERS       1
#define PMC_TABLE_INDEX_APPS            2
#define PMC_TABLE_INDEX_DOCTORS         3
#define PMC_TABLE_INDEX_CLAIMS          4
#define PMC_TABLE_INDEX_CLAIM_HEADERS   5
#define PMC_TABLE_INDEX_TIMESLOTS       6
#define PMC_TABLE_INDEX_ECHOS           7
#define PMC_TABLE_INDEX_MED_LIST        8
#define PMC_TABLE_INDEX_CONFIG          9
#define PMC_TABLE_INDEX_DOCUMENTS      10

// Bit mask bits for selecting table.  These *must* match the
// definitions above
#define PMC_TABLE_BIT_PATIENTS      0x00000001
#define PMC_TABLE_BIT_PROVIDERS     0x00000002
#define PMC_TABLE_BIT_APPS          0x00000004
#define PMC_TABLE_BIT_DOCTORS       0x00000008
#define PMC_TABLE_BIT_CLAIMS        0x00000010
#define PMC_TABLE_BIT_CLAIM_HEADERS 0x00000020
#define PMC_TABLE_BIT_TIMESLOTS     0x00000040
#define PMC_TABLE_BIT_ECHOS         0x00000080
#define PMC_TABLE_BIT_MED_LIST      0x00000100
#define PMC_TABLE_BIT_CONFIG        0x00000200
#define PMC_TABLE_BIT_DOCUMENTS     0x00000400

External  Int32u_t pmcTableBits[]
#ifdef PMC_INIT_GLOBALS
=
{
     PMC_TABLE_BIT_PATIENTS
    ,PMC_TABLE_BIT_PROVIDERS
    ,PMC_TABLE_BIT_APPS
    ,PMC_TABLE_BIT_DOCTORS
    ,PMC_TABLE_BIT_CLAIMS
    ,PMC_TABLE_BIT_CLAIM_HEADERS
    ,PMC_TABLE_BIT_TIMESLOTS
    ,PMC_TABLE_BIT_ECHOS
    ,PMC_TABLE_BIT_MED_LIST
    ,PMC_TABLE_BIT_CONFIG
    ,PMC_TABLE_BIT_DOCUMENTS
}
#endif
;

typedef struct pmcTableStatus_s
{
    Variant     name;
    Int64u_t    newModifyTime;
    Int64u_t    curModifyTime;
    Int64u_t    lastReadTime;
    Int32u_t    newDataSize;
    Int32u_t    curDataSize;
    Int32u_t    bit;
} pmcTableStatus_t, *pmcTableStatus_p;

// Generic fields names
#define PMC_SQL_FIELD_NOT_DELETED               "not_deleted"
#define PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE    1
#define PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE   0
#define PMC_SQL_FIELD_LOCK                      "lock_value"
#define PMC_SQL_FIELD_MODIFIED                  "modified"
#define PMC_SQL_FIELD_CREATED                   "created"
#define PMC_SQL_FIELD_ID                        "id"
#define PMC_SQL_FIELD_PATIENT_ID                "patient_id"
#define PMC_SQL_FIELD_PROVIDER_ID               "provider_id"
#define PMC_SQL_FIELD_SONOGRAPHER_ID            "sonographer_id"
#define PMC_SQL_FIELD_DATE                      "date"
#define PMC_SQL_FIELD_SIZE                      "size"
#define PMC_SQL_FIELD_TIME                      "time"
#define PMC_SQL_FIELD_CRC                       "crc"
#define PMC_SQL_FIELD_TYPE                      "type"
#define PMC_SQL_FIELD_NAME                      "name"
#define PMC_SQL_FIELD_DESC                      "description"
#define PMC_SQL_FIELD_ADDRESS1                  "address_1"
#define PMC_SQL_FIELD_ADDRESS2                  "address_2"
#define PMC_SQL_FIELD_CITY                      "city"
#define PMC_SQL_FIELD_PROVINCE                  "province"
#define PMC_SQL_FIELD_COUNTRY                   "country"
#define PMC_SQL_FIELD_POSTAL_CODE               "postal_code"
#define PMC_SQL_FIELD_FIRST_NAME                "first_name"
#define PMC_SQL_FIELD_LAST_NAME                 "last_name"
#define PMC_SQL_FIELD_MIDDLE_NAME               "middle_name"
#define PMC_SQL_FIELD_TITLE                     "title"
#define PMC_SQL_FIELD_RUN_CODE                  "run_code"
#define PMC_SQL_FIELD_ECHO_ID                   "echo_id"
#define PMC_SQL_FIELD_REFERRING_ID              "referring_id"

#define PMC_SQL_TRUE_VALUE                      1
#define PMC_SQL_FALSE_VALUE                     0

#define PMC_SQL_GENDER_MALE                     0
#define PMC_SQL_GENDER_FEMALE                   1

// Field names in the "show table status" command
#define PMC_SQL_FIELD_TABLE_STATUS_MODIFY_TIME  "Update_time"
#define PMC_SQL_FIELD_TABLE_STATUS_DATA_LENGTH  "Data_length"
#define PMC_SQL_FIELD_TABLE_NAME                "Name"

// Field names in the pat_history table
#define PMC_SQL_PAT_HISTORY_DATA                "value"

// Field names in the providers table
#define PMC_SQL_PROVIDERS_FIELD_DOC_SEARCH_STRING  "doc_search_string"
#define PMC_SQL_PROVIDERS_FIELD_TEST_DESC       "test_desc"
#define PMC_SQL_PROVIDERS_FIELD_CLAIM_NUMBER    "claim_number"
#define PMC_SQL_PROVIDERS_FIELD_CLINIC_NUMBER   "clinic_number"
#define PMC_SQL_PROVIDERS_FIELD_CLINIC_NAME     "clinic_name"
#define PMC_SQL_PROVIDERS_FIELD_PROVIDER_NUMBER "doctor_number"
#define PMC_SQL_PROVIDERS_FIELD_STREET_ADDRESS  "street_address"
#define PMC_SQL_PROVIDERS_FIELD_CITY_PROVINCE   "city_province"
#define PMC_SQL_PROVIDERS_FIELD_POSTAL_CODE     "postal_code"
#define PMC_SQL_PROVIDERS_FIELD_PICKLIST_ORDER  "picklist_order"
#define PMC_SQL_PROVIDERS_FIELD_APP_LENGTH      "app_length"
#define PMC_SQL_PROVIDERS_FIELD_CORP_CODE       "corporation_code"

// Field names in the patients table
#define PMC_SQL_PATIENTS_FIELD_HOME_PHONE       "home_phone"
#define PMC_SQL_PATIENTS_FIELD_PHN              "health_num"
#define PMC_SQL_PATIENTS_FIELD_PHN_PROV         "health_num_prov"
#define PMC_SQL_PATIENTS_FIELD_CELL_PHONE       "cell_phone"
#define PMC_SQL_PATIENTS_FIELD_WORK_PHONE       "work_phone"
#define PMC_SQL_PATIENTS_FIELD_WORK_DESC        "work_desc"
#define PMC_SQL_PATIENTS_FIELD_OFFICE_ID        "office_id"
#define PMC_SQL_PATIENTS_FIELD_DATE_OF_BIRTH    "date_of_birth"
#define PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH    "date_of_death"
#define PMC_SQL_PATIENTS_FIELD_CHART_STRING     "chart_string"
#define PMC_SQL_PATIENTS_FIELD_EMAIL_ADDRESS    "email_address"
#define PMC_SQL_PATIENTS_FIELD_PREFER_EMAIL     "prefer_email"
#define PMC_SQL_PATIENTS_FIELD_CONTACT_NAME     "contact_name"
#define PMC_SQL_PATIENTS_FIELD_CONTACT_DESC     "contact_desc"
#define PMC_SQL_PATIENTS_FIELD_CONTACT_PHONE    "contact_phone"
#define PMC_SQL_PATIENTS_FIELD_COMMENT          "comment"
#define PMC_SQL_PATIENTS_FIELD_GENDER           "gender"
#define PMC_SQL_PATIENTS_FIELD_PHN_PROVINCE     "health_num_prov"
#define PMC_SQL_PATIENTS_FIELD_FAMILY_DR_ID     "family_physician"
#define PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID  "referring_physician"
#define PMC_SQL_PATIENTS_FIELD_OLD_CHART        "old_chart"

// Referring doctors table fields
#define PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER     "doctor_number"
#define PMC_SQL_DOCTORS_FIELD_OTHER_NUMBER      "other_number"
#define PMC_SQL_DOCTORS_FIELD_WORK_PHONE        "work_phone"
#define PMC_SQL_DOCTORS_FIELD_WORK_FAX          "work_fax"
#define PMC_SQL_DOCTORS_FIELD_COMMENT           "comment"
#define PMC_SQL_DOCTORS_FIELD_EMAIL             "email_address"
#define PMC_SQL_DOCTORS_FIELD_DEGREES           "degrees"
#define PMC_SQL_DOCTORS_FIELD_SPECIALTY         "specialty"
#define PMC_SQL_DOCTORS_FIELD_CANCER_CLINIC     "cancer_clinic"
#define PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST       "on_msp_list"

// Available time table fields
#if PMC_AVAIL_TIME
#define PMC_SQL_FIELD_PROVIDER_ID               "provider_id"
#define PMC_SQL_AVAILTIME_FIELD_DAY_OF_WEEK     "day_of_week"
#define PMC_SQL_AVAILTIME_FIELD_REPEAT_CODE     "repeat_code"
#define PMC_SQL_AVAILTIME_FIELD_START_TIME      "start_time"
#define PMC_SQL_AVAILTIME_FIELD_STOP_TIME       "stop_time"
#endif

// Appointment table fields
#define PMC_SQL_APPS_FIELD_START_TIME           "start_time"
#define PMC_SQL_APPS_FIELD_DURATION             "duration"
#define PMC_SQL_APPS_FIELD_COLOR                "color"
#define PMC_SQL_APPS_FIELD_COMMENT_IN           "comment_in"
#define PMC_SQL_APPS_FIELD_COMMENT_OUT          "comment_out"
#define PMC_SQL_APPS_FIELD_TYPE                 "appointment_type"
#define PMC_SQL_APPS_FIELD_CONF_PHONE_DATE      "confirmed_phone_day"
#define PMC_SQL_APPS_FIELD_CONF_PHONE_TIME      "confirmed_phone_time"
#define PMC_SQL_APPS_FIELD_CONF_PHONE_ID        "confirmed_phone_provider_id"
#define PMC_SQL_APPS_FIELD_CONF_LETTER_DATE     "confirmed_letter_day"
#define PMC_SQL_APPS_FIELD_CONF_LETTER_TIME     "confirmed_letter_time"
#define PMC_SQL_APPS_FIELD_CONF_LETTER_ID       "confirmed_letter_provider_id"
#define PMC_SQL_APPS_FIELD_COMPLETED            "completed_state"
#define PMC_SQL_APPS_FIELD_REFERRING_DR_ID      "referring_physician"
#define PMC_SQL_APPS_FIELD_FAMILY_DR_ID         "family_physician"

#define PMC_SQL_CLAIMS_FIELD_SERVICE_TYPE       "service_type"
#define PMC_SQL_CLAIMS_FIELD_SERVICE_DATE       "service_day"
#define PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_EARLIEST  "service_day_earliest"
#define PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LATEST    "service_day_latest"
#define PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LAST      "service_day_last"
#define PMC_SQL_CLAIMS_FIELD_SERVICE_LOCATION   "service_location"
#define PMC_SQL_CLAIMS_FIELD_SUBMIT_DATE        "submit_day"
#define PMC_SQL_CLAIMS_FIELD_REPLY_DATE         "reply_day"
#define PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER       "claim_number"
#define PMC_SQL_CLAIMS_FIELD_CLAIM_INDEX        "claim_index"
#define PMC_SQL_CLAIMS_FIELD_FEE_SUBMITTED      "fee_submitted"
#define PMC_SQL_CLAIMS_FIELD_FEE_PAID           "fee_paid"
#define PMC_SQL_CLAIMS_FIELD_FEE_CODE           "fee_code"
#define PMC_SQL_CLAIMS_FIELD_RUN_CODE           "run_code"
#define PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED  "fee_code_approved"
#define PMC_SQL_CLAIMS_FIELD_ICD_CODE           "icd_code"
#define PMC_SQL_CLAIMS_FIELD_UNITS              "units"
#define PMC_SQL_CLAIMS_FIELD_UNITS_PAID         "units_paid"
#define PMC_SQL_CLAIMS_FIELD_COMMENT_TO_MSP     "comment_in"
#define PMC_SQL_CLAIMS_FIELD_COMMENT_FROM_MSP   "comment_out"
#define PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ          "seq_number"
#define PMC_SQL_CLAIMS_FIELD_ADJ_CODE           "adjust_code"
#define PMC_SQL_CLAIMS_FIELD_REFERRING_DR_ID    "ref_dr_id"
#define PMC_SQL_CLAIMS_FIELD_REFERRING_DR_TYPE  "ref_dr_type"
#define PMC_SQL_CLAIMS_FIELD_REFERRING_DR_NUM   "ref_dr_number"
#define PMC_SQL_CLAIMS_FIELD_APPOINTMENT_TYPE   "appointment_type"
#define PMC_SQL_CLAIMS_FIELD_DIAGNOSIS          "diagnosis"
#define PMC_SQL_CLAIMS_FIELD_EXP_CODE           "exp_code"
#define PMC_SQL_CLAIMS_FIELD_STATUS             "status"
#define PMC_SQL_CLAIMS_FIELD_REPLY_STATUS       "reply_status"
#define PMC_SQL_CLAIMS_FIELD_SUB_COUNT          "submit_count"
#define PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID    "claim_header_id"
#define PMC_SQL_CLAIMS_FIELD_FEE_PREMIUM        "fee_premium"
#define PMC_SQL_CLAIMS_FIELD_FEE_CODE_PREMIUM   "fee_code_premium"

// Claim Header table fields
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID    "claim_id_"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_0  "claim_id_0"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_1  "claim_id_1"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_2  "claim_id_2"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_3  "claim_id_3"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_4  "claim_id_4"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_5  "claim_id_5"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_6  "claim_id_6"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_7  "claim_id_7"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID_8  "claim_id_8"
#define PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_NUMBER "claim_number"
#define PMC_SQL_CLAIM_HEADERS_FIELD_DIAGNOSIS   "diagnosis"

// Timeslot table fields
#define PMC_SQL_TIMESLOTS_FIELD_AVAILABLE       "available"

// Documents List
#define PMC_SQL_DOCUMENTS_FIELD_ORIG_NAME       "orig_name"
#define PMC_SQL_DOCUMENTS_FIELD_TEMPLATE        "template"
#define PMC_SQL_DOCUMENTS_FIELD_STATUS          "imported"
#define PMC_SQL_DOCUMENTS_FIELD_DOCTOR_ID       "doctor_id"
#define PMC_SQL_DOCUMENTS_FIELD_APP_DATE        "app_date"

// Appontment History Table
#define PMC_SQL_APP_HIST_FIELD_APP_ID           "app_id"
#define PMC_SQL_APP_HIST_FIELD_ACTION           "action"
#define PMC_SQL_APP_HIST_FIELD_INT_1            "int_1"
#define PMC_SQL_APP_HIST_FIELD_INT_2            "int_2"
#define PMC_SQL_APP_HIST_FIELD_INT_3            "int_3"
#define PMC_SQL_APP_HIST_FIELD_INT_4            "int_4"
#define PMC_SQL_APP_HIST_FIELD_INT_5            "int_5"
#define PMC_SQL_APP_HIST_FIELD_STRING_1         "string_1"

// Echos table
#define PMC_SQL_ECHOS_FIELD_BYTE_SUM            "byte_sum"
#define PMC_SQL_ECHOS_FIELD_ONLINE              "online"
#define PMC_SQL_ECHOS_FIELD_BACKUP              "backup"
#define PMC_SQL_ECHOS_FIELD_READ_DATE           "read_date"
#define PMC_SQL_ECHOS_FIELD_READ_TIME           "read_time"
#define PMC_SQL_ECHOS_FIELD_COMMENT             "description"
#define PMC_SQL_ECHOS_FIELD_NAME                "name"

// Echo backups
#define PMC_SQL_ECHO_BACKUPS_FIELD_ECHO_ID      "echo_id"
#define PMC_SQL_ECHO_BACKUPS_FIELD_CD_ID        "cd_id"

// Echo details
#define PMC_SQL_ECHO_DETAILS_FIELD_REFERRING_ID "referring_id"
#define PMC_SQL_ECHO_DETAILS_NOTES              "text"
#define PMC_SQL_ECHO_DETAILS_INDICATION         "indication"
#define PMC_SQL_ECHO_DETAILS_IMAGE_QUALITY      "image_quality"
                                                              
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_RV        "cd_rv"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_AA        "cd_aa"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_LA        "cd_la"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_LVED      "cd_lved"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_LVES      "cd_lves"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_SEPT      "cd_septum"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_PW        "cd_pw"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_MI        "cd_mi"
#define PMC_SQL_ECHO_DETAILS_FIELD_CD_LVEF      "cd_lvef"

#define PMC_SQL_ECHO_DETAILS_FIELD_MV_REG       "mv_reg"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_STEN      "mv_sten"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_MRJA      "mv_mrja"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_VA        "mv_va"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_PEV       "mv_pev"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_PAV       "mv_pav"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_MG        "mg_mg"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_IVRT      "mv_ivrt"
#define PMC_SQL_ECHO_DETAILS_FIELD_MV_EDT       "mv_edt"

#define PMC_SQL_ECHO_DETAILS_FIELD_AV_REG       "av_reg"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_STEN      "av_sten"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_AJL       "av_ajl"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_APHT      "av_apht"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_MV        "av_mv"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_PG        "av_pg"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_MG        "av_mg"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_LD        "av_ld"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_LV        "av_lv"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_VTI       "av_vti"
#define PMC_SQL_ECHO_DETAILS_FIELD_AV_VA        "av_va"

#define PMC_SQL_ECHO_DETAILS_FIELD_PV_REG       "pv_reg"
#define PMC_SQL_ECHO_DETAILS_FIELD_PV_VEL       "pv_vel"
#define PMC_SQL_ECHO_DETAILS_FIELD_PV_PAT       "pv_pat"
#define PMC_SQL_ECHO_DETAILS_FIELD_PV_GRAD      "pv_grad"
#define PMC_SQL_ECHO_DETAILS_FIELD_PVF_SYS      "pvf_sys"
#define PMC_SQL_ECHO_DETAILS_FIELD_PVF_DIA      "pvf_dia"
#define PMC_SQL_ECHO_DETAILS_FIELD_PVF_AR       "pvf_ar"
#define PMC_SQL_ECHO_DETAILS_FIELD_PVF_LAA      "pvf_laa"

#define PMC_SQL_ECHO_DETAILS_FIELD_RATE         "heart_rate"       
#define PMC_SQL_ECHO_DETAILS_FIELD_RHYTHM       "heart_rhythm"
#define PMC_SQL_ECHO_DETAILS_FIELD_TV_REG       "tv_reg"
#define PMC_SQL_ECHO_DETAILS_FIELD_TV_STEN      "tv_sten"
#define PMC_SQL_ECHO_DETAILS_FIELD_TV_TRJA      "tv_trja"
#define PMC_SQL_ECHO_DETAILS_FIELD_TV_RVSP      "tv_rvsp"
#define PMC_SQL_ECHO_DETAILS_FIELD_TV_EV        "tv_ev"
#define PMC_SQL_ECHO_DETAILS_FIELD_TV_VA        "tv_va"

// Config table
#define PMC_SQL_CONFIG_FIELD_VAL_STR_1         "val_str_1"
#define PMC_SQL_CONFIG_FIELD_VAL_STR_2         "val_str_2"
#define PMC_SQL_CONFIG_FIELD_VAL_INT_1         "val_int_1"
#define PMC_SQL_CONFIG_FIELD_VAL_INT_2         "val_int_2"

// Med List table
#define PMC_SQL_MED_LIST_FIELD_GENERIC          "name"
#define PMC_SQL_MED_LIST_FIELD_BRAND            "brand"
#define PMC_SQL_MED_LIST_FIELD_CLASS            "class"

// Files table
#define PMC_SQL_ECHO_FILES_FIELD_NAME           "name"
#define PMC_SQL_ECHO_FILES_FIELD_NAME_ORIG      "orig"
#define PMC_SQL_ECHO_FILES_FIELD_SUB_TYPE       "sub_type"
#define PMC_SQL_ECHO_FILES_FIELD_PATH           "path"
#define PMC_SQL_ECHO_FILES_FIELD_ECHO_ID        "echo_id"
#define PMC_SQL_ECHO_FILES_FILED_COUNT          "sum(not_deleted)"

// Various SQL commands
#define PMC_SQL_CMD_PROVIDERS_MAX_ID        "select max( id ) as id from providers"
#define PMC_SQL_CMD_PATIENTS_COUNT          "select sum(not_deleted) from patients where not_deleted=1 and id > 0"
#define PMC_SQL_CMD_PATIENTS_COUNT_FIELD    "sum(not_deleted)"
#define PMC_SQL_CMD_CLAIMS_COUNT_FIELD      "sum(claims.not_deleted)"
#define PMC_SQL_CMD_DOCTORS_COUNT           "select sum(not_deleted) from doctors where not_deleted=1 and id > 0"
#define PMC_SQL_CMD_DOCTORS_COUNT_FIELD     "sum(not_deleted)"
#define PMC_SQL_CMD_COUNT                   "count(*)"

#define PMC_CURRENT_LIST_NAME       0
#define PMC_CURRENT_LIST_PHONE      1
#define PMC_CURRENT_LIST_NUM        2

#endif // H_pmcTables

