//---------------------------------------------------------------------------
// pmcPatientEditForm.cpp
//---------------------------------------------------------------------------
// (c) 2001-2008 Michael A. Bree
//---------------------------------------------------------------------------
// Form for displaying/editing patient details
//---------------------------------------------------------------------------

// Platform includes
#include <vcl.h>
#include <stdio.h>
#include <dir.h>
#include <process.h>

#pragma hdrstop

// Library includes
#include "mbUtils.h"

// Program includes
#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcUtils.h"
#include "pmcPatient.h"
#include "pmcPatientRecord.h"
#include "pmcPatientEditForm.h"
#include "pmcDateSelectForm.h"
#include "pmcDoctorListForm.h"
#include "pmcDocumentListForm.h"
#include "pmcPatientListForm.h"
#include "pmcSeiko240.h"
#include "pmcInitialize.h"
#include "pmcDocument.h"
#include "pmcWordCreateForm.h"
#include "pmcClaimListForm.h"
#include "pmcClaimEditForm.h"
#include "pmcEchoListForm.h"
#include "pmcConsult.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
__fastcall TPatientEditForm::~TPatientEditForm( void )
{
    pmcPatEditSection_p     section_p;

    pmcProviderListFree( this->ProviderList );

    // Delete the programatically added tabs
    while( !qEmpty( this->section_q ) )
    {
        section_p = (pmcPatEditSection_p)qRemoveFirst( this->section_q );

        delete section_p->updateLabel_p;
        delete section_p->updateEdit_p;
        delete section_p->buttonNext_p;
        delete section_p->buttonPrev_p;
        delete section_p->buttonHelp_p;
        delete section_p->memo_p;
        delete section_p->tabSheet_p;

        mbFree( section_p->orig_p );
        mbFree( section_p->subKey_p );

        // Must free the history queue
        mbStrListFree( section_p->item_q );

        mbFree( section_p );
    }

    // Delete the patient and consult objects if necessary object
    if( this->Patient_p ) delete this->Patient_p;
    if( this->Consult_p ) delete this->Consult_p;
}

//---------------------------------------------------------------------------
// Default constructor
//---------------------------------------------------------------------------
__fastcall TPatientEditForm::TPatientEditForm(TComponent* Owner)
    : TForm(Owner)
{
    mbDlgInfo( "TPatientEditForm constructor called" );

    mbDlgDebug(( "PatientEditForm default Constructor called" ));
}

