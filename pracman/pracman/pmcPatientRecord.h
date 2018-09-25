//---------------------------------------------------------------------------
// Patient record utilities, primarily printing
//---------------------------------------------------------------------------
// Copyright Michael A. Bree, Nov. 23, 2008
//---------------------------------------------------------------------------

#ifndef pmcPatientRecordH
#define pmcPatientRecordH

#include "pmcPatient.h"
#include "pmcDocument.h"

#define PMC_PDF_TEMPLATE_PAT_RECORD     "pat_record_"
#define PMC_PDF_TEMPLATE_PAT_CONSLT     "pat_consult_"


void     pmcPatRecordHTML( Int32u_t patientId );
Int32s_t pmcPatDocumentPDF( PmcPatient     *patient_p,







#endif // pmcPatientRecordH