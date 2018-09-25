//----------------------------------------------------------------------------
// File:    pmcGlobals.h
//----------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//----------------------------------------------------------------------------
// Date:    March 6, 2001
//----------------------------------------------------------------------------
// Description:
//
//
//----------------------------------------------------------------------------

#ifndef H_pmcGlobals
#define H_pmcGlobals

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>                                                                   
#include <ComCtrls.hpp>
#include "CCALENDR.h"
#include <Grids.hpp>
#include <DBCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ActnList.hpp>
#include <ImgList.hpp>
#include <StdActns.hpp>
#include <Db.hpp>
#include <DBTables.hpp>
#include <DBGrids.hpp>
#include <Dialogs.hpp>

#include "pmcDefines.h"
#include "pmcUtils.h"
#include "pmcColors.h"
#include "seiko/slp.h"
#include "mysql.h"

#ifdef  External
#undef  External
#endif

#ifdef  PMC_INIT_GLOBALS
#define External
#else
#define External extern
#endif  /* PMC_INIT_GLOBALS */

enum
{
     PMC_WINID_INVALID
    ,PMC_WINID_MAIN
    ,PMC_WINID_ECHO_LIST
    ,PMC_WINID_ECHO_IMPORT
    ,PMC_WINID_PAT_LIST
    ,PMC_WINID_PAT_EDIT
    ,PMC_WINID_DOCTOR_LIST
    ,PMC_WINID_DOCTOR_EDIT
    ,PMC_WINID_CLAIMS_LIST
    ,PMC_WINID_CLAIMS_EDIT
    ,PMC_WINID_DOCUMENT_IMPORT
    ,PMC_WINID_DOCUMENT_VIEW
    ,PMC_WINID_DOCUMENT_LIST
    ,PMC_WINID_APP_LIST
    ,PMC_WINID_ICD_LIST
    ,PMC_WINID_ECHO_DETAILS
    ,PMC_WINID_MED_LIST
};

#define PMC_CFG_TYPE_STRING      0
#define PMC_CFG_TYPE_INT         1
#define PMC_CFG_TYPE_DIR         2
#define PMC_CFG_TYPE_FILE        3
#define PMC_CFG_TYPE_BOOLEAN     4
#define PMC_CFG_TYPE_HOSTNAME    5   // Host name is a special case
#define PMC_CFG_TYPE_INVALID     6

#define PMC_IGNORE_CASE         0
#define PMC_CHECK_CASE          1

typedef struct pmcConfigStruct_s
{
    Int32u_t    init;
    Char_p      key_p;
    Char_p      str_p;
    Int32u_t    value;
    Int32u_t    required;
    Int32u_t    type;
} pmcConfigStruct_t, *pmcConfigStruct_p;

enum
{
     CFG_MSP_ARCHIVE_DIR = 0		 // Entries must correspond to array below
    ,CFG_MSP_DIR
    ,CFG_MSP_GROUP_NUMBER
    ,CFG_TEMP_DIR
    ,CFG_DOC_IMPORT_FROM_DIR
    ,CFG_DOC_IMPORT_SCANNER_DIR
    ,CFG_DOC_IMPORT_FAILED_DIR
    ,CFG_DOC_IMPORT_TO_DIR_NEW
    ,CFG_DOC_EXTRACT_TO_DIR
    ,CFG_DOC_DELETE_DIR
    ,CFG_PMCDESPL_DIR
    ,CFG_DATABASE_DESC
    ,CFG_DATABASE_MYSQL
    ,CFG_DATABASE_HOST
    ,CFG_DATABASE_PORT
    ,CFG_DATABASE_USERNAME
    ,CFG_DATABASE_PASSWORD
    ,CFG_DATABASE_LOCAL
    ,CFG_LOG_SQL
    ,CFG_LOG_TEX
    ,CFG_LOG_DIR
    ,CFG_REPORT_DIR
    ,CFG_TEMPLATE_DIR
    ,CFG_WORD_TEMPLATE_DIR
    ,CFG_TEX_TEMPLATE_DIR
    ,CFG_WORD_CREATE_DIR
    ,CFG_MERGE_DIR
    ,CFG_PHONE_LIST_DIR
    ,CFG_HOSTNAME
    ,CFG_CAN_SUBMIT_FLAG
    ,CFG_NEW_PAT_FORM_FLAG
    ,CFG_INIT_MODIFY_FLAG
    ,CFG_CLAIMS_DEBUG_FLAG
    ,CFG_SCRAMBLE_NAMES_FLAG
    ,CFG_TEMP_EXTRACT_DOCUMENTS
    ,CFG_ENABLE_DOCUMENT_DELETE
    ,CFG_FEES_FILE
    ,CFG_FEES_FILE_OLD
    ,CFG_CUTOVER_DATE
    ,CFG_ICD_FILE
    ,CFG_EXP_FILE
    ,CFG_AREA_CODE
    ,CFG_SLP_NAME
    ,CFG_SLP_DEBUG
    ,CFG_VIEWER_WORD
    ,CFG_VIEWER_TXT
    ,CFG_VIEWER_PDF
    ,CFG_VIEWER_JPG
    ,CFG_VIEWER_PPT
    ,CFG_WORD
    ,CFG_CD_BURNER
    ,CFG_ECHO_IMPORT_SOURCE
    ,CFG_ECHO_IMPORT_TARGET
    ,CFG_ECHO_MIN_MB_FREE
    ,CFG_ECHO_BACKUP_DISABLE
    ,CFG_ECHO_BACKUP_SKIP
    ,CFG_ECHO_IMPORT_STOP
    ,CFG_ECHO_VIEWER
    ,CFG_ECHO_PATH
    ,CFG_ECHO_RESTRICTED
    ,CFG_DESKTOP
    ,CFG_SKIP_SORT
    ,CFG_ALLOW_DOC_UNLOCK
    ,CFG_INVALID
};

