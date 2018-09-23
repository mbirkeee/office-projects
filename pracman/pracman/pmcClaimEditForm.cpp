//---------------------------------------------------------------------------
// File:    pmcClaimEditForm.c
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 19, 2001
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcClaimEditForm.h"
#include "pmcDateSelectForm.h"
#include "pmcUtils.h"
#include "pmcPatientListForm.h"
#include "pmcPatientEditForm.h"
#include "pmcDoctorListForm.h"
#include "pmcDoctorEditForm.h"
#include "pmcTables.h"
#include "pmcMsp.h"
#include "pmcIcdForm.h"
#include "pmcInitialize.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

__fastcall TClaimEditForm::TClaimEditForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
// Function: TClaimEditForm( )
//---------------------------------------------------------------------------
// Description:
//
// Constructor
//---------------------------------------------------------------------------

__fastcall TClaimEditForm::TClaimEditForm
(
    TComponent             *Owner,
    pmcClaimEditInfo_p      claimEditInfo_p
)
    : TForm(Owner)
{
    Int32u_t    i;
    Char_p      buf_p;

    mbMalloc( buf_p, 128 );

    UpdateClaimStatus = TRUE;
    Active = FALSE;
    ClaimHeaderLocked = FALSE;

    PatientTitle[0] = 0;

    MouseDownIndex = 0;

    PatientPhn_p = NIL;
    PatientPhnProv_p = NIL;

    // Sanity Check
    if( claimEditInfo_p == NIL )
    {
        mbDlgDebug(( "claimEditInfo_p == NIL" ));
        Close( );
    }

    ClaimEditInfo_p = claimEditInfo_p;
    ClaimEditInfo_p->claimId = 0;
    ClaimEditInfo_p->updateClaims = FALSE;
    ClaimEditInfo_p->returnCode = MB_BUTTON_CANCEL;

    ProviderId = claimEditInfo_p->providerId;
    PatientId = claimEditInfo_p->patientId;

    // Set the caption
    switch( ClaimEditInfo_p->mode )
    {
        case PMC_CLAIM_EDIT_MODE_VIEW:
            Caption = "View Claim";
            OkButton->Visible = FALSE;
            CancelButton->Caption = "Close";
            break;

        case PMC_CLAIM_EDIT_MODE_EDIT:
            Caption = "Edit Claim";
            break;

        case PMC_CLAIM_EDIT_MODE_NEW:
            Caption = "New Claim";
            break;
    }

    ProviderNumber = 0;
    ReferringDrType = 0;
    ReferringDrTypeIndex = 0;
    ReferringDrNumber = 0;
    ReferringDrId = 0;

    // Get patients default provider
    if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_NEW )
    {
        if( PatientId != 0 && ProviderId == 0 )
        {
            sprintf( buf_p, "select %s from %s where %s=%ld",
                PMC_SQL_FIELD_PROVIDER_ID,
                PMC_SQL_TABLE_PATIENTS,
                PMC_SQL_FIELD_ID, PatientId );
            ProviderId = pmcSqlSelectInt( buf_p, NIL );
        }

        // Get Patients default referring dr
        if( PatientId != 0 && ReferringDrId == 0 )
        {
            sprintf( buf_p, "select %s from %s where %s=%ld",
                PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID,
                PMC_SQL_TABLE_PATIENTS,
                PMC_SQL_FIELD_ID, PatientId );
            ReferringDrId = pmcSqlSelectInt( buf_p, NIL );
        }
    }

    if( ProviderId )
    {
        sprintf( buf_p, "select %s from %s where %s=%ld",
            PMC_SQL_PROVIDERS_FIELD_PROVIDER_NUMBER,
            PMC_SQL_TABLE_PROVIDERS,
            PMC_SQL_FIELD_ID, ProviderId );

        ProviderNumber = pmcSqlSelectInt( buf_p, NIL );
    }

    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        ClaimId[i] = 0;
        Units[i] = 0;
        FeeMultiple[i] = TRUE;
        FeeHigh[i] = 0;
        FeeLow[i] = 0;
        FeeOverride[i] = 0;
        FeeIndex[0] = 0;
        FeeDeterminant[i] = PMC_FEE_DET_INVALID;
        SubmittedDate[i] = 0;
        FeePaid[i] = 0;
        PremPaid[i] = 0;
        ReplyDate[i] = 0;
        SubmitCount[i] = 0;
        UnitsPaid[i] = 0;
        SeqNumber[i] = -1;
        Date[i] = 0;
        Status[i] = PMC_CLAIM_STATUS_NONE;
        StatusIn[i] = PMC_CLAIM_STATUS_NONE;
        ReplyStatus[i] = Status[i];
        Exists[i] = FALSE;
        ExpCode[i][0] = 0;
        FeeCodeApproved[i][0] = 0;
        OrigFeeCode[i][0] = 0;

        if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_NEW )
        {
            if( pmcDefaultServiceDate[i] != 0 )
            {
                Date[i] = pmcDefaultServiceDate[i];
            }
        }
    }

    Date[ PMC_CLAIM_COUNT ] = 0;
    Date[ PMC_CLAIM_COUNT + 1 ] = 0;

    InitializeControls( );

    if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_EDIT ||
        ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_VIEW )
    {
        ClaimsRead( );

        PageControl->ActivePage = EditStatusSheet;

        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( ReplyDate[i] ) PageControl->ActivePage = SubmitStatusSheet;
        }
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( strlen( MspCommentLabel[i]->Caption.c_str() ) > 0 ) PageControl->ActivePage = MspCommentSheet;
        }
    }
    else
    {
        PageControl->ActivePage = CommentsSheet;
    }

    if( ClaimNumber == 0 ) ClaimNumber = PMC_CLAIM_NUMBER_NOT_ASSIGNED;

    ProgramControls( );
    PatientUpdate( PatientId );
    ReferringDrUpdate( ReferringDrId, ReferringDrTypeIndex );

    // Set the location text
    LocationListBox->Text = pmcNewLocations[LocationListBox->ItemIndex].description_p;

    Active = TRUE;
    TotalFeeChargedEditUpdate( );

    if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_NEW )
    {
        SetIcd( pmcDefaultIcdCode_p );
        for( Ints_t i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( strlen( pmcDefaultClaimCode_p[i] ) == 0 ) continue;

            FeeList[i]->Text = pmcDefaultClaimCode_p[i];
            FeeListChange( i );
        }
    }

    // Must check the claim status and update the ready/not ready; i.e., an invalid
    // phn may have been fixed.  This must be done even if the user ends up
    // clicking cancel
    UpdateClaimStatus = TRUE;
    ClaimsValidate( );
    UpdateClaimStatus = FALSE;

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function: OkButtonClick( )
//---------------------------------------------------------------------------
// Description:
//
// This is going to be really ugly because of the brain damaged way that
// MCIB has specified the claims be submitted.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::OkButtonClick(TObject *Sender)
{
    Int32u_t    closeFlag = FALSE;
    Char_p      buf1_p;
    Char_p      buf2_p;
    Char_p      buf3_p;
    Char_p      buf4_p;
    Int32u_t    i, j;
    Int32u_t    updateClaimId[PMC_CLAIM_COUNT];
    Int32u_t    earliestDate = 99999999;
    Int32u_t    latestDate = 0;
    Int32u_t    updateStatus;
    Int32u_t    replyStatus;
    Int32u_t    editCount;
    bool        foundFlag;

    mbMalloc( buf1_p, 2048 );
    mbMalloc( buf2_p, 256 );
    mbMalloc( buf3_p, 128 );
    mbMalloc( buf4_p, 64 );

    ClaimEditInfo_p->returnCode = MB_BUTTON_OK;

    if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_VIEW )
    {
        closeFlag = TRUE;
        goto exit;
    }

    /* Check the status of the claims */
    ClaimsValidate( );

    editCount = ReadyCount + NotReadyCount;

    if( ReadyCount > 0 && NotReadyCount > 0 )
    {
        mbDlgInfo( "Cannot save \"Ready\" and \"Not Ready\" claims on same form.\n"
                   "Please fix all \"Not Ready\" claims before saving." );
        goto exit;
    }

    if( ProviderId == 0 )
    {
        mbDlgInfo( "A provider must be specified to save claims." );
        goto exit;
    }

    if( PatientId == 0 )
    {
        mbDlgInfo( "A patient must be specified to save claims." );
        goto exit;
    }
    else
    {
        Int32u_t    patientProviderId;
        Int32u_t    count;

        sprintf( buf1_p, "select %s from %s where %s=%ld",
            PMC_SQL_FIELD_PROVIDER_ID,
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_ID, PatientId );

        patientProviderId = pmcSqlSelectInt( buf1_p, &count );

        // Sanity check
        if( count != 1 ) mbDlgDebug(( "Error reading patient provider ID" ));

        if( patientProviderId != ProviderId )
        {
            pmcProviderDescGet( patientProviderId, buf1_p );
            pmcProviderDescGet( ProviderId, buf2_p );

            if( editCount )
            {
                if( mbDlgOkCancel( "Patient's default provider (%s) does not match claim's provider (%s).\n"
                                   "Continue saving claim?", buf1_p, buf2_p ) == MB_BUTTON_CANCEL )
                {
                    goto exit;
                }
            }
        }

        if( PatientDob )
        {
            Int32u_t    today = mbToday( );
            if( PatientDob > today )
            {
                mbDlgExclaim( "Patient's date of birth is invalid." );
                goto exit;
            }
            else
            {
                if( ( today - PatientDob ) < 10000 )
                {
                    if( editCount )
                    {
                        if( mbDlgOkCancel( "Patient is less than one year of age.  Continue saving claim?" ) == MB_BUTTON_CANCEL )
                        {
                            goto exit;
                        }
                    }
                }
            }
        }

        if( strlen( PatientTitle ) )
        {
            Int32s_t    gender = -1;
            bool        suspect = FALSE;

            if( PatientGenderLabel->Caption == 'M' ) gender = 0;
            if( PatientGenderLabel->Caption == 'F' ) gender = 1;

            if( gender == 0 )
            {
                if( mbStrPos( PatientTitle, "Mrs"  ) >= 0 ||
                    mbStrPos( PatientTitle, "Ms"   ) >= 0 ||
                    mbStrPos( PatientTitle, "Miss" ) >= 0  )
                {
                    suspect = TRUE;
                }
            }
            else if( gender == 1 )
            {
                if( mbStrPos( PatientTitle, "Mr."  ) >= 0)
                {
                    suspect = TRUE;
                }
            }
            else
            {
                suspect = TRUE;
            }
            if( suspect == TRUE )
            {
                mbDlgExclaim( "Possible invalid gender setting.  Check patient's title and gender." );
                goto exit;
            }
        }
    }

    // Do nothing if there appear to be no claims to save
    if( ClaimCount == 0 )
    {
        if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_NEW )
        {
            mbDlgInfo( "No claims to save.  Enter claims or click cancel." );
            goto exit;
        }
        else if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_EDIT )
        {
            if( mbDlgOkCancel( "Are you sure you want to clear all entries from this claim?\n" ) == MB_BUTTON_CANCEL )
            {
                goto exit;
            }
            else
            {
                for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
                {
                    // Do not need to create records that already exist
                    if( ClaimId[i] )
                    {
                        // Entry exits - can it be deleted?
                        if( Status[i] == PMC_CLAIM_STATUS_NONE )
                        {
                            mbLog( "Deleting claim id: %ld\n", ClaimId[i] );
                            // We can get rid of this entry if desired
                            sprintf( buf1_p, "update %s set %s=%ld where %s=%ld",
                            PMC_SQL_TABLE_CLAIMS,
                            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE,
                            PMC_SQL_FIELD_ID, ClaimId[i] );
                            pmcSqlExec( buf1_p );
                            ClaimId[i] = 0;
                        }
                    }
                }
                closeFlag = TRUE;
                goto exit;
            }
        }
        else
        {
            goto exit;
        }
    }

    // Do not allow a claim to be created with no service date (will not show up in lists)
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if( Status[i] != PMC_CLAIM_STATUS_NONE && Date[i] == 0 )
        {
            mbDlgInfo( "Cannot create claim without date of service specified.\n" );
            goto exit;
        }
    }

    if( ProviderNumber == ReferringDrNumber )
    {
        mbDlgInfo( "Provider and Referring doctor cannot be the same person." );
        goto exit;
    }

    /* Check for a referring Dr. Not specified is not a fatal error */
    if( ReferringDrId == 0 && editCount )
    {
        if( mbDlgOkCancel( "No referring doctor specified.  Continue saving claim?"  ) != MB_BUTTON_OK )
        {
            goto exit;
        }
    }

    if( PremiumComboBox[0]->ItemIndex != 0 )
    {
        foundFlag = FALSE;
        for( i = 0 ; i < PMC_CLAIM_COUNT  - 2 ; i++ )
        {
            for( j = 0 ; j < PMC_CLAIM_COUNT  - 2 ; j++ )
            {
                if( i != j )
                {
                    if( Date[i] != 0 && Date[j] != 0 && Date[i] != Date[j] )
                    {
                        foundFlag = TRUE;
                        if( mbDlgOkCancel( "This claim will apply a premium to services provided on different dates.\n"
                                              "Continue saving claim?"  ) != MB_BUTTON_OK )
                        {
                            goto exit;
                        }
                        break;
                    }
                }
            }
            if( foundFlag ) break;
        }
    }

    // Next, determine the new claim number
    if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_NEW )
    {
        // Sanity Check
        if( ClaimNumber !=0 && ClaimNumber != PMC_CLAIM_NUMBER_NOT_ASSIGNED )
        {
            mbDlgDebug(( "PMC_CLAIM_EDIT_MODE_NEW and ClaimNumber != 0" ));
            goto exit;
        }

        if( ReadyCount == 0 && NotReadyCount == 0 )
        {
            mbDlgDebug(( "Nothing to do" ));
            goto exit;
        }

        // Now create the claim header structure
        ClaimHeaderId = pmcSqlRecordCreate( PMC_SQL_TABLE_CLAIM_HEADERS, NIL );

        sprintf( buf1_p, "Claim ID %ld for %s created.", ClaimHeaderId, pmcProviderDescGet( ProviderId, buf2_p ) );
        mbDlgInfo( buf1_p );
    }

    // Now create records that need to be created.  Delete unused claims.
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        // Do not need to create records that already exist
        if( ClaimId[i] )
        {
            // Entry exists - can it be deleted?
            if( Status[i] == PMC_CLAIM_STATUS_NONE )
            {
                mbLog( "Deleting claim id: %ld\n", ClaimId[i] );
                // We can get rid of this entry if desired
                sprintf( buf1_p, "update %s set %s=%ld where %s=%ld",
                            PMC_SQL_TABLE_CLAIMS,
                            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE,
                            PMC_SQL_FIELD_ID, ClaimId[i] );

                pmcSqlExec( buf1_p );
                ClaimId[i] = 0;
            }
        }
        else
        {
            // Entry does not exist
            if( Status[i] == PMC_CLAIM_STATUS_READY ||
                Status[i] == PMC_CLAIM_STATUS_NOT_READY )
            {
                // Create the record
                ClaimId[i] = pmcSqlRecordCreate( PMC_SQL_TABLE_CLAIMS, NIL );

                mbLog( "Created claim (Provider ID: %ld Claim id: %ld)\n", ProviderId, ClaimId[i] );

                // Create the claim record with the fixed fields
                //                              Num    Index    Status  Reply   HdrId   Type
                sprintf( buf1_p, "update %s set %s=%ld, %s=%ld, %s=%ld, %s=%ld, %s=%ld, %s=%ld where %s=%ld",
                    PMC_SQL_TABLE_CLAIMS,
                    PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER, PMC_CLAIM_NUMBER_NOT_ASSIGNED,
                    PMC_SQL_CLAIMS_FIELD_CLAIM_INDEX, i,
                    PMC_SQL_CLAIMS_FIELD_STATUS, PMC_CLAIM_STATUS_EDIT,
                    PMC_SQL_CLAIMS_FIELD_REPLY_STATUS, PMC_CLAIM_STATUS_EDIT,
                    PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID, ClaimHeaderId,
                    PMC_SQL_CLAIMS_FIELD_APPOINTMENT_TYPE, ( i > 6 ) ? PMC_CLAIM_TYPE_HOSPITAL : PMC_CLAIM_TYPE_SERVICE,
                    PMC_SQL_FIELD_ID, ClaimId[i] );

                pmcSqlExec( buf1_p );
            }
        }
    }

    // Compute the earliest and latest service dates for all claims
    for( i = 0 ; i <  PMC_CLAIM_COUNT ; i++ )
    {
        if( ClaimId[i] )
        {
            if( Date[i] )
            {
                if( Date[i] >= latestDate ) latestDate = Date[i];

                if( Date[i] <= earliestDate ) earliestDate = Date[i];

                // Save the date to use are default on the next claim
                pmcDefaultServiceDate[i] = Date[i];
            }
        }
        else
        {
            pmcDefaultServiceDate[i] = 0;
        }
    }

    // Update earliest and latest service dates for all records in this claim
    sprintf( buf1_p, "update claims set %s=%ld,%s=%ld where %s=%ld",
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_EARLIEST, earliestDate,
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LATEST, latestDate,
        PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID, ClaimHeaderId );

    pmcSqlExec( buf1_p );

    // Mark as accepted those claims that were accepted
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        updateClaimId[i] = 0;

        if(    Status[i] == PMC_CLAIM_STATUS_REDUCED_ACCEPT
            || Status[i] == PMC_CLAIM_STATUS_REJECTED_ACCEPT )
        {
            // Sanity check
            if( ClaimId[i] == 0 ) mbDlgDebug(( "Error: expecting a claim ID" ));

            if(    ReplyStatus[i] == PMC_CLAIM_STATUS_REDUCED_ACCEPT
                || ReplyStatus[i] == PMC_CLAIM_STATUS_REJECTED_ACCEPT )
            {
                // This claim was already marked as accepted
                updateClaimId[i] = 0;
            }
            else if(    ReplyStatus[i] == PMC_CLAIM_STATUS_REDUCED
                     || ReplyStatus[i] == PMC_CLAIM_STATUS_REJECTED )
            {
                updateClaimId[i] = ClaimId[i];
                updateStatus = Status[i];
                replyStatus = Status[i];
            }
            else
            {
                updateClaimId[i] = 0;
                mbDlgDebug(( "Error: Cannot ACCEPT status %ld", ReplyStatus[i] ));
            }
        }
        else if(    Status[i] == PMC_CLAIM_STATUS_READY
                 || Status[i] == PMC_CLAIM_STATUS_NOT_READY )
        {
            if(    ReplyStatus[i] == PMC_CLAIM_STATUS_REDUCED
                || ReplyStatus[i] == PMC_CLAIM_STATUS_REJECTED )
            {
                // This must be a rejected or reduced claim that is to be resubmitted
                updateClaimId[i] = ClaimId[i];
                updateStatus = PMC_CLAIM_STATUS_EDIT;
                replyStatus = ReplyStatus[i];

                {
                    mbStrClean( CommentEdit[i]->Text.c_str(), buf3_p, TRUE );

                    // Add resubmission comment
//                    if( strlen( buf3_p ) ) strcat( buf3_p, " " );
//                    sprintf( buf2_p, " (Resub of %05ld-%ld)", ClaimNumber, atol( SeqLabel[i]->Caption.c_str( ) ) );
//                    if( mbStrPos( buf3_p, "Resub" ) < 0 )
//                    {
//                        strcat( buf3_p, buf2_p );
//                    }
//                    *(buf3_p+ PMC_MSP_COMMENT_LENGTH - 1) = 0;  // Null terminate at max comment length

                    CommentEdit[i]->Text = buf3_p;
                }
            }
        }

        if( updateClaimId[i] )
        {
            sprintf( buf1_p, "update %s set %s=%ld, %s=%ld where %s=%ld",
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_CLAIMS_FIELD_STATUS, updateStatus,
                PMC_SQL_CLAIMS_FIELD_REPLY_STATUS, replyStatus,
                PMC_SQL_FIELD_ID, updateClaimId[i] );

            nbDlgDebug(( buf1_p ));
            pmcSqlExec( buf1_p );
        }
    }

    // The "referring Dr.", icd code and location code handling is really
    // ugly.  I was told by MSP that all claims in a form would be rejected.
    // This is true but 1 claim could be paid, and another reduced, requiring
    // only a subset of the claims to be resubmitted.  The user may want to
    // change a parm (rf dr, loc, icd) that applies to the whole claim.  The
    // simplest solution is to change all in the claim (even paid claims).
    // I had put these parms in the claims (not the header) for future
    // flexibility... but perhaps they should have gone in the header.

    // Basically, this section of code has evolved into an update of all the
    // fields that are "claim wide"

    mbStrClean( IcdComboBox->Text.c_str(), buf2_p, TRUE );

    // Store the selected ICD code for use as default in next claim
    strcpy( pmcDefaultIcdCode_p, IcdComboBox->Text.c_str());

    //                               refDr  refId   refType Loc     ICD       ProvId
    sprintf( buf1_p, "update %s set %s=%ld, %s=%ld, %s=%ld, %s=%ld, %s=\"%s\" where %s=%ld",
                        PMC_SQL_TABLE_CLAIMS,
                        PMC_SQL_CLAIMS_FIELD_REFERRING_DR_NUM,  ReferringDrNumber,
                        PMC_SQL_CLAIMS_FIELD_REFERRING_DR_ID,   ReferringDrId,
                        PMC_SQL_CLAIMS_FIELD_REFERRING_DR_TYPE, ReferringDrTypeIndex,
                        PMC_SQL_FIELD_PROVIDER_ID,              ProviderId,
                        PMC_SQL_CLAIMS_FIELD_ICD_CODE,          buf2_p,
                        PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID,   ClaimHeaderId );

    nbDlgDebug(( buf1_p ));
    pmcSqlExec( buf1_p );

    // Now save only claimIds of those claims that will be updated
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if( Status[i] == PMC_CLAIM_STATUS_READY ||
            Status[i] == PMC_CLAIM_STATUS_NOT_READY )
        {
            // Sanity check
            if( ClaimId[i] == 0 )
            {
                mbDlgDebug(( "Error: expecting a claim ID" ));
            }
            updateClaimId[i] = ClaimId[i];
        }
        else
        {
            // Will not touch these entries
            updateClaimId[i] = 0;
        }
    }

    // Now update only those records that need to be updated
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        // Erase any saved claim codes
        strcpy( pmcDefaultClaimCode_p[i], "" );

        if( updateClaimId[i] )
        {
            // Return ID of first claim that we encounter (for redrawing the list screen)
            if( ClaimEditInfo_p->claimId == 0 )  ClaimEditInfo_p->claimId = updateClaimId[i];

            mbStrClean( CommentEdit[i]->Text.c_str(),   buf3_p, TRUE );

            // Must put fee code into known format, because of the brain-dead MSP
            // file formats, we must search the database for the fee code.  Hence
            // the known string format is required in the database.
            pmcFeeCodeFormatDatabase( FeeList[i]->Text.c_str(), buf4_p );

            // Save the claim codes for use in the next new claim
            strcpy( pmcDefaultClaimCode_p[i], FeeList[i]->Text.c_str() );
            mbStrToUpper( pmcDefaultClaimCode_p[i] );

            // Only update records with above statuses
            //                               patId  Claim  Date    Date    Loc.    FeeCode
            sprintf( buf1_p, "update %s set %s=%ld, %s=%d, %s=%ld, %s=%ld, %s=%ld, %s=\"%s\", "
                           // Units   Fee     Comment
                           " %s=%ld, %s=%ld, %s=\"%s\", %s=%ld where %s=%ld",
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_FIELD_PATIENT_ID,               PatientId,
                PMC_SQL_CLAIMS_FIELD_SERVICE_TYPE,      ClaimType[i],
                PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,      Date[i],
                PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LAST, ( i > 6 ) ? Date[i+2] : 0,
                PMC_SQL_CLAIMS_FIELD_SERVICE_LOCATION,
                    pmcLocationCodeGet( LocationListBox->ItemIndex, PremiumComboBox[i]->ItemIndex ),
                PMC_SQL_CLAIMS_FIELD_FEE_CODE,          buf4_p,
                PMC_SQL_CLAIMS_FIELD_UNITS,             Units[i],
                PMC_SQL_CLAIMS_FIELD_FEE_SUBMITTED,     FeeCharged[i],
                PMC_SQL_CLAIMS_FIELD_COMMENT_TO_MSP,    buf3_p,
                PMC_SQL_CLAIMS_FIELD_STATUS,            Status[i],
                PMC_SQL_FIELD_ID,                       updateClaimId[i] );

            pmcSqlExec( buf1_p );
        }
    }

    // Save the location index.  This will be the "default" for the next
    // time this form is instantiated.
    pmcDefaultSeviceLocation = LocationListBox->ItemIndex;

    // Update the claim header structure
    mbStrClean( DiagnosisEdit->Text.c_str(), buf2_p, TRUE );

    sprintf( buf1_p, "update %s set %s=%ld,%s=%ld,%s=\"%s\"",
            PMC_SQL_TABLE_CLAIM_HEADERS,
            PMC_SQL_FIELD_PROVIDER_ID, ProviderId,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
            PMC_SQL_CLAIM_HEADERS_FIELD_DIAGNOSIS, buf2_p );

    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        sprintf( buf2_p, ",%s%ld=%ld", PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID, i, ClaimId[i] );
        strcat( buf1_p, buf2_p );
    }
    sprintf( buf2_p, " where %s=%ld", PMC_SQL_FIELD_ID, ClaimHeaderId );
    strcat( buf1_p, buf2_p );

    nbDlgDebug(( buf1_p ));

    pmcSqlExec( buf1_p );

    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if( ClaimId[i] != 0 )
        {
            // Mark the claims as undeleted
            sprintf( buf1_p, "update claims set %s=%ld where %s=%ld and %s=%ld",
                PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
                PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID, ClaimHeaderId,
                PMC_SQL_FIELD_ID, ClaimId[i] );
            pmcSqlExec( buf1_p );
        }
    }

    // Log
    if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_NEW )
    {
        mbLog( "Created claim (headerId: %ld)\n", ClaimHeaderId );
    }
    else
    {
        mbLog( "Updated claim (headerId: %ld)\n", ClaimHeaderId );
    }

    pmcCheckReferringDr( PatientId, ReferringDrId );

    closeFlag = TRUE;

