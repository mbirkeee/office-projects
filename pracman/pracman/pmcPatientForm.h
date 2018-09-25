//---------------------------------------------------------------------------
// File:    pmcMainForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcPatientFormH
#define pmcPatientFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>


#ifdef  External
#undef  External
#endif

#ifdef  PMC_INIT_GLOBALS
#define External
#else
#define External extern
#endif  /* PMC_INIT_GLOBALS */


//---------------------------------------------------------------------------

typedef struct pmcPatientFormInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    patientId;
    bool        canDelete;
} pmcPatientFormInfo_t, *pmcPatientFormInfo_p;

class TPatientForm : public TForm
{
__published:	// IDE-managed Components
    TButton *CancelButton;
    TBevel *Bevel1;
    TPageControl *PageControl1;
    TTabSheet *DetailsSheet;
    TTabSheet *AppointmentsSheet;
    TTabSheet *ClaimsSheet;
    TTabSheet *DocumentsSheet;
    TTabSheet *MedicalSheet;
    TRichEdit *RichEdit1;
    TBevel *Bevel2;
    TBevel *Bevel3;
    TBevel *Bevel4;
    TLabel *BannerNameLabel;
    TLabel *BannerPhnLabel;
    TLabel *BannerPhoneLabel;
    TDrawGrid *AppListGrid;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);

private:	// User declarations
    void __fastcall UpdateBanner( Int32u_t patientId );

    pmcPatientFormInfo_p    FormInfo_p;

public:		// User declarations
    __fastcall TPatientForm(TComponent* Owner);
    __fastcall TPatientForm(TComponent* Owner, pmcPatientFormInfo_p formInfo_p);
};

//---------------------------------------------------------------------------
// Structure for linked list of non modal patient windows
//---------------------------------------------------------------------------

typedef struct pmcPatientFormList_s
{
    qLinkage_t              linkage;
    TPatientForm           *form_p;
    pmcPatientFormInfo_t    info;
} pmcPatientFormList_t, *pmcPatientFormList_p;

//---------------------------------------------------------------------------
// Function prototypes
//---------------------------------------------------------------------------

pmcPatientFormList_p    pmcPatientFormListAdd( void );
void                    pmcPatientFormListClean( void );
void                    pmcPatientFormNonModal( Int32u_t patientId );

#define PMC_PATIENT_FORM_MODE_DETAILS   1
#define PMC_PATIENT_FORM_MODE_APPS      1
#define PMC_PATIENT_FORM_MODE_CLAIMS   1
#define PMC_PATIENT_FORM_MODE_DOCUMENTS   1
#define PMC_PATIENT_FORM_MODE_   1



#endif