External pmcConfigStruct_t pmcCfg[]
#ifdef PMC_INIT_GLOBALS
=
{
     { FALSE,   "msp_archive_dir:",     NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "msp_dir:",             NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "msp_group_number:",    NIL,    0,      TRUE,   PMC_CFG_TYPE_INT        }
    ,{ FALSE,   "temp_dir:",            NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "import_from_dir:",     NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "scanner_dir:",         NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "import_failed_dir:",   NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "import_to_dir:",       NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "extract_to_dir:",      NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "delete_dir:",          NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "pmcdespl_dir:",        NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "database_desc:",       NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "database_mysql:",      NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "database_host:",       NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "database_port:",       NIL,    0,      FALSE,  PMC_CFG_TYPE_INT        }
    ,{ FALSE,   "database_username:",   NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "database_password:",   NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "database_local:",      NIL,    TRUE,   FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "log_sql:",             NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "log_tex:",             NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "log_dir:",             NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "report_dir:",          NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "template_dir:",        NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "word_template_dir:",   NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "tex_template_dir:",    NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "word_create_dir:",     NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "merge_dir:",           NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "phone_list_dir:",      NIL,    0,      TRUE,   PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "hostname:",            NIL,    0,      TRUE,   PMC_CFG_TYPE_HOSTNAME   }
    ,{ FALSE,   "can_submit:",          NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "new_pat_form:",        NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "init_modify_times:",   NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "claims_debug:",        NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "can_scramble_names:",  NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "can_extract_documents:",NIL,   FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "enable_document_delete:",NIL,  FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "fees_file:",           NIL,    0,      TRUE,   PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "old_fees_file:",       NIL,    0,      TRUE,   PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "cutover_date:",        NIL,    0,      TRUE,   PMC_CFG_TYPE_INT        }
    ,{ FALSE,   "icd_file:",            NIL,    0,      TRUE,   PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "exp_file:",            NIL,    0,      TRUE,   PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "area_code:",           NIL,    0,      TRUE,   PMC_CFG_TYPE_INT        }
    ,{ FALSE,   "slp_name:",            NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "slp_debug",            NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "viewer_word:",         NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "viewer_txt:",          NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "viewer_pdf:",          NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "viewer_jpg:",          NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "viewer_ppt:",          NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "word_exe:",            NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "cd_burner:",           NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "echo_import_source:",  NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "echo_import_target:",  NIL,    0,      FALSE,  PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "echo_min_mb_free:",    NIL,    0,      FALSE,  PMC_CFG_TYPE_INT        }
    ,{ FALSE,   "echo_backup_disable:", NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "echo_backup_skip:",    NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "echo_import_stop:",    NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "echo_viewer:",         NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "echo_path:",           NIL,    0,      FALSE,  PMC_CFG_TYPE_STRING     }
    ,{ FALSE,   "echo_restricted:",     NIL,    0,      FALSE,  PMC_CFG_TYPE_FILE       }
    ,{ FALSE,   "desktop:",             NIL,    0,      FALSE,  PMC_CFG_TYPE_DIR        }
    ,{ FALSE,   "skip_sort:",           NIL,    TRUE,   FALSE,  PMC_CFG_TYPE_BOOLEAN    }
    ,{ FALSE,   "allow_doc_unlock:",    NIL,    FALSE,  FALSE,  PMC_CFG_TYPE_BOOLEAN    }
   ,{ FALSE,   PMC_INVALID_STRING,     NIL,    0,      FALSE,  PMC_CFG_TYPE_INVALID    }
}
#endif
;
//----------------------------------------------------------------------------
// Sort modes
//----------------------------------------------------------------------------

enum
{
     PMC_SORT_NONE
    ,PMC_SORT_DATE_ASCENDING
    ,PMC_SORT_DATE_DESCENDING
    ,PMC_SORT_STUDY_DATE_ASCENDING
    ,PMC_SORT_STUDY_DATE_DESCENDING
    ,PMC_SORT_NAME_ASCENDING
    ,PMC_SORT_NAME_DESCENDING
    ,PMC_SORT_DESC_ASCENDING
    ,PMC_SORT_DESC_DESCENDING
    ,PMC_SORT_PROV_ASCENDING
    ,PMC_SORT_PROV_DESCENDING
    ,PMC_SORT_TYPE_ASCENDING
    ,PMC_SORT_TYPE_DESCENDING
    ,PMC_SORT_CODE_ASCENDING
    ,PMC_SORT_CODE_DESCENDING
    ,PMC_SORT_NUMBER_ASCENDING
    ,PMC_SORT_NUMBER_DESCENDING
    ,PMC_SORT_PHN_ASCENDING
    ,PMC_SORT_PHN_DESCENDING
    ,PMC_SORT_EXP_ASCENDING
    ,PMC_SORT_EXP_DESCENDING
    ,PMC_SORT_ID_ASCENDING
    ,PMC_SORT_ID_DESCENDING
    ,PMC_SORT_COMMENT_ASCENDING
    ,PMC_SORT_COMMENT_DESCENDING
};

//----------------------------------------------------------------------------
// List modes
//----------------------------------------------------------------------------

enum
{
     PMC_LIST_MODE_LIST
    ,PMC_LIST_MODE_SELECT
};

//----------------------------------------------------------------------------
// Edit modes
//----------------------------------------------------------------------------

enum
{
     PMC_EDIT_MODE_VIEW
    ,PMC_EDIT_MODE_EDIT
    ,PMC_EDIT_MODE_NEW
    ,PMC_EDIT_MODE_DELETE
};

#define PMC_LIST_MODE_LIST      (1)
#define PMC_LIST_MODE_SELECT    (2)

#define PMC_IMPORT_FAIL_PHN_DOCUMENT    0x00000001
#define PMC_IMPORT_FAIL_PHN_FILENAME    0x00000002
#define PMC_IMPORT_FAIL_PHN_DIFFER      0x00000004
#define PMC_IMPORT_FAIL_PHN_PATIENT     0x00000008
#define PMC_IMPORT_FAIL_DATE_FILENAME   0x00000010
#define PMC_IMPORT_FAIL_PROVIDER_NONE   0x00000020
#define PMC_IMPORT_FAIL_PROVIDER_MULTI  0x00000040
#define PMC_IMPORT_FAIL_PHN_PATIENT_IN  0x00000080
#define PMC_IMPORT_FAIL_DUPLICATE_DOC   0x00000100
#define PMC_IMPORT_FAIL_DUPLICATE_NAME  0x00000200
#define PMC_IMPORT_FAIL_INVALID         0x00000400

External Char_p pmcImportFailStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
    "PHN not found in document contents."
   ,"PHN not found in document file name."
   ,"PHNs in document contents and file name differ"
   ,"No patient with this PHN found in database."
   ,"Document date could not be determined from the file name."
   ,"No provider could be determined from the document contents."
   ,"A unique provider cound not be determined from the document contents."
   ,"Detected PHN does not match document list patient."
   ,"Document already in database"
   ,"Duplicate file name"
}
#endif
;


// A lock for testing the locking mechanism
External    mbLock_t  pmcTestLock;


enum
{
     PMC_DAY_VIEW_COL_LAST_NAME = 0
    ,PMC_DAY_VIEW_COL_FIRST_NAME
    ,PMC_DAY_VIEW_COL_TITLE
    ,PMC_DAY_VIEW_COL_AREA_CODE
    ,PMC_DAY_VIEW_COL_HOME_PHONE
    ,PMC_DAY_VIEW_COL_DURATION
    ,PMC_DAY_VIEW_COL_APP_TYPE
    ,PMC_DAY_VIEW_COL_REF_DR
    ,PMC_DAY_VIEW_COL_CONF_PHONE
    ,PMC_DAY_VIEW_COL_CONF_LETTER
    ,PMC_DAY_VIEW_COL_COMMENT
};

