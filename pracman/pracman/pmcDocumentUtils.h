//---------------------------------------------------------------------------
// Document utility functions
//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------

#ifndef pmcDocumentUtilsH
#define pmcDocumentUtilsH

typedef struct pmcDocumentItem_s
{
    qLinkage_t  linkage;
    Char_p      name_p;
    Char_p      origName_p;
    Char_p      description_p;
    Char_p      patient_p;
    Int32u_t    providerId;
    Int32u_t    patientId;
    Int32u_t    type;
    Int32u_t    size;
    Int32u_t    id;
    Int32u_t    date;
    Int32u_t    status;
    Int32u_t    createdDate;
    Int32u_t    notDeleted;
} pmcDocumentItem_t, *pmcDocumentItem_p;

// Utility Function Prototypes

Int32s_t            pmcDocumentView( mbFileListStruct_p file_p, Int32u_t tempFlag );
Int32s_t            pmcDocumentListInit( void );
Int32s_t            pmcDocumentListUpdate( void );
pmcDocumentItem_p  *pmcDocumentListGet( Int32u_p count_p );
void                pmcDocumentListFreeItem( pmcDocumentItem_p doc_p );
Char_p              pmcDocumentStatusString( Int32u_t status );
Int32s_t            pmcDocumentStatusImageIndex( Int32u_t status );

#endif // pmcDocumentUtilsH
