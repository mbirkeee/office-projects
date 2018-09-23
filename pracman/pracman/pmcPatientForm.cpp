//---------------------------------------------------------------------------
// Function: pmcPatientForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date: Jan. 19 2002
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#pragma hdrstop

#include "mbUtils.h"
#include "pmcGlobals.h"
#include "pmcPatientForm.h"
#include "pmcPatientListForm.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TPatientForm::TPatientForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
__fastcall TPatientForm::TPatientForm
(
    TComponent*             Owner,
    pmcPatientFormInfo_p    formInfo_p
)
    : TForm(Owner)
{
    FormInfo_p = formInfo_p;
    UpdateBanner( formInfo_p->patientId );
}
//---------------------------------------------------------------------------
void __fastcall TPatientForm::CancelButtonClick(TObject *Sender)
{
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TPatientForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    Action = caFree;
    FormInfo_p->canDelete = TRUE;
}

//---------------------------------------------------------------------------
// Function: pmcPatientFormListAdd
//---------------------------------------------------------------------------
// Description:
//
// This function adds an entry to the linked list of patient form windows
//---------------------------------------------------------------------------

pmcPatientFormList_p    pmcPatientFormListAdd( )
{
    pmcPatientFormList_p    list_p;

    // First clean out any stale entries in the list
    pmcPatientFormListClean( );

    // Allocate memory for new entry
    mbCalloc( list_p, sizeof( pmcPatientFormList_t ) );

    // Can't delete this entry until the window itself sets this to true
    list_p->info.canDelete = FALSE;

    // Put entry in the list
    mbLockAcquire( pmcPatientFormListLock );
    qInsertLast( pmcPatientForm_q, list_p );
    mbLockRelease( pmcPatientFormListLock );

    return list_p;
}

//---------------------------------------------------------------------------
// Function: pmcPatientFormListClean
//---------------------------------------------------------------------------
// Description:
//
// This function adds an entry to the linked list of patient form windows
//---------------------------------------------------------------------------

void pmcPatientFormListClean( )
{
    Ints_t                  size, i;
    pmcPatientFormList_p    list_p;

    mbLockAcquire( pmcPatientFormListLock );

    size = pmcPatientForm_q->size;
    for( i = 0 ; i < size ; i++ )
    {
        list_p = (pmcPatientFormList_p)qRemoveFirst( pmcPatientForm_q );
        if( list_p->info.canDelete )
        {
            // if( list_p->form_p )delete list_p->form_p;

            mbFree( list_p );
        }
        else
        {
            qInsertLast( pmcPatientForm_q, list_p );
        }
    }

    mbLockRelease( pmcPatientFormListLock );
    return;
}

//---------------------------------------------------------------------------
// Function: pmcPatientFormNonModal
//---------------------------------------------------------------------------
// Description:
//
// This function adds an entry to the linked list of patient form windows
//---------------------------------------------------------------------------

void pmcPatientFormNonModal( Int32u_t patientId )
{
    TPatientListForm       *patListForm_p;
    PmcPatListInfo_t        patListInfo;
    pmcPatientFormList_p    list_p;

    if( patientId == 0 )
    {
        // First lets pull up the patient list form
        patListInfo.patientId = 0;
        patListInfo.providerId = 0;
        patListInfo.mode = PMC_LIST_MODE_LIST;
        patListInfo.allowGoto = FALSE;

        patListForm_p = new TPatientListForm( NIL, &patListInfo );
        patListForm_p->ShowModal( );
        delete patListForm_p;

        if( patListInfo.returnCode == MB_BUTTON_OK ) patientId = patListInfo.patientId;
    }

    if( patientId )
    {
        list_p = pmcPatientFormListAdd( );
        if( list_p == NIL )
        {
            mbDlgDebug(( "Error adding entry to patient form list." ));
            goto exit;
        }

        // Set any info that should be passed into the form
        list_p->info.patientId = patientId;

        // Create the form
        list_p->form_p = new TPatientForm( NIL, &list_p->info );

        // Show the form
        list_p->form_p->Show( );
    }
exit:
    return;
}

//---------------------------------------------------------------------------
// Function: UpdateBanner
//---------------------------------------------------------------------------
// Description:
//
// Update the banner labels based on the patient id
//---------------------------------------------------------------------------

void __fastcall TPatientForm::UpdateBanner( Int32u_t patientId )
{
    Char_p              buf_p;
    Char_p              buf2_p;
    Char_p              buf3_p;
    PmcSqlPatient_p     pat_p;

    mbMalloc( pat_p, sizeof( PmcSqlPatient_t ) );
    mbMalloc( buf_p,       128 );
    mbMalloc( buf2_p,       64 );
    mbMalloc( buf3_p,       64 );

    sprintf( buf_p, "" );
    sprintf( buf2_p, "" );
    sprintf( buf3_p, "" );

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
            }
            pmcFormatPhnDisplay( pat_p->phn, pat_p->phnProv, buf3_p );
            strcpy( buf2_p, pat_p->formattedPhoneDay );
        }
        else
        {
            sprintf( buf_p, "Unknown" );
        }
    }

    BannerNameLabel->Caption = buf_p;
    BannerPhoneLabel->Caption = buf2_p;
    BannerPhnLabel->Caption = buf3_p;

    // Set the window's caption
    Caption = buf_p;

    mbFree( pat_p );
    mbFree( buf_p );
    mbFree( buf2_p );
    mbFree( buf3_p );

    return;
}