External Char_p pmcDayViewStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
    "Last Name",
    "First Name",
    "",
    "Area",
    "Phone",
    "Min.",
    "Type",
    "Ref. Dr.",
    "",
    "",
    "Comment"
}
#endif
;

External Char_p pmcPatListStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
    "Last Name",
    "First Name",
    "",
    "Area",
    "Phone",
    "PHN"
}
#endif
;

External Char_p pmcDocListStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
    "Last Name",
    "First Name",
    "Area",
    "Phone",
    "Area",
    "Fax",
    "MSP Number"
}
#endif
;

External Char_p pmcClaimListStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "Last Name"    // 0
    ,"First Name"
    ,"PHN"
    ,"Service Date"
    ,"Claim No."
    ,"Provider"     // 5
    ,"Submit Date"
    ,"Code"
    ,"Units"
    ,"Claimed"
    ,"Paid"         // 10
    ,"Code"
    ,"Exp"
    ,"Reply Date"
    ,"Prem."
    ,"ID"           // 15
    ,"Run Code"
    ,"Sub Count"
    ,"Count"
}
#endif
;

enum
{
     PMC_CLAIM_LIST_COL_LAST_NAME = 0
    ,PMC_CLAIM_LIST_COL_FIRST_NAME
    ,PMC_CLAIM_LIST_COL_PHN
    ,PMC_CLAIM_LIST_COL_DATE
    ,PMC_CLAIM_LIST_COL_CLAIM
    ,PMC_CLAIM_LIST_COL_PROVIDER
    ,PMC_CLAIM_LIST_COL_DATE_SUBMIT
    ,PMC_CLAIM_LIST_COL_FEE_CODE
    ,PMC_CLAIM_LIST_COL_UNITS
    ,PMC_CLAIM_LIST_COL_CLAIMED
    ,PMC_CLAIM_LIST_COL_PAID
    ,PMC_CLAIM_LIST_COL_FEE_CODE_APPROVED
    ,PMC_CLAIM_LIST_COL_EXP_CODE
    ,PMC_CLAIM_LIST_COL_DATE_REPLY
    ,PMC_CLAIM_LIST_COL_PREMIUM
    ,PMC_CLAIM_LIST_COL_ID
    ,PMC_CLAIM_LIST_COL_RUN_CODE
    ,PMC_CLAIM_LIST_COL_SUB_COUNT
    ,PMC_CLAIM_LIST_COL_COUNT
};

External Char_p pmcAppListStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "Date"    // 0
    ,"Year"
    ,"Day"
    ,"Time"
    ,"Min."
    ,"Provider"
    ,"Type"
    ,"Ref. Dr."
    ,"P"
    ,"L"
    ,"Comment"
    ,""
}
#endif
;

enum
{
     PMC_APP_LIST_COL_DATE = 0          //  0
    ,PMC_APP_LIST_COL_YEAR
    ,PMC_APP_LIST_COL_DAY
    ,PMC_APP_LIST_COL_TIME
    ,PMC_APP_LIST_COL_DURATION
    ,PMC_APP_LIST_COL_PROVIDER          // 5
    ,PMC_APP_LIST_COL_TYPE
    ,PMC_APP_LIST_COL_REFERRING
    ,PMC_APP_LIST_COL_CONFIRMED_PHONE
    ,PMC_APP_LIST_COL_CONFIRMED_LETTER
    ,PMC_APP_LIST_COL_COMMENT           // 10
    ,PMC_APP_LIST_COL_COUNT
};

External qHead_t    pmcDocumentQueue;
External qHead_p    pmcDocument_q;

External qHead_t    pmcDocumentDeletedQueue;
External qHead_p    pmcDocumentDeleted_q;

External qHead_t    pmcDBMedQueue;
External qHead_p    pmcDBMed_q;

External qHead_t    pmcDBConfigQueue;
External qHead_p    pmcDBConfig_q;

External qHead_t    pmcAppointCutBufHead;
External qHead_p    pmcAppointCutBuf_q;

External mbLock_t   pmcPatListLock;
External mbLock_t   pmcDocListLock;

External qHead_t    pmcPatNameQueue;
External qHead_p    pmcPatName_q;

External qHead_t    pmcPatDeleteQueue;
External qHead_p    pmcPatDelete_q;

External qHead_t    pmcPatPhoneQueue;
External qHead_p    pmcPatPhone_q;

External qHead_t    pmcPatPhnQueue;
External qHead_p    pmcPatPhn_q;

External qHead_t    pmcDocNameQueue;
External qHead_p    pmcDocName_q;

External qHead_t    pmcDocNumQueue;
External qHead_p    pmcDocNum_q;

External qHead_t    pmcDocPhoneQueue;
External qHead_p    pmcDocPhone_q;

External qHead_t    pmcDocDeleteQueue;
External qHead_p    pmcDocDelete_q;

External qHead_t    pmcFeeQueue;
External qHead_p    pmcFee_q;

External qHead_t    pmcOldFeeQueue;
External qHead_p    pmcOldFee_q;

External qHead_t    pmcIcdQueueHead;
External qHead_p    pmcIcd_q;

External qHead_t    pmcIcdSingleQueueHead;
External qHead_p    pmcIcdSingle_q;

External qHead_t    gLocalExchange;
External qHead_p    gLocalExchange_q;

External qHead_t    gPatHistoryTypeQueue;
External qHead_p    gPatHistoryType_q;

External qHead_t    pmcPickListCfgQueueHead;
External qHead_p    pmcPickListCfg_q;

External qHead_t    pmcExpQueueHead;
External qHead_p    pmcExp_q;

External qHead_t    pmcProviderListHead;
External qHead_p    pmcProvider_q;

External qHead_t    pmcProviderViewListHead;
External qHead_p    pmcProviderView_q;

External qHead_t    pmcCreatedDocumentQueueHead;
External qHead_p    pmcCreatedDocument_q;

External qHead_t    pmcPatientFormQueueHead;
External qHead_p    pmcPatientForm_q;
External mbLock_t   pmcPatientFormListLock;

External Char_p pmcDurationStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "0 "        // 0
    ,"15 min"
    ,"30 min"
    ,"45 min "
    ,"1 hour"
    ,"1:15"      // 5
    ,"1:30"
    ,"1:45"
    ,"2:00"
    ,"2:15"
    ,"2:30"      // 10
    ,"2:45"
    ,"3:00"
    ,"3:15"
    ,"3:30"
    ,"3:45"      // 15
    ,"4:00"
    ,"4:15"
    ,"4:30"
    ,"4:45"
    ,"5:00"      // 20
    ,"5:15"
    ,"5:30"
    ,"5:45"
    ,"6:00"
    ,"6:15"      // 25
    ,"6:30"
    ,"6:45"
    ,"7:00"
    ,"7:15"
    ,"7:30"      // 30
    ,"7:45"
    ,"8:00"
    ,"8:15"
    ,"8:30"
    ,"8:45"      // 35
    ,"9:00"
    ,"9:15"
    ,"9:45"
    ,"10:00"
    ,"10:15"     // 40
    ,"10:30"
    ,"10:45"
}
#endif
;