exit:

    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( buf4_p );
    if( closeFlag ) Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::CancelButtonClick(TObject *Sender)
{
    ClaimEditInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::FormClose
(
    TObject        *Sender,
    TCloseAction   &Action
)
{
    Ints_t          i;

    // Unlock the claim header if required.
    if( ClaimHeaderLocked )
    {
        pmcSqlRecordUnlock( PMC_SQL_TABLE_CLAIM_HEADERS, ClaimHeaderId );
    }

    // Free allocated memory
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        pmcPickListFree( FeeList[i] );
    }
    
    pmcPickListFree( IcdComboBox );
    pmcPickListFree( LocationListBox );
    pmcPickListFree( ReferringDrTypeListBox );

    if( PatientPhn_p ) mbFree( PatientPhn_p );
    if( PatientPhnProv_p ) mbFree( PatientPhnProv_p );

    Action = caFree;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::SetIcd(Char_p str_p)
{
    Char_t  code[4];

    if( strlen( str_p ) == 0 ) return;

    strncpy( code, str_p, 3 );
    code[3] = 0;
    mbStrToUpper( code );

    pmcIcdVerify( code, pmcBuf1_p, FALSE );
    IcdDescEdit->Text = pmcBuf1_p;
    IcdComboBox->Text = code;

    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::IcdComboBoxChange(TObject *Sender)
{
    if( !Active ) return;

    SetIcd( IcdComboBox->Text.c_str() );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::IcdComboBoxExit(TObject *Sender)
{
    if( !Active ) return;

    SetIcd( IcdComboBox->Text.c_str() );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::InitializeControls( void )
{
    Ints_t  i;
    Char_p  buf_p;

    mbMalloc( buf_p, 256 );

    ClaimType[0] = PMC_CLAIM_TYPE_SERVICE;
    ClaimType[1] = PMC_CLAIM_TYPE_SERVICE;
    ClaimType[2] = PMC_CLAIM_TYPE_SERVICE;
    ClaimType[3] = PMC_CLAIM_TYPE_SERVICE;
    ClaimType[4] = PMC_CLAIM_TYPE_SERVICE;
    ClaimType[5] = PMC_CLAIM_TYPE_SERVICE;
    ClaimType[6] = PMC_CLAIM_TYPE_SERVICE;
    ClaimType[7] = PMC_CLAIM_TYPE_HOSPITAL;
    ClaimType[8] = PMC_CLAIM_TYPE_HOSPITAL;

    FeeList[0] = FeeList0;
    FeeList[1] = FeeList1;
    FeeList[2] = FeeList2;
    FeeList[3] = FeeList3;
    FeeList[4] = FeeList4;
    FeeList[5] = FeeList5;
    FeeList[6] = FeeList6;
    FeeList[7] = FeeList7;
    FeeList[8] = FeeList8;

    PremiumComboBox[0] = PremiumComboBox0;
    PremiumComboBox[1] = PremiumComboBox1;
    PremiumComboBox[2] = PremiumComboBox2;
    PremiumComboBox[3] = PremiumComboBox3;
    PremiumComboBox[4] = PremiumComboBox4;
    PremiumComboBox[5] = PremiumComboBox5;
    PremiumComboBox[6] = PremiumComboBox6;
    PremiumComboBox[7] = PremiumComboBox7;
    PremiumComboBox[8] = PremiumComboBox8;

    FeeChargedEdit[0] = FeeChargedEdit0;
    FeeChargedEdit[1] = FeeChargedEdit1;
    FeeChargedEdit[2] = FeeChargedEdit2;
    FeeChargedEdit[3] = FeeChargedEdit3;
    FeeChargedEdit[4] = FeeChargedEdit4;
    FeeChargedEdit[5] = FeeChargedEdit5;
    FeeChargedEdit[6] = FeeChargedEdit6;
    FeeChargedEdit[7] = FeeChargedEdit7;
    FeeChargedEdit[8] = FeeChargedEdit8;

    UnitsEdit[0] = UnitsEdit0;
    UnitsEdit[1] = UnitsEdit1;
    UnitsEdit[2] = UnitsEdit2;
    UnitsEdit[3] = UnitsEdit3;
    UnitsEdit[4] = UnitsEdit4;
    UnitsEdit[5] = UnitsEdit5;
    UnitsEdit[6] = UnitsEdit6;
    UnitsEdit[7] = UnitsEdit7;
    UnitsEdit[8] = UnitsEdit8;

    StatusLabel[0] = StatusLabel0;
    StatusLabel[1] = StatusLabel1;
    StatusLabel[2] = StatusLabel2;
    StatusLabel[3] = StatusLabel3;
    StatusLabel[4] = StatusLabel4;
    StatusLabel[5] = StatusLabel5;
    StatusLabel[6] = StatusLabel6;
    StatusLabel[7] = StatusLabel7;
    StatusLabel[8] = StatusLabel8;

    MspCommentLabel[0] = MspCommentLabel0;
    MspCommentLabel[1] = MspCommentLabel1;
    MspCommentLabel[2] = MspCommentLabel2;
    MspCommentLabel[3] = MspCommentLabel3;
    MspCommentLabel[4] = MspCommentLabel4;
    MspCommentLabel[5] = MspCommentLabel5;
    MspCommentLabel[6] = MspCommentLabel6;
    MspCommentLabel[7] = MspCommentLabel7;
    MspCommentLabel[8] = MspCommentLabel8;

    FeePaidLabel[0] = PaidLabel0;
    FeePaidLabel[1] = PaidLabel1;
    FeePaidLabel[2] = PaidLabel2;
    FeePaidLabel[3] = PaidLabel3;
    FeePaidLabel[4] = PaidLabel4;
    FeePaidLabel[5] = PaidLabel5;
    FeePaidLabel[6] = PaidLabel6;
    FeePaidLabel[7] = PaidLabel7;
    FeePaidLabel[8] = PaidLabel8;

    PremPaidLabel[0] = PremPaid0;
    PremPaidLabel[1] = PremPaid1;
    PremPaidLabel[2] = PremPaid2;
    PremPaidLabel[3] = PremPaid3;
    PremPaidLabel[4] = PremPaid4;
    PremPaidLabel[5] = PremPaid5;
    PremPaidLabel[6] = PremPaid6;
    PremPaidLabel[7] = PremPaid7;
    PremPaidLabel[8] = PremPaid8;

    FeeCodeApprovedLabel[0] = CodeLabel0;
    FeeCodeApprovedLabel[1] = CodeLabel1;
    FeeCodeApprovedLabel[2] = CodeLabel2;
    FeeCodeApprovedLabel[3] = CodeLabel3;
    FeeCodeApprovedLabel[4] = CodeLabel4;
    FeeCodeApprovedLabel[5] = CodeLabel5;
    FeeCodeApprovedLabel[6] = CodeLabel6;
    FeeCodeApprovedLabel[7] = CodeLabel7;
    FeeCodeApprovedLabel[8] = CodeLabel8;

    CommentEdit[0] = CommentEdit0;
    CommentEdit[1] = CommentEdit1;
    CommentEdit[2] = CommentEdit2;
    CommentEdit[3] = CommentEdit3;
    CommentEdit[4] = CommentEdit4;
    CommentEdit[5] = CommentEdit5;
    CommentEdit[6] = CommentEdit6;
    CommentEdit[7] = CommentEdit7;
    CommentEdit[8] = CommentEdit8;

    SubmitLabel[0] = SubmitLabel0;
    SubmitLabel[1] = SubmitLabel1;
    SubmitLabel[2] = SubmitLabel2;
    SubmitLabel[3] = SubmitLabel3;
    SubmitLabel[4] = SubmitLabel4;
    SubmitLabel[5] = SubmitLabel5;
    SubmitLabel[6] = SubmitLabel6;
    SubmitLabel[7] = SubmitLabel7;
    SubmitLabel[8] = SubmitLabel8;

    ReplyLabel[0] = ReplyLabel0;
    ReplyLabel[1] = ReplyLabel1;
    ReplyLabel[2] = ReplyLabel2;
    ReplyLabel[3] = ReplyLabel3;
    ReplyLabel[4] = ReplyLabel4;
    ReplyLabel[5] = ReplyLabel5;
    ReplyLabel[6] = ReplyLabel6;
    ReplyLabel[7] = ReplyLabel7;
    ReplyLabel[8] = ReplyLabel8;

    UnitsPaidLabel[0] = UnitsPaidLabel0;
    UnitsPaidLabel[1] = UnitsPaidLabel1;
    UnitsPaidLabel[2] = UnitsPaidLabel2;
    UnitsPaidLabel[3] = UnitsPaidLabel3;
    UnitsPaidLabel[4] = UnitsPaidLabel4;
    UnitsPaidLabel[5] = UnitsPaidLabel5;
    UnitsPaidLabel[6] = UnitsPaidLabel6;
    UnitsPaidLabel[7] = UnitsPaidLabel7;
    UnitsPaidLabel[8] = UnitsPaidLabel8;

    SeqLabel[0] = SeqLabel0;
    SeqLabel[1] = SeqLabel1;
    SeqLabel[2] = SeqLabel2;
    SeqLabel[3] = SeqLabel3;
    SeqLabel[4] = SeqLabel4;
    SeqLabel[5] = SeqLabel5;
    SeqLabel[6] = SeqLabel6;
    SeqLabel[7] = SeqLabel7;
    SeqLabel[8] = SeqLabel8;

    EditStatusLabel[0] = EditStatusLabel0;
    EditStatusLabel[1] = EditStatusLabel1;
    EditStatusLabel[2] = EditStatusLabel2;
    EditStatusLabel[3] = EditStatusLabel3;
    EditStatusLabel[4] = EditStatusLabel4;
    EditStatusLabel[5] = EditStatusLabel5;
    EditStatusLabel[6] = EditStatusLabel6;
    EditStatusLabel[7] = EditStatusLabel7;
    EditStatusLabel[8] = EditStatusLabel8;

    ExpLabel[0] = ExpLabel0;
    ExpLabel[1] = ExpLabel1;
    ExpLabel[2] = ExpLabel2;
    ExpLabel[3] = ExpLabel3;
    ExpLabel[4] = ExpLabel4;
    ExpLabel[5] = ExpLabel5;
    ExpLabel[6] = ExpLabel6;
    ExpLabel[7] = ExpLabel7;
    ExpLabel[8] = ExpLabel8;


    DateEdit[0] = DateEdit0;
    DateEdit[1] = DateEdit1;
    DateEdit[2] = DateEdit2;
    DateEdit[3] = DateEdit3;
    DateEdit[4] = DateEdit4;
    DateEdit[5] = DateEdit5;
    DateEdit[6] = DateEdit6;
    DateEdit[7] = DateEdit7;
    DateEdit[8] = DateEdit8;
    DateEdit[9] = DateEdit9;
    DateEdit[10]= DateEdit10;

    DateButton[0] = DateButton0;
    DateButton[1] = DateButton1;
    DateButton[2] = DateButton2;
    DateButton[3] = DateButton3;
    DateButton[4] = DateButton4;
    DateButton[5] = DateButton5;
    DateButton[6] = DateButton6;
    DateButton[7] = DateButton7;
    DateButton[8] = DateButton8;
    DateButton[9] = DateButton9;
    DateButton[10]= DateButton10;

    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        // Initialize all controls
        pmcPickListCfgBuild( FeeList[i],
             ( ClaimType[i] == PMC_CLAIM_TYPE_SERVICE ) ? PMC_PICKLIST_CFG_MSP_SERVICE : PMC_PICKLIST_CFG_MSP_HOSPITAL );

        pmcPickListBuild( PremiumComboBox[i], &pmcPremiums[0], PMC_PREMIUM_NONE );

        FeeList[i]->Text = "";
        EditStatusLabel[i]->Caption = "";
        MspCommentLabel[i]->Caption = "";
        // Disable all controls
        DateEdit[i]->Enabled        = FALSE;
        FeeList[i]->Enabled         = FALSE;
        FeeChargedEdit[i]->Enabled  = FALSE;
        UnitsEdit[i]->Enabled       = FALSE;
        CommentEdit[i]->Enabled     = FALSE;
        PremiumComboBox[i]->Enabled = FALSE;
    }

    for( i = 0 ; i < PMC_CLAIM_COUNT + 2 ; i++ )
    {
        DateButton[i]->Enabled = FALSE;
    }
    pmcPickListCfgBuild( IcdComboBox, PMC_PICKLIST_CFG_ICD );

    pmcPickListBuild( ReferringDrTypeListBox, &pmcDrType[0], 0 );
    pmcPickListBuild( LocationListBox, &pmcNewLocations[0], pmcDefaultSeviceLocation );

    PatientSelectButton->Enabled = FALSE;
    ProviderListBox->Enabled = FALSE;
    ReferringDrSelectButton->Enabled = FALSE;
    ReferringDrTypeListBox->Enabled = FALSE;
    PatientEditButton->Enabled = FALSE;
    ReferringDrEditButton->Enabled = FALSE;
    ReferringDrClearButton->Enabled = FALSE;
    IcdComboBox->Enabled = FALSE;
    LocationListBox->Enabled = FALSE;
    ReferringDrEdit->Enabled = FALSE;
    DiagnosisEdit->Enabled = FALSE;

    mbFree( buf_p );

    return;
}

//---------------------------------------------------------------------------
// Function: ProgramControls( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ProgramControls( void )
{
    Int32u_t  i;

    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        UnitsEditUpdate( i );
        SubmitStatusUpdate( i );
        FeeChargedEditUpdate( i );
    }

    for( i = 0 ; i < PMC_CLAIM_COUNT + 2 ; i++ )
    {
        DateEditUpdate( i );
    }

    // Build pick list of providers
    ProviderId = pmcProviderListBuild( ProviderListBox, ProviderId, TRUE, TRUE );
    ProviderIndex = pmcProviderIndexGet( ProviderListBox, ProviderId );
    ProviderUpdate( ProviderId );

    return;
}

//---------------------------------------------------------------------------
// Function:    PatientUpdate( )
//---------------------------------------------------------------------------
// Description:
//
// Update the patient name, phn, and dob display based on patient id
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::PatientUpdate
(
    Int32u_t    patientId
)
{
    Char_p          buf_p;
    Char_p          buf2_p;
    Char_p          buf3_p;
    MbDateTime      dateTime;
    PmcSqlPatient_p pat_p;

    mbMalloc( pat_p,    sizeof(PmcSqlPatient_t) );
    mbMalloc( buf_p,    128 );
    mbMalloc( buf2_p,   64 );
    mbMalloc( buf3_p,   64 );

    sprintf( buf_p, "" );
    sprintf( buf2_p, "" );
    sprintf( buf3_p, "" );

    PatientDob = 0;

    if( patientId )
    {
        if( pmcSqlPatientDetailsGet( patientId, pat_p ) == TRUE )
        {
            sprintf( buf_p, "" );
            if( strlen( pat_p->lastName ) )
            {
                sprintf( buf2_p, "%s, ", pat_p->lastName );
                strcat( buf_p, buf2_p );
            }
            if( strlen( pat_p->firstName ) )
            {
                sprintf( buf2_p, "%s ", pat_p->firstName );
                strcat( buf_p, buf2_p );
            }
            if( strlen( pat_p->title ) )
            {
                sprintf( buf2_p, "(%s)", pat_p->title );
                strcat( buf_p, buf2_p );
                strcpy( PatientTitle, pat_p->title );
            }

            // Get Date of birth
            sprintf( buf2_p, "" );
            if( pat_p->birthDate != 0 )
            {
                PatientDob = pat_p->birthDate;
                dateTime.SetDate( pat_p->birthDate );
                sprintf( buf2_p, "%s", dateTime.MDY_DateString( ) );
            }
            if( pat_p->gender == 0 )
            {
                PatientGenderLabel->Caption = "M";
            }
            else
            {
                PatientGenderLabel->Caption = "F";
            }
            pmcFormatPhnDisplay( pat_p->phn, pat_p->phnProv, buf3_p );

            // Store the Phn
            mbFree( PatientPhn_p );
            mbMallocStr( PatientPhn_p, pat_p->phn);
            mbStrAlphaNumericOnly( PatientPhn_p );

            // Store the PhnProvince
            mbFree( PatientPhnProv_p );
            mbMallocStr( PatientPhnProv_p, pat_p->phnProv );
            mbStrAlphaNumericOnly( PatientPhnProv_p );
        }
        else
        {
            sprintf( buf_p, "Unknown" );
        }
    }

    PatientEdit->Text = buf_p;
    PatientDobLabel->Caption = buf2_p;
    PatientPhnLabel->Caption = buf3_p;

    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( buf3_p );
    mbFree( pat_p );

    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function:    FeeListChange( )
//---------------------------------------------------------------------------
// Description:
//
// Since these are IDE managed fuunctions don't bother
// making them inline funtions.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::FeeList0Change(TObject *Sender) { FeeListChange( 0 ); }
void __fastcall TClaimEditForm::FeeList1Change(TObject *Sender) { FeeListChange( 1 ); }
void __fastcall TClaimEditForm::FeeList2Change(TObject *Sender) { FeeListChange( 2 ); }
void __fastcall TClaimEditForm::FeeList3Change(TObject *Sender) { FeeListChange( 3 ); }
void __fastcall TClaimEditForm::FeeList4Change(TObject *Sender) { FeeListChange( 4 ); }
void __fastcall TClaimEditForm::FeeList5Change(TObject *Sender) { FeeListChange( 5 ); }
void __fastcall TClaimEditForm::FeeList6Change(TObject *Sender) { FeeListChange( 6 ); }
void __fastcall TClaimEditForm::FeeList7Change(TObject *Sender) { FeeListChange( 7 ); }
void __fastcall TClaimEditForm::FeeList8Change(TObject *Sender) { FeeListChange( 8 ); }

void __fastcall TClaimEditForm::FeeListChange( Int32u_t index )
{
    Char_t      code[8];
    Int32u_t    feeHigh;
    Int32u_t    feeLow;
    Int32u_t    multiple;
    Int32u_t    determinant;
    Int32u_t    feeIndex;

    if( !Active ) return;

    strncpy( code, FeeList[index]->Text.c_str(), 5 );
    code[4] == 0;
    mbStrToUpper( code );

    if( pmcFeeCodeVerify( code, Date[index], &feeHigh, &feeLow, &multiple,
                          &determinant, &feeIndex, ClaimType[index] ) == TRUE )
    {
        if( ( feeIndex != FeeIndex[index] ) || ( FeeOverride[index] == 0 ) )
        {
            FeeHigh[index]          = feeHigh;
            FeeLow[index]           = feeLow;
            FeeMultiple[index]      = multiple;
            FeeDeterminant[index]   = determinant;
            FeeIndex[index]         = feeIndex;
            FeeOverride[index]      = 0;
        }
        else
        {
            // Fee code did not change
        }
    }
    else
    {
        FeeIndex[index] = 0;
        FeeOverride[index] = 0;
        FeeHigh[index] = 0;
        FeeLow[index] = 0;
        FeeDeterminant[index] = 0;
        FeeMultiple[index] = TRUE;
    }

    // Set the units automaticaly
    if( strlen( code ) )
    {
        if( Units[index] == 0  ) Units[index] = 1;
    }
    else
    {
        if( Units[index] == 1 ) Units[index] = 0;
    }

    UnitsEditUpdate( index );
    ComputeFeeCharged( index );
    FeeChargedEditUpdate( index );
    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function:    ComputeFeeCharged
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ComputeFeeCharged( Int32u_t index )
{
    if( FeeOverride[index] != 0 )
    {
        FeeCharged[index] = FeeOverride[index];
    }
    else
    {
        if( FeeDeterminant[index] == PMC_FEE_DET_SP )
        {
            if( ReferringDrId == 0 )
            {
                FeeCharged[index] = FeeLow[index] * Units[index];
            }
            else
            {
                FeeCharged[index] = FeeHigh[index] * Units[index];
            }
        }
        else
        {
            FeeCharged[index] = FeeHigh[index] * Units[index];
        }
    }
    return;
}

//---------------------------------------------------------------------------
// Function:    FeeListExit
//---------------------------------------------------------------------------
// Description:
//
// Since these are IDE managed fuunctions don't bother
// making them inline funtions.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::FeeList0Exit(TObject *Sender) { FeeListExit( 0 ); }
void __fastcall TClaimEditForm::FeeList1Exit(TObject *Sender) { FeeListExit( 1 ); }
void __fastcall TClaimEditForm::FeeList2Exit(TObject *Sender) { FeeListExit( 2 ); }
void __fastcall TClaimEditForm::FeeList3Exit(TObject *Sender) { FeeListExit( 3 ); }
void __fastcall TClaimEditForm::FeeList4Exit(TObject *Sender) { FeeListExit( 4 ); }
void __fastcall TClaimEditForm::FeeList5Exit(TObject *Sender) { FeeListExit( 5 ); }
void __fastcall TClaimEditForm::FeeList6Exit(TObject *Sender) { FeeListExit( 6 ); }
void __fastcall TClaimEditForm::FeeList7Exit(TObject *Sender) { FeeListExit( 7 ); }
void __fastcall TClaimEditForm::FeeList8Exit(TObject *Sender) { FeeListExit( 8 ); }

void __fastcall TClaimEditForm::FeeListExit( Int32u_t index )
{
    Char_t      code[8];
    Int32u_t    feeHigh;
    Int32u_t    feeLow;
    Int32u_t    multiple;
    Int32u_t    determinant;
    Int32u_t    feeIndex;

    if( !Active ) return;

    strncpy( code, FeeList[index]->Text.c_str(), 5 );
    code[4] == 0;
    mbStrToUpper( code );

    if( ( strlen( code ) == 0 ) && SubmitCount[index] > 0 )
    {
        mbDlgExclaim( "Cannot clear a previously submitted claim." );
        pmcFeeCodeFormatDisplay( OrigFeeCode[index], code );
        FeeList[index]->Text = code;
    }

    strncpy( code, FeeList[index]->Text.c_str(), 5 );
    code[4] == 0;
    mbStrToUpper( code );

    if( pmcFeeCodeVerify( code, Date[index], &feeHigh, &feeLow, &multiple, &determinant, &feeIndex, ClaimType[index] ) == TRUE )
    {
        if( feeIndex != FeeIndex[index] )
        {
            FeeHigh[index]          = feeHigh;
            FeeLow[index]           = feeLow;
            FeeMultiple[index]      = multiple;
            FeeDeterminant[index]   = determinant;
            FeeIndex[index]         = feeIndex;
            FeeOverride[index]      = 0;
        }
        else
        {
            // Fee code did not change
        }
    }
    else
    {
        mbDlgExclaim( "Invalid fee code '%s'.", code );
        FeeHigh[index] = 0;
        FeeLow[index] = 0;
        FeeMultiple[index] = TRUE;
        FeeDeterminant[index] = PMC_FEE_DET_INVALID;
    }

    // Set the units automaticaly
    if( strlen( code ) )
    {
        if( Units[index] == 0  ) Units[index] = 1;
    }
    else
    {
        if( Units[index] == 1 ) Units[index] = 0;
    }
    UnitsEditUpdate( index );

    FeeChargedEditUpdate( index );
    TotalFeeChargedEditUpdate( );
    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function:  FeeChargedEditUpdate( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::FeeChargedEditUpdate( Int32u_t index )
{
    Char_t  buf[32];
    Char_t  buf2[32];

    if( FeeCharged[index] == 0 )
    {
        sprintf( buf2, "" );
    }
    else
    {
        Int32u_t    temp;
        temp = FeeCharged[index];
        sprintf( buf, "%8.2f", (float)temp / 100.0 );
        sprintf( buf2, "%9s", buf );
    }
    FeeChargedEdit[index]->Text = buf2;
    TotalFeeChargedEditUpdate( );

    if( !Active ) return;

    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function: UnitsEditChange( )
//---------------------------------------------------------------------------
// Description:
//
// Since these are IDE managed fuunctions don't bother
// making them inline funtions.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::UnitsEdit0Change(TObject *Sender) { UnitsEditChange( 0 ); }
void __fastcall TClaimEditForm::UnitsEdit1Change(TObject *Sender) { UnitsEditChange( 1 ); }
void __fastcall TClaimEditForm::UnitsEdit2Change(TObject *Sender) { UnitsEditChange( 2 ); }
void __fastcall TClaimEditForm::UnitsEdit3Change(TObject *Sender) { UnitsEditChange( 3 ); }
void __fastcall TClaimEditForm::UnitsEdit4Change(TObject *Sender) { UnitsEditChange( 4 ); }
void __fastcall TClaimEditForm::UnitsEdit5Change(TObject *Sender) { UnitsEditChange( 5 ); }
void __fastcall TClaimEditForm::UnitsEdit6Change(TObject *Sender) { UnitsEditChange( 6 ); }
void __fastcall TClaimEditForm::UnitsEdit7Change(TObject *Sender) { UnitsEditChange( 7 ); }
void __fastcall TClaimEditForm::UnitsEdit8Change(TObject *Sender) { UnitsEditChange( 8 ); }

void __fastcall TClaimEditForm::UnitsEditChange( Int32u_t index )
{
    Char_t  buf[16];

    if( !Active ) return;

    strcpy( buf, UnitsEdit[index]->Text.c_str( ) );
    mbStrDigitsOnly( buf );
    Units[index] = atol( buf );

    ComputeFeeCharged( index );
    FeeChargedEditUpdate( index );
    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function: UnitsEditExit(  )
//---------------------------------------------------------------------------
// Description:
//
// Since these are IDE managed functions don't bother
// making them inline funtions.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::UnitsEdit0Exit(TObject *Sender) { UnitsEditExit( 0 ); }
void __fastcall TClaimEditForm::UnitsEdit1Exit(TObject *Sender) { UnitsEditExit( 1 ); }
void __fastcall TClaimEditForm::UnitsEdit2Exit(TObject *Sender) { UnitsEditExit( 2 ); }
void __fastcall TClaimEditForm::UnitsEdit3Exit(TObject *Sender) { UnitsEditExit( 3 ); }
void __fastcall TClaimEditForm::UnitsEdit4Exit(TObject *Sender) { UnitsEditExit( 4 ); }
void __fastcall TClaimEditForm::UnitsEdit5Exit(TObject *Sender) { UnitsEditExit( 5 ); }
void __fastcall TClaimEditForm::UnitsEdit6Exit(TObject *Sender) { UnitsEditExit( 6 ); }
void __fastcall TClaimEditForm::UnitsEdit7Exit(TObject *Sender) { UnitsEditExit( 7 ); }
void __fastcall TClaimEditForm::UnitsEdit8Exit(TObject *Sender) { UnitsEditExit( 8 ); }

void __fastcall TClaimEditForm::UnitsEditExit( Int32u_t index )
{
    Char_t  buf[16];

    if( !Active ) return;

    strcpy( buf, UnitsEdit[index]->Text.c_str( ) );
    mbStrDigitsOnly( buf );
    Units[index] = atol( buf );
    UnitsEditUpdate( index );
    FeeChargedEditUpdate( index );
    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function: UnitsEditUpdate( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::UnitsEditUpdate( Int32u_t index )
{
    Char_t  buf[16];

    sprintf( buf, "" );
    if( Units[index] > 0 )
    {
        sprintf( buf, "%ld", Units[index] );
    }
    UnitsEdit[index]->Text = buf;
    return;
}

//---------------------------------------------------------------------------
// Function: TotalFeeChargedEditUpdate( )
//---------------------------------------------------------------------------
// Description:
//
// Since these are IDE managed fuunctions don't bother
// making them inline funtions.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::TotalFeeChargedEditUpdate( void )
{
    Ints_t  i;
    Char_t  buf2[32];
    Char_t  buf[32];

    if( !Active ) return;

    TotalFeeCharged = 0;
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        TotalFeeCharged += FeeCharged[i];
    }

    sprintf( buf, "%8.2f", (float)TotalFeeCharged/ 100.0 );
    sprintf( buf2, "%9s", buf );
    TotalFeeChargedLabel->Caption = buf2;
}

//---------------------------------------------------------------------------
// Function: DateButtonClick( )
//---------------------------------------------------------------------------
// Description:
//
// Since these are IDE managed functions don't bother
// making them inline funtions.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::DateButton0Click(TObject *Sender)  { DateButtonClick( 0 ); }
void __fastcall TClaimEditForm::DateButton1Click(TObject *Sender)  { DateButtonClick( 1 ); }
void __fastcall TClaimEditForm::DateButton2Click(TObject *Sender)  { DateButtonClick( 2 ); }
void __fastcall TClaimEditForm::DateButton3Click(TObject *Sender)  { DateButtonClick( 3 ); }
void __fastcall TClaimEditForm::DateButton4Click(TObject *Sender)  { DateButtonClick( 4 ); }
void __fastcall TClaimEditForm::DateButton5Click(TObject *Sender)  { DateButtonClick( 5 ); }
void __fastcall TClaimEditForm::DateButton6Click(TObject *Sender)  { DateButtonClick( 6 ); }
void __fastcall TClaimEditForm::DateButton7Click(TObject *Sender)  { DateButtonClick( 7 ); }
void __fastcall TClaimEditForm::DateButton8Click(TObject *Sender)  { DateButtonClick( 8 ); }
void __fastcall TClaimEditForm::DateButton9Click(TObject *Sender)  { DateButtonClick( 9 ); }
void __fastcall TClaimEditForm::DateButton10Click(TObject *Sender) { DateButtonClick(10 ); }

void __fastcall TClaimEditForm::DateButtonClick( Int32u_t index )
{
    TDateSelectForm     *dateSelectForm_p;
    pmcDateSelectInfo_t  dateSelectInfo;
    Int32s_t             days;

    mbMalloc( dateSelectInfo.string_p, 128 );

    dateSelectInfo.mode = PMC_DATE_SELECT_PARMS;

    sprintf( dateSelectInfo.string_p, "Select Date" );
    dateSelectInfo.dateIn = Date[index];
    dateSelectInfo.caption_p = NIL;

    dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode == MB_BUTTON_OK )
    {
        Date[index] = dateSelectInfo.dateOut;

        // Update all the other dates if they are blank
//        for( Int32u_t i = index ; i < 7 ; i++ )
//        {
//            if( Date[i] == 0 )
//            {
//                Date[i] = Date[index];
//                DateEditUpdate( i );
//            }
//        }

        DateEditUpdate( index );

        if( index >= 7 )
        {
            // This date is one of the hospital visit dates.
            if( Date[7] && Date[9] )
            {
                days = 1 + Date[9] - Date[7];
                if( days < 1 || days > 15 )
                {
                    // Do nothing
                }
                else
                {
                    Units[7] = days;
                    UnitsEditUpdate( 7 );
                }
           }
           if( Date[8] && Date[10] )
           {
                days = 1 + Date[10] - Date[8];
                if( days < 1 || days > 15 )
                {
                    // Do nothing
                }
                else
                {
                    Units[8] = days;
                    UnitsEditUpdate( 8 );
                }
            }
        }
    }

    // Change of date could change the fee
    if( index == 9 )  index = 7;
    if( index == 10 ) index = 8;
    FeeListChange( index );

    //ClaimsValidate( );
    mbFree( dateSelectInfo.string_p );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::DateEditUpdate( Int32u_t index )
{
    Int32u_t date = Date[index];

    if( date != 0 )
    {
        MbDateTime dateTime = MbDateTime( date, 0 );
        DateEdit[index]->Text = dateTime.MDY_DateString( );
    }
    else
    {
        DateEdit[index]->Text = "";
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::SubmitStatusUpdate( Int32u_t index )
{
    Char_p      buf_p;
    MbDateTime  dateTime;
    Int32u_t    date;
    Int32u_t    i;
    Int32u_t    feePaidTotal = 0;
    Int32u_t    premPaidTotal = 0;
    Int32u_t    grandTotal = 0;

    mbCalloc( buf_p, 256 );

    date = SubmittedDate[index];

    if( date != 0 )
    {
        dateTime.SetDate( date );
        SubmitLabel[index]->Caption = dateTime.MDY_DateString( );
    }
    else
    {
        SubmitLabel[index]->Caption = buf_p;
    }

    date = ReplyDate[index];

    if( date != 0 )
    {
        dateTime.SetDate( date );
        ReplyLabel[index]->Caption = dateTime.MDY_DateString( );
    }
    else
    {
        ReplyLabel[index]->Caption = buf_p;
    }

    // Format the sequence number for display
    *buf_p = 0;
    if( SeqNumber[index] >= 0 && SubmittedDate[index] > 0 )
    {
        sprintf( buf_p, "%ld", SeqNumber[index] );
    }
    SeqLabel[index]->Caption = buf_p;

    // Format the sequence number for display
    *buf_p = 0;
    if( UnitsPaid[index] >= 0 && ReplyDate[index] > 0 )
    {
        sprintf( buf_p, "%ld", UnitsPaid[index] );
    }
    UnitsPaidLabel[index]->Caption = buf_p;

    // Format the Exp code for display
    *buf_p = 0;
    if( strlen( ExpCode[index] ) > 0 ) strcpy( buf_p, ExpCode[index] );
    ExpLabel[index]->Caption = buf_p;

    // Format the Exp code for display
    *buf_p = 0;
    if( strlen( FeeCodeApproved[index] ) > 0 )
    {
        pmcFeeCodeFormatDisplay( FeeCodeApproved[index], buf_p );
    }
    FeeCodeApprovedLabel[index]->Caption = buf_p;

    // Format the fee paid for display.
    *buf_p = 0;

    // MAB:20020901: Remove dependance on SubmittedDate so that claim added by MSP will display
    // if( SubmittedDate[index] && ReplyDate[index] )
    if( ReplyDate[index] )
    {
        sprintf( buf_p, "%7.2f", (float)FeePaid[index]/100.0 );
    }
    FeePaidLabel[index]->Caption = buf_p;

    // Format the premium paid for display
    *buf_p = 0;
    if( SubmittedDate[index] && ReplyDate[index] && PremPaid[index] )
    {
        Int32u_t    test;
        test = PremPaid[index];
        sprintf( buf_p, "%7.2f", (float)test / 100.0 );
    }
    PremPaidLabel[index]->Caption = buf_p;

    // This is kind if ugly but just update totals every time this function is called
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        feePaidTotal += FeePaid[i];
        premPaidTotal += PremPaid[i];
    }
    grandTotal = feePaidTotal + premPaidTotal;

    *buf_p = 0;
    sprintf( buf_p, "%7.2f", ((float)feePaidTotal / 100.0) );
    TotalPaidLabel->Caption = buf_p;

    *buf_p = 0;
    sprintf( buf_p, "%7.2f", ((float)premPaidTotal / 100.0) );
    TotalPremPaidLabel->Caption = buf_p;

    *buf_p = 0;
    sprintf( buf_p, "%7.2f", ((float)grandTotal / 100.0) );
    GrandTotalLabel->Caption = buf_p;

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function: PatientSelectButtonClick( )
//---------------------------------------------------------------------------
// Description:
//
// Change the selected patient
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::PatientSelectButtonClick(TObject *Sender)
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;
    Char_p              buf_p;

    mbMalloc( buf_p, 128 );

    patListInfo.mode = PMC_LIST_MODE_SELECT;
    patListInfo.patientId = PatientId;
    patListInfo.providerId = ProviderId;
    patListInfo.allowGoto = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
          PatientId = patListInfo.patientId;
    }
    else
    {
        // User pressed cancel button
    }
    // It is possible that patient record was edited then cancel pressed in patient list form,
    // so update the patient anyway
    PatientUpdate( PatientId );

    // Lets update the referring Dr too
    if( PatientId != 0 && ReferringDrId == 0 )
    {
        sprintf( buf_p, "select %s from %s where %s=%ld",
            PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID,
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_ID, PatientId );
        ReferringDrId = pmcSqlSelectInt( buf_p, NIL );

        // A -1 paramter will cause the ReferringDrUpdate() function to attempt to determine
        // the referring Dr type
        ReferringDrUpdate( ReferringDrId, -1 );
    }
    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
// Function: LocationListBoxChange( )
//---------------------------------------------------------------------------
// Description:
//
// Edit the selected patient record
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::LocationListBoxChange(TObject *Sender)
{
    // Just display text in the box
    if( LocationListBox->ItemIndex == -1 )
    {
        LocationListBox->ItemIndex = LocationCodeIndex;
    }
    LocationCodeIndex = LocationListBox->ItemIndex;
    LocationListBox->Text = pmcNewLocations[ LocationListBox->ItemIndex].description_p;
    ClaimsValidate( );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::LocationListBoxExit(TObject *Sender)
{
    // Just display text in the box
    if( LocationListBox->ItemIndex == -1 )
    {
        LocationListBox->ItemIndex = LocationCodeIndex;
    }
    LocationCodeIndex = LocationListBox->ItemIndex;
    LocationListBox->Text = pmcNewLocations[ LocationListBox->ItemIndex].description_p;
    ClaimsValidate( );
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::PremiumComboBox0Change(TObject *Sender) { PremiumComboBoxChange( 0 ); }
void __fastcall TClaimEditForm::PremiumComboBox1Change(TObject *Sender) { PremiumComboBoxChange( 1 ); }
void __fastcall TClaimEditForm::PremiumComboBox2Change(TObject *Sender) { PremiumComboBoxChange( 2 ); }
void __fastcall TClaimEditForm::PremiumComboBox3Change(TObject *Sender) { PremiumComboBoxChange( 3 ); }
void __fastcall TClaimEditForm::PremiumComboBox4Change(TObject *Sender) { PremiumComboBoxChange( 4 ); }
void __fastcall TClaimEditForm::PremiumComboBox5Change(TObject *Sender) { PremiumComboBoxChange( 5 ); }
void __fastcall TClaimEditForm::PremiumComboBox6Change(TObject *Sender) { PremiumComboBoxChange( 6 ); }

void __fastcall TClaimEditForm::PremiumComboBoxChange( Int32u_t index )
{
    Int32s_t    newIndex, i;

    // MAB: set all premiums equal to the premium in the first premium selector box
    if( index != 0 ) mbDlgDebug(( "Expecting only index 0" ));

#if 0
    if( PremiumComboBox[index]->ItemIndex == -1 )
    {
        PremiumComboBox[index]->ItemIndex = PremiumComboBoxIndex[index];
    }

    PremiumComboBoxIndex[index] = PremiumComboBox[index]->ItemIndex ;
    PremiumComboBox[index]->Text = pmcPremiums[ PremiumComboBox[index]->ItemIndex].description_p;
#endif

    newIndex = PremiumComboBox[index]->ItemIndex;
    if( newIndex == -1 )
    {
        newIndex =  PremiumComboBoxIndex[index];
    }

    for( i = 0 ; i < PMC_CLAIM_COUNT - 2 ; i++ )
    {
        PremiumComboBox[i]->ItemIndex = newIndex;
        PremiumComboBoxIndex[i] = newIndex ;
        PremiumComboBox[i]->Text = pmcPremiums[ PremiumComboBox[i]->ItemIndex].description_p;
    }

    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::PremiumComboBox6Exit(TObject *Sender) { PremiumComboBoxExit( 6 ); }
void __fastcall TClaimEditForm::PremiumComboBox5Exit(TObject *Sender) { PremiumComboBoxExit( 5 ); }
void __fastcall TClaimEditForm::PremiumComboBox4Exit(TObject *Sender) { PremiumComboBoxExit( 4 ); }
void __fastcall TClaimEditForm::PremiumComboBox3Exit(TObject *Sender) { PremiumComboBoxExit( 3 ); }
void __fastcall TClaimEditForm::PremiumComboBox2Exit(TObject *Sender) { PremiumComboBoxExit( 2 ); }
void __fastcall TClaimEditForm::PremiumComboBox1Exit(TObject *Sender) { PremiumComboBoxExit( 1 ); }
void __fastcall TClaimEditForm::PremiumComboBox0Exit(TObject *Sender) { PremiumComboBoxExit( 0 ); }

void __fastcall TClaimEditForm::PremiumComboBoxExit( Int32u_t index )
{
    if( PremiumComboBox[index]->ItemIndex == -1 )
    {
        PremiumComboBox[index]->ItemIndex = PremiumComboBoxIndex[index];
    }
    PremiumComboBoxIndex[index] = PremiumComboBox[index]->ItemIndex ;
    PremiumComboBox[index]->Text = pmcPremiums[ PremiumComboBox[index]->ItemIndex].description_p;
    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function: FeeChargedEditExit( )
//---------------------------------------------------------------------------
// Description:
//
// Change the selected patient
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::FeeChargedEdit0Exit(TObject *Sender) { FeeChargedEditExit( 0 ); }
void __fastcall TClaimEditForm::FeeChargedEdit1Exit(TObject *Sender) { FeeChargedEditExit( 1 ); }
void __fastcall TClaimEditForm::FeeChargedEdit2Exit(TObject *Sender) { FeeChargedEditExit( 2 ); }
void __fastcall TClaimEditForm::FeeChargedEdit3Exit(TObject *Sender) { FeeChargedEditExit( 3 ); }
void __fastcall TClaimEditForm::FeeChargedEdit4Exit(TObject *Sender) { FeeChargedEditExit( 4 ); }
void __fastcall TClaimEditForm::FeeChargedEdit5Exit(TObject *Sender) { FeeChargedEditExit( 5 ); }
void __fastcall TClaimEditForm::FeeChargedEdit6Exit(TObject *Sender) { FeeChargedEditExit( 6 ); }
void __fastcall TClaimEditForm::FeeChargedEdit7Exit(TObject *Sender) { FeeChargedEditExit( 7 ); }
void __fastcall TClaimEditForm::FeeChargedEdit8Exit(TObject *Sender) { FeeChargedEditExit( 8 ); }

void __fastcall TClaimEditForm::FeeChargedEditExit( Int32u_t index )
{
    Char_t  buf[32];

    Int32u_t    newFeeCharged;

    sprintf( buf, FeeChargedEdit[index]->Text.c_str( ) );
    mbStrDigitsOnly( buf );

    newFeeCharged = atol( buf );
    FeeOverride[index] = 0;

    mbStrClean( FeeList[index]->Text.c_str(), buf, TRUE );
    if( ( strlen( buf ) == 0 ) )
    {
        // There does not appear to be a fee code in place
        if( newFeeCharged != 0 )
        {
            mbDlgInfo( "Cannot charge a fee without a fee code." );
        }
        FeeHigh[index] = 0;
        FeeLow[index] = 0;
    }
    else
    {
        // OK, there is a fee code.  Check to see if a "non standard" fee
        if( newFeeCharged != 0 )
        {
            if(    ( newFeeCharged != ( FeeHigh[index] * Units[index] ) )
                && ( newFeeCharged != ( FeeLow[index]  * Units[index] ) ) )
            {
                nbDlgDebug(( "Detected a fee override" ));
                FeeOverride[index] = newFeeCharged;
            }
        }
    }
    FeeListChange( index );
}

//---------------------------------------------------------------------------
// Function: ReferringDrUpdate( )
//---------------------------------------------------------------------------
// Description:
//
//
///  Int32s_t        pmcSqlDoctorDetailsGet
///  (
//      Int32u_t    id,
//      Char_p      firstName_p,
//      Char_p      lastName_p,
//      Char_p      province_p,
//      Int32u_p    doctorNumber_p,
//      Int32u_p    cancerClinic_p,
//      Char_p      otherNumber_p
//  );
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ReferringDrUpdate
(
    Int32u_t    doctorId,
    Int32s_t    typeIndex
)
{
    Char_p      buf_p;
    Char_p      buf2_p;
    PmcSqlDoctor_p    dr_p;

    mbMalloc( buf_p, 128 );
    mbMalloc( buf2_p, 32 );
    mbMalloc( dr_p, sizeof(PmcSqlDoctor_t) );

    sprintf( buf_p, "" );
    sprintf( buf2_p, "" );

    if( pmcSqlDoctorDetailsGet( doctorId, dr_p ) == TRUE )
    {
        if(   dr_p->mspActive == PMC_SQL_FALSE_VALUE
            && doctorId != 0
            && ClaimEditInfo_p->mode != PMC_CLAIM_EDIT_MODE_VIEW )
        {
            // MAB:20020331: Remove this warning and instead update the ClaimValidate()
            // function to look for invalid MSP numbers.
            //  mbDlgInfo( "Selected referring doctor (%s)\n"
            //              "does not have an active MSP number.\n"
            //              "Please select a different referring doctor.", lastName_p );
            //  goto exit;
            dr_p->mspNumber = 0;
        }
        ReferringDrId = doctorId;
        ReferringDrNumber = dr_p->mspNumber;

        sprintf( buf_p, "" );
        if( strlen( dr_p->lastName ) )
        {
            sprintf( buf2_p, "%s", dr_p->lastName );
            strcat( buf_p, buf2_p );
        }
        if( strlen( dr_p->firstName ) )
        {
            sprintf( buf2_p, ", %s", dr_p->firstName );
            strcat( buf_p, buf2_p );
        }
    }
    else
    {
        sprintf( buf_p, "Unknown" );
        ReferringDrType = 0;
        ReferringDrTypeIndex = 0;
        ReferringDrNumber = 0;
        typeIndex = 0;
    }

    if( typeIndex == -1 )
    {
        // Attempt to determine what kind of doctor this is (Sask, out of province, or cancer clinic)
        typeIndex = pmcDrTypeIndexGet( dr_p->province, dr_p->cancerClinic );
    }

    if( typeIndex == -1 )
    {
       typeIndex = ReferringDrTypeIndex;
    }
    ReferringDrTypeIndex = typeIndex;
    ReferringDrTypeListBox->ItemIndex = typeIndex;
    ReferringDrTypeListBox->Text = pmcDrType[ ReferringDrTypeIndex ].description_p;
    ReferringDrType = pmcDrType[ReferringDrTypeIndex].code;

    if( ReferringDrTypeIndex == 0 )
    {
        sprintf( buf2_p, "%04d", ReferringDrNumber );
    }
    else
    {
        sprintf( buf2_p, "%04d", ReferringDrType );
    }

    ReferringDrNumberLabel->Caption = buf2_p;
    ReferringDrEdit->Text = buf_p;

    // 20020316: The referring doctor (or lack thereof) can now influence
    // the fee charged.  Must therefore update the fee chanrged whenever
    // the referring Dr changes
    for( Ints_t i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        ComputeFeeCharged( i );
        FeeChargedEditUpdate( i );
    }
    ClaimsValidate( );

    nbDlgDebug(( "Ref dr id: %ld type: %ld number: %ld", ReferringDrId, ReferringDrType, ReferringDrNumber ));

    mbFree( dr_p );
    mbFree( buf_p );
    mbFree( buf2_p );
    return;
}

//---------------------------------------------------------------------------
// Function: ReferringDrTypeListBoxChange( )
//---------------------------------------------------------------------------
// Description:
//
// Change the selected patient
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ReferringDrTypeListBoxChange(TObject *Sender)
{
    // Just display text in the box
    ReferringDrUpdate( ReferringDrId, ReferringDrTypeListBox->ItemIndex );
}

//---------------------------------------------------------------------------
// Function: ClaimsValidate( )
//---------------------------------------------------------------------------
// Description:
//
// This function checks the status of all the claims.  It also determines
// the mode of the claim button.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ClaimsValidate( void )
{
    Int32u_t    i, j;
    Char_p      buf1_p;
    Char_p      buf2_p;
    Int32u_t    yearMonth = 0;
    bool        badDates = FALSE;
    bool        errorFlag = FALSE;
    bool        enable;
    Int32u_t    errorFlagNotifyCount = 0;
    Int32u_t    submitCount;
    Int32u_t    enabledCount = 0;
    Int32u_t    today;
    Int32u_t    fee;

    mbMalloc( buf1_p, 256 );
    mbMalloc( buf2_p, 128 );

    today = mbToday( );

    ReadyCount = 0;
    SubmittedCount = 0;
    PaidCount = 0;
    RejectedCount = 0;
    NotReadyCount = 0;
    ReducedCount = 0;
    ClaimCount = 0;
    NoneCount = 0;
    ReducedAcceptCount = 0;
    RejectedAcceptCount = 0;

    // This first loop checks individual claims
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        EditStatusLabel[i]->Caption = "";

        // First look for statuses that cannot be changed
        if(    Status[i] == PMC_CLAIM_STATUS_SUBMITTED
            || Status[i] == PMC_CLAIM_STATUS_PAID
            || Status[i] == PMC_CLAIM_STATUS_REJECTED
            || Status[i] == PMC_CLAIM_STATUS_REDUCED
            || Status[i] == PMC_CLAIM_STATUS_REDUCED_ACCEPT
            || Status[i] == PMC_CLAIM_STATUS_REJECTED_ACCEPT )
        {
            ClaimCount++;
            continue;
        }

        // Sanity Check
        if(    Status[i] != PMC_CLAIM_STATUS_READY
            && Status[i] != PMC_CLAIM_STATUS_NOT_READY
            && Status[i] != PMC_CLAIM_STATUS_EDIT
            && Status[i] != PMC_CLAIM_STATUS_NONE )
        {
            mbDlgExclaim( "Got invalid claim status" );
            continue;
        }

        // Not submitted; check to see it if is ready
        mbStrClean( FeeList[i]->Text.c_str(), buf1_p, TRUE );
        if( strlen( buf1_p ) )
        {
            // Ok, there appears to be something here.  Set status to READY for now
            ClaimCount++;
            Status[i] = PMC_CLAIM_STATUS_READY;

            if( pmcFeeCodeVerify( buf1_p, Date[i], &fee, NIL, &FeeMultiple[i], NIL, NIL, ClaimType[i] ) )
            {
                // The fee cannot be 0
                if( fee == 0 )
                {
                    Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                    sprintf( buf2_p, "Fee code '%s' not valid on this date of service.", buf1_p );
                    EditStatusLabel[i]->Caption = buf2_p;
                    continue;
                }
                else if( FeeCharged[i] == 0 )
                {
                    Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                    EditStatusLabel[i]->Caption = "Fee charged cannot be 0.";
                    continue;
                }

                if( FeeCharged[i] > PMC_MAX_FEE )
                {
                    Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                    EditStatusLabel[i]->Caption = "Fee charged cannot exceed 9999.99";
                    continue;
                }

                if( Units[i] > 1 && FeeMultiple[i] == FALSE )
                {
                    Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                    EditStatusLabel[i]->Caption = "Multiple units not allowed";
                    continue;
                }

                if( i < 7 )
                {
                    if( Date[i] == 0 )
                    {
                        Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                        EditStatusLabel[i]->Caption = "Service date required.";
                        continue;
                    }
                    else
                    {
                        Status[i] = PMC_CLAIM_STATUS_READY;
                    }
                }
                else
                {
                    // This must be a hospital record.  It requires two dates
                    if( Date[i] == 0 )
                    {
                        Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                        EditStatusLabel[i]->Caption = "First visit date required.";
                        continue;
                    }
                    else if( Date[i + 2] == 0 )
                    {
                        Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                        EditStatusLabel[i]->Caption = "Last visit date required.";
                        continue;
                    }
                    else
                    {
                        Status[i] = PMC_CLAIM_STATUS_READY;
                    }
                }
            }
            else
            {
                // The fee code is not valid
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                sprintf( buf2_p, "Fee code '%s' is invalid.", buf1_p );
                EditStatusLabel[i]->Caption = buf2_p;
            }
        }
        else
        {
            // There is no fee... so this claim is clear
            Status[i] = PMC_CLAIM_STATUS_NONE;
        }
    }

    // Check location code
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if( pmcLocationCodeVerify( pmcLocationCodeGet( LocationListBox->ItemIndex, PremiumComboBox[i]->ItemIndex ) ) == FALSE )
        {
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[i]->Caption = "Premium code not permitted with this location of service.";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Check that ICD code is specified
    mbStrClean( IcdComboBox->Text.c_str(), buf1_p, TRUE );
    if( pmcIcdVerify( buf1_p, NIL, FALSE ) != TRUE )
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[i]->Caption = "Invalid ICD code.";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Check the referring doctor number
    if( ReferringDrId != 0 && ReferringDrNumber == 0 )
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[i]->Caption = "Referring Dr. does not have an active MSP number.";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Do a rudimentary check on the hospital visit dates
    if( Date[7] && Date[9] )
    {
        Int32u_t    yearMonthStart;
        Int32u_t    yearMonthEnd;
        if( Date[7] > Date[9] )
        {
            if( Status[7] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[7]->Caption = "First visit date is after last visit date.";
                Status[7] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
        yearMonthStart = Date[7] / 100;
        yearMonthEnd   = Date[9] / 100;
        if( yearMonthStart != yearMonthEnd )
        {
            EditStatusLabel[7]->Caption = "First visit date and last visit date in different months.";
            Status[7] = PMC_CLAIM_STATUS_NOT_READY;
        }
    }

    if( Date[8] && Date[10] )
    {
        Int32u_t    yearMonthStart;
        Int32u_t    yearMonthEnd;
        if( Date[8] > Date[10] )
        {
            if( Status[8] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[8]->Caption = "First visit date is after last visit date.";
                Status[8] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
        yearMonthStart = Date[8]  / 100;
        yearMonthEnd   = Date[10] / 100;
        if( yearMonthStart != yearMonthEnd )
        {
            EditStatusLabel[8]->Caption = "First visit date and last visit date in different months.";
            Status[8] = PMC_CLAIM_STATUS_NOT_READY;
        }
    }

    errorFlag = FALSE;
    errorFlagNotifyCount = 0;

    // Nested loop to look for duplicate claims
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        for( j = 0 ; j < PMC_CLAIM_COUNT ; j++ )
        {
            // Don't compare claim to itself
            if( i != j )
            {
                // Check for similar non-zero dates
                if( Date[i] != 0 && ( Date[i] == Date[j] ) )
                {
                    // Get fee codes into known state for compare
                    pmcFeeCodeFormatDatabase( FeeList[i]->Text.c_str(), buf1_p );
                    pmcFeeCodeFormatDatabase( FeeList[j]->Text.c_str(), buf2_p );
                    if( strcmp( buf1_p, buf2_p ) == 0 )
                    {
                        if( strlen( buf1_p ) > 0 )
                        {
                            EditStatusLabel[j]->Caption = "Duplicate fee code - service date pair detected.";
                            errorFlag = TRUE;
                            if( Status[j] == PMC_CLAIM_STATUS_READY )
                            {
                                Status[j] = PMC_CLAIM_STATUS_NOT_READY;
                                errorFlagNotifyCount++;
                            }
                        }
                    }
                }
            }
        }
    }

    // Sanity Check
    //if( errorFlag == TRUE && errorFlagNotifyCount == 0 )
    //{
    //    mbDlgDebug(( "Duplicate claim error. Contact system administrator." ));
    //}

    // Ensure month and year are the same for all claims
    yearMonth = 0;
    for( i = 0 ; i < PMC_CLAIM_COUNT  ; i++ )
    {
        if( Date[i] && Status[i] != PMC_CLAIM_STATUS_NONE )
        {
            if( yearMonth == 0 ) yearMonth = ( Date[i] / 100 );

            if( ( Date[i] / 100 ) != yearMonth ) badDates = TRUE;
        }
    }

    errorFlag = FALSE;
    errorFlagNotifyCount = 0;

    if( badDates )
    {
        errorFlag = TRUE;
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            // Basically, the controls will be disabled if the claim is PAID or SUBMITTED
            if( Status[i] != PMC_CLAIM_STATUS_NONE )
            {
                EditStatusLabel[i]->Caption = "Year and month must be the same for all claims.";
            }
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                errorFlagNotifyCount++;
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Check for service dates in the future
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if( Status[i] == PMC_CLAIM_STATUS_READY )
        {
            if( Date[i] > today )
            {
                EditStatusLabel[i]->Caption = "Date of service is in future.";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Sanity Check
    if( errorFlag == TRUE && errorFlagNotifyCount == 0 )
    {
        // MAB:20020330: Can't remember why I poppped up this mbox.  It is coming up
        // when there are legitimate date errors, e.g., different months on the
        // same claim.  Disable for now
        // mbDlgDebug(( "Date error. Contact system administrator." ));
    }

    // Check validity of patient
    if( PatientId == 0 )
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[i]->Caption = "No patient specified.";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Check validity of patient
    if( PatientId != 0 && PatientDob == 0 )
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            // No date of birth for specified patient
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[i]->Caption = "No date of birth specified for patient.";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    if( PatientId != 0 && mbStrPos( PatientPhnProv_p, PMC_PHN_DEFAULT_PROVINCE ) == 0 )
    {
        // Check validity of Sask PHN
        if( pmcPhnVerifyString( PatientPhn_p ) == FALSE )
        {
            for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
            {
                if( Status[i] == PMC_CLAIM_STATUS_READY )
                {
                    EditStatusLabel[i]->Caption = "Invalid PHN.";
                    Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                }
            }
        }
    }

    // Check that provider is specified
    if( ProviderId == 0 )
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
                EditStatusLabel[i]->Caption = "No provider specified.";
            }
        }
    }

    if( ProviderNumber == ReferringDrNumber )
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[i]->Caption = "Provider and referring doctor cannot be the same.";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Check the referringdr number
    if( ReferringDrNumber > 9999 )
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( Status[i] == PMC_CLAIM_STATUS_READY )
            {
                EditStatusLabel[i]->Caption = "Invalid referring Dr. number (must be < 10000).";
                Status[i] = PMC_CLAIM_STATUS_NOT_READY;
            }
        }
    }

    // Check to see if any claims stored as not ready are now ready.  This
    // can happen if for example, the claim was saved with a bad phn, but the
    // phn was fixed externally. We want to update the status in the record
    // here, because the user could press "cancel", but we want the claim
    // status to be updated regardless
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if( Status[i] == PMC_CLAIM_STATUS_READY && StatusIn[i] == PMC_CLAIM_STATUS_NOT_READY && UpdateClaimStatus == TRUE )
        {
            mbDlgExclaim( "Updating claim status to 'Ready' (Claim id %ld)", ClaimId[i] );
            sprintf( buf1_p, "update %s set %s=%ld where %s=%ld",
                PMC_SQL_TABLE_CLAIMS,
                PMC_SQL_CLAIMS_FIELD_STATUS, PMC_CLAIM_STATUS_READY,
                PMC_SQL_FIELD_ID, ClaimId[i] );
            pmcSqlExec( buf1_p );

            // This is getting really ugly, but we must inform the claim
            // list to update the list even if the used clicks cancel
            StatusIn[i] =  PMC_CLAIM_STATUS_READY;
            ClaimEditInfo_p->updateClaims = TRUE;
        }
    }


    // Update the status fields
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        StatusLabel[i]->Caption = pmcClaimStatusStrings[ Status[i] ];
        StatusLabel[i]->Color = (TColor)pmcClaimStatusColors[ Status[i] ];
    }

    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        switch( Status[i] )
        {
            case PMC_CLAIM_STATUS_READY:
                ReadyCount++;
                break;
            case PMC_CLAIM_STATUS_NOT_READY:
                NotReadyCount++;
                break;
            case PMC_CLAIM_STATUS_SUBMITTED:
                SubmittedCount++;
                break;
            case PMC_CLAIM_STATUS_PAID:
                PaidCount++;
                break;
            case PMC_CLAIM_STATUS_REJECTED:
                RejectedCount++;
                break;
            case PMC_CLAIM_STATUS_NONE:
                NoneCount++;
                break;
            case PMC_CLAIM_STATUS_REJECTED_ACCEPT:
                RejectedAcceptCount++;
                break;
            case PMC_CLAIM_STATUS_REDUCED_ACCEPT:
                ReducedAcceptCount++;
                break;
            case PMC_CLAIM_STATUS_REDUCED:
                ReducedCount++;
                break;
            default:
                mbDlgDebug(( "Status %d not enumerated", Status[i] ));
                break;
        }
    }

    // Count all previously submitted claims on this form
    for( i = 0, submitCount = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        submitCount += SubmitCount[i];
    }

    // Enable selected controls - Enable no controls if some claims submitted
    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        enable = FALSE;

        if( ClaimEditInfo_p->mode != PMC_CLAIM_EDIT_MODE_VIEW && SubmittedCount == 0 )
        {
            // Check for resubmissions on this claim

            if( Status[i] == PMC_CLAIM_STATUS_READY ||
                Status[i] == PMC_CLAIM_STATUS_NOT_READY )
            {
               enable = TRUE;
            }
            if( Status[i] == PMC_CLAIM_STATUS_NONE && submitCount == 0 )
            {
               enable = TRUE;
            }
        }
        if( enable ) enabledCount++;

        DateButton[i]->Enabled      = enable;
        UnitsEdit[i]->Enabled       = enable;
        FeeList[i]->Enabled         = enable;
        CommentEdit[i]->Enabled     = enable;
        FeeChargedEdit[i]->Enabled  = enable;
        PremiumComboBox[i]->Enabled = enable;

        if( i == 7 || i == 8 ) DateButton[i+2]->Enabled = enable;
    }

    enable = FALSE;
    if( SubmittedCount > 0 )
    {
        // Do not allow provider or patient to be changed if claim is pending or paid
    }
    else
    {
        if( ClaimEditInfo_p->mode != PMC_CLAIM_EDIT_MODE_VIEW && enabledCount > 0 )
        {
            enable = TRUE;
        }
    }

    ReferringDrSelectButton->Enabled    = enable;
    ReferringDrTypeListBox->Enabled     = enable;
    ReferringDrClearButton->Enabled     = enable;
    IcdComboBox->Enabled                = enable;
    LocationListBox->Enabled            = enable;
    DiagnosisEdit->Enabled              = enable;

    enable = TRUE;
    if(    SubmittedCount > 0
        || PaidCount > 0
        || ReducedAcceptCount > 0
        || RejectedAcceptCount > 0
        || ReducedCount > 0
        || RejectedCount > 0 )
    {
        enable = FALSE;
    }

    if( ClaimEditInfo_p->mode != PMC_CLAIM_EDIT_MODE_VIEW )
    {
        PatientSelectButton->Enabled        = enable;
        PatientEdit->Enabled                = enable;
        if( SubmittedCount == 0 && ClaimNumber == PMC_CLAIM_NUMBER_NOT_ASSIGNED )
        {
            ProviderListBox->Enabled = TRUE;
        }
    }

    // Allow patient and dr edits all the time... after all... it can
    // be done anyway from the patient and dr lists.
    enable = FALSE;

// MAB:20060522 Comment this check out if( ClaimEditInfo_p->mode != PMC_CLAIM_EDIT_MODE_NEW )
    {
        enable = TRUE;
    }
    PatientEditButton->Enabled          = enable;
    ReferringDrEditButton->Enabled      = enable;

    mbFree( buf1_p );
    mbFree( buf2_p );

    return;
}

//---------------------------------------------------------------------------
// Function: ClaimsRead( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ClaimsRead( void )
{
    Char_p          buf_p;
    Char_p          buf2_p;
    Int32u_t        index;
    Int32u_t        id;
    Int32u_t        claimNumber;
    Int32u_t        headerProviderId;
    Int64u_t        timeModified;
    Int64u_t        timeCreated;
    Int32u_t        serviceDateLast;
    MbDateTime      dateTime;
    Boolean_t       icdCodeFlag = FALSE;
    Int32s_t        locationCode;
    Int32s_t        locationIndex;
    Int32s_t        premiumIndex;
    Int32s_t        testLocationIndex = -1;
    Int32u_t        viewCount;
    Int32u_t        noneCount;
    MbSQL           sql;
    Boolean_t       result = FALSE;

    mbMalloc( buf_p, 2048 );
    mbMalloc( buf2_p, 512 );

    ProviderId      = ClaimEditInfo_p->providerId;
    ClaimNumber     = ClaimEditInfo_p->claimNumber;
    ClaimHeaderId   = ClaimEditInfo_p->claimHeaderId;

    if( ClaimNumber == 0 ) ClaimNumber = PMC_CLAIM_NUMBER_NOT_ASSIGNED;

    // Must lock the claim header record in order to edit claims
    if( ClaimEditInfo_p->mode == PMC_CLAIM_EDIT_MODE_EDIT )
    {
        for( ; ; )
        {
            if( !pmcSqlRecordLock( PMC_SQL_TABLE_CLAIM_HEADERS, ClaimHeaderId, TRUE ) )
            {
                if( mbDlgYesNo( "Claim locked for editing by another user.\n\n"
                                   "To unlock this claim for editing, click Yes.\n"
                                   "To leave the claim locked, click No.\n\n"
                                   "WARNING: Unlocking a locked claim can cause database corruption.\n"
                                   "Unlock only if you are sure it is safe to do so." ) == MB_BUTTON_NO )
                {
                    mbDlgInfo( "Claim remains locked.  Opening for viewing only.\n" );
                    PatientSelectButton->Enabled = FALSE;
                    ProviderListBox->Enabled = FALSE;
                    PatientEdit->Enabled = FALSE;

                    ClaimEditInfo_p->mode = PMC_CLAIM_EDIT_MODE_VIEW;
                    ClaimHeaderLocked = FALSE;
                    break;
                }
                else
                {
                    sprintf( buf_p, "update %s set %s=0 where %s=%ld",
                        PMC_SQL_TABLE_CLAIM_HEADERS,
                        PMC_SQL_FIELD_LOCK,
                        PMC_SQL_FIELD_ID, ClaimHeaderId );

                    if( pmcSqlExec( buf_p ) == MB_RET_OK )
                    {
                        mbDlgInfo( "Claim unlocked." );
                    }
                    else
                    {
                        mbDlgInfo( "Failed to unlock claim." );
                    }

                }
            }
            else
            {
                ClaimHeaderLocked = TRUE;
                break;
            }
        }
    }

    // Read the claim header
    //                        0   1   2   3   4   5   6   7   8  9
    sprintf( buf_p, "select %s0,%s1,%s2,%s3,%s4,%s5,%s6,%s7,%s8,%s,"
                           " %s, %s, %s from %s where %s=%ld",
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 0
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 1
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 2
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 3
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 4
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 5
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 6
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 7
        PMC_SQL_CLAIM_HEADERS_FIELD_CLAIM_ID,       // 8
        PMC_SQL_FIELD_PROVIDER_ID,                  // 9
        PMC_SQL_CLAIM_HEADERS_FIELD_DIAGNOSIS,      // 10
        PMC_SQL_FIELD_CREATED,                      // 11
        PMC_SQL_FIELD_MODIFIED,                     // 12

        PMC_SQL_TABLE_CLAIM_HEADERS,
        PMC_SQL_FIELD_ID, ClaimHeaderId );


    index = 0;

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet() )
    {
        index++;

        ClaimId[0] = sql.Int32u( 0 );
        ClaimId[1] = sql.Int32u( 1 );
        ClaimId[2] = sql.Int32u( 2 );
        ClaimId[3] = sql.Int32u( 3 );
        ClaimId[4] = sql.Int32u( 4 );
        ClaimId[5] = sql.Int32u( 5 );
        ClaimId[6] = sql.Int32u( 6 );
        ClaimId[7] = sql.Int32u( 7 );
        ClaimId[8] = sql.Int32u( 8 );

        headerProviderId  = sql.Int32u( 9 );

        DiagnosisEdit->Text = sql.String( 10 );

        timeCreated = sql.DateTimeInt64u( 11 );
        timeModified = sql.DateTimeInt64u( 12 );
    }

    // Sanity checks
    if( index != 1 ) mbDlgDebug(( "Error reading claim header, index: %ld", index ));

    if( headerProviderId != ProviderId )
    {
        mbDlgDebug(( "Provider id mismatch (%ld != %ld)", headerProviderId, ProviderId ));
    }

    // Set the claim ID
    sprintf( buf_p, "%ld", ClaimHeaderId );
    ClaimHeaderIdLabel->Caption = buf_p;

    // Create Time
    {
        dateTime.SetDateTime64( timeCreated );
        sprintf( buf_p, "%s %s", dateTime.MDY_DateString( ), dateTime.HMS_TimeString( ) );
        ClaimCreatedLabel->Caption = buf_p;

        dateTime.SetDateTime64( timeModified );
        sprintf( buf_p, "%s %s", dateTime.MDY_DateString( ), dateTime.HMS_TimeString(  ) );
        ClaimModifiedLabel->Caption = buf_p;
    }

    //                       0  1  2  3  4  5  6  7  8  9
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,"
                           "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,"
                           "%s,%s,%s,%s,%s,%s,%s,%s,%s from %s "
                           "where %s=%ld and %s=%ld",

        PMC_SQL_FIELD_ID,                       // 0
        PMC_SQL_CLAIMS_FIELD_CLAIM_INDEX,       // 1
        PMC_SQL_FIELD_PATIENT_ID,               // 2
        PMC_SQL_CLAIMS_FIELD_REFERRING_DR_ID,   // 3
        PMC_SQL_CLAIMS_FIELD_APPOINTMENT_TYPE,  // 4
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE,      // 5
        PMC_SQL_CLAIMS_FIELD_SERVICE_DATE_LAST, // 6
        PMC_SQL_CLAIMS_FIELD_ICD_CODE,          // 7
        PMC_SQL_CLAIMS_FIELD_SERVICE_LOCATION,  // 8
        PMC_SQL_CLAIMS_FIELD_FEE_CODE,          // 9
        PMC_SQL_CLAIMS_FIELD_UNITS,             // 10
        PMC_SQL_CLAIMS_FIELD_FEE_SUBMITTED,     // 11
        PMC_SQL_CLAIMS_FIELD_COMMENT_TO_MSP,    // 12
        PMC_SQL_CLAIMS_FIELD_SUBMIT_DATE,       // 13
        PMC_SQL_CLAIMS_FIELD_REPLY_DATE,        // 14
        PMC_SQL_CLAIMS_FIELD_FEE_PAID,          // 15
        PMC_SQL_CLAIMS_FIELD_EXP_CODE,          // 16
        PMC_SQL_CLAIMS_FIELD_REFERRING_DR_TYPE, // 17
        PMC_SQL_CLAIMS_FIELD_STATUS,            // 18
        PMC_SQL_CLAIMS_FIELD_CLAIM_SEQ,         // 19
        PMC_SQL_CLAIMS_FIELD_EXP_CODE,          // 20
        PMC_SQL_CLAIMS_FIELD_FEE_PAID,          // 21
        PMC_SQL_CLAIMS_FIELD_FEE_CODE_APPROVED, // 22
        PMC_SQL_CLAIMS_FIELD_SUB_COUNT,         // 23
        PMC_SQL_CLAIMS_FIELD_REPLY_STATUS,      // 24
        PMC_SQL_CLAIMS_FIELD_COMMENT_FROM_MSP,  // 25
        PMC_SQL_CLAIMS_FIELD_CLAIM_NUMBER,      // 26
        PMC_SQL_CLAIMS_FIELD_FEE_PREMIUM,       // 27
        PMC_SQL_CLAIMS_FIELD_UNITS_PAID,        // 28

        PMC_SQL_TABLE_CLAIMS,
        PMC_SQL_CLAIMS_FIELD_CLAIM_HEADER_ID, ClaimHeaderId,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( )  )
    {
        // First, read the claim's index.
        index = sql.Int32u( 1 );

        // Sanity check
        if( index >= PMC_CLAIM_COUNT )
        {
            mbDlgDebug(( "Error: bad index: %ld", index ));
            continue;
        }

        // Read claim ID
        id = sql.Int32u( 0 );

        // Sanity check
        if( ClaimId[index] != id )
        {
            mbDlgDebug(( "Error: id %ld does not match claimHeader %ld", id, ClaimId[index] ));
        }

        PatientId                   = sql.Int32u( 2 );
        ReferringDrId               = sql.Int32u( 3 );

        ClaimType[index]            = sql.Int32u( 4 );
        Date[index]                 = sql.Int32u( 5 );
        serviceDateLast             = sql.Int32u( 6 );
        FeeList[index]->Text        = sql.String( 9 );
        Units[index]                = sql.Int32u( 10 );
        FeeCharged[index]           = sql.Int32u( 11 );
        CommentEdit[index]->Text    = sql.String( 12 );
        SubmittedDate[index]        = sql.Int32u( 13 );
        ReplyDate[index]            = sql.Int32u( 14 );
        ReferringDrTypeIndex        = sql.Int32u( 17 );
        Status[index]               = sql.Int32u( 18 );
        StatusIn[index]             = Status[index];
        SeqNumber[index]            = sql.Int32u( 19 );
        strcpy( ExpCode[index], sql.String( 20 ) );
        FeePaid[index]              = sql.Int32u( 21 );
        strcpy( FeeCodeApproved[index], sql.String( 22 ) );
        SubmitCount[index]          = sql.Int32u( 23 );
        ReplyStatus[index]          = sql.Int32u( 24 );
        MspCommentLabel[index]->Caption = sql.String( 25 );
        PremPaid[index]             = sql.Int32u( 27 );
        UnitsPaid[index]            = sql.Int32u( 28 );

         // Must keep copy of the original status (for accepts and resubmits)
        strcpy( OrigFeeCode[index], FeeList[index]->Text.c_str() );

        locationCode = sql.Int32u( 8 );
        pmcLocationIndexesGet( locationCode, &locationIndex, &premiumIndex );
        LocationListBox->ItemIndex = locationIndex;
        PremiumComboBox[index]->ItemIndex = premiumIndex;

        // SanityCheck
        if( testLocationIndex== -1 )
        {
            testLocationIndex = locationIndex;
        }
        else
        {
            if( locationIndex != testLocationIndex ) mbDlgExclaim( "Location index error\n" );
        }

        if( index > 6 ) Date[index+2] = serviceDateLast;

        // Format fee code for display
        pmcFeeCodeFormatDisplay( FeeList[index]->Text.c_str(), buf_p );
        FeeList[index]->Text = buf_p;

        // Read the fee so that its available for edits
        if( !pmcFeeCodeVerify( buf_p, Date[index], &FeeHigh[index], &FeeLow[index],
                               &FeeMultiple[index], &FeeDeterminant[index],
                               &FeeIndex[index], ClaimType[index] ) )
        {
            // mbDlgExclaim( "Invalid fee code '%s' read.", buf_p  );
        }
        else
        {
            if( FeeCharged[index] != FeeHigh[index] * Units[index] )
            {
                if( FeeCharged[index] != FeeLow[index] * Units[index] )
                {
                    nbDlgDebug(( "Detected override on claims read" ));
                    FeeOverride[index] = FeeCharged[index];
                }
                else
                {
                    FeeOverride[index] = 0;
                }
            }
        }

        // Read the ICD code the first time it is encountered.  It is stored by
        // all claim records, but should be the same in all claim records
        if( icdCodeFlag == FALSE )
        {
            mbStrClean( sql.String( 7 ), buf_p, TRUE );
            IcdComboBox->Text = buf_p;
            if( !pmcIcdVerify( buf_p, buf2_p, FALSE ) )
            {
                //mbDlgExclaim( "Invalid ICD code '%s' read.", buf_p  );
            }
            IcdDescEdit->Text = buf2_p;
            icdCodeFlag = TRUE;
        }

        claimNumber = sql.Int32u( 26 );
        sprintf( buf_p, "Not yet assigned" );
        if( claimNumber != PMC_CLAIM_NUMBER_NOT_ASSIGNED )
        {
            sprintf( buf_p, "%ld", claimNumber );
        }
        ClaimNumberLabel->Caption = buf_p;

        // Fee Paid includes the premuim, so subtract it here
        if( FeePaid[index] < PremPaid[index] )
        {
            mbDlgDebug(( "Error: fee paid %ld < premium paid %ld (claim number %ld)",
                FeePaid[index], PremPaid[index], claimNumber ));
        }
        else
        {
            FeePaid[index] -= PremPaid[index];
        }
    }

    // 20010916: Must do a referring dr update immediately after the claim
    // is read so that the referring dr number is set before the claim
    // is validated
    ReferringDrUpdate( ReferringDrId, ReferringDrTypeIndex );

    // 20010909: For now set all premiums the same.  Set all with
    // index to first non-zero index.  This should maintain
    // previous with differing indexes
    for( Ints_t i = 0 ; i < PMC_CLAIM_COUNT - 2 ; i++ )
    {
        if( PremiumComboBox[i]->ItemIndex != 0 )
        {
            premiumIndex = PremiumComboBox[i]->ItemIndex;
            for( Ints_t j = 0 ; j < PMC_CLAIM_COUNT - 2 ; j++ )
            {
                if( PremiumComboBox[j]->ItemIndex == 0 ) PremiumComboBox[j]->ItemIndex = premiumIndex;
            }
            break;
        }
    }

    viewCount = 0;
    noneCount = 0;
    for( Ints_t i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if( Status[i] == PMC_CLAIM_STATUS_NONE ) noneCount++;
        if( Status[i] == PMC_CLAIM_STATUS_PAID ||
            Status[i] == PMC_CLAIM_STATUS_SUBMITTED ||
            Status[i] == PMC_CLAIM_STATUS_REDUCED_ACCEPT ||
            Status[i] == PMC_CLAIM_STATUS_REJECTED_ACCEPT )
        {
            noneCount++;
        }
    }
    if( noneCount > 0 )
    {
        if( noneCount + viewCount == PMC_CLAIM_COUNT )
        {
            // Set this claim t oview mode only
            ClaimEditInfo_p->mode = PMC_CLAIM_EDIT_MODE_VIEW;
            OkButton->Visible = FALSE;
            CancelButton->Caption = "Close";
        }
    }
    result = TRUE;

exit:
    if( result == FALSE )
    {
        mbDlgExclaim(( "Error" ));
    }

    mbFree( buf_p );
    mbFree( buf2_p );
}

//---------------------------------------------------------------------------
// Function: ProviderListBoxChange( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ProviderListBoxChange(TObject *Sender)
{
    bool        update = FALSE;
    Char_p      buf_p;

    mbMalloc( buf_p, 128 );

    NewProviderId = pmcProviderIdGet( ProviderListBox->Text.c_str() );

    // If the provider id is unknown, set it back to what it was before
    if( NewProviderId == 0 )
    {
        ProviderListBox->ItemIndex = ProviderIndex;
        update = TRUE;
    }
    else
    {
        ProviderIndex = ProviderListBox->ItemIndex;

        if( NewProviderId != ProviderId )
        {
            ProviderId = NewProviderId;

            // Update claim edit info struct so that updated provider gets passed
            // back to calling code
            ClaimEditInfo_p->providerId = ProviderId;
            update = TRUE;
        }
        else
        {
            ProviderListBox->Text = pmcProviderDescGet( ProviderId, buf_p );
        }
    }
    if( update ) ProviderUpdate( ProviderId );

    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function: ProviderUpdate( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ProviderUpdate( Int32u_t providerId )
{
    PmcSqlProvider_p    provider_p = NIL;
    Char_t              buf[8];

    if( mbMalloc( provider_p, sizeof(PmcSqlProvider_t) ) == NIL ) goto exit;

    if( pmcSqlProviderDetailsGet( providerId, provider_p ) == FALSE ) goto exit;

    ProviderNumber = provider_p->billingNumber;
    ProviderListBox->Text = provider_p->description;

    sprintf( buf, "%ld", provider_p->clinicNumber );
    ProviderClinicNumberLabel->Caption = buf;

    sprintf( buf, "%ld", provider_p->billingNumber );
    ProviderBillingNumberLabel->Caption = buf;

    ClaimsValidate( );
exit:
    mbFree( provider_p );
    return;
}

//---------------------------------------------------------------------------
// Function: ReferringDrClearButtonClick( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ReferringDrClearButtonClick(TObject *Sender)
{
    ReferringDrId = 0;
    ReferringDrType = 0;
    ReferringDrTypeIndex = 0;

    ReferringDrUpdate( ReferringDrId, ReferringDrTypeIndex );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::PatientViewButtonClick(TObject *Sender)
{
    PatientEditView( PMC_EDIT_MODE_VIEW );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::PatientEditButtonClick(TObject *Sender)
{
    PatientEditView( PMC_EDIT_MODE_EDIT );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------


void __fastcall TClaimEditForm::PatientEditView( Int32u_t mode )
{
    pmcPatEditInfo_t        patEditInfo;
    TPatientEditForm       *patEditForm;

    if( PatientId )
    {
        patEditInfo.patientId = PatientId;
        patEditInfo.mode = mode;
        sprintf( patEditInfo.caption, "Patient Details" );

        patEditForm = new TPatientEditForm( NULL, &patEditInfo );
        patEditForm->ShowModal();
        delete patEditForm;

        if( patEditInfo.returnCode == MB_BUTTON_OK )
        {
            PatientUpdate( PatientId );
        }
        else
        {
            // User must have clicked cancel button
        }
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ReferringViewButtonClick(TObject *Sender)
{
    ReferringDrEditView( PMC_EDIT_MODE_VIEW );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ReferringDrEditButtonClick(TObject *Sender)
{
    ReferringDrEditView( PMC_EDIT_MODE_EDIT );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ReferringDrEditView( Int32u_t mode )
{
    pmcDocEditInfo_t       docEditInfo;
    TDoctorEditForm       *docEditForm;

    if( ReferringDrId )
    {
        docEditInfo.id = ReferringDrId;
        docEditInfo.mode = mode;
        sprintf( docEditInfo.caption, "Doctor Details" );

        docEditForm = new TDoctorEditForm( NULL, &docEditInfo );
        docEditForm->ShowModal();
        delete docEditForm;

        if( docEditInfo.returnCode == MB_BUTTON_OK )
        {
            if( mode == PMC_EDIT_MODE_EDIT )
            {
                ReferringDrUpdate( docEditInfo.id, -1 );
            }
        }
    }
}

//---------------------------------------------------------------------------
// Function: ReferringSelectButtonClick( )
//---------------------------------------------------------------------------
// Description:
//
// Change the selected patient
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ReferringDrSelectButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    // Put up the doctor list form
    docListInfo.mode = PMC_LIST_MODE_SELECT;
    docListInfo.doctorId = ReferringDrId;
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;

    if( docListInfo.returnCode == MB_BUTTON_OK )
    {
        // Do some checks on the referring Dr in the ReferringDrUpdate function
        ReferringDrUpdate( docListInfo.doctorId, -1 );
    }
}

//---------------------------------------------------------------------------
// Function: ExpLabelClick
//---------------------------------------------------------------------------
// Description:
//
//  Display the Exp Code.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ExpLabel0Click(TObject *Sender){ ExpLabelClick( 0 ); }
void __fastcall TClaimEditForm::ExpLabel1Click(TObject *Sender){ ExpLabelClick( 1 ); }
void __fastcall TClaimEditForm::ExpLabel2Click(TObject *Sender){ ExpLabelClick( 2 ); }
void __fastcall TClaimEditForm::ExpLabel3Click(TObject *Sender){ ExpLabelClick( 3 ); }
void __fastcall TClaimEditForm::ExpLabel4Click(TObject *Sender){ ExpLabelClick( 4 ); }
void __fastcall TClaimEditForm::ExpLabel5Click(TObject *Sender){ ExpLabelClick( 5 ); }
void __fastcall TClaimEditForm::ExpLabel6Click(TObject *Sender){ ExpLabelClick( 6 ); }
void __fastcall TClaimEditForm::ExpLabel7Click(TObject *Sender){ ExpLabelClick( 7 ); }
void __fastcall TClaimEditForm::ExpLabel8Click(TObject *Sender){ ExpLabelClick( 8 ); }

void __fastcall TClaimEditForm::ExpLabelClick( Int32u_t index )
{
    if( strlen( ExpLabel[index]->Caption.c_str() ) )
    {
        pmcExpBox( ExpLabel[index]->Caption.c_str() );
    }
}

//---------------------------------------------------------------------------
// Function: StatusLabelXMouseDown
//---------------------------------------------------------------------------
// Description:
//
//  Record the claim index when the mouse is clicked
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::StatusLabel0MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 0 ); }
void __fastcall TClaimEditForm::StatusLabel1MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 1 ); }
void __fastcall TClaimEditForm::StatusLabel2MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 2 ); }
void __fastcall TClaimEditForm::StatusLabel3MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 3 ); }
void __fastcall TClaimEditForm::StatusLabel4MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 4 ); }
void __fastcall TClaimEditForm::StatusLabel5MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 5 ); }
void __fastcall TClaimEditForm::StatusLabel6MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 6 ); }
void __fastcall TClaimEditForm::StatusLabel7MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 7 ); }
void __fastcall TClaimEditForm::StatusLabel8MouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y) { StatusLabelMouseDown( X, Y, 8 ); }

