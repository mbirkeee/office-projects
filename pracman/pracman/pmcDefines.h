//----------------------------------------------------------------------------
// File:    pmcDefines.h
//----------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//----------------------------------------------------------------------------
// Date:    March 6, 2001
//----------------------------------------------------------------------------
// Description:
//
//----------------------------------------------------------------------------

#ifndef pmcDefines_h
#define pmcDefines_h

#define PMC_NAME                "Practice Manager"

//#define PMC_DEFAULT_DIRECTORY   "C:\\Program Files\\pracman"
#define PMC_INVALID_STRING      "_INVALID_"

// Compile options
#define PMC_SQL_SANITY_CHECK        0

// NOTE: Requires support compiled into mbUtils
#define PMC_TRACK_MALLOC            0

// Include extra debugging code
#define PMC_DEBUG                   1

// Feature that initializes the app history database for the first time
#define PMC_APP_HISTORY_CREATE      0

// Include available time database support
#define PMC_AVAIL_TIME              0

// Allow patient name scrambling
#define PMC_SCRAMBLE_PATIENT_NAMES  0

// PMC Version
#define PMC_MAJOR_VERSION           1
#define PMC_MINOR_VERSION           1

#define PMC_DBL_CLICK_SELECT        0
#define PMC_DBL_CLICK_EDIT          1

#define PMC_PHN_DEFAULT_PROVINCE    "SK"
#define PMC_DEFAULT_PROVINCE        "SK"

#define PMC_CLAIM_TYPE_SERVICE      0
#define PMC_CLAIM_TYPE_HOSPITAL     1
#define PMC_CLAIM_TYPE_ANY          2

#define PMC_MAX_FEE                 (999999)

#define PMC_MAX_SASK_DR_NUMBER      (9999)
#define PMC_FLOAT_NOT_SET           (-9999.9)
#define PMC_MIN_FLOAT               (-9999.0)

#define PMC_MAX_COMMENT_LEN         (1023)
#define PMC_MAX_NAME_LEN            (127)
#define PMC_MAX_TITLE_LEN           (15)
#define PMC_MAX_PROV_LEN            (31)
#define PMC_MAX_PHN_LEN             (31)
#define PMC_MAX_OTHER_NUMBER_LEN    (63)
#define PMC_MAX_DESC_LEN            (127)
#define PMC_MAX_PHONE_LEN           (31)
#define PMC_MAX_ADDRESS_LEN         (127)
#define PMC_MAX_CITY_LEN            (63)
#define PMC_MAX_COUNTRY_LEN         (63)
#define PMC_MAX_POSTAL_CODE_LEN     (31)

#endif // pmcDefines_h