//---------------------------------------------------------------------------
// Constructor for Patient Edit Form
//---------------------------------------------------------------------------
__fastcall TPatientEditForm::TPatientEditForm
(
    TComponent         *Owner,
    pmcPatEditInfo_p    patEditInfo_p
) : TForm(Owner)
{
    Int32u_t            consultId;
    Boolean_t           consultPending = FALSE;

    // These are used to keep track of the selected consult dr and app info
    // after a consult has been approved (or deleted)
    this->consultDrId = 0;
    this->consultDateLetter = 0;
    this->consultDateApp = 0;

    SaveButton->Enabled = FALSE;
    ForceChangeFlag = FALSE;

    // Some stuff must be initialized even before an error return
    this->Patient_p = NIL;
    this->Consult_p = NIL;

    // Queue for list of programatically added sections
    this->section_q = qInitialize( &this->sectionQueue );

    // Set the size/position of this form ... must be done after section_q init
    {
        Ints_t  height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_PAT_EDIT, &height, &width, &top, &left ) == MB_RET_OK )
        {
            SetBounds( left, top, width, height );
        }
    }

    if( patEditInfo_p )
    {
        PatEditInfo_p = patEditInfo_p;
    }
    else
    {
        mbDlgDebug(( "Error: Called without a patient edit info struct" ));
        return;
    }

    PatEditInfo_p->returnCode = MB_BUTTON_CANCEL;

    Caption = PatEditInfo_p->caption;

    // Get the patient details from the database
    this->Patient_p = new PmcPatient( PatEditInfo_p->patientId );

    // Before locking the record, get any pending consult.  This causes
    // the patient record to open in view only mode
    consultId = pmcConsultFindLast( this->Patient_p->id( ) );

    if( consultId != 0 )
    {
        // Read the consult from the database
        this->Consult_p = new PmcDocument( consultId );

        if( this->Consult_p->lock( TRUE ) != MB_RET_OK )
        {
            mbDlgInfo( "Unable to lock the consult document for editing. Opening in view-only mode." );
            PatEditInfo_p->mode = PMC_EDIT_MODE_VIEW;
        }
        else if( this->Consult_p->status() == PMC_DOCUMENT_STATUS_PENDING )
        {
            mbDlgInfo( "This patient has an outstanding consult pending approval and filing.\n"
                       "Until the consult is approved and filed, the patient record cannot be modified.\n\n"
                       "Opening the patient record in view-only mode.");
            consultPending = True;
        }
    }

    // Attempt to lock the record.  Open in view only mode if locking fails.
    if( PatEditInfo_p->mode == PMC_EDIT_MODE_EDIT )
    {
        if( consultPending == TRUE )
        {
            if( this->Patient_p->lock( TRUE ) != MB_RET_OK )
            {
                PatEditInfo_p->mode = PMC_EDIT_MODE_VIEW;
            }
        }
        else
        {
            if( this->Patient_p->lock( TRUE ) != MB_RET_OK )
            {
                mbDlgInfo( "Unable to lock patient record for editing. Opening in view-only mode." );
                PatEditInfo_p->mode = PMC_EDIT_MODE_VIEW;
            }
        }
    }

    // Record the actual mode in which the form was opened
    this->obtainedMode = PatEditInfo_p->mode;

    if( consultPending == TRUE )
    {
        // If there is a consult pending, switch document to edit mode, but
        // only after lock has been obtained (or not)
         PatEditInfo_p->mode = PMC_EDIT_MODE_VIEW;
    }

    if( Patient_p->id() == 0 )
    {
        // This must be a new patient
        Patient_p->citySet( "Saskatoon" );
        Patient_p->countrySet( "Canada" );
        Patient_p->provinceSet( "SK" );
        Patient_p->phnProvinceSet( "SK" );
        Patient_p->titleSet( "Mr." );
        Patient_p->genderSet( PMC_SQL_GENDER_MALE );
        Patient_p->providerIdSet( PatEditInfo_p->providerId );
        Patient_p->phnInit(PatEditInfo_p->searchString );
    }

    // Populate the controls
    {
        Char_t  buf[32];
        sprintf( buf, "%d", Patient_p->id() );
        IdEdit->Text =      buf;
    }
    FirstNameEdit->Text =   Patient_p->firstName( );
    LastNameEdit->Text =    Patient_p->lastName( );
    MiddleNameEdit->Text =  Patient_p->middleName( );

    Address1Edit->Text =    Patient_p->address1( );
    Address2Edit->Text =    Patient_p->address2( );

    CountryEdit->Text =     Patient_p->country( );
    PostalCodeEdit->Text =  Patient_p->postalCode( );
    ProvList->Text =        Patient_p->province( );

    PhnEdit->Text =         Patient_p->phn( );
    PhnProvList->Text =     Patient_p->phnProvince( );
    ChartEdit->Text =       Patient_p->chartString( );
    EmailEdit->Text =       Patient_p->emailAddress( );
    ContactNameEdit->Text = Patient_p->contactName( );
    ContactDescEdit->Text = Patient_p->contactDesc( );
    ContactPhoneEdit->Text= Patient_p->contactPhone( );
    CommentEdit->Text  =    Patient_p->comment( );
    WorkDescEdit->Text =    Patient_p->workDesc( );

    DateOfBirthEdit->Text = Patient_p->dateString( Patient_p->dateOfBirth( ) );
    DeceasedDateEdit->Text= Patient_p->dateString( Patient_p->dateDeceased( ) );

    GenderRadio->ItemIndex = ( Patient_p->gender( ) == PMC_SQL_GENDER_MALE ) ? 0 : 1;

    HomePhoneEdit->Text =   Patient_p->phoneHome( );
    WorkPhoneEdit->Text =   Patient_p->phoneWork( );
    CellPhoneEdit->Text =   Patient_p->phoneCell( );
    ModifiedEdit->Text =    Patient_p->timeModified( );
    CreatedEdit->Text =     Patient_p->timeCreated( );
    AgeEdit->Text =         Patient_p->ageString( );

    // Must get the familly and referring Dr info
    UpdateDoctorEdit( this->Patient_p->referringId(), PMC_PATEDIT_REF_DR );
    UpdateDoctorEdit( this->Patient_p->familyDrId(),  PMC_PATEDIT_FAM_DR );

    // Keeps track of which dynamically added history section is being shown
    this->ActiveSection_p = NIL;

    // Loop through the list of editable text items, and programatically
    // add tabs for each one
    {
        mbStrList_p             item_p;
        pmcPatEditSection_p     section_p;

        mbLockAcquire( gPatHistoryType_q->lock );
        qWalk( item_p, gPatHistoryType_q, mbStrList_p )
        {
            mbCalloc( section_p, sizeof(pmcPatEditSection_t) );

            // Record the substitution keys
            mbMallocStr( section_p->subKey_p, item_p->str2_p );

            section_p->tabSheet_p = new TTabSheet( NIL );
            section_p->memo_p = new TMemo( NIL );
            section_p->memo_p->Parent = section_p->tabSheet_p;
            section_p->memo_p->Visible = TRUE;
            section_p->memo_p->Top = 40;
            section_p->memo_p->Left = 8;
            section_p->memo_p->WantTabs = TRUE;
            section_p->memo_p->OnChange = SectionMemoChange;
            section_p->memo_p->ScrollBars = ssVertical;
            section_p->memo_p->Font->Charset = GREEK_CHARSET;
            section_p->memo_p->Font->Size = 10;

            // NOTE: The anchoring does not seem to be working on these dynmic controls
            //section_p->memo_p->Anchors = TAnchors() << akLeft << akTop << akRight << akBottom;
            this->MemoSize( section_p->memo_p );

            section_p->buttonPrev_p = new TSpeedButton( NIL );
            section_p->buttonPrev_p->Parent = section_p->tabSheet_p;
            section_p->buttonPrev_p->Height = 22;
            section_p->buttonPrev_p->Width = 23;
            section_p->buttonPrev_p->Left = 212;
            section_p->buttonPrev_p->Top = 8;
            section_p->buttonPrev_p->Glyph = this->HiddenButtonPrev->Glyph;
            section_p->buttonPrev_p->Anchors = TAnchors() << akLeft << akTop;
            section_p->buttonPrev_p->OnClick = this->SectionButtonPrevClick;

            section_p->buttonNext_p = new TSpeedButton( NIL );
            section_p->buttonNext_p->Parent = section_p->tabSheet_p;
            section_p->buttonNext_p->Height = 22;
            section_p->buttonNext_p->Width = 23;
            section_p->buttonNext_p->Left = 244;
            section_p->buttonNext_p->Top = 8;
            section_p->buttonNext_p->Glyph = this->HiddenButtonNext->Glyph;
            section_p->buttonNext_p->Anchors = TAnchors() << akLeft << akTop;
            section_p->buttonNext_p->OnClick = this->SectionButtonNextClick;

            section_p->buttonHelp_p = new TButton( NIL );
            section_p->buttonHelp_p->Parent = section_p->tabSheet_p;
            section_p->buttonHelp_p->Height = 22;
            section_p->buttonHelp_p->Width = 60;
            section_p->buttonHelp_p->Left = 344;
            section_p->buttonHelp_p->Top = 8;
            section_p->buttonHelp_p->Caption = "Help";
            section_p->buttonHelp_p->Anchors = TAnchors() << akLeft << akTop;
            section_p->buttonHelp_p->OnClick = this->SectionButtonHelpClick;

            section_p->updateEdit_p = new TMaskEdit( NIL );
            section_p->updateEdit_p->Parent = section_p->tabSheet_p;
            section_p->updateEdit_p->Height = 21;
            section_p->updateEdit_p->Width = 141;
            section_p->updateEdit_p->Top = 8;
            section_p->updateEdit_p->Left = 56;
            section_p->updateEdit_p->Anchors = TAnchors() << akLeft << akTop;
            section_p->updateEdit_p->Color = clInfoBk;
            section_p->updateEdit_p->ReadOnly = TRUE;

            section_p->updateLabel_p = new TLabel( NIL );
            section_p->updateLabel_p->Parent = section_p->tabSheet_p;
            section_p->updateLabel_p->Height = 19;
            section_p->updateLabel_p->Width = 57;
            section_p->updateLabel_p->Top = 12;
            section_p->updateLabel_p->Left = 8;
            section_p->updateLabel_p->Layout = tlCenter;
            section_p->updateLabel_p->Anchors = TAnchors() << akLeft << akTop;
            section_p->updateLabel_p->Caption = "Updated";

            section_p->tabSheet_p->Caption = item_p->str_p;
            section_p->type = item_p->handle;
            section_p->tabSheet_p->PageControl = this->PageControl;
            section_p->modified = FALSE;

            mbMallocStr( section_p->orig_p, "" );

            section_p->buttonPrev_p->Enabled = FALSE;
            section_p->buttonNext_p->Enabled = FALSE;

            // Must initialize history queue
            section_p->item_q = qInitialize( &section_p->itemQueue );

            // Attempt to read list of histories (does not get actual data)
            if( pmcSqlPatHistoryListGet( this->Patient_p->id(), section_p->type, section_p->item_q, TRUE ) > 0 )
            {
                section_p->last_p = qLast( section_p->item_q, mbStrList_p );
                section_p->first_p = qFirst( section_p->item_q, mbStrList_p );
                section_p->current_p = section_p->last_p;
            }
            else
            {
                mbDlgError( "This should never happen!!!" );
            }
            qInsertLast( this->section_q,  section_p );
        }
        mbLockRelease( gPatHistoryType_q->lock );
    }

    ConsultControlsUpdate( );
    ControlsUpdate( );

    ForceChangeFlag = FALSE;
    BannerUpdate( );

    return;
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::FormActivate(TObject *Sender)
{
    if( PatEditInfo_p->mode == PMC_EDIT_MODE_EDIT ||
        PatEditInfo_p->mode == PMC_EDIT_MODE_NEW )
    {
        LastNameEdit->SetFocus();
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ConsultControlsUpdate( void )
{
    // The preview consult button is always enabled
    this->PreviewConsultButton->Enabled = TRUE;
    this->ConsultStatusEdit->ReadOnly = TRUE;

    if( this->Consult_p == NIL )
    {
        this->NewConsultButton->Caption = "New";
        this->NewConsultButton->Enabled = TRUE;

        this->ApproveConsultButton->Enabled = FALSE;
        this->SubmitConsultButton->Enabled = FALSE;

        this->ConsultDrButton->Enabled = FALSE;
        this->ConsultAppDateButton->Enabled = FALSE;
        this->ConsultLetterDateButton->Enabled = FALSE;

        this->ConsultDrEdit->Text = "";

        this->ConsultDrEdit->Color = clInfoBk;
        this->ConsultLetterDateEdit->Color = clInfoBk;
        this->ConsultAppDateEdit->Color = clInfoBk;

        this->ConsultStatusEdit->Text = "No consult found";
        this->ConsultStatusEdit->Color = clWhite;
    }
    else
    {
        this->consultDateLetter = this->Consult_p->date();;
        this->consultDateApp = this->Consult_p->appDate();;
        this->consultDrId = this->Consult_p->drId();;

        // Set the consult type pick list
        if( Consult_p->type( ) == PMC_DOCUMENT_TYPE_CONSLT )
        {
            this->ConsultTypeComboBox->ItemIndex = PMC_CONSULT_TYPE_PICKLIST_INDEX_CONSULT;
        }
        else
        {
            this->ConsultTypeComboBox->ItemIndex = PMC_CONSULT_TYPE_PICKLIST_INDEX_FOLLOWUP;
        }

        this->NewConsultButton->Caption = "Delete";
        this->NewConsultButton->Enabled = True;

        if( this->Consult_p->status() == PMC_DOCUMENT_STATUS_FILED )
        {
            this->NewConsultButton->Caption = "New";
            this->ConsultStatusEdit->Text = "Filed";
            this->ConsultStatusEdit->Color = clLime;

            delete this->Consult_p;
            this->Consult_p = NIL;

            this->NewConsultButton->Enabled = TRUE;

            this->ApproveConsultButton->Enabled = FALSE;
            this->SubmitConsultButton->Enabled = FALSE;

            this->ConsultDrButton->Enabled = FALSE;
            this->ConsultAppDateButton->Enabled = FALSE;
            this->ConsultLetterDateButton->Enabled = FALSE;

            this->ConsultTypeComboBox->Enabled = TRUE;
            this->ConsultTypeComboBox->Color = clWhite;

        }
        else if( this->Consult_p->status() == PMC_DOCUMENT_STATUS_ACTIVE )
        {
            this->ConsultStatusEdit->Text = "Active";
            this->ConsultStatusEdit->Color = clYellow;

            this->ConsultTypeComboBox->Color = clWhite;
            this->ConsultDrEdit->Color = clWhite;
            this->ConsultLetterDateEdit->Color = clWhite;
            this->ConsultAppDateEdit->Color = clWhite;

            this->ConsultTypeComboBox->Enabled = TRUE;
            this->ConsultDrButton->Enabled = TRUE;
            this->ConsultAppDateButton->Enabled = TRUE;
            this->ConsultLetterDateButton->Enabled = TRUE;

            this->ApproveConsultButton->Enabled = FALSE;
            this->SubmitConsultButton->Caption = "Submit for Approval";
            this->SubmitConsultButton->Enabled = TRUE;
        }
        else if( this->Consult_p->status() == PMC_DOCUMENT_STATUS_PENDING )
        {
            this->ConsultTypeComboBox->Enabled = FALSE;
            this->ConsultDrButton->Enabled = FALSE;
            this->ConsultAppDateButton->Enabled = FALSE;
            this->ConsultLetterDateButton->Enabled = FALSE;

            this->ConsultTypeComboBox->Color = clInfoBk;
            this->ConsultDrEdit->Color = clInfoBk;
            this->ConsultLetterDateEdit->Color = clInfoBk;
            this->ConsultAppDateEdit->Color = clInfoBk;

            this->ConsultStatusEdit->Text = "Pending Approval";
            this->ConsultStatusEdit->Color = clAqua;

            this->ApproveConsultButton->Enabled = TRUE;
            this->SubmitConsultButton->Caption = "Requires Revision";
            this->SubmitConsultButton->Enabled = TRUE;
        }
        else
        {
            this->ConsultStatusEdit->Text = "UNKNOWN";
            this->ConsultStatusEdit->Color = clRed;
        }
    }

    ConsultLetterDateEdit->Text = Patient_p->dateString( this->consultDateLetter );
    ConsultAppDateEdit->Text = Patient_p->dateString( this->consultDateApp );
    UpdateDoctorEdit( this->consultDrId, PMC_PATEDIT_CONSULT_DR );

    // Override some button settings if in view mode
    if( this->PatEditInfo_p->mode == PMC_EDIT_MODE_VIEW )
    {
        this->NewConsultButton->Enabled = FALSE;
    }

    if( this->obtainedMode != PMC_EDIT_MODE_EDIT )
    {
        this->NewConsultButton->Enabled = FALSE;
        this->ApproveConsultButton->Enabled = FALSE;
        this->SubmitConsultButton->Enabled = FALSE;

        this->ConsultTypeComboBox->Enabled = FALSE;
        this->ConsultDrButton->Enabled = FALSE;
        this->ConsultAppDateButton->Enabled = FALSE;
        this->ConsultLetterDateButton->Enabled = FALSE;

        this->ConsultTypeComboBox->Color = clInfoBk;
        this->ConsultDrEdit->Color = clInfoBk;
        this->ConsultLetterDateEdit->Color = clInfoBk;
        this->ConsultAppDateEdit->Color = clInfoBk;
    }

    // To disable feature entirely
#if 0
    {
        this->NewConsultButton->Enabled = FALSE;
        this->ApproveConsultButton->Enabled = FALSE;
        this->SubmitConsultButton->Enabled = FALSE;

        this->ConsultDrEdit->Color = clBtnFace;
        this->ConsultAppDateEdit->Color = clBtnFace;
        this->ConsultLetterDateEdit->Color = clBtnFace;
        this->ConsultStatusEdit->Color = clBtnFace;
        this->ConsultStatusEdit->Text = "Not yet implemeneted";
    }
#endif
    return;
}

//---------------------------------------------------------------------------
//  Enable/disable/program the controls according to current edit mode
//---------------------------------------------------------------------------
void __fastcall  TPatientEditForm::ControlsUpdate(  )
{
    Boolean_t               enabled = TRUE;
    Int32u_t                providerId;
    pmcPatEditSection_p     section_p;
    Char_t                  buf[256];

    if( this->PatEditInfo_p->mode == PMC_EDIT_MODE_VIEW ) enabled = FALSE;

    Boolean_t readOnly = ( enabled == TRUE ) ? FALSE   : TRUE;
    TColor    color    = ( enabled == TRUE ) ? clWhite : clInfoBk;

    FirstNameEdit->ReadOnly      = readOnly; FirstNameEdit->Color      = color;
    LastNameEdit->ReadOnly       = readOnly; LastNameEdit->Color       = color;
    MiddleNameEdit->ReadOnly     = readOnly; MiddleNameEdit->Color     = color;
    Address1Edit->ReadOnly       = readOnly; Address1Edit->Color       = color;
    Address2Edit->ReadOnly       = readOnly; Address2Edit->Color       = color;
    CountryEdit->ReadOnly        = readOnly; CountryEdit->Color        = color;
    PostalCodeEdit->ReadOnly     = readOnly; PostalCodeEdit->Color     = color;
    ChartEdit->ReadOnly          = readOnly; ChartEdit->Color          = color;
    EmailEdit->ReadOnly          = readOnly; EmailEdit->Color          = color;
    ContactNameEdit->ReadOnly    = readOnly; ContactNameEdit->Color    = color;
    ContactDescEdit->ReadOnly    = readOnly; ContactDescEdit->Color    = color;
    ContactPhoneEdit->ReadOnly   = readOnly; ContactPhoneEdit->Color   = color;
    CommentEdit->ReadOnly        = readOnly; CommentEdit->Color        = color;
    DateOfBirthEdit->ReadOnly    = readOnly; DateOfBirthEdit->Color    = color;
    DeceasedDateEdit->ReadOnly   = readOnly; DeceasedDateEdit->Color   = color;
    PhnEdit->ReadOnly            = readOnly; PhnEdit->Color            = color;
    HomePhoneEdit->ReadOnly      = readOnly; HomePhoneEdit->Color      = color;
    WorkPhoneEdit->ReadOnly      = readOnly; WorkPhoneEdit->Color      = color;
    CellPhoneEdit->ReadOnly      = readOnly; CellPhoneEdit->Color      = color;
    WorkDescEdit->ReadOnly       = readOnly; WorkDescEdit->Color       = color;
    FamilyDrEdit->ReadOnly       = readOnly; FamilyDrEdit->Color       = color;
    ReferringDrEdit->ReadOnly    = readOnly; ReferringDrEdit->Color    = color;

    ComboBoxCity->Color         = color;
    ProviderList->Color         = color;
    TitleList->Color            = color;
    ProvList->Color             = color;
    PhnProvList->Color          = color;

    ReferringDrButton->Enabled  = enabled;
    FamilyDrButton->Enabled     = enabled;
    BirthDateButton->Enabled    = enabled;
    DeceasedDateButton->Enabled = enabled;
    GenderRadio->Enabled        = enabled;

    // Program the pick lists according to the enabled status of the patient.
    // This is done so that the text remains black.  If the picklists were
    // simply disabled, the text would appear grey, unlike the other text
    // in the form
    if( enabled == TRUE )
    {
        OkButton->Visible = TRUE;
        SaveButton->Visible = TRUE;
        CancelButton->Caption = "Cancel";

        ComboBoxCity->Style = csDropDown;
        TitleList->Style    = csDropDown;
        ProvList->Style     = csDropDown;
        PhnProvList->Style  = csDropDown;

        pmcPickListBuildNew( TitleList,     PMC_DB_CONFIG_TITLE );
        pmcPickListBuildNew( ComboBoxCity,  PMC_DB_CONFIG_CITY );
        pmcPickListBuildNew( ProvList,      PMC_DB_CONFIG_PROVINCE );
        pmcPickListBuildNew( PhnProvList,   PMC_DB_CONFIG_PROVINCE );

        providerId = pmcProviderListBuild( ProviderList, this->Patient_p->providerId(), TRUE, TRUE );
        this->Patient_p->providerIdSet( providerId );
    }
    else
    {
        OkButton->Visible = FALSE;
        SaveButton->Visible = FALSE;
        CancelButton->Caption = "Close";

        // If in view mode, the combo boxes and list boxes must be reprogrammed
        ComboBoxCity->Items->Clear();
        ComboBoxCity->Items->Add( this->Patient_p->city() );
        ComboBoxCity->Style = csDropDownList;
        ComboBoxCity->ItemIndex = 0;

        TitleList->Items->Clear( );
        TitleList->Items->Add( this->Patient_p->title( ) );
        TitleList->Style = csDropDownList;
        TitleList->ItemIndex = 0;

        ProvList->Items->Clear();
        ProvList->Items->Add( this->Patient_p->province( ) );
        ProvList->Style = csDropDownList;
        ProvList->ItemIndex = 0;

        PhnProvList->Items->Clear();
        PhnProvList->Items->Add( this->Patient_p->phnProvince( ) );
        PhnProvList->Style = csDropDownList;
        PhnProvList->ItemIndex = 0;

        ProviderList->Items->Clear();
        ProviderList->Items->Add( pmcProviderDescGet( this->Patient_p->providerId(), buf ) );
        ProviderList->Style = csDropDownList;
        ProviderList->ItemIndex = 0;
    }

    ComboBoxCity->Text = Patient_p->city( );
    TitleList->Text = Patient_p->title( );
    ProvList->Text = Patient_p->province( );
    PhnProvList->Text = Patient_p->phnProvince( );
    ProviderList->Text = pmcProviderDescGet( providerId, buf );

    // This loop determines if one of the "dynamic" tabs is visible
    qWalk( section_p, this->section_q, pmcPatEditSection_p )
    {
        if( section_p->tabSheet_p == PageControl->ActivePage )
        {
            if( enabled == TRUE )
            {
                if( section_p->current_p == section_p->last_p )
                {
                    section_p->memo_p->Color = clWindow;
                    section_p->memo_p->ReadOnly = FALSE;
                }
            }
            else
            {
                section_p->memo_p->Color = clInfoBk;
                section_p->memo_p->ReadOnly = TRUE;
            }
            break;
        }
    }

    if( pmcCfg[CFG_SLP_NAME].str_p )
    {
        PrintAddressLabelButton->Enabled = TRUE;
        PrintReqLabelButton->Enabled     = TRUE;
    }
    //PreferEmailCheck->ReadOnly   = readOnly; PreferEmailCheck->Color   = color;
}

//---------------------------------------------------------------------------
// Update text at top of window, and check for changes to the patient data
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::BannerUpdate( )
{
    Boolean_t   changed = FALSE;

    Char_t  buf[512];
    Char_t  buf2[512];

    sprintf( buf, "%s, %s", LastNameEdit->Text.c_str( ), FirstNameEdit->Text.c_str( ) );
    LabelPatientName->Caption = buf;
    pmcFormatPhnDisplay( PhnEdit->Text.c_str(), PhnProvList->Text.c_str(), buf );
    sprintf( buf2, "PHN: %s", buf );
    LabelPHN->Caption = buf2;

    if( this->Consult_p )
    {
        if( this->Consult_p->changed( ) == TRUE ) changed = TRUE;
    }

    if( this->Patient_p->changed( ) ) changed = TRUE;

    if( this->ForceChangeFlag == TRUE  || this->SectionsModifiedCheck( ) == TRUE ) changed = TRUE;

    SaveButton->Enabled = changed;
}

//---------------------------------------------------------------------------
// Save changes or prompt if necessary
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::FormClose(TObject *Sender,  TCloseAction &Action)
{
    Boolean_t   closeFlag = TRUE;
    Boolean_t   consultChanged = FALSE;

    Action = caNone;

    if( this->Consult_p )
    {
        if( this->Consult_p->changed() == TRUE ) consultChanged = TRUE;
    }

    if(    this->Patient_p->changed( ) == TRUE
        || this->ForceChangeFlag == TRUE
        || this->SectionsModifiedCheck( ) == TRUE
        || consultChanged == TRUE )
    {
        if( PatEditInfo_p->returnCode == MB_BUTTON_OK )
        {
            if( this->Patient_p->save( ) == FALSE )
            {
                closeFlag = FALSE;
            }
            else
            {
                // Save the history sections
                this->SectionsSave( );

                if( consultChanged == TRUE )
                {
                    this->Consult_p->save();
                }
            }
        }
        else
        {
            // The OK button was not clicked, but there are changes
            if( mbDlgYesNo( "This patient record has been changed.  If you cancel, the changes will be lost.\n\n"
                            "Do you really want to cancel?" ) == MB_BUTTON_NO )
            {
                closeFlag = FALSE;
            }
        }
    }

    PatEditInfo_p->patientId = this->Patient_p->id( );

    if( closeFlag == TRUE )
    {
        mbPropertyWinSave( PMC_WINID_PAT_EDIT, Height, Width, Top, Left );
        Action = caFree;
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::OkButtonClick(TObject *Sender)
{
    PatEditInfo_p->returnCode = MB_BUTTON_OK;
    Close();
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::CancelButtonClick(TObject *Sender)
{
    PatEditInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close();
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::BirthDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    if( BirthDateButton->Enabled == FALSE ) return;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NULL;
    dateSelectInfo.dateIn = this->Patient_p->dateOfBirth( );
    dateSelectInfo.caption_p = "Select Birth Date";
    dateSelectInfo.clearEnabled = TRUE;

    // Show the date selection form
    dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        this->Patient_p->dateOfBirthSet( dateSelectInfo.dateOut );
        this->DateOfBirthEdit->Text = this->Patient_p->dateString( this->Patient_p->dateOfBirth( ) );
    }
    AgeEdit->Text = this->Patient_p->ageString();
    BannerUpdate( );
    this->DeceasedDateEdit->SetFocus();
}

//---------------------------------------------------------------------------
// Handle provider picklist change
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ProviderListChange(TObject *Sender)
{
    Int32u_t    newProviderId;
    Char_t      buf[64];

    // Determine the provider ID based on the name
    newProviderId = pmcProviderIdGet( ProviderList->Text.c_str() );
    if( newProviderId == 0 )
    {
        // I think this means we were unable to determine the provider,
        // so set the index to the set id
        ProviderList->ItemIndex = pmcProviderIndexGet( ProviderList, this->Patient_p->providerId() );
    }
    else
    {
        this->Patient_p->providerIdSet( newProviderId );

        // Must also update any active consults
        if( this->Consult_p )
        {
            this->Consult_p->providerIdSet( newProviderId );
        }
    }
    ProviderList->Text = pmcProviderDescGet( this->Patient_p->providerId(), buf );
    BannerUpdate( );
    return;
}

//---------------------------------------------------------------------------
// Set the deceased date
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::DeceasedDateButtonClick(TObject *Sender)
{
    TDateSelectForm        *dateSelectForm_p;
    pmcDateSelectInfo_t     dateSelectInfo;

    if( DeceasedDateButton->Enabled == FALSE ) return;

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NIL;
    dateSelectInfo.dateIn = this->Patient_p->dateDeceased( );
    dateSelectInfo.clearEnabled = TRUE;
    dateSelectInfo.caption_p = "Deceased Date";

    dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        this->Patient_p->dateDeceasedSet( dateSelectInfo.dateOut );
        DeceasedDateEdit->Text = this->Patient_p->dateString( this->Patient_p->dateDeceased( ) );
    }
    BannerUpdate( );
    AgeEdit->Text = this->Patient_p->ageString();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PrintLabelClick(TObject *Sender)
{
    mbDlgDebug(( "Got print label click" ));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::UpdateDoctorEdit
(
    Int32u_t        doctorId,
    Int32u_t        fieldFlag
)
{
    Char_t          buf[128];
    PmcSqlDoctor_p  doctor_p;

    mbMalloc( doctor_p, sizeof(PmcSqlDoctor_t) );

    if( doctorId == 0 )
    {
        buf[0] = 0;
        goto exit;
    }

    if( pmcSqlDoctorDetailsGet( doctorId, doctor_p ) )
    {
        sprintf( buf, "%s, %s (Dr. #%ld)", doctor_p->lastName, doctor_p->firstName, doctor_p->mspNumber );
    }
    else
    {
        // Error
        sprintf( buf, "Unknown (id: %ld)", doctorId );
    }

exit:
    if( fieldFlag == PMC_PATEDIT_REF_DR )
    {
        ReferringDrEdit->Text = buf;
    }
    else if( fieldFlag == PMC_PATEDIT_FAM_DR )
    {
        FamilyDrEdit->Text = buf;
    }
    else if( fieldFlag == PMC_PATEDIT_CONSULT_DR )
    {
        ConsultDrEdit->Text = buf;
    }
    else
    {
        mbDlgDebug(( "Invalid doctor update mode: %ld", fieldFlag ));
    }
    mbFree( doctor_p );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
// Update family dr. Set referrer to family dr if referrer is NIL.
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::FamilyDrButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    if( FamilyDrButton->Enabled == FALSE ) return;

    // Put up the doctor list form
    docListInfo.mode = PMC_LIST_MODE_SELECT;
    docListInfo.doctorId = this->Patient_p->familyDrId( );
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;

    if( docListInfo.returnCode == MB_BUTTON_OK )
    {
        this->Patient_p->familyDrIdSet( docListInfo.doctorId );
        UpdateDoctorEdit( this->Patient_p->familyDrId( ), PMC_PATEDIT_FAM_DR );

        if( this->Patient_p->referringId( ) == 0 )
        {
            this->Patient_p->referringIdSet( docListInfo.doctorId );
            UpdateDoctorEdit( this->Patient_p->referringId( ), PMC_PATEDIT_REF_DR );
        }
    }
}

//---------------------------------------------------------------------------
// Update referring dr.  Set family to referrer if family is NIL.
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ReferringDrButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    if( ReferringDrButton->Enabled == FALSE ) return;

    // Put up the doctor list form
    docListInfo.mode = PMC_LIST_MODE_SELECT;
    docListInfo.doctorId = this->Patient_p->referringId( );
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;

    if( docListInfo.returnCode == MB_BUTTON_OK )
    {
        this->Patient_p->referringIdSet( docListInfo.doctorId );
        UpdateDoctorEdit( this->Patient_p->referringId( ), PMC_PATEDIT_REF_DR );

        if( this->Patient_p->familyDrId( ) == 0 )
        {
            this->Patient_p->familyDrIdSet( docListInfo.doctorId );
            UpdateDoctorEdit( this->Patient_p->familyDrId( ), PMC_PATEDIT_FAM_DR );
        }
    }
}

//---------------------------------------------------------------------------
// This is now obsolete
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PrintRecordButtonClick(TObject *Sender)
{
    pmcPatRecordHTML( this->Patient_p->id() );
}

//---------------------------------------------------------------------------
// Display the consult document with the current patient settings
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PrintDocument( Char_p template_p )
{
    // Need to get the various sections
    pmcPatEditSection_p     section_p;
    PmcDocument            *consult_p = NIL;
    PmcDocument            *deleteConsult_p = NIL;
    qHead_t                 dataQueue;
    qHead_p                 data_q;
    Char_t                  watermark[64];
    Char_t                  updateTime[128];
    Char_t                  updateTimeKey[128];

    // Make a copy of the data
    data_q = qInitialize( &dataQueue );

    qWalk( section_p, this->section_q, pmcPatEditSection_p )
    {
        // Must get the data from the database if necessary.  This could
        // be the case if the record is printed but the dynamic tab was
        // never viewed.
        pmcSqlPatHistoryEntryGet( section_p->last_p );

        // This makes a copy of the data, along with the section key.
        // handle -> date; handle2 -> time
        if( strlen( section_p->last_p->str_p ) == 0 )
        {
            mbLog( "Skipping section (No data): %s", section_p->last_p->str2_p );
            continue;
        }

        mbStrToLatin( section_p->last_p->str_p );

        // Add the value
        mbStrListAddNew( data_q, section_p->last_p->str_p,
                                 section_p->subKey_p,
                                 section_p->last_p->handle,
                                 section_p->last_p->handle2 );
        // Add the update time
        sprintf( updateTimeKey, "%sUPDATE_TIME_", section_p->subKey_p );
        sprintf( updateTime, "%s", this->Patient_p->timeString( section_p->last_p->handle2 ) );
        mbStrListAddNew( data_q, updateTime, updateTimeKey, 0, 0 );

        // Add the update date
        sprintf( updateTimeKey, "%sUPDATE_DATE_", section_p->subKey_p );
        sprintf( updateTime, "%s", this->Patient_p->dateString( section_p->last_p->handle ) );
        mbStrListAddNew( data_q, updateTime, updateTimeKey, 0, 0 );
    }

    sprintf( watermark, "" );

    if(    this->Patient_p->changed( )
        || this->ForceChangeFlag == TRUE
        || this->SectionsModifiedCheck( ) == TRUE )
    {
        sprintf( watermark, "Not Saved" );
    }
    else
    {
        if( mbStrPos( template_p, "consult" ) >= 0 )
        {
            sprintf( watermark, "Preview" );
        }
    }

    if( this->Consult_p )
    {
        consult_p = this->Consult_p;

        if( this->Consult_p->status() == PMC_DOCUMENT_STATUS_PENDING )
        {
            sprintf( watermark, "Pending" );
        }
    }
    else
    {
       Int32u_t    consultDateLetter = this->consultDateLetter;
       Int32u_t    consultDateApp = this->consultDateApp;
       Int32u_t    consultDrId = this->consultDrId;

       // Create a temp, fake consult doc to get provider info into document preview
       consult_p = new PmcDocument( );
       deleteConsult_p = consult_p;

       if( consultDrId == 0 ) consultDrId = this->Patient_p->referringId();
       if( consultDrId == 0 ) consultDrId = this->Patient_p->familyDrId();
       if( consultDateLetter == 0 ) consultDateLetter = mbToday();

       consult_p->appDateSet( consultDateApp );
       consult_p->dateSet( consultDateLetter );
       consult_p->drIdSet( consultDrId );
    }

    // Pass in the patient object to this function
    pmcPatDocumentPDF( this->Patient_p, data_q, watermark, template_p, consult_p, TRUE, NIL );

    // Delete the temp consult object, if required
    if( deleteConsult_p )
    {
        delete deleteConsult_p;
    }

    mbStrListFree( data_q );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PrintRecordNewButtonClick(TObject *Sender)
{
    this->PrintDocument( PMC_PDF_TEMPLATE_PAT_RECORD );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PreviewConsultButtonClick(TObject *Sender)
{
    if( this->ConsultTypeComboBox->ItemIndex == PMC_CONSULT_TYPE_PICKLIST_INDEX_CONSULT )
    {
        this->PrintDocument( PMC_PDF_TEMPLATE_PAT_CONSLT );
    }
    else
    {
        this->PrintDocument( PMC_PDF_TEMPLATE_PAT_FOLLOWUP );
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PrintReqLabelButtonClick(TObject *Sender)
{
    pmcLabelPrintPatReq( 0, 0, this->Patient_p->id() );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PrintAddressLabelButtonClick(TObject *Sender)
{
    pmcLabelPrintPatAddress( this->Patient_p->id() );
}

//---------------------------------------------------------------------------
// Handle title change.  Guess gender based on title
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::TitleListChange(TObject *Sender)
{
    Char_t      buf[32];
    Int32u_t    gender;

    strcpy( buf, TitleList->Text.c_str() );

    if( mbStrPos( buf, "Mrs"  ) >= 0 ||
        mbStrPos( buf, "Ms"   ) >= 0 ||
        mbStrPos( buf, "Miss" ) >= 0  )
    {
        GenderRadio->ItemIndex = 1;
    }
    else if( mbStrPos( buf, "Mr." ) >= 0 ||
             mbStrPos( buf, "Fr" ) >= 0 )
    {
        GenderRadio->ItemIndex = 0;
    }
    gender = ((Int32u_t)GenderRadio->ItemIndex == 0 ) ? PMC_SQL_GENDER_MALE : PMC_SQL_GENDER_FEMALE;
    this->Patient_p->titleSet( TitleList->Text.c_str() );
    this->Patient_p->genderSet( gender );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::DateOfBirthEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    BirthDateButtonClick( Sender );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::DeceasedDateEditMouseDown(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    DeceasedDateButtonClick( Sender );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ReferringDrEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ReferringDrButtonClick( Sender );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::FamilyDrEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    FamilyDrButtonClick( Sender );
}

//---------------------------------------------------------------------------
// Check all sections for modifications.  Return TRUE if any modified,
// FALSE otherwise
//---------------------------------------------------------------------------
Boolean_t __fastcall TPatientEditForm::SectionsModifiedCheck( void )
{
    pmcPatEditSection_p     section_p;
    Boolean_t               result = FALSE;

    qWalk( section_p, this->section_q, pmcPatEditSection_p )
    {
        if( section_p->modified == TRUE )
        {
            result = TRUE;
            break;
        }
    }
    return result;
}

//---------------------------------------------------------------------------
// Save the sections to the database
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::SectionsSave( void )
{
    pmcPatEditSection_p     section_p;
    Char_p                  stringCleaned_p;
    mbStrList_p             entry_p;
    Char_t                  buf[128];
    MbDateTime              dateTime;

    // This loop determines if one of the "dynamic" tabs is visible
    qWalk( section_p, this->section_q, pmcPatEditSection_p )
    {
        if( section_p->modified == TRUE )
        {
            // Convert any special chars before saving
            mbStrToLatin( section_p->last_p->str_p );

            // Clean the string
            mbMalloc( stringCleaned_p, strlen( section_p->last_p->str_p ) + 10 );
            mbStrClean( section_p->last_p->str_p, stringCleaned_p, FALSE );

            // Do the actual save
            if( pmcSqlPatHistoryPut( this->Patient_p->id(), section_p->type, stringCleaned_p ) == 0 )
            {
                mbDlgError( "Error updating patient history" );
            }
            section_p->modified = FALSE;

            // Record the update time
            section_p->last_p->handle = mbToday();
            section_p->last_p->handle2 = mbTime();

            dateTime.SetDateTime( mbToday(), mbTime() );
            sprintf( buf, "%s %s",  dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );
            mbRemallocStr( section_p->last_p->str2_p, buf );

            if( strlen( section_p->orig_p ) == 0 )
            {
                // This is the first entry ever in this section
                mbRemallocStr( section_p->last_p->str_p, stringCleaned_p );
            }
            else
            {
                // This is an updated section

                // Put the original cleaned string into the data section
                mbRemallocStr( section_p->last_p->str_p, section_p->orig_p );

                // Allocate a new entry
                mbMalloc( entry_p, sizeof(mbStrList_t) );
                mbMallocStr( entry_p->str_p, stringCleaned_p );
                mbMallocStr( entry_p->str2_p, buf );

                entry_p->handle = 0;
                entry_p->handle2 = 0;

                section_p->last_p = entry_p;
                section_p->current_p = entry_p;

                // Put the new entry at the end of the list
                qInsertLast( section_p->item_q, entry_p );
            }

            // Update the "new" original string
            mbRemallocStr( section_p->orig_p, stringCleaned_p );

            mbFree( stringCleaned_p );

            // In case time on current display changed
            this->SectionDisplayUpdate( this->ActiveSection_p );
        }
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PageControlChange(TObject *Sender)
{
    pmcPatEditSection_p     section_p;
    Boolean_t               found = FALSE;

    // This loop determines if one of the "dynamic" tabs is visible
    qWalk( section_p, this->section_q, pmcPatEditSection_p )
    {
        if( section_p->tabSheet_p == PageControl->ActivePage )
        {
            // Found that one of the dynamic tabs has become visible
            found = TRUE;
            break;
        }
    }

    if( found == FALSE )
    {
        this->ActiveSection_p = NIL;
    }
    else
    {
        this->ActiveSection_p = section_p;
        this->SectionDisplayUpdate( section_p );
        section_p->memo_p->SetFocus( );
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::SectionDisplayUpdate( pmcPatEditSection_p section_p )
{
    Int32u_t gotFlag;

    // Get the history
    gotFlag = pmcSqlPatHistoryEntryGet( section_p->current_p );

    mbStrToLatin( section_p->last_p->str_p );

    // Enable/disable Next button
    if( section_p->current_p == section_p->last_p )
    {
        section_p->buttonNext_p->Enabled = FALSE;

        if( this->PatEditInfo_p->mode == PMC_EDIT_MODE_VIEW )
        {
            section_p->memo_p->Color = clInfoBk;
            section_p->memo_p->ReadOnly = TRUE;
        }
        else
        {
            section_p->memo_p->Color = clWindow;
            section_p->memo_p->ReadOnly = FALSE;
        }

        // Are on the most recent version, save it for detecting changes
        if( strlen( section_p->orig_p ) == 0 && gotFlag == TRUE )
        {

            // For detecting changes
            mbRemallocStr( section_p->orig_p, section_p->last_p->str_p );
        }
    }
    else
    {
        section_p->buttonNext_p->Enabled = TRUE;
        section_p->memo_p->Color = clInfoBk;
        section_p->memo_p->ReadOnly = TRUE;
    }

    // Enable/disable prev button
    if( section_p->current_p == section_p->first_p )
    {
        section_p->buttonPrev_p->Enabled = FALSE;
    }
    else
    {
        section_p->buttonPrev_p->Enabled = TRUE;
    }

    // Finally, display the text.  Do last to avoid change before orig string set
    section_p->memo_p->Text = section_p->current_p->str_p;
    section_p->memo_p->Modified = TRUE;
    section_p->updateEdit_p->Text = section_p->current_p->str2_p;
}

//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::Memo_medicationsChange(TObject *Sender)
{
#if 0
    pmcMedEntry_p   med_p;
    TListItem      *item_p;
    Int32u_t        matchCount = 0;
    Char_t          search[256];

    mbStrClean( Memo_medications->Text.c_str(), search, TRUE );
    pmcUpperCase( search );

    ListView_Suggestions->Items->BeginUpdate();
    ListView_Suggestions->Items->Clear();

    qWalk( med_p, pmcDBMed_q, pmcMedEntry_p )
    {
        // Record first item in the list
        if( mbStrPos( med_p->brand_p, search ) >= 0 )
        {
            item_p = ListView_Suggestions->Items->Add( );
            item_p->Caption = med_p->brand_p;
            if( ++matchCount > 50 ) break;
        }
    }
    ListView_Suggestions->Items->EndUpdate();
#endif
}
#if 0
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ListView_SuggestionsDblClick(
      TObject *Sender)
{
    mbDlgInfo( "got double click" );
}
#endif

//---------------------------------------------------------------------------
void __fastcall   TPatientEditForm::SectionMemoChange(TObject *Sender)
{
    pmcPatEditSection_p section_p;

    section_p = this->ActiveSection_p;

    if( section_p->current_p == section_p->last_p )
    {
        // Copy the memo text to the section structure
        mbRemallocStr( section_p->last_p->str_p, section_p->memo_p->Text.c_str() );
        mbStrToLatin( section_p->last_p->str_p );

        section_p->last_p->handle = mbToday();
        section_p->last_p->handle2 = mbTime();

        // Compare the original input string to its current value...
        if( strcmp( section_p->orig_p, section_p->last_p->str_p ) == 0 )
        {
            section_p->modified = FALSE;
        }
        else
        {
            section_p->modified = TRUE;
        }
        BannerUpdate( );
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::SectionButtonPrevClick( TObject *Sender )
{
    pmcPatEditSection_p section_p;

    section_p = this->ActiveSection_p;

    // Get previous item in linked list of histories.  Assume based on the fact
    // that the prev button was enabled that there *is* a prev entry
    section_p->current_p = (mbStrList_p)section_p->current_p->linkage.blink;
    this->SectionDisplayUpdate( section_p );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::SectionButtonNextClick( TObject *Sender )
{
    pmcPatEditSection_p section_p;

    section_p = this->ActiveSection_p;

    // Get next item in linked list of histories.  Assume based on the fact
    // that the next button was enabled that there *is* a prev entry
    section_p->current_p = (mbStrList_p)section_p->current_p->linkage.flink;
    this->SectionDisplayUpdate( section_p );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::SectionButtonHelpClick( TObject *Sender )
{
    mbDlgInfo( "To create a heading:\n"
               "\n"
               "        Underline the heading with at least three dashes.  The heading text,\n"
               "        and the dashes, must be at the beginning of the line.\n"
               "\n"
               "                This is a heading\n"
               "                ---\n"
               "\n"
               "To create a bullet list:\n"
               "\n"
               "        Begin the line with a single dash followed by a space, then the text.\n"
               "\n"
               "                - This is a bullet\n"
               "                - This is another bullet\n"
               "\n"
               "To create a numbered list:\n"
               "\n"
               "        Begin the line with '1: ', or '1. ', followed by the text. Don't worry about\n"
               "        the actual point numbers, they are computed automatically.\n"
               "\n"
               "                1: this is numbered point 1\n"
               "                1: this is numbered point 2\n"
               "\n"
               "Special Characters:\n"
               "\n"
               "                ^0  -  degrees (e.g., 360^0)\n"
               "                ^a  -  alpha\n"
               "                ^b  -  beta\n"
               );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::GenderRadioClick(TObject *Sender)
{
    Int32u_t gender = ((Int32u_t)GenderRadio->ItemIndex == 0 ) ? PMC_SQL_GENDER_MALE : PMC_SQL_GENDER_FEMALE;
    this->Patient_p->genderSet( gender );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
// Save the record.  It could be new
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::SaveButtonClick(TObject *Sender)
{
    if( this->Patient_p->save( ) == TRUE )
    {
        Char_t buf[32];
        SaveButton->Enabled = FALSE;
        this->ForceChangeFlag = FALSE;

        // Save the sections... won't do anything if the sections not modified
        this->SectionsSave( );

        // This catches the case where a new record was first saved
        sprintf( buf, "%d", this->Patient_p->id( ) );
        IdEdit->Text = buf;
        PatEditInfo_p->patientId = this->Patient_p->id( );
    }
    else
    {
        SaveButton->Enabled = TRUE;
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::MiddleNameEditChange(TObject *Sender)
{
    this->Patient_p->middleNameSet( MiddleNameEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::Address1EditChange(TObject *Sender)
{
    this->Patient_p->address1Set( Address1Edit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::Address2EditChange(TObject *Sender)
{
    this->Patient_p->address2Set( Address2Edit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ComboBoxCityChange(TObject *Sender)
{
    this->Patient_p->citySet( ComboBoxCity->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ProvListChange(TObject *Sender)
{
    this->Patient_p->provinceSet( ProvList->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PostalCodeEditChange(TObject *Sender)
{
    this->Patient_p->postalCodeSet( PostalCodeEdit->Text.c_str() );
    this->ForceChangeFlag = TRUE;
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PostalCodeEditExit(TObject *Sender)
{
    this->Patient_p->postalCodeSet( PostalCodeEdit->Text.c_str() );
    PostalCodeEdit->Text = this->Patient_p->postalCode( );
    BannerUpdate( );
}
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::CountryEditChange(TObject *Sender)
{
    this->Patient_p->countrySet( CountryEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::HomePhoneEditChange(TObject *Sender)
{
    this->Patient_p->phoneHomeSet( HomePhoneEdit->Text.c_str() );
    this->ForceChangeFlag = TRUE;
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::HomePhoneEditExit(TObject *Sender)
{
    this->Patient_p->phoneHomeSet( HomePhoneEdit->Text.c_str() );
    HomePhoneEdit->Text =    Patient_p->phoneHome( );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::WorkPhoneEditChange(TObject *Sender)
{
    this->Patient_p->phoneWorkSet( WorkPhoneEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::WorkPhoneEditExit(TObject *Sender)
{
    this->Patient_p->phoneWorkSet( WorkPhoneEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::CellPhoneEditChange(TObject *Sender)
{
    this->Patient_p->phoneCellSet( CellPhoneEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::CellPhoneEditExit(TObject *Sender)
{
    this->Patient_p->phoneCellSet( CellPhoneEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::EmailEditChange(TObject *Sender)
{
    this->Patient_p->emailAddressSet( EmailEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::WorkDescEditChange(TObject *Sender)
{
    this->Patient_p->workDescSet( WorkDescEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ContactNameEditChange(TObject *Sender)
{
    this->Patient_p->contactSet( ContactNameEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ContactPhoneEditChange(TObject *Sender)
{
    this->Patient_p->contactPhoneSet( ContactPhoneEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ContactDescEditChange(TObject *Sender)
{
    this->Patient_p->contactDescSet( ContactDescEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::CommentEditChange(TObject *Sender)
{
    this->Patient_p->commentSet( CommentEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::LastNameEditChange(TObject *Sender)
{
    this->Patient_p->lastNameSet( LastNameEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::FirstNameEditChange(TObject *Sender)
{
    this->Patient_p->firstNameSet( FirstNameEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PhnEditChange(TObject *Sender)
{
    this->Patient_p->phnSet( PhnEdit->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PhnEditExit(TObject *Sender)
{
    this->Patient_p->phnSet( PhnEdit->Text.c_str() );
    PhnEdit->Text = this->Patient_p->phn( );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PhnProvListChange(TObject *Sender)
{
    this->Patient_p->phnProvinceSet( PhnProvList->Text.c_str() );
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PhnProvListExit(TObject *Sender)
{
    this->Patient_p->phnProvinceSet( PhnProvList->Text.c_str() );
    PhnProvList->Text = this->Patient_p->phnProvince( );
    BannerUpdate( );
}

//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::ReferringDrEditChange(TObject *Sender)
{
    BannerUpdate( );
}
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::FamilyDrEditChange(TObject *Sender)
{
    BannerUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::DocumentsButtonClick(TObject *Sender)
{
    TDocumentListForm          *form_p;
    pmcDocumentListInfo_t       formInfo;

    formInfo.patientId = this->Patient_p->id( );
    form_p = new TDocumentListForm( NIL, &formInfo );
    form_p->ShowModal();
    delete form_p;
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::LastConsultButtonClick(TObject *Sender)
{
    Int32u_t        id;

    // This should find a word or PDF consult letter
    id =  pmcConsultFindLastFiled( this->Patient_p->id( ) );

    if( id == 0 )
    {
        mbDlgInfo( "No consult letters found for this patient." );
    }
    else
    {
        PmcDocument document = PmcDocument( id );
        document.view( );
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::AppointmentsButtonClick(TObject *Sender)
{
    pmcViewAppointments( this->Patient_p->id( ), TRUE, TRUE,
        FALSE, 0, 0, TRUE, PMC_LIST_MODE_LIST );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::NewWordDocButtonClick(TObject *Sender)
{
    pmcWordCreate( this->Patient_p->id( ), 0 );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ClaimsButtonClick(TObject *Sender)
{
    pmcClaimListInfo_t      claimListInfo;
    TClaimListForm         *claimListForm_p;
    MbDateTime              dateTime;

    claimListInfo.patientId = this->Patient_p->id( );

    dateTime.SetDate( mbToday( ) );

    claimListInfo.latestDate = mbToday( );
    claimListInfo.earliestDate = dateTime.BackYears( 10 );

    claimListInfo.providerId = 0;
    claimListInfo.providerAllFlag = TRUE;
    claimListInfo.patientAllFlag = FALSE;

    claimListForm_p = new TClaimListForm( NULL, &claimListInfo );
    claimListForm_p->ShowModal();
    delete claimListForm_p;
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::EchosButtonClick(TObject *Sender)
{
    pmcEchoListForm( this->Patient_p->id( ) );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::NewConsultButtonClick(TObject *Sender)
{
    Int32u_t    documentId;
    MbString    desc;
    Char_p      letterName_p;
    Int32u_t    type;

    if( this->Consult_p == NIL )
    {
        if( this->ConsultTypeComboBox->ItemIndex == PMC_CONSULT_TYPE_PICKLIST_INDEX_CONSULT )
        {
            type = PMC_DOCUMENT_TYPE_CONSLT;
            letterName_p = "Consult Letter";
        }
        else
        {
            type = PMC_DOCUMENT_TYPE_FOLLOWUP;
            letterName_p = "Followup Letter";
        }

        documentId =  pmcConsultNew( this->Patient_p->id( ), type );
        this->Consult_p = new PmcDocument( documentId );

        if( this->Consult_p->lock( TRUE ) != TRUE )
        {
            mbDlgExclaim( "Failed to lock the consult document for editing" );
        }
        else
        {
            mbSprintf( &desc, "%s %s %s", this->Patient_p->firstName(),
                                          this->Patient_p->lastName(),
                                          letterName_p );
                                          
            this->Consult_p->descriptionSet( desc.get() );
            this->Consult_p->statusSet( PMC_DOCUMENT_STATUS_ACTIVE );
            this->Consult_p->dateSet( mbToday() );

            mbLog( "Created new consult document: ID: %lu desc: '%s'\n",
                this->Consult_p->id(), this->Consult_p->description() );

            if( this->Patient_p->referringId() == 0 )
            {
                this->Consult_p->drIdSet( this->Patient_p->familyDrId() );
            }
            else
            {
                this->Consult_p->drIdSet( this->Patient_p->referringId() );
            }
            
            this->Consult_p->providerIdSet( this->Patient_p->providerId() );
            this->Consult_p->save( );
        }
    }
    else
    {
        if( mbDlgYesNo( "Are you sure you want to delete the active consult?\n\n"
                        "The consult will no longer appear in the documents list,\n"
                        "but can be re-created at any time." ) == MB_BUTTON_YES )
        {
            mbLog( "Deleting consult id:  %lu status: %s\n",
                Consult_p->id(), pmcDocumentStatusString( Consult_p->status() ) );

            pmcSqlRecordDelete( PMC_SQL_TABLE_DOCUMENTS, Consult_p->id() );
            delete this->Consult_p;
            this->Consult_p = NIL;
        }
    }
    ConsultControlsUpdate( );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::NewClaimButtonClick(TObject *Sender)
{
    pmcClaimEditInfo_t    claimEditInfo;
    TClaimEditForm       *claimEditForm;

    claimEditInfo.mode = PMC_CLAIM_EDIT_MODE_NEW;
    claimEditInfo.patientId = this->Patient_p->id();
    claimEditInfo.providerId = this->Patient_p->providerId();
    claimEditForm = new TClaimEditForm( this, &claimEditInfo );
    claimEditForm->ShowModal();
    delete claimEditForm;
}

//---------------------------------------------------------------------------
// This button is dual purpose.  It serves to submit the document for
// approval, and reactivate the document if necessary
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::SubmitConsultButtonClick(TObject *Sender)
{
    Int32s_t    result;

    // First, sanity check the consult
    if( this->Consult_p == NIL )
    {
        mbDlgError( "Cannot find consult object" );
        goto exit;
    }

    if( this->Consult_p->status( ) == PMC_DOCUMENT_STATUS_PENDING )
    {
        mbLog( "Re-activate document %ld\n", this->Consult_p->id( ) );

        this->Consult_p->statusSet( PMC_DOCUMENT_STATUS_ACTIVE );
        this->Consult_p->save( );

        // Put the pat record into its original requested mode
        //this->PatEditInfo_p->mode = this->requestedMode;
        this->PatEditInfo_p->mode = this->obtainedMode;

        ConsultControlsUpdate( );
        ControlsUpdate( );
        
        goto exit;
    }

    if( this->Consult_p->status( ) != PMC_DOCUMENT_STATUS_ACTIVE )
    {
        mbDlgError( "Consult is not active" );
        goto exit;
    }

    // Save consult document to database if necessary
    if( this->Consult_p->changed() )
    {
        this->Consult_p->save();
    }

    if(    this->Patient_p->changed( )
        || this->ForceChangeFlag == TRUE
        || this->SectionsModifiedCheck( ) == TRUE )
    {
        result = mbDlgYesNo( "The patient record has been modified.\n\n"
                             "It must be saved to the database before\n"
                             "the consult can be submitted for approval.\n\n"
                             "Would you like to save the patient record now?" );

        if( result == MB_BUTTON_NO )
        {
            mbDlgInfo( "Consult remains active." );
            goto exit;
        }

        if( this->Patient_p->save( ) == FALSE )
        {
            goto exit;
        }

        this->SectionsSave( );

        SaveButton->Enabled = FALSE;
    }

    result = mbDlgYesNo( "The patient record will be locked until the consult is approved and filed.\n\n"
                         "Proceed?" );

    if( result == MB_BUTTON_NO )
    {
        mbDlgInfo( "Consult remains active." );
        goto exit;
    }

    // Change the document to pending
    this->Consult_p->statusSet( PMC_DOCUMENT_STATUS_PENDING );
    this->Consult_p->save( );

    // Put the pat record into view only mode
    this->PatEditInfo_p->mode = PMC_EDIT_MODE_VIEW;

    ConsultControlsUpdate( );
    ControlsUpdate( );

exit:
    return;
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ConsultDrButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    if( ConsultDrButton->Enabled == FALSE ) return;

    // Sanity check... should not happen
    if( this->Consult_p == NIL )
    {
        mbDlgError( "Cannot find consult object" );
        return;
    }

    // Put up the doctor list form
    docListInfo.mode = PMC_LIST_MODE_SELECT;
    docListInfo.doctorId = this->Consult_p->drId( );
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;

    if( docListInfo.returnCode == MB_BUTTON_OK )
    {
        this->Consult_p->drIdSet( docListInfo.doctorId );
        UpdateDoctorEdit( this->Consult_p->drId( ), PMC_PATEDIT_CONSULT_DR );
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ConsultLetterDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    if( ConsultLetterDateButton->Enabled == FALSE ) return;

    // Sanity check... should not happen
    if( this->Consult_p == NIL )
    {
        mbDlgError( "Cannot find consult object." );
        return;
    }

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NULL;
    dateSelectInfo.dateIn = this->Consult_p->date( );
    dateSelectInfo.caption_p = "Select Consult Letter Date";

    // Show the date selection form
    dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        this->Consult_p->dateSet( dateSelectInfo.dateOut );
        this->ConsultLetterDateEdit->Text = this->Patient_p->dateString( this->Consult_p->date( ) );
        BannerUpdate( );
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ConsultAppDateButtonClick(TObject *Sender)
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;

    if( ConsultAppDateButton->Enabled == FALSE ) return;

    // Sanity check... should not happen
    if( this->Consult_p == NIL )
    {
        mbDlgError( "Cannot find consult object." );
        return;
    }

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;
    dateSelectInfo.string_p = NULL;
    dateSelectInfo.dateIn = this->Consult_p->appDate( );
    dateSelectInfo.caption_p = "Select Appointment Date";

    // Show the date selection form
    dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        this->Consult_p->appDateSet( dateSelectInfo.dateOut );
        this->ConsultAppDateEdit->Text = this->Patient_p->dateString( this->Consult_p->appDate( ) );
        BannerUpdate( );
    }
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ConsultAppDateEditMouseDown(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ConsultAppDateButtonClick( Sender );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ConsultLetterDateEditMouseDown(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ConsultLetterDateButtonClick( Sender );
}

//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::ConsultDrEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    ConsultDrButtonClick( Sender );
}

//---------------------------------------------------------------------------
// This function sets the size of the dynamically added memo fields.  It is
// required becase I cannot get the anchors to work correctly for these
// memo fields.
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::MemoSize( TMemo *memo_p )
{
    memo_p->Height = this->PageControl->Height - 80;
    memo_p->Width = this->PageControl->Width - 25;
}

//---------------------------------------------------------------------------
// This function is called when the tab sheet is resized.  It is required to
// resize the dynamically added memo fields, which I cannot seem to resize
// via the anchor fields.
//---------------------------------------------------------------------------
void __fastcall TPatientEditForm::PageControlResize(TObject *Sender)
{
    pmcPatEditSection_p section_p;

    if( this->section_q == NIL ) return;

    qWalk( section_p, this->section_q, pmcPatEditSection_p )
    {
        this->MemoSize( section_p->memo_p );
    }
}
//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::ApproveConsultButtonClick(TObject *Sender)
{
    if( this->Consult_p == NIL )
    {
        mbDlgError( "Consult record not found" );
        return;
    }

    if( this->Consult_p->import( ) == MB_RET_OK )
    {
        // Restore original patient mode
        this->PatEditInfo_p->mode = this->obtainedMode;

        ConsultControlsUpdate( );
        ControlsUpdate( );

        mbDlgInfo( "Consult approved and filed." );
    }
    else
    {
        mbDlgInfo( "Consult not filed." );
    }
}
//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::ConsultTypeComboBoxChange(TObject *Sender)
{
    if( this->Consult_p != NIL )
    {
        Int32u_t    type;
        MbString    desc;
        Char_p      letterName_p;

        if( this->ConsultTypeComboBox->ItemIndex == PMC_CONSULT_TYPE_PICKLIST_INDEX_CONSULT )
        {
            type = PMC_DOCUMENT_TYPE_CONSLT;
            letterName_p = "Consult Letter";
        }
        else
        {
            type = PMC_DOCUMENT_TYPE_FOLLOWUP;
            letterName_p = "Followup Letter";
        }

        mbSprintf( &desc, "%s %s %s", this->Patient_p->firstName(),
                                      this->Patient_p->lastName(),
                                      letterName_p );

        this->Consult_p->typeSet( type );
        this->Consult_p->descriptionSet( desc.get() );
        BannerUpdate();
    }
}
//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::DateOfBirthEditClick(TObject *Sender)
{
    BirthDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::DateOfBirthEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    BirthDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::DeceasedDateEditClick(TObject *Sender)
{
    DeceasedDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TPatientEditForm::DeceasedDateEditKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    DeceasedDateButtonClick( Sender );
}
//---------------------------------------------------------------------------

