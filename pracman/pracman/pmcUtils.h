//---------------------------------------------------------------------------
// Function:    pmcUtils.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 29 2002
//---------------------------------------------------------------------------
// Description:
//
// Practice Manager utility functions
//---------------------------------------------------------------------------

#ifndef pmcUtils_h
#define pmcUtils_h

#include <stdlib.h>

#include "mbUtils.h"
#include "mbSqlLib.h"
#include "pmcUtilsSql.h"

#ifdef  External
#undef  External
#endif

// It looks like the first run codes we recevied were in the T series.  Also,
// it looks like the run codes are about to wrap soon (2004).  So by internally
// wrapping the runc codes at TA, we will run for years before wrapping when
// internally computing IDs from the run codes.

#define PMC_MSP_RUN_CODE_SHIFT      (494)   // TA
#define PMC_MSP_RUN_CODE_WRAP       (676)   // ZZ + 1

// We do not have claims in the database up to this point
#define PMC_MSP_RUN_CODE_IGNORE_BEFORE  "WI"

// Types for items in the DB config table
enum
{
     PMC_DB_CONFIG_IMAGE_QUALITY    = 1     // 1
    ,PMC_DB_CONFIG_REGURGITATION
    ,PMC_DB_CONFIG_STENOSIS
    ,PMC_DB_CONFIG_RHYTHM
    ,PMC_DB_CONFIG_CITY                     // 5
    ,PMC_DB_CONFIG_LOCALX
    ,PMC_DB_CONFIG_CONSULT_SECTIONS
    ,PMC_DB_CONFIG_PROVINCE
    ,PMC_DB_CONFIG_TITLE
};

// Echo states
enum
{
     PMC_ECHO_STATUS_NEW = 0                // 0
    ,PMC_ECHO_STATUS_IN_PROGRESS
    ,PMC_ECHO_STATUS_PENDING
    ,PMC_ECHO_STATUS_NO_ECHO
    ,PMC_ECHO_STATUS_MAX                    // 4
};

// Index for Color Square Image List on main form.
// Note: The images in that list must match the
// indexes below

enum
{
     PMC_COLORSQUARE_INDEX_WHITE = 0
    ,PMC_COLORSQUARE_INDEX_BLUE
    ,PMC_COLORSQUARE_INDEX_GREEN
    ,PMC_COLORSQUARE_INDEX_RED
    ,PMC_COLORSQUARE_INDEX_YELLOW
    ,PMC_COLORSQUARE_INDEX_NAVY
    ,PMC_COLORSQUARE_INDEX_MAX
};

#define PMC_PICKLIST_CFG_ICD            0
#define PMC_PICKLIST_CFG_MSP_SERVICE    1
#define PMC_PICKLIST_CFG_MSP_HOSPITAL   2

enum
{
     PMC_EXTRACT_PHN_STATE_WAIT_1 = 0
    ,PMC_EXTRACT_PHN_STATE_WAIT_2
    ,PMC_EXTRACT_PHN_STATE_WAIT_3
    ,PMC_EXTRACT_PHN_STATE_WAIT_4
    ,PMC_EXTRACT_PHN_STATE_WAIT_COLON
    ,PMC_EXTRACT_PHN_STATE_WAIT_DIGIT
    ,PMC_EXTRACT_PHN_STATE_IN_DIGITS
    ,PMC_EXTRACT_PHN_STATE_WAIT_DONE
};

enum
{
     PMC_FORMAT_NAME_STYLE_FMLT = 0
    ,PMC_FORMAT_NAME_STYLE_TFML
};

#define PMC_PICKLIST_END_CODE       999999

typedef struct pmcPickListStruct_s
{
    Char_p      description_p;
    Int32s_t    code;
    Int32s_t    index;
} pmcPickListStruct_t, *pmcPickListStruct_p;

void            pmcMakeMailingAddress
(
    Char_p      title_p,
    Char_p      firstName_p,
    Char_p      lastName_p,
    Char_p      address1_p,
    Char_p      address2_p,
    Char_p      city_p,
    Char_p      province_p,
    Char_p      postalCode_p,
    Char_p      nameOut_p,
    Char_p      address1Out_p,
    Char_p      address2Out_p,
    Char_p      cityProvCodeOut_p,
    bool        allCapsFlag
);

typedef struct      pmcExpStruct_s
{
    qLinkage_t      linkage;
    Char_t          code[4];
    Char_p          string_p;
} pmcExpStruct_t, *pmcExpStruct_p;

// A generic linked list for building dynamic pick lists from
// the configuration file.