#define PMC_MCIB_STRING             "MCIB"
#define PMC_CANCER_CLINIC_STRING    "Cancer Clinic"
#define PMC_ALL_OTHERS_STRING       "All Others"

// WARNING - These Dr types are stored in each claim in the database
// and cannot be changed!!

External  pmcPickListStruct_t pmcDrType[]
#ifdef PMC_INIT_GLOBALS
=
{
    { PMC_MCIB_STRING,              0000, 0  },     // MCIB must be index 0
    { PMC_CANCER_CLINIC_STRING,     0002, 1  },
    { "BC",                         9909, 2  },
    { "AB",                         9908, 3  },
    { "MB",                         9907, 4  },
    { "ON",                         9906, 5  },
    { "PQ",                         9905, 6  },
    { "NB",                         9900, 7  },
    { "NS",                         9900, 8  },
    { "PE",                         9900, 9  },
    { "NF",                         9900, 10 },
    { "YT",                         9900, 11 },
    { "NT",                         9900, 12 },
    { "NU",                         9900, 13 },
    { PMC_ALL_OTHERS_STRING,        9900, 14 },
    { "",          PMC_PICKLIST_END_CODE, -1 }
}
#endif
;

#define PMC_PREMIUM_NONE        (0)
#define PMC_PREMIUM_ONE         (1)
#define PMC_PREMIUM_TWO         (2)

External  pmcPickListStruct_t pmcPremiums[]
#ifdef PMC_INIT_GLOBALS
=
{
    { " ",      PMC_PREMIUM_NONE,           0  },
    { "1",      PMC_PREMIUM_ONE,            1  },
    { "2",      PMC_PREMIUM_TWO,            2  },
    { "",       PMC_PICKLIST_END_CODE,     -1  }
}
#endif
;

External Int32s_t pmcDefaultSeviceLocation
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External  pmcPickListStruct_t pmcNewLocations[]
#ifdef PMC_INIT_GLOBALS
=
{
    { "Office",                             0,  0    },
    { "Hospital",                           1,  1    },
    { "Out-Patient",                        2,  2    },
    { "Patient's Home",                     3,  3    },
    { "Other",                              4,  4    },
    { "9",                                  5,  5    },
    { "",               PMC_PICKLIST_END_CODE, -1    }
}
#endif
;

enum
{
     PMC_MSP_CORP_CODE_PRACTITIONER
    ,PMC_MSP_CORP_CODE_1
    ,PMC_MSP_CORP_CODE_2
    ,PMC_MSP_CORP_CODE_3
    ,PMC_MSP_CORP_INVALID
};

External Int8u_t    pmcMspCorpCode[]
#ifdef PMC_INIT_GLOBALS
=
{
     ' '
    ,'A'
    ,'B'
    ,'C'
    ,'X'
}
#endif
;

External Char_p    pmcMspCorpCodeDescription[]
#ifdef PMC_INIT_GLOBALS
=
{
     "Practitioner"
    ,"Corporation 1 (A)"
    ,"Corporation 2 (B)"
    ,"Corporation 3 (C)"
    ,"Invalid Corp. Code"
}
#endif
;

// MAB: 20021116: At some point I should consider a generic file storage
// mechanism.  To start with I have the echo files in a generic table.
enum
{
    PMC_FILE_TYPE_ECHO = 10
};

// New document tpyes MUST be added to the end of the list
enum
{
     PMC_DOCUMENT_TYPE_ANY = 0         // 0
    ,PMC_DOCUMENT_TYPE_WORD
    ,PMC_DOCUMENT_TYPE_PDF
    ,PMC_DOCUMENT_TYPE_TXT
    ,PMC_DOCUMENT_TYPE_JPG
    ,PMC_DOCUMENT_TYPE_DCM             // 5
    ,PMC_DOCUMENT_TYPE_PPT
    ,PMC_DOCUMENT_TYPE_CONSLT
    ,PMC_DOCUMENT_TYPE_PDF_FIXED
    ,PMC_DOCUMENT_TYPE_FOLLOWUP
    ,PMC_DOCUMENT_TYPE_INVALID
};


External Char_p pmcDocumentTypeFilterStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "*.*"
    ,"*.doc"
    ,"*.pdf"
    ,"*.txt"
    ,"*.jpg"
    ,"*.dcm"
    ,"*.ppt"
    ,"*.cslt"
    ,"*.pdf"
    ,"*.cslt"
    ,"invalid"
}
#endif
;

External Char_p pmcDocumentTypeDescStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "Unknown"
    ,"Word"
    ,"PDF"
    ,"Text"
    ,"JPEG"
    ,"DICOM"
    ,"PowerPoint"
    ,"Consult"
    ,"PDF"
    ,"Followup"
    ,"Invalid"
}
#endif
;

External Char_p pmcDocumentTypeExtStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     ".*"
    ,".doc"
    ,".pdf"
    ,".txt"
    ,".jpg"
    ,".dcm"
    ,".ppt"
    ,".clst"
    ,".pdf"
    ,".cslt"
    ,"invalid\\"
}
#endif
;

External    pmcPickListStruct_t pmcDocumentBatchTypes[]
#ifdef PMC_INIT_GLOBALS
=
{
     { "Word (*.doc)"   ,       PMC_DOCUMENT_TYPE_WORD   ,  0 }
    ,{ "PDF (*.pdf)"    ,       PMC_DOCUMENT_TYPE_PDF    ,  1 }
    ,{ "Text (*.txt)"   ,       PMC_DOCUMENT_TYPE_TXT    ,  2 }
    ,{ "JPEG (*.jpg)"   ,       PMC_DOCUMENT_TYPE_JPG    ,  3 }
    ,{ "DICOM (*.dcm)"  ,       PMC_DOCUMENT_TYPE_DCM    ,  4 }
    ,{ "PowerPoint (*.ppt)"  ,  PMC_DOCUMENT_TYPE_PPT    ,  5 }
    ,{ "ANY  (*.*)"     ,       PMC_DOCUMENT_TYPE_ANY    ,  6 }
    ,{ ""               ,       PMC_PICKLIST_END_CODE    , -1 }
}
#endif
;

External    pmcPickListStruct_t pmcDocumentTypes[]
#ifdef PMC_INIT_GLOBALS
=
{
     { "WORD (*.doc)"   ,       PMC_DOCUMENT_TYPE_WORD   ,  0 }
    ,{ "PDF (*.pdf)"    ,       PMC_DOCUMENT_TYPE_PDF    ,  1 }
    ,{ "TEXT (*.txt)"   ,       PMC_DOCUMENT_TYPE_TXT    ,  2 }
    ,{ "JPEG (*.jpg)"   ,       PMC_DOCUMENT_TYPE_JPG    ,  3 }
    ,{ "DICOM (*.dcm)"  ,       PMC_DOCUMENT_TYPE_DCM    ,  4 }
    ,{ "PowerPoint (*.ppt)" ,   PMC_DOCUMENT_TYPE_PPT    ,  5 }
    ,{ "UNKNOWN"        ,       PMC_DOCUMENT_TYPE_ANY    ,  6 }
    ,{ ""               ,       PMC_PICKLIST_END_CODE    , -1 }
}
#endif
;

