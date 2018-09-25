//---------------------------------------------------------------------------
// File:    pmcMsp.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    May 4, 2002
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcMspH
#define pmcMspH

#define PMC_CLAIM_RECORD_CODE_HEADER                "10"
#define PMC_CLAIM_RECORD_CODE_SERVICE               "50"
#define PMC_CLAIM_RECORD_CODE_HOSPITAL              "57"
#define PMC_CLAIM_RECORD_CODE_COMMENT               "60"
#define PMC_CLAIM_RECORD_CODE_RECIPROCAL            "89"
#define PMC_CLAIM_RECORD_CODE_TRAILER               "90"

#define PMC_MSP_EXP_CODE_PREMIUM                    "FZ"

#define PMC_CLAIM_NUMBER_NOT_ASSIGNED               999999

#define PMC_CLAIM_NUMBER_MIN                        10000
#define PMC_CLAIM_NUMBER_MAX                        99999

// RETURNS record types
#define PMC_RETURN_TYPE_UNKNOWN                     0
#define PMC_RETURN_TYPE_PAID                        1
#define PMC_RETURN_TYPE_TOTAL                       2
#define PMC_RETURN_TYPE_MESSAGE                     3
#define PMC_RETURN_TYPE_VISIT                       4
#define PMC_RETURN_TYPE_HOSPITAL                    5
#define PMC_RETURN_TYPE_COMMENT                     6
#define PMC_RETURN_TYPE_RECIPROCAL                  7
#define PMC_RETURN_TYPE_RECOVERY                    8
#define PMC_RETURN_TYPE_ADDITIONAL                  9
#define PMC_RETURN_TYPE_CODE_CHANGE_MULTI           10

#define PMC_RETURN_FORM_TYPE_PAPER                  1
#define PMC_RETURN_FORM_TYPE_ELECTRONIC_6           6
#define PMC_RETURN_FORM_TYPE_ELECTRONIC_9           9

#define PMC_CLAIM_RECORD_TYPE_HEADER                1
#define PMC_CLAIM_RECORD_TYPE_SERVICE               2
#define PMC_CLAIM_RECORD_TYPE_HOSPITAL              3
#define PMC_CLAIM_RECORD_TYPE_COMMENT               4
#define PMC_CLAIM_RECORD_TYPE_RECIPROCAL            5
#define PMC_CLAIM_RECORD_TYPE_TRAILER               6
#define PMC_CLAIM_RECORD_TYPE_DEBUG                 7

#define PMC_CLAIM_RECORD_LEN_HEADER                 98
#define PMC_CLAIM_RECORD_LEN_SERVICE                98
#define PMC_CLAIM_RECORD_LEN_HOSPITAL               98
#define PMC_CLAIM_RECORD_LEN_COMMENT                98
#define PMC_CLAIM_RECORD_LEN_RECIPROCAL             98
#define PMC_CLAIM_RECORD_LEN_DEBUG                  98
#define PMC_CLAIM_RECORD_LEN_TRAILER                98

#define PMC_CLAIM_SUBMISSION_TYPE_PHYSICIAN         "1"
#define PMC_CLAIM_SUBMISSION_TYPE                   "9"

#define PMC_MSP_FILENAME_CLAIMSIN                   "CLAIMSIN"
#define PMC_MSP_FILENAME_RETURNS                    "RETURNS"
#define PMC_MSP_FILENAME_VALIDRPT                   "VALIDRPT"
#define PMC_MSP_FILENAME_NOTICE                     "NOTICE"
#define PMC_MSP_FILENAME_INFO                       "CLAIMSIN_INFO"

#define PMC_MSP_FILE_TYPE_INVALID                   0
#define PMC_MSP_FILE_TYPE_RETURNS                   1
#define PMC_MSP_FILE_TYPE_NOTICE                    2
#define PMC_MSP_FILE_TYPE_CLAIMSIN                  3
#define PMC_MSP_FILE_TYPE_VALIDRPT                  4

#define PMC_MSP_COMMENT_LENGTH                      77

#define PMC_MSP_TOTAL_RECORD_STRING_TOTAL           "TOTAL"
#define PMC_MSP_TOTAL_RECORD_STRING_SMA_DUES        "SMA DUES"
#define PMC_MSP_TOTAL_RECORD_STRING_PAPER_CLAIMS    "PAPER CLMS"

#define PMC_MSP_ADJUST_RECOVERY                        '-'
#define PMC_MSP_ADJUST_ADDITIONAL                      '+'
#define PMC_MSP_ADJUST_NONE                            ' '

#endif // pmcMspH