typedef struct      pmcPickListCfg_s
{
    qLinkage_t      linkage;
    Int32s_t        type;
    Char_p          string_p;
} pmcPickListCfg_t, *pmcPickListCfg_p;

typedef struct      pmcTokenStruct_s
{
    qLinkage_t      linkage;
    Char_p          string_p;
    bool            returned;
} pmcTokenStruct_t, *pmcTokenStruct_p;

//---------------------------------------------------------------------------
// Simple phone class.
//---------------------------------------------------------------------------

class MbPhone
{
public:

    __fastcall  MbPhone( Char_p str_p );
    __fastcall ~MbPhone( void );

    void __fastcall set( Char_p str_p );
    Char_p __fastcall digitsGet( void ) { return this->digits; }
    Char_p __fastcall formattedGet( void ) { return this->formatted; }

private:

    Char_t  raw[36];
    Char_t  formatted[36];
    Char_t  digits[36];
};

//---------------------------------------------------------------------------
// Simple Queue class
//---------------------------------------------------------------------------
class MbQueue
{
public:
    __fastcall  MbQueue( void );
    __fastcall ~MbQueue( void );

private:
    qHead_t     queueHead;
    qHead_p     queue_p;
};

//---------------------------------------------------------------------------
// Simple cursor object that sets the cursor, stores the original cursor,
// and restores the origianl cursor when it goes out of scope.
//---------------------------------------------------------------------------
class MbCursor
{
public:
    // Constructors
    __fastcall MbCursor( TCursor cursor )
    {
        this->origCursor = Screen->Cursor;
        Screen->Cursor = cursor;
    }

    // Destructor
    __fastcall ~MbCursor( void )
    {
        Screen->Cursor =  this->origCursor;
    }
private:
    TCursor origCursor;
};

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

void        pmcPatIdStore( Int32u_t id );
Int32u_t    pmcPatIdRetrieve( Boolean_t clearFlag );

// Simple registry
Int32s_t    pmcRegInit( Char_p hostname_p, Char_p username_p );
Int32s_t    pmcRegClose( void );
Int32s_t    pmcRegIntPut( Char_p key_p, Int32s_t value );
Int32s_t    pmcRegIntGet( Char_p key_p, Int32s_p value_p );

void            pmcPickListBuildNew
(
    TComboBox              *list_p,
    Int32u_t                type
);

Int32u_t        pmcDateSelect
(
    Int32u_t    mode,
    Char_p      string_p,
    Int32u_t    startDate
);

Char_p          mbStrTex
(
    Char_p      in_p,
    Char_p      out_p,
    Int32u_t    maxLength
);

Int32s_t        pmcTemplateSub
(
    Char_p      inName_p,
    Char_p      outName_p,
    qHead_p     sub_q
);

Char_p pmcFloatToStr( Float_t value, Int32u_t decimals );

Char_p          pmcIntToRunCode
(
    Int32u_t    value,
    Char_p      runCode_p
);

Int32u_t        pmcRunCodeToInt
(
    Char_p      runCode_p
);

Char_p          pmcFormatName
(
    Char_p      titleIn_p,
    Char_p      firstIn_p,
    Char_p      middleIn_p,
    Char_p      lastIn_p,
    Char_p      result_p,
    Int32u_t    style
);

void            pmcPhoneFormat
(
    Char_p      in_p,
    Char_p      areaOut_p,
    Char_p      phoneOut_p,
    bool       *isLocal_p
);

Char_p          pmcPhoneFormatString
(
    Char_p      in_p,
    Char_p      out_p
);

Ints_t          pmcPhoneFix
(
    Char_p      in_p,
    bool       *modifiedFlag_p
);

Char_p          pmcMakeFileName
(
    Char_p      path_p,
    Char_p      in_p
);


Char_p          pmcStringStripDoubleQuotes
(
    Char_p      in_p
);

Ints_t          pmcProvinceFix
(
    Char_p      in_p,
    bool       *modifiedFlag_p
);

Char_p          pmcFormatPhnSearch
(
    Char_p      phn_p,
    Char_p      result_p
);

Char_p          pmcFormatPhnDisplay
(
    Char_p      phn_p,
    Char_p      prov_p,
    Char_p      result_p
);

Int32u_t        pmcEchoPatSelect
(
    Char_p      echoName_p,
    Int32u_t    echoDate,
    Int32u_t    providerId,
    Int32u_p    id_p,
    Boolean_t   cancelButton
);

__int64         pmcAtoI64
(
    Char_p      string_p
);

Ints_t          pmcPopupItemIndex
(
    TPopupMenu *menu_p,
    Char_p      str_p
);