typedef struct pmcLocationCodeStruct_s
{
    Int32s_t    locationIndex;
    Int32s_t    premiumIndex;
    Int32s_t    code;
    Int32s_t    valid;
} pmcLocationCodeStruct_t, *pmcLocationCodeStruct_p;


External  pmcLocationCodeStruct_t pmcLocationCode[]
#ifdef PMC_INIT_GLOBALS
=
{
    { 0, 0,  0x31, TRUE},  // Office -
    { 1, 0,  0x32, TRUE},  // Hospital
    { 2, 0,  0x33, TRUE},  // Out Patient
    { 3, 0,  0x34, TRUE},  // Patient's Home
    { 4, 0,  0x35, TRUE},  // Other
    { 5, 0,  0x39, TRUE},  // 9 ??
    { 1, 1,  0x42, TRUE},  // Hospital    Premium 1
    { 2, 1,  0x43, TRUE},  // Out Patient Premium 1
    { 3, 1,  0x44, TRUE},  // Home        Premium 1
    { 4, 1,  0x45, TRUE},  // Other       Premium 1
    { 1, 2,  0x4B, TRUE},  // Hospital    Premium 2
    { 2, 2,  0x4D, TRUE},  // Out Patient Premium 2
    { 3, 2,  0x50, TRUE},  // Home        Premium 2
    { 4, 2,  0x54, TRUE},  // Other       Premium 2
    { 0, 1,     1, FALSE}, // Invalid
    { 5, 1,     2, FALSE}, // Invalid
    { 0, 2,     3, FALSE}, // Invalid
    { 5, 2,     4, FALSE}, // Invalid
    { 0, 0,  PMC_PICKLIST_END_CODE }
}
#endif
;

enum
{
     PMC_APP_COMPLETED_STATE_NONE = 0
    ,PMC_APP_COMPLETED_STATE_COMPLETED
    ,PMC_APP_COMPLETED_STATE_ARRIVED
    ,PMC_APP_COMPLETED_STATE_CANCELLED
};

enum
{
     PMC_CLAIM_STATUS_NONE = 0           // 0
    ,PMC_CLAIM_STATUS_NOT_READY          // 1
    ,PMC_CLAIM_STATUS_READY              // 2
    ,PMC_CLAIM_STATUS_SUBMITTED          // 3
    ,PMC_CLAIM_STATUS_PAID               // 4
    ,PMC_CLAIM_STATUS_REJECTED           // 5
    ,PMC_CLAIM_STATUS_REDUCED            // 6
    ,PMC_CLAIM_STATUS_REJECTED_ACCEPT    // 7
    ,PMC_CLAIM_STATUS_REDUCED_ACCEPT     // 8
    ,PMC_CLAIM_STATUS_EDIT               // 9
    ,PMC_CLAIM_STATUS_INVALID            // 10
};

External Char_p pmcClaimStatusStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     ""
    ,"Not Ready"
    ,"Ready"
    ,"Submitted"
    ,"Paid"
    ,"Rejected"
    ,"Reduced"
    ,"Rejected"
    ,"Reduced"
    ,"Edit"
    ,"Invalid"
}
#endif
;

External Int32s_t pmcClaimStatusColors[]
#ifdef PMC_INIT_GLOBALS
=
{
     (Int32s_t)clBtnFace
    ,(Int32s_t)clRed            // Not Ready
    ,(Int32s_t)0x00FFE4B7       // Ready
    ,(Int32s_t)clYellow         // Submitted
    ,(Int32s_t)0x00A3FAAB       // Paid
    ,(Int32s_t)clRed            // Rejected
    ,(Int32s_t)0x000080FF       // Reduced (Orange)
    ,(Int32s_t)clGreen          // Rejected - Accept
    ,(Int32s_t)clGreen          // Reduced - Accept
    ,(Int32s_t)0x00FFE4B7       // Edit
    ,(Int32s_t)clRed            // Invalid
}
#endif
;



External Int32u_t   pmcTimeSlotInts[]
#ifdef PMC_INIT_GLOBALS
=
{
     00000,
     80000,
     81500,
     83000,
     84500,
     90000,
     91500,
     93000,
     94500,
    100000,
    101500,
    103000,
    104500,
    110000,
    111500,
    113000,
    114500,
    120000,
    121500,
    123000,
    124500,
    130000,
    131500,
    133000,
    134500,
    140000,
    141500,
    143000,
    144500,
    150000,
    151500,
    153000,
    154500,
    160000,
    161500,
    163000,
    164500,
    170000,
    171500,
    173000,
    174500
}
#endif
;

External Char_p pmcTimeSlotString[]
#ifdef PMC_INIT_GLOBALS
=
{
    " Invalid",
    "    8:00",
    "    8:15",
    "    8:30",
    "    8:45",
    "    9:00",
    "    9:15",
    "    9:30",
    "    9:45",
    "   10:00",
    "   10:15",
    "   10:30",
    "   10:45",
    "   11:00",
    "   11:15",
    "   11:30",
    "   11:45",
    "   12:00",
    "   12:15",
    "   12:30",
    "   12:45",
    "    1:00",
    "    1:15",
    "    1:30",
    "    1:45",
    "    2:00",
    "    2:15",
    "    2:30",
    "    2:45",
    "    3:00",
    "    3:15",
    "    3:30",
    "    3:45",
    "    4:00",
    "    4:15",
    "    4:30",
    "    4:45",
    "    5:00",
    "    5:15",
    "    5:30",
    "    5:45"
}
#endif
;

typedef struct pmcLinkageStruct_s
{
    qLinkage_t  linkage;
    Void_p      record_p;
} pmcLinkageStruct_t, *pmcLinkageStruct_p;

#define PMC_AREA_CODE_LEN   3
#define PMC_TITLE_LEN       15
#define PMC_GENDER_LEN      3