void __fastcall TClaimEditForm::StatusLabelMouseDown( int X, int Y, Int32u_t index )
{
    MouseDownIndex = index;
    MouseDownX = X;
    MouseDownY = Y;
}

//---------------------------------------------------------------------------
// Function: StatusLabelXMouseDown
//---------------------------------------------------------------------------
// Description:
//
//  Figure out what options to enable on the popup menu
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ClaimEditPopupPopup(TObject *Sender)
{
    Int32u_t    i, j;
    bool        enableResubmit = FALSE;

    i =  MouseDownIndex;

    // By default, disable all popup options
    pmcPopupItemEnableAll( ClaimEditPopup, FALSE );

    if( ClaimEditInfo_p->mode != PMC_CLAIM_EDIT_MODE_VIEW )
    {
        if(    Status[i] == PMC_CLAIM_STATUS_READY
            || Status[i] == PMC_CLAIM_STATUS_NOT_READY
            || Status[i] == PMC_CLAIM_STATUS_EDIT )
        {
            {
                if( SubmitCount[i] == 0 )
                {
                    // This claim has never been submitted... allow it to be cleared
                    pmcPopupItemEnable( ClaimEditPopup, PMC_CLAIM_EDIT_POPUP_CLEAR,     TRUE );
                }
                else
                {
                    // This must be a "ready claim" that was previously submitted.  Allow accept
                    pmcPopupItemEnable( ClaimEditPopup, PMC_CLAIM_EDIT_POPUP_ACCEPT,    TRUE );
                }
            }
        }
        else if( Status[i] == PMC_CLAIM_STATUS_REJECTED )
        {
            pmcPopupItemEnable( ClaimEditPopup, PMC_CLAIM_EDIT_POPUP_ACCEPT,    TRUE );

            // MAB:20020413: Check the statuses.  If there is anything that
            // is not REJECTED then do not enable resubmit
            enableResubmit = TRUE;
            for( j = 0 ; j < PMC_CLAIM_COUNT ; j++ )
            {
                if(    Status[j] != PMC_CLAIM_STATUS_REJECTED
                    && Status[j] != PMC_CLAIM_STATUS_NONE )
                {
                    enableResubmit = FALSE;
                }
            }
            if( enableResubmit ) pmcPopupItemEnable( ClaimEditPopup, PMC_CLAIM_EDIT_POPUP_RESUBMIT,  TRUE );
        }

        else if( Status[i] == PMC_CLAIM_STATUS_REDUCED )
        {
            pmcPopupItemEnable( ClaimEditPopup, PMC_CLAIM_EDIT_POPUP_ACCEPT,    TRUE );

            // MAB:20020413: Check the statuses.  If there is anything that
            // is not REDUCED or feePaid != 0 then do not allow resubmit
            enableResubmit = TRUE;
            for( j = 0 ; j < PMC_CLAIM_COUNT ; j++ )
            {
                if(    Status[j] != PMC_CLAIM_STATUS_REDUCED
                    && Status[j] != PMC_CLAIM_STATUS_NONE )
                {
                    enableResubmit = FALSE;
                }
                if( FeePaid[j] != 0 ) enableResubmit = FALSE;
            }
            if( enableResubmit ) pmcPopupItemEnable( ClaimEditPopup, PMC_CLAIM_EDIT_POPUP_RESUBMIT,  TRUE );
        }
    }

    if( strlen( ExpLabel[MouseDownIndex]->Caption.c_str() ) )
    {
        pmcPopupItemEnable( ClaimEditPopup, PMC_CLAIM_EDIT_POPUP_EXP, TRUE );
    }
}