void            pmcPopupItemEnable
(
    TPopupMenu *menu_p,
    Char_p      str_p,
    bool        enable
);

void            pmcPopupItemEnableAll
(
    TPopupMenu *menu_p,
    bool        enable
);

Int32s_t        pmcFileStringSearch
(
    Char_p      fileName_p,
    Char_p      stringIn_p,
    Int32u_t    caseMode,
    Int32u_t    skipWhiteSpace,
    Int32u_p    offset_p
);

Int32s_t        pmcFileExtractSaskPhn
(
   mbFileListStruct_p       file_p,
   Char_p                   phn_p
);

Int32s_t        pmcFileExtractSaskPhnWord
(
    Char_p      fileName_p,
    Char_p      phn_p
);

Int32s_t        pmcFileExtractSaskShspWord
(
    Char_p      fileName_p,
    Char_p      phn_p
);

Int32s_t        pmcFileNameExtractSaskPhn
(
    Char_p      nameIn_p,
    Char_p      phn_p,
    Int32u_p    date_p
);

Int32s_t        pmcFilePathAndNameGet
(
    Char_p      fullName_p,
    Char_p      path_p,
    Char_p      name_p
);

Int32s_t        pmcDirDriveAndPathGet
(
    Char_p      dirIn_p,
    Char_p      drive_p,
    Char_p      path_p

);

Char_p          pmcFormatString
(
    Char_p      string_p,
    ...
);

void            pmcAppointmentConfirmPhone
(
    Int32u_t    appointId,
    Int32u_t    providerId,
    Int32u_t    configFlag
);

void            pmcAppointmentConfirmLetter
(
    Int32u_t    appointId,
    Int32u_t    confirmFlag
);

void            pmcViewAppointmentConfirmation
(
    Int32u_t    appointId
);

Int32u_t        pmcViewAppointments
(
    Int32u_t    patientId,
    bool        showFuture,
    bool        showPast,
    bool        allowGoto,
    Int32u_p    gotoProviderId_p,
    Int32u_p    gotoDate_p,
    bool        allowPatientSelect,
    Int32u_t    mode
);

void            pmcButtonDown
(
    TCanvas    *canvas_p,
    TRect      *rect_p,
    Char_p      string_p
);

void            pmcButtonUp
(
    void
);

Int32u_t        pmcProviderListBuild
(
    TComboBox  *providerList_p,
    Int32u_t    providerId,
    Int32u_t    billingNumberRequiredFlag ,
    Int32u_t    initializeIndexFlag
);

void            pmcProviderListFree
(
    TComboBox  *providerList_p
);

Char_p          pmcProviderDescGet
(
    Int32u_t    providerId,
    Char_p      result_p
);

Int32u_t        pmcProviderIdGet
(

    Char_p      desc_p
);

Int32u_t        pmcProviderNumberToId
(
    Int32u_t    providerNumber
);

Int32u_t        pmcProviderNumberGet
(
    Int32u_t    providerId
);

Int32s_t        pmcProviderIndexGet
(
    TComboBox  *comboBox_p,
    Int32u_t    providerId
);

void            pmcPickListFree
(
    TComboBox  *list_p
);

void            pmcPickListCfgBuild
(
    TComboBox  *list_p,
    Int32s_t    type
);

void            pmcPickListBuild
(
    TComboBox  *list_p,
    pmcPickListStruct_p data_p,
    Int32s_t    code
);

Int32s_t        pmcPickListCodeGet
(
    pmcPickListStruct_p     data_p,
    Int32s_t    index
);

Int32s_t        pmcPickListIndexGet
(
    pmcPickListStruct_p     data_p,
    Int32s_t    code
);

Int32s_t        pmcPickListCodeVerify
(
    pmcPickListStruct_p     data_p,
    Int32s_t    code
);

Int32s_t        pmcDrTypeIndexGet
(
    Char_p      province,
    Int32u_t    cancerClinic
);

Int32s_t        pmcLocationCodeVerify
(
    Int32s_t    code
);

Int32s_t        pmcLocationCodeGet
(
    Int32s_t    locationIndex,
    Int32s_t    premiumIndex
);

Int32s_t        pmcLocationIndexesGet
(
    Int32s_t    code,
    Int32s_p    locationIndex_p,
    Int32s_p    premiumIndex_p
);


Char_p          pmcCleanMailName
(
    Char_p      buf_p,
    bool        allCapsFlag
);

Ints_t          pmcPostalCodeFix
(
    Char_p      in_p,
    bool       *modifiedFlag_p
);

Char_p          pmcFixCapitalization
(
    Char_p      in_p
);

Int32s_t        pmcFileTypeGet
(
    Char_p      fileName_p,
    Int32s_t    type
);

