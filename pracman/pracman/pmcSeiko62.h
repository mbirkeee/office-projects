//---------------------------------------------------------------------------
// File:    pmcSeiko62.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2007, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    October 7, 2007
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------


#ifndef H_pmcSeiko240
#define H_pmcSeiko240

#include <stdlib.h>

#include "mbTypes.h"
#include "pmcGlobals.h"

#ifdef  External
#undef  External
#endif

#define PMC_SLP_REQ_LINE( _parm ) ( 10 + _parm * 50 )
#define PMC_SLP_REQ_OFFSET 150

#define PMC_SLP_ADD_LINE( _parm ) ( 59 + _parm * 56 )
#define PMC_SLP_ADD_OFFSET 15

Int32u_t            pmcLabelPrintPatReq
(
    Int32u_t        appointId,
    Int32u_t        providerIdIn,
    Int32u_t        patientIdIn
);

Int32u_t            pmcLabelPrintPatAddress
(
    Int32u_t        id
);

Int32u_t            pmcLabelPrintDrAddress
(
    Int32u_t        id
);

Int32u_t            pmcLabelPrintAddress
(
    Char_p          add1_p,
    Char_p          add2_p,
    Char_p          add3_p,
    Char_p          add4_p,
    Char_p          add5_p,
    Char_p          add6_p,
    Char_p          add7_p,
    Char_p          add8_p
);

void pmcTestLabelPrinter( void );


#endif /* H_pmcSeiko62 */