//---------------------------------------------------------------------------
// Function: ClaimEditPopupExpClick
//---------------------------------------------------------------------------
// Description:
//
//  Display Exp code.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ClaimEditPopupExpClick(TObject *Sender)
{
    ExpLabelClick( MouseDownIndex );
}

//---------------------------------------------------------------------------
// Function: ClaimEditPopupClearClick
//---------------------------------------------------------------------------
// Description:
//
//  Display Exp code.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ClaimEditPopupClearClick(TObject *Sender)
{
    Int32u_t    index;

    index = MouseDownIndex;

    FeeList[index]->Text = "";
    Units[index] = 0;
    Date[index] = 0;
    CommentEdit[index]->Text = "";
    Date[index] = 0;
    DateEditUpdate( index );
    UnitsEditUpdate( index );
    FeeCharged[index] = 0;
    FeeChargedEditUpdate( index );
    FeeOverride[index] = 0;
    FeeIndex[index] = 0;
    FeeDeterminant[index] = PMC_FEE_DET_INVALID;
    ClaimsValidate( );
}

//---------------------------------------------------------------------------
// Function: ClaimEditPopupAcceptClick
//---------------------------------------------------------------------------
// Description:
//
//  Display Exp code.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ClaimEditPopupAcceptClick(TObject *Sender)
{
    Int32u_t    i;

    i = MouseDownIndex;

    // Sanity Check
    if(    ReplyStatus[i] != PMC_CLAIM_STATUS_REJECTED
        && ReplyStatus[i] != PMC_CLAIM_STATUS_REDUCED )
    {
        mbDlgExclaim( "Error: Got Accept with wrong status" );
        goto exit;
    }

    if( mbDlgOkCancel( "Once a claim has been accepted it cannot be resubmitted. Continue?" ) == MB_BUTTON_CANCEL )
    {
        goto exit;
    }

    if( ReplyStatus[i] == PMC_CLAIM_STATUS_REJECTED )
    {
        Status[i] = PMC_CLAIM_STATUS_REJECTED_ACCEPT;
    }
    else if(  ReplyStatus[i] == PMC_CLAIM_STATUS_REDUCED )
    {
        Status[i] = PMC_CLAIM_STATUS_REDUCED_ACCEPT;
    }
    else
    {
        // Sanity Check
        mbDlgDebug(( "Got bad accept click (ReplyStatus: %d)", ReplyStatus[i] ));
    }
    ClaimsValidate( );

exit:
    return;

}

