//----------------------------------------------------------------------------
// File:    pmcInitialize.h
//----------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//----------------------------------------------------------------------------
// Date:    March 6, 2001
//----------------------------------------------------------------------------
// Description:
//
//
//----------------------------------------------------------------------------

#ifndef H_pmcInitialize
#define H_pmcInitialize

#include <stdlib.h>

#include "mbTypes.h"
#include "pmcGlobals.h"

#ifdef  External
#undef  External
#endif

typedef struct pmcMysqlItem_s
{
    qLinkage_t  linkage;
    Char_t      desc[512];
    Char_t      mysql[512];
    Char_t      host[512];
    Char_t      username[512];
    Char_t      password[512];
    Int32u_t    port;
    Boolean_t   local;
} pmcMysqlItem_t, *pmcMysqlItem_p;

#define PMC_EXCHANGE_SIZE       4
#define PMC_AREA_EXCHANGE_SIZE  8

typedef struct pmcLocalExchange_s
{
    qLinkage_t  linkage;
    Char_t      exchange[PMC_EXCHANGE_SIZE];
    Char_t      areaExchange[PMC_AREA_EXCHANGE_SIZE];
} pmcLocalExchange_t, *pmcLocalExchange_p;

Int32s_t        pmcLocalExchangeAdd( Char_p buf_p );
Int32u_t        pmcLocalExchangeUpdate( void );
Int32s_t        pmcLocalExchangeInit( void );

Int32s_t        pmcPatHistoryTypeInit( void );

#define PMC_FEE_CLASS_NONE              0
#define PMC_FEE_CLASS_DIAGNOSTIC        1
#define PMC_FEE_CLASS_DAY_SURGERY_0     2
#define PMC_FEE_CLASS_DAY_SURGERY_10    3
#define PMC_FEE_CLASS_DAY_SURGERY_42    4

#define PMC_FEE_DET_INVALID             0
#define PMC_FEE_DET_GP_ONE_FEE          1
#define PMC_FEE_DET_SP_ONE_FEE          2
#define PMC_FEE_DET_SP_OR_GP_ONE_FEE    3
#define PMC_FEE_DET_SP_OR_GP            4
#define PMC_FEE_DET_SP                  5
#define PMC_FEE_DET_ULTRASOUND          6
#define PMC_FEE_DET_RADIOLOGIST         7
#define PMC_FEE_DET_CHIROPRACTOR        8

typedef struct      pmcFeeStruct_s
{
    qLinkage_t      linkage;
    Char_t          code[8];
    Int8u_t         classification;
    Int8u_t         multiple;
    Int8u_t         addOn;
    Int8u_t         determinant;
    Int32u_t        feeHigh;
    Int32u_t        feeLow;
    Int32u_t        oldFeeHigh;
    Int32u_t        oldFeeLow;
    Int32u_t        index;
 } pmcFeeStruct_t,  *pmcFeeStruct_p;

typedef struct      pmcIcdStruct_s
{
    qLinkage_t      linkage;
    Char_t          code[4];
    Char_p          description_p;
    Char_p          search_p;
} pmcIcdStruct_t,  *pmcIcdStruct_p;

typedef struct pmcMedEntry_s
{
    qLinkage_t      linkage;
    Char_p          brand_p;
    Char_p          generic_p;
    Char_p          class_p;
} pmcMedEntry_t,  *pmcMedEntry_p;

//----------------------------------------------------------------------------
// Function Prototypes
//----------------------------------------------------------------------------

Int32s_t        pmcDBConfigInit( Boolean_t forceFlag );
Int32s_t        pmcDBConfigFree( void );

Int32s_t        pmcDBMedListInit( Boolean_t forceFlag );
Int32s_t        pmcDBMedListFree( qHead_p queue_p );
Int32s_t        pmcDBMedListFreeEnfry( pmcMedEntry_p item_p );

void                pmcPickListCfgAdd
(
    Char_p          in_p,
    Int32u_t        type
);


Int32s_t            pmcInitIcdFileRead
(
    Char_p          fileName_p
);

Int32s_t            pmcInitExpFileRead
(
    Char_p          fileName_p
);

Int32s_t            pmcInitFeesFileRead
(
    Char_p          fileName_p
);

Int32s_t            pmcInitFeesFileReadOld
(
    Char_p          fileName_p
);

Int32s_t            pmcInitialize
(
    void
);

Int32s_t            pmcShutdown
(
    void
);

Int32s_t            pmcInitFeeFilesMerge
(
    void
);

Int32s_t            pmcInitFormularyFileRead
(
    Char_p fileName_p
);

#define PMC_INIT_FILE                   "pracman.ini"
#define PMC_PROPERTY_FILE               "pracman.prefs"

#define PMC_INIT_PICKLIST_ICD           "icd_pick_list:"
#define PMC_INIT_PICKLIST_MSP_SERVICE   "msp_service_list:"
#define PMC_INIT_PICKLIST_MSP_HOSPITAL  "msp_hospital_list:"

#endif /* H_pmcInitialize */