Int32u_t        pmcAppointPatientId
(
    Int32u_t    appointId
);

Int32u_t        pmcAppointDoctorId
(
    Int32u_t    appointId
);

Int32s_t        pmcPhnVerifyString
(
    Char_p      phn_p
);

Int32s_t        pmcIcdVerify
(
    Char_p      code_p,
    Char_p      desc_p,
    bool        overwrightFlag
);

Int32s_t        pmcFeeCodeVerify
(
    Char_p      code_p,
    Int32u_t    dateOfService,
    Int32u_p    feeHigh_p,
    Int32u_p    feeLow_p,
    Int32u_p    multiple_p,
    Int32u_p    determinant_p,
    Int32u_p    index_p,
    Int32u_t    claimType
);

Int32s_t        pmcFeeCodeFormatDatabase
(
    Char_p      code_p,
    Char_p      out_p
);

Char_p          pmcFeeCodeFormatDisplay
(
    Char_p      in_p,
    Char_p      out_p
);

Int32u_t        pmcExpBox
(
    Char_p      code_p
);

Int32s_t        pmcTokenInit
(
    qHead_p     token_q,
    Char_p      bufIn_p
);

pmcTokenStruct_p    pmcTokenNext
(
    qHead_p     token_q
);

bool            pmcTokenDone
(
    qHead_p     token_q,
    bool        forceDone
);

Int32s_t        pmcFileIndexToType
(
    Int32s_t    index
);

Int32s_t        pmcFileTypeToIndex
(
    Int32s_t    code
);

Int32s_t        pmcProviderCounterGet
(
    Int32u_t    providerId,
    Int32u_t    counter,
    Int32u_t    mode
);

Char_p      pmcDocumentDirFromName( Char_p documentName_p );
void        pmcCloseSplash( void );
void        pmcSuspendPollIncDebug( Char_p function_p, Int32u_t line );
void        pmcSuspendPollDecDebug( Char_p function_p, Int32u_t line );
Int32s_t    pmcCheckDeceasedApps( Int32u_t patientId, Int32u_t deceasedDate );
Int32s_t    pmcCheckReferringDrApps( Int32u_t patientId, Int32u_t doctorId );
Int32s_t    pmcCheckReferringDr( Int32u_t patientId, Int32u_t doctorId );

#define pmcSuspendPollDec( ) pmcSuspendPollDecDebug( __FUNC__, __LINE__ )
#define pmcSuspendPollInc( ) pmcSuspendPollIncDebug( __FUNC__, __LINE__ )

/*********************************************************************
 * Macros
 *--------------------------------------------------------------------
 */

#define PMC_COMPUTE_SHIFT( _shift, _diff ) {        \
    if( _diff > 3 ) {                               \
            _shift = _diff / 2;                     \
    } else if( _diff == 3 ) {                       \
            _shift = 2;                             \
    } else if( _diff == 0 ) {                       \
            _shift = 0;                             \
    } else {                                        \
            _shift = 1;                             \
    }                                               \
}


#define    PMC_CHECK_USE( _parm )                   \
if( _parm ) {                                       \
    mbLog( "%s:%d: not ready\n", __FUNC__, __LINE__ );\
    _parm = 0;                                      \
}

#define PMC_INC_USE( _parm ) _parm++;

#define PMC_DEC_USE( _parm )                        \
{                                                   \
    if( _parm == 0 ) {                              \
        mbDlgDebug(( "cannot dec 0 use count!" ));  \
    } else {                                        \
        _parm--;                                    \
    }                                               \
}

#define PMC_REPORT_FOOTER( _parm )                  \
{                                                   \
    fprintf( _parm, "<CENTER><H6><I>\n" );          \
    fprintf( _parm, "Spadina Cardiology<BR>\n" );   \
    fprintf( _parm, "Suite 203 - 728 Spadina Crescent East, Saskatoon, SK S7K 4H7<BR>\n" );\
    fprintf( _parm, "Phone: (306) 975-0600    Fax: (306) 978-5254<BR>" );\
    fprintf( _parm, "</I></H6></CENTER>\n" );       \
}

#define PMC_GET_FIELD( _bufin, _bufout, _start, _stop )\
{                                                   \
    Ints_t  _i, _j;                                 \
    for( _i = (_start), _j = 1 ; _i <= (_stop) ; _i++, _j++ ) _bufout[_j-1] = _bufin[_i-1];\
    _bufout[_j-1] = 0;                              \
}
#define PMC_STRING_DASHED_LINE "______________________________________________________________________________"

#endif /* H_pmcUtils */