//---------------------------------------------------------------------------
// Function: ClaimEditPopupAcceptClick
//---------------------------------------------------------------------------
// Description:
//
//  Display Exp code.
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::ClaimEditPopupResubmitClick
(
      TObject *Sender
)
{
    Int32u_t    i;
    bool        error = FALSE;

    i = MouseDownIndex;

    for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        if(    Status[i] != PMC_CLAIM_STATUS_REJECTED
            && Status[i] != PMC_CLAIM_STATUS_REDUCED
            && Status[i] != PMC_CLAIM_STATUS_NONE )
        {
            error = TRUE;
        }
        if( Status[i] !=  PMC_CLAIM_STATUS_NONE && FeePaid[i] != 0 )
        {
            error = TRUE;
        }
    }

    if( error )
    {
        mbDlgDebug(( "Error resubmitting claim\n" ));
    }
    else
    {
        for( i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
        {
            if( Status[i] !=  PMC_CLAIM_STATUS_NONE ) Status[i] = PMC_CLAIM_STATUS_EDIT;
        }
    }
    ClaimsValidate( );

    return;
}

//---------------------------------------------------------------------------
// Function: Status Label Click
//---------------------------------------------------------------------------
// Description:
//
//  Bring up popup menu on left click
//---------------------------------------------------------------------------