typedef struct pmcPatRecordStruct_s
{
    pmcLinkageStruct_t nameLinkage;
    pmcLinkageStruct_t phoneLinkage;
    pmcLinkageStruct_t phnLinkage;
    pmcLinkageStruct_t deleteLinkage;
    Char_p      firstName_p;
    Int32u_t    firstNameLen;
    Char_p      lastName_p;
    Int32u_t    lastNameLen;
    Char_p      lastNameSearch_p;
    Int32u_t    lastNameSearchLen;
    Char_p      homePhone_p;
    Int32u_t    homePhoneLen;
    Char_p      homePhoneSearch_p;
    Int32u_t    homePhoneSearchLen;
    Char_p      phn_p;
    Int32u_t    phnLen;
    Char_p      phnSearch_p;
    Int32u_t    phnSearchLen;
    Char_t      areaCode[PMC_AREA_CODE_LEN+1];
    Char_t      gender[PMC_GENDER_LEN+1];
    Char_t      title[PMC_TITLE_LEN+1];
    Int64u_t    homePhoneInt64;
    Int64u_t    phnInt64;
    Int32u_t    id;
    Int32u_t    offset;
    Int32u_t    notDeleted;
    Boolean_t   displayAreaCode;
} pmcPatRecordStruct_t, *pmcPatRecordStruct_p;

typedef struct pmcDocRecordStruct_s
{
    pmcLinkageStruct_t nameLinkage;
    pmcLinkageStruct_t phoneLinkage;
    pmcLinkageStruct_t numLinkage;
    pmcLinkageStruct_t deleteLinkage;
    Char_p      firstName_p;
    Int32u_t    firstNameLen;
    Char_p      lastName_p;
    Int32u_t    lastNameLen;
    Char_p      lastNameSearch_p;
    Int32u_t    lastNameSearchLen;
    Char_p      workPhone_p;
    Int32u_t    workPhoneLen;
    Char_p      workPhoneSearch_p;
    Int32u_t    workPhoneSearchLen;
    Char_p      workFax_p;
    Int32u_t    workFaxLen;
    Char_p      num_p;
    Int32u_t    numLen;
    Char_t      workAreaCode[PMC_AREA_CODE_LEN+1];
    Char_t      faxAreaCode[PMC_AREA_CODE_LEN+1];
    Int64u_t    workPhoneInt64;
    Int32u_t    numInt32;
    Int32u_t    id;
    Int32u_t    offset;
    Int32u_t    notDeleted;
#if PMC_DEBUG
    Int32u_t    magicNumber;
#endif
    Boolean_t   displayWorkAreaCode;
    Boolean_t   displayFaxAreaCode;
} pmcDocRecordStruct_t, *pmcDocRecordStruct_p;

#define PMC_DOC_RECORD_MAGIC_NUMBER 0xF420CB23

#define PMC_INVALID_DOCTOR_NUMBER   (99999999)
#define PMC_INVALID_PHN             (999999999999i64)
#define PMC_INVALID_PHONE           (9999999999i64)

#define PMC_VALID_PHN_LEN           (9)

#define PMC_REPEAT_INVALID          (0)
#define PMC_REPEAT_DAILY            (1)
#define PMC_REPEAT_WEEKLY           (2)
#define PMC_REPEAT_MONTHLY          (3)
#define PMC_REPEAT_ONCE             (4)

#define PMC_DAY_OF_WEEK_MON         (1)
#define PMC_DAY_OF_WEEK_TUE         (2)
#define PMC_DAY_OF_WEEK_WED         (3)
#define PMC_DAY_OF_WEEK_THU         (4)
#define PMC_DAY_OF_WEEK_FRI         (5)
#define PMC_DAY_OF_WEEK_SAT         (6)
#define PMC_DAY_OF_WEEK_SUN         (7)

#if PMC_AVAIL_TIME
typedef struct pmcAvailTimeStruct_s
{
    qLinkage_t      linkage;
    Int32u_t        providerId;
    MDateTime       startTime;
    MDateTime       stopTime;
    Int16u_t        dayOfWeek;
    Int16u_t        repeatCode;
} pmcAvailTimeStruct_t, *pmcAvailTimeStruct_p;

External qHead_t pmcAvailTimeListHead;
External qHead_p pmcAvailTime_q;

#endif // PMC_AVAIL_TIME

typedef struct pmcAppointCellStruct_s
{
    AnsiString      str;
    Int32s_t        color;
    Int16u_t        last;
    Int16u_t        first;
} pmcAppointCellStruct_t, *pmcAppointCellStruct_p;

#define PMC_MAX_APPS_PER_SLOT           8
#define PMC_TIMESLOTS_PER_DAY           41
#define PMC_TIMESLOT_DURATION           15

#define PMC_DAY_VIEW_ARRAY_COLS         11

#define PMC_DAY_VIEW_COLS               1
#define PMC_WEEK_VIEW_COLS              7
#define PMC_MONTH_VIEW_COLS             31
#define PMC_PROVIDER_VIEW_COLS          8

External pmcAppointCellStruct_t DayViewCellArray      [ PMC_DAY_VIEW_ARRAY_COLS ][ PMC_TIMESLOTS_PER_DAY  ];
External pmcAppointCellStruct_t WeekViewCellArray     [ PMC_WEEK_VIEW_COLS      ][ PMC_TIMESLOTS_PER_DAY  ];
External pmcAppointCellStruct_t MonthViewCellArray    [ PMC_MONTH_VIEW_COLS     ][ PMC_TIMESLOTS_PER_DAY  ];
External pmcAppointCellStruct_t ProviderViewCellArray [ PMC_PROVIDER_VIEW_COLS  ][ PMC_TIMESLOTS_PER_DAY  ];

#define PMC_DAY_VIEW_LAST_NAME_LEN      63
#define PMC_DAY_VIEW_FIRST_NAME_LEN     39
#define PMC_DAY_VIEW_TITLE_LEN          15
#define PMC_DAY_VIEW_DUR_LEN            7
#define PMC_DAY_VIEW_HOME_PHONE_LEN     23
#define PMC_DAY_VIEW_AREA_CODE_LEN      7
#define PMC_DAY_VIEW_COMMENT_IN_LEN     127
#define PMC_DAY_VIEW_TYPE_FIELD_LEN     15
#define PMC_DAY_VIEW_REF_DR_LEN         39
#define PMC_DAY_VIEW_CONF_LETTER_LEN    3
#define PMC_DAY_VIEW_CONF_PHONE_LEN     3

typedef struct pmcTimeSlotInfo
{
    Int16u_t            countt;
    Int16u_t            available;
    Int32u_t            date;
    Int32u_t            providerId;
    Int32u_t            appointId[PMC_MAX_APPS_PER_SLOT+1];
    Int32u_t            patientId[PMC_MAX_APPS_PER_SLOT+1];
    Int32u_t            startTime;
    Int32u_t            color;
    Int16u_t            last;
    Int16u_t            first;
    Int16u_t            confirmedPhoneState;
    Int16u_t            confirmedLetterState;
    Char_t              refDrName[PMC_DAY_VIEW_REF_DR_LEN+1];
    Char_t              dur[PMC_DAY_VIEW_DUR_LEN+1];
    Char_t              firstName[PMC_DAY_VIEW_FIRST_NAME_LEN+1];
    Char_t              lastName[PMC_DAY_VIEW_LAST_NAME_LEN+1];
    Char_t              title[PMC_DAY_VIEW_TITLE_LEN+1];
    Char_t              homePhone[PMC_DAY_VIEW_HOME_PHONE_LEN+1];
    Char_t              areaCode[PMC_DAY_VIEW_AREA_CODE_LEN+1];
    Char_t              commentIn[PMC_DAY_VIEW_COMMENT_IN_LEN+1];
    Char_t              type[PMC_DAY_VIEW_TYPE_FIELD_LEN+1];
} pmcTimeSlotInfo_t, *pmcTimeSlotInfo_p;

