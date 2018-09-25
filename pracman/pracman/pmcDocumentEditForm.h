//---------------------------------------------------------------------------
// File:    pmcDocumentEditForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    August 26, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcDocumentEditFormH
#define pmcDocumentEditFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

#include "pmcGlobals.h"
#include "pmcUtils.h"
#include <ComCtrls.hpp>
#include <Buttons.hpp>

//---------------------------------------------------------------------------

typedef struct pmcDocumentEditInfo_s
{
    Int32u_t                returnCode;
    Int32u_t                providerId;
    Int32u_t                patientId;
    Int32u_t                date;
    Int32u_t                terminateBatch;
    Int32u_t                batchMode;
    Int32u_t                failReasonMask;
    Int32u_t                newFlag;
    Int32u_t                createdDate;
    Int32u_t                createdTime;
    Int32u_t                modifiedDate;
    Int32u_t                modifiedTime;
    Int32u_t                id;
    Int32u_t                lockFailed;
    Char_p                  description_p;
    mbFileListStruct_p      file_p;
    Int32u_t                status;
} pmcDocumentEditInfo_t, *pmcDocumentEditInfo_p;

class TDocumentEditForm : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TButton *CancelButton;
    TButton *OkButton;
    TButton *ViewButton;
    TCheckBox *TerminateBatchCheckBox;
    TGroupBox *PatientGroupBox;
    TEdit *PatientEdit;
    TGroupBox *ProviderGroupBox;
    TComboBox *ProviderComboBox;
    TGroupBox *DetailsGroupBox;
    TEdit *OriginalNameEdit;
    TComboBox *DocumentTypeComboBox;
    TLabel *OrigNameLabel;
    TLabel *Label1;
    TGroupBox *DescriptionGroupBox;
    TEdit *DescriptionEdit;
    TBitBtn *PatientListButton;
    TGroupBox *StatusGroupBox;
    TListView *FailListView;
    TLabel *PhnLabel;
    TButton *PatientClearButton;
    TBevel *Bevel2;
    TButton *PatientEditButton;
    TLabel *Label2;
    TEdit *DateEdit;
    TLabel *Label3;
    TButton *ProviderClearButton;
    TGroupBox *CreatedGroupBox;
    TLabel *Label4;
    TLabel *Label5;
    TLabel *Label6;
    TLabel *Label7;
    TBevel *Bevel3;
    TBevel *Bevel4;
    TBevel *Bevel5;
    TBitBtn *DateEditButton;
    TLabel *Label8;
    TLabel *CreatedLabel;
    TLabel *IdLabel;
    TLabel *ModifiedLabel;
    TButton *FailButton;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall DocumentTypeComboBoxChange(TObject *Sender);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall ViewButtonClick(TObject *Sender);
    void __fastcall PatientListButtonClick(TObject *Sender);
    void __fastcall PatientEditButtonClick(TObject *Sender);
    void __fastcall DateEditButtonClick(TObject *Sender);
    void __fastcall DateEditMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall ProviderComboBoxChange(TObject *Sender);
    void __fastcall PatientEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
    void __fastcall PatientClearButtonClick(TObject *Sender);
    void __fastcall ProviderClearButtonClick(TObject *Sender);
    void __fastcall PatientEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall FailButtonClick(TObject *Sender);
private:	// User declarations

    void __fastcall FailReasonUpdate( void );
    void __fastcall PatientUpdate( Int32u_t patientId );
    void __fastcall DateUpdate( Int32u_t date );
    void __fastcall PatientList( Int32u_t key );

    Int32s_t    DocumentTypeComboBoxIndex;
    Int32s_t    ProviderIndex;
    Int32u_t    LockId;

    pmcDocumentEditInfo_p   EditInfo_p;

public:		// User declarations
    __fastcall TDocumentEditForm(TComponent* Owner);
    __fastcall TDocumentEditForm(TComponent* Owner, pmcDocumentEditInfo_p editInfo_p );

    Int32s_t __fastcall disableControls( void );

};


#endif