void __fastcall TClaimEditForm::StatusLabel0Click(TObject *Sender) { StatusLabelClick( 0 ); }
void __fastcall TClaimEditForm::StatusLabel1Click(TObject *Sender) { StatusLabelClick( 1 ); }
void __fastcall TClaimEditForm::StatusLabel2Click(TObject *Sender) { StatusLabelClick( 2 ); }
void __fastcall TClaimEditForm::StatusLabel3Click(TObject *Sender) { StatusLabelClick( 3 ); }
void __fastcall TClaimEditForm::StatusLabel4Click(TObject *Sender) { StatusLabelClick( 4 ); }
void __fastcall TClaimEditForm::StatusLabel5Click(TObject *Sender) { StatusLabelClick( 5 ); }
void __fastcall TClaimEditForm::StatusLabel6Click(TObject *Sender) { StatusLabelClick( 6 ); }
void __fastcall TClaimEditForm::StatusLabel7Click(TObject *Sender) { StatusLabelClick( 7 ); }
void __fastcall TClaimEditForm::StatusLabel8Click(TObject *Sender) { StatusLabelClick( 8 ); }

void __fastcall TClaimEditForm::StatusLabelClick( Int32u_t index )
{
    TPoint  screenOrigin, clientPoint;

    screenOrigin.x = 0;
    screenOrigin.y = 0;
    clientPoint = StatusLabel[index]->ClientToScreen( screenOrigin );
    ClaimEditPopup->Popup( clientPoint.x +  MouseDownX, clientPoint.y + MouseDownY );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::PatientListButtonClick(TObject *Sender)
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;

    patListInfo.patientId = 0;
    patListInfo.providerId = 0;
    patListInfo.mode = PMC_LIST_MODE_LIST;
    patListInfo.allowGoto = FALSE;
    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::DoctorListButtonClick(TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    docListInfo.doctorId = 0;
    docListInfo.mode = PMC_LIST_MODE_LIST;
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;
}


void __fastcall TClaimEditForm::PatientEditMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    PatientSelectButtonClick( Sender );
}
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::IcdListButtonClick(TObject *Sender)
{
    pmcIcdListInfo_t       info;
    TIcdForm              *form_p;

    info.mode = PMC_LIST_MODE_SELECT;
    info.codeIn[0] = 0;
    if( strlen( IcdComboBox->Text.c_str() ) ) strcpy( info.codeIn, IcdComboBox->Text.c_str() );
    form_p = new TIcdForm( this, &info );
    form_p->ShowModal( );
    delete form_p;

    if( IcdComboBox->Enabled == TRUE )
    {
        if( info.returnCode == MB_BUTTON_OK && strlen( info.code ) > 0 )
        {
            IcdComboBox->Text = info.code;
            IcdComboBoxExit( Sender );
        }
        IcdComboBox->SetFocus();
    }
    else
    {
        CancelButton->SetFocus();
    }
}
//---------------------------------------------------------------------------

void __fastcall TClaimEditForm::FeeCodePopupPopup(TObject *Sender)
{
    mbDlgInfo( "Fee code edit selected" );    
}
//---------------------------------------------------------------------------