enum
{
     PMC_APP_VIEW_TYPE_DAY
    ,PMC_APP_VIEW_TYPE_WEEK
    ,PMC_APP_VIEW_TYPE_MONTH
    ,PMC_APP_VIEW_TYPE_PROVIDER
};

typedef struct pmcAppViewInfo_s
{
    Int32u_t            providerId;
    Int32u_t            endDragProviderId;
    Int32u_t            dayOfWeek;
    Int32u_t            startDate;
    Int32s_t            cols;
    Int32u_t            type;
    Boolean_t           updateForce;
    Boolean_t           updateSkip;
    Int32u_t            notReady;
    TDrawGrid          *grid_p;
    pmcTimeSlotInfo_p   slot_p;
} pmcAppViewInfo_t, *pmcAppViewInfo_p;

typedef struct pmcAppInfo_s
{
    qLinkage_t          linkage;
    Int32u_t            id;
    Int32u_t            patientId;
    Int32u_t            providerId;
    Int32u_t            date;
    Int32u_t            startTime;
    Int32u_t            stopTime;
    Int32u_t            duration;
    Int32u_t            completed;
    Int32u_t            type;
    Int32u_t            conflictCount;
    Int32u_t            appCount;
    Int32u_t            unavail;
    Int32u_t            confirmedPhoneDate;
    Int32u_t            confirmedPhoneTime;
    Int32u_t            confirmedLetterDate;
    Int32u_t            confirmedLetterTime;
    Int32u_t            confirmedPhoneId;
    Int32u_t            confirmedLetterId;
    Int16u_t            confirmedPhoneState;
    Int16u_t            confirmedLetterState;
    Int32u_t            refDrId;
    Char_t              refDrName[PMC_DAY_VIEW_REF_DR_LEN+1];
    Char_t              firstName[PMC_DAY_VIEW_FIRST_NAME_LEN+1];
    Char_t              lastName[PMC_DAY_VIEW_LAST_NAME_LEN+1];
    Char_t              title[PMC_DAY_VIEW_TITLE_LEN+1];
    Char_t              homePhone[PMC_DAY_VIEW_HOME_PHONE_LEN+1];
    Char_t              areaCode[PMC_DAY_VIEW_AREA_CODE_LEN+1];
    Char_t              commentIn[PMC_DAY_VIEW_COMMENT_IN_LEN+1];
} pmcAppInfo_t, *pmcAppInfo_p;

External Int32s_t   pmcColor[ PMC_COLOR_COUNT ];

External    Char_p  gPmcEmptyString_p
#ifdef PMC_INIT_GLOBALS
= ""
#endif
;

External Boolean_t pmcInitDone
#ifdef PMC_INIT_GLOBALS
= FALSE
#endif
;

External Int32u_t   pmcDefaultServiceDate[]
#ifdef PMC_INIT_GLOBALS
= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#endif
;


External Char_p     pmcDefaultClaimCode_p[]
#ifdef PMC_INIT_GLOBALS
= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#endif
;

External Char_p     pmcDefaultIcdCode_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   pmcTimerTicks
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   pmcStoredPatientId
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   pmcTimerSkips
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   pmcFileCounter
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   pmcIcdContainsOhs
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   pmcIcdCaseSensitive
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Int32u_t    pmcFeeContainsOhs
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t    pmcFeeCaseSensitive
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Char_p     pmcClaimsin_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p     pmcInfo_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p     pmcValidrpt_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p     pmcReturns_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p     pmcNotice_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p     pmcBuf1_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p     pmcBuf2_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t   pmcEchoSortMode;
External Int32u_t   pmcEchoSortReadDateMode;
External Int32u_t   pmcEchoSortStudyNameMode;
External Int32u_t   pmcEchoSortPatNameMode;
External Int32u_t   pmcEchoSortStudyDateMode;
External Int32u_t   pmcEchoSortIddMode;
External Int32u_t   pmcEchoSortCommentMode;

External Int32u_t   pmcEchoFilterBackedUpChecked;
External Int32u_t   pmcEchoFilterNotBackedUpChecked;
External Int32u_t   pmcEchoFilterOnlineChecked;
External Int32u_t   pmcEchoFilterOfflineChecked;
External Int32u_t   pmcEchoFilterReadChecked;
External Int32u_t   pmcEchoFilterNotReadChecked;

External Int32u_t   pmcEchoReadBackItemIndex;

External Int64u_t   pmcDayViewAppLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t   pmcWeekViewAppLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t   pmcMonthViewAppLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t    pmcDayViewSlotLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t pmcWeekViewSlotLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t pmcMonthViewSlotLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t pmcProviderViewAppLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t pmcProviderViewSlotLastReadTime
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int64u_t pmcEchoMinBytesFree
#ifdef PMC_INIT_GLOBALS
= 0i64
#endif
;

External Int32u_t       pmcThreadTicks
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t       pmcThreadSkips
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

#if OBSOLETE

External Int32u_t       pmcMySQLKeepAliveQueries
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

#endif

External Int32u_t       pmcLastSQLTime
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t       pmcStartTime
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t       gSuspendPoll
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t       pmcInPoll
#ifdef PMC_INIT_GLOBALS
= FALSE
#endif
;

External Int32u_t       pmcSqlLockValue
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Boolean_t      pmcMainFormCloseRan
#ifdef PMC_INIT_GLOBALS
= FALSE
#endif
;

External Boolean_t      pmcClaimListReducedAcceptCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcClaimListRejectedAcceptCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcClaimListPaidCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcClaimListReadyCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcClaimListNotReadyCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcClaimListSubmittedCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcClaimListReducedCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcClaimListRejectedCheck
#ifdef PMC_INIT_GLOBALS
= TRUE
#endif
;

External Boolean_t      pmcNewPatForm
#ifdef PMC_INIT_GLOBALS
= FALSE
#endif
;

External Boolean_t      pmcSplashFlag
#ifdef PMC_INIT_GLOBALS
= FALSE
#endif
;

External Int32u_t       pmcSlashCount
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t       pmcWinVersion
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Int32u_t       pmcSystemId
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p         gPmcRegUsername_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_p         gPmcRegHostname_p
#ifdef PMC_INIT_GLOBALS
= 0
#endif
;

External Char_t         pmcHomeDir[256];

External Char_t         pmcUserName[64]
#ifdef PMC_INIT_GLOBALS
= "Unknown"
#endif
;

