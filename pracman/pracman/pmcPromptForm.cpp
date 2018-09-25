//---------------------------------------------------------------------------
// File:    pmcPromptForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb 15, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"

#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcUtils.h"

#include "pmcPromptForm.h"
#include "pmcAppHistoryForm.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TAppCommentForm::TAppCommentForm(TComponent* Owner)
    : TForm(Owner)
{

}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

__fastcall TAppCommentForm::TAppCommentForm
(
    TComponent         *Owner,
    pmcAppEditInfo_p    appEditInfo_p
) : TForm(Owner)
{
    Char_p              buf_p;
    MbSQL               sql;
    Boolean_t           result = FALSE;

    mbMalloc( buf_p, 512 );

    AppEditInfo_p = appEditInfo_p;


    AppEditInfo_p->returnCode = MB_BUTTON_CANCEL;

    CommentInEdit->MaxLength = PMC_MAX_COMMENT_LEN - 1;
    CommentOutEdit->MaxLength = PMC_MAX_COMMENT_LEN - 1;

    CommentInEdit->Text = "";
    CommentOutEdit->Text = "";

    sprintf( buf_p, "select %s,%s from %s where %s=%ld",
        PMC_SQL_APPS_FIELD_COMMENT_IN,
        PMC_SQL_APPS_FIELD_COMMENT_OUT,
        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_ID, appEditInfo_p->appointId );


    if( sql.Query( buf_p ) == FALSE ) goto exit;

    if( sql.RowCount( ) != 1 ) goto exit;

    if( sql.RowGet() == FALSE ) goto exit;

    // Get the strings
    CommentInEdit->Text  = sql.String( 0 );
    CommentOutEdit->Text = sql.String( 1 );

    result = TRUE;

    if( AppEditInfo_p->mode == PMC_EDIT_MODE_VIEW )
    {
        OkButton->Visible = FALSE;
        CancelButton->Caption = "Close";
        CommentInEdit->Enabled = FALSE;
        CommentOutEdit->Enabled = FALSE;
    }


exit:

    if( result == FALSE )
    {
        mbDlgError( "Error reading appointment comments from database" );
    }
    mbFree( buf_p );

}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TAppCommentForm::OkButtonClick(TObject *Sender)
{
    Char_p  buf_p;

    mbMalloc( buf_p, PMC_MAX_COMMENT_LEN );

    AppEditInfo_p->returnCode = MB_BUTTON_OK;

    pmcSqlExecString( PMC_SQL_TABLE_APPS,
                      PMC_SQL_APPS_FIELD_COMMENT_IN,
                      mbStrClean( CommentInEdit->Text.c_str(), buf_p, TRUE ),
                      AppEditInfo_p->appointId );

    pmcAppHistory( AppEditInfo_p->appointId, PMC_APP_ACTION_COMMENT, 0, 0, 0, 0, buf_p );

#if 0
    pmcSqlExecString( PMC_SQL_TABLE_APPS,
                      PMC_SQL_APPS_FIELD_COMMENT_OUT,
                      mbStrClean( CommentOutEdit->Text.c_str(), buf_p, TRUE ),
                      AppEditInfo_p->appointId );

    pmcAppHistory( AppEditInfo_p->appointId, PMC_APP_ACTION_COMMENT_2, 0, 0, 0, 0, buf_p );
#endif

    mbFree( buf_p );
    Close();
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TAppCommentForm::CancelButtonClick(TObject *Sender)
{
    nbDlgDebug(( "CANCEL button clicked" ));
    AppEditInfo_p->returnCode = MB_BUTTON_CANCEL;
    Close();
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TAppCommentForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
   Action = caFree;
}
//---------------------------------------------------------------------------

