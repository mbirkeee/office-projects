//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// PmcPatient class definition
//---------------------------------------------------------------------------

#ifndef pmcConsultH
#define pmcConsultH

#define PMC_CONSULT_LETTER_FILENAME_TAG_WORD    "Letter Consult"
#define PMC_CONSLT_LETTER_FILENAME_TAG_PDF      "Consult Letter"
#define PMC_FOLLOWUP_LETTER_FILENAME_TAG_PDF    "Followup Letter"

Int32u_t pmcConsultNew( Int32u_t patientId, Int32u_t type );
Int32u_t pmcConsultFindLast( Int32u_t patientId  );
Int32u_t pmcConsultFindLastFiled( Int32u_t patientId );

#endif  // pmcConsultH