External TRect   *pmcButtonRect_p;
External TCanvas *pmcButtonCanvas_p;
External Char_p   pmcButtonString_p;

#define PMC_COUNTER_READ                0
#define PMC_COUNTER_UPDATE              1

#define PMC_PROVIDER_COUNT_PATIENTS     0
#define PMC_PROVIDER_COUNT_APPS         1
#define PMC_PROVIDER_COUNT_DOCUMENTS    2

typedef struct pmcProviderList_s
{
    qLinkage_t  linkage;
    Char_p      description_p;
    Char_p      docSearchString_p;
    Char_p      lastName_p;
    Int32u_t    id;
    Int32u_t    providerNumber;
    Int32u_t    lastNameLen;
    Int32s_t    patientCount;
    Int32s_t    appointmentCount;
    Int32s_t    documentCount;
    Int32s_t    picklistOrder;
    Ints_t      listIndex;

    // 20040110: Add queue of run codes used when processing RETURNS files
    qHead_t     runCodeQueue;
    qHead_p     runCode_q;
} pmcProviderList_t, *pmcProviderList_p;


typedef struct pmcMemList_s
{
    qLinkage_t  linkage;
    Void_p      ptr;
    Ints_t      size;
    Int32u_t    line;
    Char_p      file_p;
} pmcMemList_t, *pmcMemList_p;

enum
{
     PMC_APP_LETTER_RESULT_FAILURE
    ,PMC_APP_LETTER_RESULT_SUCCESS
    ,PMC_APP_LETTER_RESULT_DECEASED
    ,PMC_APP_LETTER_RESULT_NO_PATIENT
    ,PMC_APP_LETTER_RESULT_NO_FIRST_NAME
    ,PMC_APP_LETTER_RESULT_NO_LAST_NAME
    ,PMC_APP_LETTER_RESULT_NO_TITLE
    ,PMC_APP_LETTER_RESULT_NO_ADDRESS
    ,PMC_APP_LETTER_RESULT_NO_CITY
    ,PMC_APP_LETTER_RESULT_NO_PROVINCE
    ,PMC_APP_LETTER_RESULT_NO_POSTAL_CODE
    ,PMC_APP_LETTER_RESULT_NO_TEMPLATE
    ,PMC_APP_LETTER_RESULT_OUTPUT_FILE_FAILED
    ,PMC_APP_LETTER_RESULT_NO_PROVIDER_DESC
    ,PMC_APP_LETTER_RESULT_NO_TEST_DESC
    ,PMC_APP_LETTER_RESULT_NO_DATE
    ,PMC_APP_LETTER_RESULT_NO_TIME
    ,PMC_APP_LETTER_RESULT_NO_REFERRING
    ,PMC_APP_LETTER_RESULT_NO_DATE_PRINT
    ,PMC_APP_LETTER_RESULT_DB_FAILED
};

External Char_p pmcPatAppLetterResultStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "Failure"
    ,"Success"
    ,"Patient deceased"
    ,"No patient"
    ,"No first name"
    ,"No last name"
    ,"No title"
    ,"No address"
    ,"No city"
    ,"No province"
    ,"No postal code"
    ,"No template file"
    ,"Failed to open output file"
    ,"No provider description"
    ,"No test description"
    ,"No appointment date"
    ,"No appointment time"
    ,"No referring Dr."
    ,"No print date"
    ,"Failed to read database"
}
#endif
;

enum
{
     PMC_SUB_STR_TITLE = 0          // 0
    ,PMC_SUB_STR_FIRST_NAME
    ,PMC_SUB_STR_LAST_NAME
    ,PMC_SUB_STR_ADDRESS1
    ,PMC_SUB_STR_ADDRESS2
    ,PMC_SUB_STR_CITY               // 5
    ,PMC_SUB_STR_PROVINCE
    ,PMC_SUB_STR_POSTAL_CODE
    ,PMC_SUB_STR_PROVIDER_DESC
    ,PMC_SUB_STR_PROVIDER_TEST_DESC
    ,PMC_SUB_STR_APP_DATE           // 10
    ,PMC_SUB_STR_APP_TIME
    ,PMC_SUB_STR_PRINT_DATE
    ,PMC_SUB_STR_REF_DR             // 13
    ,PMC_SUB_STR_COUNT
};

External Char_p pmcSubStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     "[__TITLE__]"                  // 0
    ,"[__FIRST_NAME__]"
    ,"[__LAST_NAME__]"
    ,"[__ADDRESS1__]"
    ,"[__ADDRESS2__]"
    ,"[__CITY__]"                   // 5
    ,"[__PROVINCE__]"
    ,"[__POSTAL_CODE__]"
    ,"[__PROVIDER_DESC__]"
    ,"[__PROVIDER_TEST_DESC__]"
    ,"[__APP_DATE__]"               // 10
    ,"[__APP_TIME__]"
    ,"[__PRINT_DATE__]"
    ,"[__REFERRING_DR__]"           // 13
}
#endif PMC_INIT_GLOBALS
;

External Int32u_t pmcSubStringErrCode[]
#ifdef PMC_INIT_GLOBALS
=
{
     PMC_APP_LETTER_RESULT_NO_TITLE         // 0
    ,PMC_APP_LETTER_RESULT_NO_FIRST_NAME
    ,PMC_APP_LETTER_RESULT_NO_LAST_NAME
    ,PMC_APP_LETTER_RESULT_NO_ADDRESS
    ,PMC_APP_LETTER_RESULT_NO_ADDRESS
    ,PMC_APP_LETTER_RESULT_NO_CITY          // 5
    ,PMC_APP_LETTER_RESULT_NO_PROVINCE
    ,PMC_APP_LETTER_RESULT_NO_POSTAL_CODE
    ,PMC_APP_LETTER_RESULT_NO_PROVIDER_DESC
    ,PMC_APP_LETTER_RESULT_NO_TEST_DESC
    ,PMC_APP_LETTER_RESULT_NO_DATE          // 10
    ,PMC_APP_LETTER_RESULT_NO_TIME
    ,PMC_APP_LETTER_RESULT_NO_DATE_PRINT
    ,PMC_APP_LETTER_RESULT_NO_REFERRING     // 13
}
#endif PMC_INIT_GLOBALS
;

enum
{
     PMC_APP_TYPE_NONE  =  0
    ,PMC_APP_TYPE_NEW
    ,PMC_APP_TYPE_REVIEW
    ,PMC_APP_TYPE_URGENT
    ,PMC_APP_TYPE_CLINIC
    ,PMC_APP_TYPE_HOSPITAL
    ,PMC_APP_TYPE_INVALID
};

External Char_p pmcAppTypeStrings[]
#ifdef PMC_INIT_GLOBALS
=
{
     ""
    ,"New"
    ,"Review"
    ,"Urgent"
    ,"Clinic"
    ,"Hospital"
}
#endif PMC_INIT_GLOBALS
;

#endif // H_pmcGlobals


