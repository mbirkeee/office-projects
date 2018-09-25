//---------------------------------------------------------------------------
// File:    pmcEchoDetailsForm.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2006, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Dec. 2006
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <vcl.h>
#include <dir.h>
#include <process.h>
#pragma hdrstop

#include "mbUtils.h"                               

#include "pmcTables.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcColors.h"
#include "pmcInitialize.h"
#include "pmcEchoDetailsForm.h"
#include "pmcEchoListForm.h"
#include "pmcDoctorListForm.h"
#include "pmcPatientListForm.h"
#include "pmcPatientEditForm.h"
#include "pmcDateSelectForm.h"
#include "pmcDocumentEditForm.h"
#include "pmcBatchImportForm.h"
#include "pmcPDF.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TEchoDetailsForm::TEchoDetailsForm(TComponent* Owner)
    : TForm(Owner)
{
}

//---------------------------------------------------------------------------
__fastcall TEchoDetailsForm::TEchoDetailsForm(TComponent* Owner, pmcEchoDetailsInfo_p info_p )
    : TForm(Owner)
{
    Boolean_t               success = FALSE;
    TCursor                 origCursor;

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    Info_p = info_p;

    info_p->abortFlag = FALSE;
    MemoChanged = FALSE;
    ChangeCount = 0;
    EchoId = info_p->echoId;
    OkFlag = FALSE;
    ReadOnly = FALSE;

    mbMalloc( Cmd_p, 1024 );

    // Position the form
    {
        Ints_t  height, width, top, left;
        if( mbPropertyWinGet( PMC_WINID_ECHO_DETAILS, &height, &width, &top, &left ) == MB_RET_OK )
        {
            SetBounds( left, top, width, height );
        }
    }

    // Set up the "regurgitation combo boxes
    ComboBox_mv_reg->MaxLength = 128;
    ComboBox_av_reg->MaxLength = 128;
    ComboBox_pv_reg->MaxLength = 128;
    ComboBox_tv_reg->MaxLength = 128;

    ComboBox_mv_reg->Text = "";
    ComboBox_av_reg->Text = "";
    ComboBox_pv_reg->Text = "";
    ComboBox_tv_reg->Text = "";

    pmcPickListBuildNew( ComboBox_mv_reg, PMC_DB_CONFIG_REGURGITATION );
    pmcPickListBuildNew( ComboBox_av_reg, PMC_DB_CONFIG_REGURGITATION );
    pmcPickListBuildNew( ComboBox_pv_reg, PMC_DB_CONFIG_REGURGITATION );
    pmcPickListBuildNew( ComboBox_tv_reg, PMC_DB_CONFIG_REGURGITATION );

    // Set up the "regurgitation combo boxes
    ComboBox_mv_sten->MaxLength = 128;
    ComboBox_av_sten->MaxLength = 128;
    ComboBox_tv_sten->MaxLength = 128;

    ComboBox_mv_sten->Text = "";
    ComboBox_av_sten->Text = "";
    ComboBox_tv_sten->Text = "";

    pmcPickListBuildNew( ComboBox_mv_sten, PMC_DB_CONFIG_STENOSIS );
    pmcPickListBuildNew( ComboBox_av_sten, PMC_DB_CONFIG_STENOSIS );
    pmcPickListBuildNew( ComboBox_tv_sten, PMC_DB_CONFIG_STENOSIS );

    ComboBox_ImageQuality->MaxLength = 128;
    ComboBox_ImageQuality->Text = "";
    pmcPickListBuildNew( ComboBox_ImageQuality, PMC_DB_CONFIG_IMAGE_QUALITY );

    ComboBox_rhythm->MaxLength = 128;
    ComboBox_rhythm->Text = "";

    pmcPickListBuildNew( ComboBox_rhythm, PMC_DB_CONFIG_RHYTHM );

    // Get the details
    if( ( Echo_p = pmcSqlEchoDetailsGet( EchoId, TRUE ) ) == NIL ) goto exit;

    pmcProviderListBuild( ComboBox_Provider, Echo_p->provider.id, TRUE, TRUE );

    if( Echo_p->readDate != PMC_ECHO_STATUS_NO_ECHO )
    {
        AssignPatient( Echo_p );
    }

    ControlsUpdate( );

    if( pmcSqlEchoLock( EchoId, Echo_p->studyName_p ) == FALSE )
    {
        mbDlgInfo( "Opening in read-only mode." );
        ControlsDisable( );
        ReadOnly = TRUE;
    }

    success = TRUE;
    ChangeCount = 0;
    LabelChangeCount->Caption = "0";
    Button_Save->Enabled = FALSE;

    if( Echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
       Button_Submit->Enabled = FALSE;
       Button_Approve->Enabled = FALSE;
    }
    else
    {
        Button_Submit->Enabled  = ( Echo_p->readDate == PMC_ECHO_STATUS_IN_PROGRESS || Echo_p->readDate == PMC_ECHO_STATUS_NEW ) ? TRUE : FALSE;
        Button_Approve->Enabled = ( Echo_p->readDate == PMC_ECHO_STATUS_IN_PROGRESS || Echo_p->readDate == PMC_ECHO_STATUS_PENDING ) ? TRUE : FALSE;
    }

exit:

    if( !success )
    {
        mbDlgInfo( "Unable to retrieve data for this study.\n"
                   "If study is locked by another user, try again when study is unlocked.\n"
                   "Otherwise, please inform system administrator." );

        info_p->abortFlag = TRUE;

        pmcSqlEchoDetailsFree( Echo_p );
        Echo_p = NIL;
    }

    Screen->Cursor = origCursor;
}

//---------------------------------------------------------------------------
void __fastcall TEchoDetailsForm::ControlsGet( PmcSqlEchoDetails_p echo_p )
{
    Char_p  buf_p;

    // Allocate a giant chunk of memory
    mbMalloc( buf_p, 1000000 );

    memcpy( echo_p, Echo_p, sizeof( PmcSqlEchoDetails_t ) );

    mbMallocStr( echo_p->comment_p      , mbStrClean( Edit_Comment->Text.c_str()            , buf_p, TRUE ) );
    mbMallocStr( echo_p->studyName_p    , mbStrClean( Edit_StudyName->Text.c_str()          , buf_p, TRUE ) );
    mbMallocStr( echo_p->text_p         , mbStrClean( Memo1->Text.c_str()                   , buf_p, TRUE ) );
    mbMallocStr( echo_p->indication_p   , mbStrClean( Edit_Indication->Text.c_str()         , buf_p, TRUE ) );
    mbMallocStr( echo_p->imageQuality_p , mbStrClean( ComboBox_ImageQuality->Text.c_str()   , buf_p, TRUE ) );
    mbMallocStr( echo_p->rhythm_p       , mbStrClean( ComboBox_rhythm->Text.c_str()         , buf_p, TRUE ) );
    mbMallocStr( echo_p->mv_reg_p       , mbStrClean( ComboBox_mv_reg->Text.c_str()         , buf_p, TRUE ) );
    mbMallocStr( echo_p->mv_sten_p      , mbStrClean( ComboBox_mv_sten->Text.c_str()        , buf_p, TRUE ) );
    mbMallocStr( echo_p->av_reg_p       , mbStrClean( ComboBox_av_reg->Text.c_str()         , buf_p, TRUE ) );
    mbMallocStr( echo_p->av_sten_p      , mbStrClean( ComboBox_av_sten->Text.c_str()        , buf_p, TRUE ) );
    mbMallocStr( echo_p->pv_reg_p       , mbStrClean( ComboBox_pv_reg->Text.c_str()         , buf_p, TRUE ) );
    mbMallocStr( echo_p->tv_reg_p       , mbStrClean( ComboBox_tv_reg->Text.c_str()         , buf_p, TRUE ) );
    mbMallocStr( echo_p->tv_sten_p      , mbStrClean( ComboBox_tv_sten->Text.c_str()        , buf_p, TRUE ) );

    echo_p->cd_rv   = StrToFloat( Edit_cd_rv->Text.c_str()   );
    echo_p->cd_aa   = StrToFloat( Edit_cd_aa->Text.c_str()   );
    echo_p->cd_la   = StrToFloat( Edit_cd_la->Text.c_str()   );
    echo_p->cd_lved = StrToFloat( Edit_cd_lved->Text.c_str() );
    echo_p->cd_lves = StrToFloat( Edit_cd_lves->Text.c_str() );
    echo_p->cd_sept = StrToFloat( Edit_cd_sept->Text.c_str() );
    echo_p->cd_pw   = StrToFloat( Edit_cd_pw->Text.c_str()   );
    echo_p->cd_mi   = StrToFloat( Edit_cd_mi->Text.c_str()   );
    echo_p->cd_lvef = StrToFloat( Edit_cd_lvef->Text.c_str() );

    echo_p->mv_mrja = StrToFloat( Edit_mv_mrja->Text.c_str() );
    echo_p->mv_va   = StrToFloat( Edit_mv_va->Text.c_str()   );
    echo_p->mv_pev  = StrToFloat( Edit_mv_pev->Text.c_str()  );
    echo_p->mv_pav  = StrToFloat( Edit_mv_pav->Text.c_str()  );
    echo_p->mv_mg   = StrToFloat( Edit_mv_mg->Text.c_str()   );
    echo_p->mv_ivrt = StrToFloat( Edit_mv_ivrt->Text.c_str() );
    echo_p->mv_edt  = StrToFloat( Edit_mv_edt->Text.c_str()  );
                                                              
    echo_p->av_ajl  = StrToFloat( Edit_av_ajl->Text.c_str()  );
    echo_p->av_apht = StrToFloat( Edit_av_apht->Text.c_str() );
    echo_p->av_mv   = StrToFloat( Edit_av_mv->Text.c_str()   );
    echo_p->av_pg   = StrToFloat( Edit_av_pg->Text.c_str()   );
    echo_p->av_mg   = StrToFloat( Edit_av_mg->Text.c_str()   );
    echo_p->av_ld   = StrToFloat( Edit_av_ld->Text.c_str()   );
    echo_p->av_lv   = StrToFloat( Edit_av_lv->Text.c_str()   );
    echo_p->av_vti  = StrToFloat( Edit_av_vti->Text.c_str()  );
    echo_p->av_va   = StrToFloat( Edit_av_va->Text.c_str()   );
                                                              
    echo_p->pv_vel  = StrToFloat( Edit_pv_vel->Text.c_str()  );
    echo_p->pv_pat  = StrToFloat( Edit_pv_pat->Text.c_str()  );
    echo_p->pv_grad = StrToFloat( Edit_pv_grad->Text.c_str() );
    echo_p->pvf_sys = StrToFloat( Edit_pvf_sys->Text.c_str() );
    echo_p->pvf_dia = StrToFloat( Edit_pvf_dia->Text.c_str() );
    echo_p->pvf_ar  = StrToFloat( Edit_pvf_ar->Text.c_str()  );
    echo_p->pvf_laa = StrToFloat( Edit_pvf_laa->Text.c_str() );
                                                              
    echo_p->rate    = StrToFloat( Edit_rate->Text.c_str()    );
    echo_p->tv_trja = StrToFloat( Edit_tv_trja->Text.c_str() );
    echo_p->tv_rvsp = StrToFloat( Edit_tv_rvsp->Text.c_str() );
    echo_p->tv_ev   = StrToFloat( Edit_tv_ev->Text.c_str()   );
    echo_p->tv_va   = StrToFloat( Edit_tv_va->Text.c_str()   );

    mbFree( buf_p );
    return;
}

//---------------------------------------------------------------------------
void __fastcall TEchoDetailsForm::ControlsUpdate( void )
{
    MbDateTime              dateTime;
    Char_p                  buf_p;

    mbMalloc( buf_p, 1024 );
    ControlUpdateInProgress = TRUE;

    ReferringDrUpdate( );
    PatientUpdate( );

    Edit_StudyName->Text = Echo_p->studyName_p;

    // Set form caption
    sprintf( buf_p, "Study Name: %s", Echo_p->studyName_p );
    Caption = buf_p;

    LabelEchoId->Caption = itoa( EchoId, Cmd_p, 10 );

    if( Echo_p->studyDate )
    {
        dateTime.SetDate( Echo_p->studyDate );
        LabelStudyDate->Caption = dateTime.MDY_DateString( );
    }

    if( Echo_p->readDate > PMC_ECHO_STATUS_MAX )
    {
        dateTime.SetDate( Echo_p->readDate );
        LabelReadDate->Caption = dateTime.MDY_DateString( );
        LabelReadDate->Color = clLime;
    }
    else if( Echo_p->readDate == PMC_ECHO_STATUS_PENDING )
    {
        LabelReadDate->Caption = "Pending";
        LabelReadDate->Color = clAqua;
    }
    else if( Echo_p->readDate == PMC_ECHO_STATUS_IN_PROGRESS )
    {
        LabelReadDate->Caption = "In Progress";
        LabelReadDate->Color = clYellow;
    }
    else if( Echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
        LabelReadDate->Caption = "No Echo";
        LabelReadDate->Color = clRed;
    }
    else
    {
        LabelReadDate->Caption = "New";
        LabelReadDate->Color = clRed;
    }

    // Get a bunch of strings
    Edit_Comment->Text          = Echo_p->comment_p;
    Edit_Indication->Text       = Echo_p->indication_p;
    ComboBox_ImageQuality->Text = Echo_p->imageQuality_p;

    LabelSonographer->Caption   = Echo_p->sono.lastName;

    Memo1->Text = Echo_p->text_p;

    // Cardiac dimensions
    Edit_cd_rv->Text     = pmcFloatToStr( Echo_p->cd_rv    , 1 );
    Edit_cd_aa->Text     = pmcFloatToStr( Echo_p->cd_aa    , 1 );
    Edit_cd_la->Text     = pmcFloatToStr( Echo_p->cd_la    , 1 );
    Edit_cd_lved->Text   = pmcFloatToStr( Echo_p->cd_lved  , 1 );
    Edit_cd_lves->Text   = pmcFloatToStr( Echo_p->cd_lves  , 1 );
    Edit_cd_sept->Text   = pmcFloatToStr( Echo_p->cd_sept  , 1 );
    Edit_cd_pw->Text     = pmcFloatToStr( Echo_p->cd_pw    , 1 );
    Edit_cd_mi->Text     = pmcFloatToStr( Echo_p->cd_mi    , 1 );
    Edit_cd_lvef->Text   = pmcFloatToStr( Echo_p->cd_lvef  , 1 );

    // Mitral Valve
    ComboBox_mv_reg->Text   = Echo_p->mv_reg_p;
    ComboBox_mv_sten->Text  = Echo_p->mv_sten_p;
    Edit_mv_mrja->Text      = pmcFloatToStr( Echo_p->mv_mrja  , 1 );
    Edit_mv_va->Text        = pmcFloatToStr( Echo_p->mv_va    , 1 );
    Edit_mv_pev->Text       = pmcFloatToStr( Echo_p->mv_pev   , 1 );
    Edit_mv_pav->Text       = pmcFloatToStr( Echo_p->mv_pav   , 1 );
    Edit_mv_mg->Text        = pmcFloatToStr( Echo_p->mv_mg    , 1 );
    Edit_mv_ivrt->Text      = pmcFloatToStr( Echo_p->mv_ivrt  , 3 );
    Edit_mv_edt->Text       = pmcFloatToStr( Echo_p->mv_edt   , 1 );

    // Aortic valve
    ComboBox_av_reg->Text   = Echo_p->av_reg_p;
    ComboBox_av_sten->Text  = Echo_p->av_sten_p;
    Edit_av_ajl->Text       = pmcFloatToStr( Echo_p->av_ajl   , 1 );
    Edit_av_apht->Text      = pmcFloatToStr( Echo_p->av_apht  , 1 );
    Edit_av_mv->Text        = pmcFloatToStr( Echo_p->av_mv    , 1 );
    Edit_av_pg->Text        = pmcFloatToStr( Echo_p->av_pg    , 1 );
    Edit_av_mg->Text        = pmcFloatToStr( Echo_p->av_mg    , 1 );
    Edit_av_ld->Text        = pmcFloatToStr( Echo_p->av_ld    , 1 );
    Edit_av_lv->Text        = pmcFloatToStr( Echo_p->av_lv    , 1 );
    Edit_av_vti->Text       = pmcFloatToStr( Echo_p->av_vti   , 1 );
    Edit_av_va->Text        = pmcFloatToStr( Echo_p->av_va    , 1 );

    // Pulmonic valve
    ComboBox_pv_reg->Text   = Echo_p->pv_reg_p;
    Edit_pv_vel->Text       = pmcFloatToStr( Echo_p->pv_vel  , 1 );
    Edit_pv_pat->Text       = pmcFloatToStr( Echo_p->pv_pat  , 3 );
    Edit_pv_grad->Text      = pmcFloatToStr( Echo_p->pv_grad , 1 );
    Edit_pvf_sys->Text      = pmcFloatToStr( Echo_p->pvf_sys , 1 );
    Edit_pvf_dia->Text      = pmcFloatToStr( Echo_p->pvf_dia , 1 );
    Edit_pvf_ar->Text       = pmcFloatToStr( Echo_p->pvf_ar  , 1 );
    Edit_pvf_laa->Text      = pmcFloatToStr( Echo_p->pvf_laa , 1 );

    // Tricuspid valve
    ComboBox_tv_reg->Text   = Echo_p->tv_reg_p;
    ComboBox_tv_sten->Text  = Echo_p->tv_sten_p;
    Edit_tv_trja->Text      = pmcFloatToStr( Echo_p->tv_trja , 1 );
    Edit_tv_rvsp->Text      = pmcFloatToStr( Echo_p->tv_rvsp , 1 );
    Edit_tv_ev->Text        = pmcFloatToStr( Echo_p->tv_ev   , 1 );
    Edit_tv_va->Text        = pmcFloatToStr( Echo_p->tv_va   , 1 );

    // Heart rate and thythm
    ComboBox_rhythm->Text   = Echo_p->rhythm_p;
    Edit_rate->Text         = pmcFloatToStr( Echo_p->rate    , 1 );

    ControlUpdateInProgress = FALSE;
    mbFree( buf_p );

    return;
}

//---------------------------------------------------------------------------

Float_t __fastcall TEchoDetailsForm::StrToFloat( Char_p str_p )
{
    Float_t     result;
    Char_t      clean[128];
    mbStrClean( str_p, clean, TRUE );

    if( strlen( clean ) == 0 ) return PMC_FLOAT_NOT_SET;

    result = atof( clean );
    return result;
}


//---------------------------------------------------------------------------
void __fastcall TEchoDetailsForm::Save( Boolean_t reloadFlag, Boolean_t promptFlag )
{
    TCursor         origCursor;
    Char_p          clean0_p = NIL;
    Char_p          clean1_p = NIL;
    Char_p          clean2_p = NIL;
    Char_p          clean3_p = NIL;
    Char_p          clean4_p = NIL;
    Char_p          clean5_p = NIL;
    Char_p          clean6_p = NIL;
    Char_p          clean7_p = NIL;
    Char_p          clean8_p = NIL;
    Char_p          clean9_p = NIL;

    Char_p          command_p = NIL;
    Int32s_t        len1, len2;
    Int32u_t        detailsId;

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    if( Echo_p == NIL ) goto exit;

    detailsId = Echo_p->detailsId;

    if( promptFlag )
    {
        Int32s_t    result;
        if( Echo_p->readDate > PMC_ECHO_STATUS_MAX )
        {
            result = mbDlgYesNo( "The echo %s has already been approved.\n\n"
                             "If the report is changed, it will revert to status 'In Progress'.\n\n"
                             "Are you sure you want to save the changes?\n\n" ,  Echo_p->studyName_p );

            if( result == MB_BUTTON_NO ) goto exit;
            Echo_p->readDate = PMC_ECHO_STATUS_IN_PROGRESS;
        }
        else if( Echo_p->readDate == PMC_ECHO_STATUS_PENDING )
        {
            result = mbDlgYesNo( "The echo %s has already been submitted for approval.\n\n"
                             "If the report is changed, it will revert to status 'In Progress'.\n\n"
                             "Are you sure you want to save the changes?\n\n" ,  Echo_p->studyName_p );

            if( result == MB_BUTTON_NO ) goto exit;
            if( Echo_p->readDate != PMC_ECHO_STATUS_NO_ECHO ) Echo_p->readDate = PMC_ECHO_STATUS_IN_PROGRESS;
        }
    }

    if( reloadFlag && Echo_p->readDate != PMC_ECHO_STATUS_NO_ECHO ) Echo_p->readDate = PMC_ECHO_STATUS_IN_PROGRESS;

    // Update items in the echo structure
    {
        len1 = strlen( Edit_Comment->Text.c_str() );
        len2 = strlen( Edit_StudyName->Text.c_str() );

        mbRemalloc( clean1_p, len1 + 1 );
        mbRemalloc( clean2_p, len2 + 1 );
        mbRemalloc( command_p, len1 + len2 + 1000 );

        mbStrClean( Edit_Comment->Text.c_str(),   clean1_p, TRUE );
        mbStrClean( Edit_StudyName->Text.c_str(), clean2_p, TRUE );

        if( Echo_p->provider.id == 0 )
        {
            mbDlgError( "Attempting to save provider id 0" );
            Echo_p->provider.id = 1;
        }

        sprintf( command_p, "update %s set %s=\"%s\",%s=\"%s\",%s=%lu,%s=%lu,%s=%lu where %s=%lu"
                 ,PMC_SQL_TABLE_ECHOS

                 ,PMC_SQL_ECHOS_FIELD_COMMENT   , clean1_p
                 ,PMC_SQL_ECHOS_FIELD_NAME      , clean2_p
                 ,PMC_SQL_FIELD_PATIENT_ID      , Echo_p->patient.id
                 ,PMC_SQL_FIELD_PROVIDER_ID     , Echo_p->provider.id
                 ,PMC_SQL_ECHOS_FIELD_READ_DATE , Echo_p->readDate

                 ,PMC_SQL_FIELD_ID              , EchoId );

        Sql.Update( command_p );
    }

    // Handle some strings
    if( MemoChanged == TRUE )
    {
        len1 = strlen( Memo1->Text.c_str() );

        mbRemalloc( clean1_p,  len1 + 100 );
        mbRemalloc( command_p, len1 + 100 );

        mbStrClean( Memo1->Text.c_str(), clean1_p, TRUE );

        sprintf( command_p, "update %s set %s=\"%s\" where %s=%lu\n"
            ,PMC_SQL_TABLE_ECHO_DETAILS
            ,PMC_SQL_ECHO_DETAILS_NOTES     , clean1_p
            ,PMC_SQL_FIELD_ID               , detailsId );

        Sql.Update( command_p );
    }

    // Handle some more strings
    {
        mbRemalloc( command_p, 4096 );
        mbRemalloc( clean0_p,  2048 );
        mbRemalloc( clean1_p,  128 );
        mbRemalloc( clean2_p,  128 );
        mbRemalloc( clean3_p,  128 );
        mbRemalloc( clean4_p,  128 );
        mbRemalloc( clean5_p,  128 );
        mbRemalloc( clean6_p,  128 );
        mbRemalloc( clean7_p,  128 );
        mbRemalloc( clean8_p,  128 );
        mbRemalloc( clean9_p,  128 );

        mbStrClean( Edit_Indication->Text.c_str(),          clean0_p, TRUE );
        mbStrClean( ComboBox_ImageQuality->Text.c_str(),    clean1_p, TRUE );
        mbStrClean( ComboBox_mv_reg->Text.c_str(),          clean2_p, TRUE );
        mbStrClean( ComboBox_mv_sten->Text.c_str(),         clean3_p, TRUE );
        mbStrClean( ComboBox_av_reg->Text.c_str(),          clean4_p, TRUE );
        mbStrClean( ComboBox_av_sten->Text.c_str(),         clean5_p, TRUE );
        mbStrClean( ComboBox_pv_reg->Text.c_str(),          clean6_p, TRUE );
        mbStrClean( ComboBox_tv_reg->Text.c_str(),          clean7_p, TRUE );
        mbStrClean( ComboBox_tv_sten->Text.c_str(),         clean8_p, TRUE );
        mbStrClean( ComboBox_rhythm->Text.c_str(),          clean9_p, TRUE );

        //                                         0         1         2         3         4         5        6         7         8         9
        sprintf( command_p, "update %s set %s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\",%s=\"%s\" where %s=%lu"
            ,PMC_SQL_TABLE_ECHO_DETAILS
            ,PMC_SQL_ECHO_DETAILS_INDICATION        , clean0_p
            ,PMC_SQL_ECHO_DETAILS_IMAGE_QUALITY     , clean1_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_REG      , clean2_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_STEN     , clean3_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_REG      , clean4_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_STEN     , clean5_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_PV_REG      , clean6_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_TV_REG      , clean7_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_TV_STEN     , clean8_p
            ,PMC_SQL_ECHO_DETAILS_FIELD_RHYTHM      , clean9_p

            ,PMC_SQL_FIELD_ID                   , detailsId );

        Sql.Update( command_p );
    }

    // Handle the rest of the echo details
    {
        mbRemalloc( command_p, 4096 );

        //                                      0     1     2     3     4     5     6     7     8     9
        sprintf( command_p, "update %s set %s=%lu,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                                         ",%s=%f, %s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                                         ",%s=%f, %s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                                         ",%s=%f, %s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                            " where %s=%lu"

            ,PMC_SQL_TABLE_ECHO_DETAILS

            ,PMC_SQL_FIELD_REFERRING_ID             ,Echo_p->referring.id                      //  0
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_RV       ,StrToFloat( Edit_cd_rv->Text.c_str()   )  //  1
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_AA       ,StrToFloat( Edit_cd_aa->Text.c_str()   )  //  2
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LA       ,StrToFloat( Edit_cd_la->Text.c_str()   )  //  3
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVED     ,StrToFloat( Edit_cd_lved->Text.c_str() )  //  4
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVES     ,StrToFloat( Edit_cd_lves->Text.c_str() )  //  5
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_SEPT     ,StrToFloat( Edit_cd_sept->Text.c_str() )  //  6
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_PW       ,StrToFloat( Edit_cd_pw->Text.c_str()   )  //  7
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_MI       ,StrToFloat( Edit_cd_mi->Text.c_str()   )  //  8
            ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVEF     ,StrToFloat( Edit_cd_lvef->Text.c_str() )  //  9

            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_MRJA     ,StrToFloat( Edit_mv_mrja->Text.c_str() )  // 10
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_VA       ,StrToFloat( Edit_mv_va->Text.c_str()   )  // 11
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_PEV      ,StrToFloat( Edit_mv_pev->Text.c_str()  )  // 12
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_PAV      ,StrToFloat( Edit_mv_pav->Text.c_str()  )  // 13
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_MG       ,StrToFloat( Edit_mv_mg->Text.c_str()   )  // 14
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_IVRT     ,StrToFloat( Edit_mv_ivrt->Text.c_str() )  // 15
            ,PMC_SQL_ECHO_DETAILS_FIELD_MV_EDT      ,StrToFloat( Edit_mv_edt->Text.c_str()  )  // 16

            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_AJL      ,StrToFloat( Edit_av_ajl->Text.c_str()  )   // 17
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_APHT     ,StrToFloat( Edit_av_apht->Text.c_str() )   // 18
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_MV       ,StrToFloat( Edit_av_mv->Text.c_str()   )   // 19
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_PG       ,StrToFloat( Edit_av_pg->Text.c_str()   )   // 20
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_MG       ,StrToFloat( Edit_av_mg->Text.c_str()   )   // 21
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_LD       ,StrToFloat( Edit_av_ld->Text.c_str()   )   // 22
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_LV       ,StrToFloat( Edit_av_lv->Text.c_str()   )   // 23
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_VTI      ,StrToFloat( Edit_av_vti->Text.c_str()  )   // 24
            ,PMC_SQL_ECHO_DETAILS_FIELD_AV_VA       ,StrToFloat( Edit_av_va->Text.c_str()   )   // 25

            ,PMC_SQL_ECHO_DETAILS_FIELD_PV_VEL      ,StrToFloat( Edit_pv_vel->Text.c_str()  )   // 26
            ,PMC_SQL_ECHO_DETAILS_FIELD_PV_PAT      ,StrToFloat( Edit_pv_pat->Text.c_str()  )   // 27
            ,PMC_SQL_ECHO_DETAILS_FIELD_PV_GRAD     ,StrToFloat( Edit_pv_grad->Text.c_str() )   // 28
            ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_SYS     ,StrToFloat( Edit_pvf_sys->Text.c_str() )   // 29
            ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_DIA     ,StrToFloat( Edit_pvf_dia->Text.c_str() )   // 30
            ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_AR      ,StrToFloat( Edit_pvf_ar->Text.c_str()  )   // 31
            ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_LAA     ,StrToFloat( Edit_pvf_laa->Text.c_str() )   // 32

            ,PMC_SQL_ECHO_DETAILS_FIELD_RATE        ,StrToFloat( Edit_rate->Text.c_str()    )   // 33
            ,PMC_SQL_ECHO_DETAILS_FIELD_TV_TRJA     ,StrToFloat( Edit_tv_trja->Text.c_str() )   // 34
            ,PMC_SQL_ECHO_DETAILS_FIELD_TV_RVSP     ,StrToFloat( Edit_tv_rvsp->Text.c_str() )   // 35
            ,PMC_SQL_ECHO_DETAILS_FIELD_TV_EV       ,StrToFloat( Edit_tv_ev->Text.c_str()   )   // 36
            ,PMC_SQL_ECHO_DETAILS_FIELD_TV_VA       ,StrToFloat( Edit_tv_va->Text.c_str()   )   // 37

            ,PMC_SQL_FIELD_ID                       ,detailsId );

        Sql.Update( command_p );
    }

    if( reloadFlag == FALSE ) goto exit;

    pmcSqlEchoDetailsFree( Echo_p );

    // Get the details
    if( ( Echo_p = pmcSqlEchoDetailsGet( EchoId, TRUE ) ) == NIL ) goto exit;

    ControlsUpdate( );

    ChangeCount = 0;
    MemoChanged = FALSE;
    LabelChangeCount->Caption = "0";
    Button_Save->Enabled = FALSE;

    if( Echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
        Button_Submit->Enabled  = FALSE;
        Button_Approve->Enabled = FALSE;
    }
    else
    {
        Button_Submit->Enabled  = ( Echo_p->readDate == PMC_ECHO_STATUS_IN_PROGRESS || Echo_p->readDate == PMC_ECHO_STATUS_NEW )     ? TRUE : FALSE;
        Button_Approve->Enabled = ( Echo_p->readDate == PMC_ECHO_STATUS_IN_PROGRESS || Echo_p->readDate == PMC_ECHO_STATUS_PENDING ) ? TRUE : FALSE;
    }

exit:

    mbFree( clean0_p );
    mbFree( clean1_p );
    mbFree( clean2_p );
    mbFree( clean3_p );
    mbFree( clean4_p );
    mbFree( clean5_p );
    mbFree( clean6_p );
    mbFree( clean7_p );
    mbFree( clean8_p );
    mbFree( clean9_p );
    mbFree( command_p );

    Screen->Cursor = origCursor;
    return;
}

//---------------------------------------------------------------------------
void __fastcall TEchoDetailsForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    TCursor                 origCursor;

    origCursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;

    if( ChangeCount != 0 && OkFlag == FALSE )
    {
        if( mbDlgYesNo( "Close Echo Report window without saving changes?" ) == MB_BUTTON_NO )
        {
            Screen->Cursor = origCursor;
            Action = caNone;
            return;
        }
    }

    if( ChangeCount == 0 || OkFlag == FALSE ) goto exit;

    Save( FALSE, TRUE );

exit:

    Info_p->returnCode = OkFlag;

    mbFree( Cmd_p );
    if( ReadOnly == FALSE )  pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, EchoId );

    pmcSqlEchoDetailsFree( Echo_p );
    Echo_p = NIL;

    mbPropertyWinSave( PMC_WINID_ECHO_DETAILS, Height, Width, Top, Left );
    Screen->Cursor = origCursor;
    Action = caFree;
}


//---------------------------------------------------------------------------

Int32s_t pmcEchoDetailsForm
(
    Int32u_t    echoId
)
{
    Int32s_t                        returnCode = FALSE;
    TEchoDetailsForm               *form_p;
    pmcEchoDetailsInfo_p            info_p;

    mbMalloc( info_p, sizeof( pmcEchoDetailsInfo_t ) );

    info_p->echoId = echoId;

    form_p = new TEchoDetailsForm( NIL, info_p );

    if( info_p->abortFlag == FALSE )
    {
        form_p->ShowModal( );
        returnCode = info_p->returnCode;
    }

    delete form_p;
    mbFree( info_p );

    return returnCode;
}

//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_CancelClick(TObject *Sender)
{
    Close( );
}

//---------------------------------------------------------------------------
// Function: ReferringDrUpdate( )
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::ReferringDrUpdate( void )
{
    Char_p          buf_p;
    Char_p          buf2_p;

    mbMalloc( buf_p,    128 );
    mbMalloc( buf2_p,   128 );

    sprintf( buf_p, "" );
    sprintf( buf2_p, "" );

    if( pmcSqlDoctorDetailsGet( Echo_p->referring.id, &Echo_p->referring ) == TRUE )
    {
        sprintf( buf_p, "" );
        if( strlen( Echo_p->referring.lastName ) )
        {
            sprintf( buf2_p, "%s", Echo_p->referring.lastName );
            strcat( buf_p, buf2_p );
        }
        if( strlen( Echo_p->referring.firstName ) )
        {
            sprintf( buf2_p, ", %s", Echo_p->referring.firstName );
            strcat( buf_p, buf2_p );
        }
    }
    else
    {
        sprintf( buf_p, "Unknown" );
    }

    {
        Char_t      buf[64];
        LabelDrPhone->Caption = pmcPhoneFormatString( Echo_p->referring.phone, buf );
        LabelDrFax->Caption   = pmcPhoneFormatString( Echo_p->referring.fax, buf );
    }

    Edit_ReferringDr->Text = buf_p;

    mbFree( buf_p );
    mbFree( buf2_p );
    return;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::PatientUpdate( void )
{
    Char_p          buf_p;
    Char_p          buf2_p;

    mbMalloc( buf_p,    128 );
    mbMalloc( buf2_p,   128 );

    if( pmcSqlPatientDetailsGet( Echo_p->patient.id, &Echo_p->patient ) == TRUE )
    {
        sprintf( buf_p, "" );
        if( strlen( Echo_p->patient.lastName ) )
        {
            sprintf( buf2_p, "%s", Echo_p->patient.lastName );
            strcat( buf_p, buf2_p );
        }
        if( strlen( Echo_p->patient.firstName ) )
        {
            sprintf( buf2_p, ", %s", Echo_p->patient.firstName );
            strcat( buf_p, buf2_p );
        }
        if( strlen( Echo_p->patient.title ) )
        {
            sprintf( buf2_p, "  (%s)", Echo_p->patient.title );
            strcat( buf_p, buf2_p );
        }
        Edit_Patient->Text = buf_p;

        // Get Date of birth
        if( Echo_p->patient.birthDate != 0 )
        {
            MbDateTime dateTime = MbDateTime( Echo_p->patient.birthDate, 0 );
            PatientDobLabel->Caption = dateTime.MDY_DateString( );
        }
        else
        {
            PatientDobLabel->Caption = "";
        }

        if( Echo_p->patient.gender == 0 )
        {
            LabelPatientGender->Caption = "M";
        }
        else
        {
            LabelPatientGender->Caption = "F";
        }
        pmcFormatPhnDisplay( Echo_p->patient.phn, Echo_p->patient.phnProv, buf2_p );
        PatientPhnLabel->Caption = buf2_p;
    }
    else
    {
        sprintf( buf_p, "Unknown" );
        Edit_Patient->Text = buf_p;
        PatientPhnLabel->Caption = "";
        LabelPatientGender->Caption = "";
        PatientDobLabel->Caption = "";
    }

    mbFree( buf_p );
    mbFree( buf2_p );
    return;
}

//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::EchoDetailsChange(TObject *Sender)
{
    Char_t  buf[32];

    if( ControlUpdateInProgress ) return;

    ChangeCount++;

    if(     Echo_p->readDate != PMC_ECHO_STATUS_IN_PROGRESS
        &&  Echo_p->readDate != PMC_ECHO_STATUS_NO_ECHO )
    {
         Echo_p->readDate = PMC_ECHO_STATUS_IN_PROGRESS;
         ControlsUpdate( );
    }

    LabelChangeCount->Caption = itoa( ChangeCount, buf, 10 );
    Button_Save->Enabled = TRUE;

    if( Echo_p->readDate != PMC_ECHO_STATUS_NO_ECHO )
    {
        Button_Submit->Enabled  = TRUE;
        Button_Approve->Enabled = TRUE;
    }

}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_OKClick(TObject *Sender)
{
    OkFlag = TRUE;
    Close( );
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_ReferringDrSelectClick(
      TObject *Sender)
{
    TDoctorListForm    *docListForm_p;
    pmcDocListInfo_t    docListInfo;

    // Put up the doctor list form
    docListInfo.mode = PMC_LIST_MODE_SELECT;
    docListInfo.doctorId = Echo_p->referring.id;
    docListForm_p = new TDoctorListForm( this, &docListInfo );
    docListForm_p->ShowModal( );
    delete docListForm_p;

    if( docListInfo.returnCode == MB_BUTTON_OK )
    {
        if( docListInfo.doctorId != Echo_p->referring.id )
        {
            Echo_p->referring.id = docListInfo.doctorId;
            ReferringDrUpdate( );
            pmcCheckReferringDr( Echo_p->patient.id, Echo_p->referring.id );
            EchoDetailsChange( NIL );
        }
    }
    return;
}

//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_SaveClick(TObject *Sender)
{
    Save( TRUE, TRUE );
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Memo1Change(TObject *Sender)
{
    MemoChanged = TRUE;
    EchoDetailsChange( NIL );
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_PatientSelectClick(TObject *Sender)
{
    TPatientListForm   *patListForm_p;
    PmcPatListInfo_t    patListInfo;
    Boolean_t           updateRef = FALSE;
    Boolean_t           patientUpdated = FALSE;
    PmcSqlDoctor_p      drNew_p = NIL;

    patListInfo.mode = PMC_LIST_MODE_SELECT;
    patListInfo.patientId = Echo_p->patient.id;
    patListInfo.providerId = Echo_p->provider.id;
    patListInfo.allowGoto = FALSE;

    patListForm_p = new TPatientListForm( this, &patListInfo );
    patListForm_p->ShowModal( );
    delete patListForm_p;

    if( patListInfo.returnCode == MB_BUTTON_OK )
    {
        if( Echo_p->patient.id != patListInfo.patientId )
        {
            Echo_p->patient.id = patListInfo.patientId;
            PatientUpdate( );
            patientUpdated = TRUE;
            if( Echo_p->patient.refDrId != 0 )
            {
                if( Echo_p->referring.id == 0 )
                {
                    updateRef = TRUE;
                }
                else
                {
                    mbMalloc( drNew_p, sizeof(PmcSqlDoctor_t) );
                    if( pmcSqlDoctorDetailsGet( Echo_p->patient.refDrId, drNew_p ) == TRUE )
                    {
                        if( mbDlgYesNo( "Change referring doctor to new patient's default referring doctor, %s %s?\n",
                            drNew_p->firstName, drNew_p->lastName ) == MB_BUTTON_YES )
                        {
                            updateRef = TRUE;
                        }
                    }
                    mbFree( drNew_p )
                }

                if( updateRef )
                {
                    Echo_p->referring.id = Echo_p->patient.refDrId;
                    ReferringDrUpdate( );
                }
            }

            EchoDetailsChange( NIL );
        }
    }

    // Could have edited patient record...
    if( patientUpdated == FALSE ) PatientUpdate( );

    return;
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::ComboBox_ProviderChange(TObject *Sender)
{
    Int32u_t newId;

    newId = pmcProviderIdGet( ComboBox_Provider->Text.c_str() );

    if( newId == 0 ) newId = 1;

    if( newId != Echo_p->provider.id )
    {
        EchoDetailsChange( NIL );
        Echo_p->provider.id = newId;
    }

     pmcProviderListBuild( ComboBox_Provider, Echo_p->provider.id, TRUE, TRUE );
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::ControlsDisable( void )
{
    Edit_cd_rv->Enabled     = FALSE;
    Edit_cd_aa->Enabled     = FALSE;
    Edit_cd_la->Enabled     = FALSE;
    Edit_cd_lved->Enabled   = FALSE;
    Edit_cd_lves->Enabled   = FALSE;
    Edit_cd_sept->Enabled   = FALSE;
    Edit_cd_pw->Enabled     = FALSE;
    Edit_cd_mi->Enabled     = FALSE;
    Edit_cd_lvef->Enabled   = FALSE;

    Edit_mv_mrja->Enabled   = FALSE;
    Edit_mv_va->Enabled     = FALSE;
    Edit_mv_pev->Enabled    = FALSE;
    Edit_mv_pav->Enabled    = FALSE;
    Edit_mv_mg->Enabled     = FALSE;
    Edit_mv_ivrt->Enabled   = FALSE;
    Edit_mv_edt->Enabled    = FALSE;

    Edit_av_ajl->Enabled    = FALSE;
    Edit_av_apht->Enabled   = FALSE;
    Edit_av_mv->Enabled     = FALSE;
    Edit_av_pg->Enabled     = FALSE;
    Edit_av_mg->Enabled     = FALSE;
    Edit_av_ld->Enabled     = FALSE;
    Edit_av_lv->Enabled     = FALSE;
    Edit_av_vti->Enabled    = FALSE;
    Edit_av_va->Enabled     = FALSE;

    Edit_pv_vel->Enabled    = FALSE;
    Edit_pv_pat->Enabled    = FALSE;
    Edit_pv_grad->Enabled   = FALSE;
    Edit_pvf_sys->Enabled   = FALSE;
    Edit_pvf_dia->Enabled   = FALSE;
    Edit_pvf_ar->Enabled    = FALSE;
    Edit_pvf_laa->Enabled   = FALSE;

    Edit_rate->Enabled      = FALSE;
    Edit_tv_trja->Enabled   = FALSE;
    Edit_tv_rvsp->Enabled   = FALSE;
    Edit_tv_ev->Enabled     = FALSE;
    Edit_tv_va->Enabled     = FALSE;

    ComboBox_mv_reg->Enabled        = FALSE;
    ComboBox_av_reg->Enabled        = FALSE;
    ComboBox_pv_reg->Enabled        = FALSE;
    ComboBox_tv_reg->Enabled        = FALSE;
    ComboBox_mv_sten->Enabled       = FALSE;
    ComboBox_av_sten->Enabled       = FALSE;
    ComboBox_tv_sten->Enabled       = FALSE;
    ComboBox_rhythm->Enabled        = FALSE;
    ComboBox_ImageQuality->Enabled  = FALSE;
    ComboBox_Provider->Enabled      = FALSE;

    Edit_StudyName->Enabled     = FALSE;
    Edit_Patient->Enabled       = FALSE;
    Edit_Comment->Enabled       = FALSE;
    Edit_Indication->Enabled    = FALSE;
    Edit_ReferringDr->Enabled   = FALSE;

    Button_PatientSelect->Enabled       = FALSE;
    Button_ReferringDrSelect->Enabled   = FALSE;

    Memo1->Enabled = FALSE;
}

//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_PreviewClick(TObject *Sender)
{
    if( ChangeCount == 0 )
    {
        if( Echo_p->readDate > PMC_ECHO_STATUS_MAX )
        {
            pmcEchoReportGenerate( EchoId, NIL, "", TRUE, FALSE );
        }
        else if( Echo_p->readDate > PMC_ECHO_STATUS_NO_ECHO )
        {
            pmcEchoReportGenerate( EchoId, NIL, "No Echo", TRUE, FALSE );
        }
        else
        {
            pmcEchoReportGenerate( EchoId, NIL, "DRAFT", TRUE, FALSE );
        }
    }
    else
    {
        PmcSqlEchoDetails_p echo_p;
        mbCalloc( echo_p, sizeof( PmcSqlEchoDetails_t ) );
        ControlsGet( echo_p );
        pmcEchoReportGenerate( EchoId, echo_p, "Not Saved", TRUE, FALSE );
        pmcSqlEchoDetailsFree( echo_p );
    }
}
//---------------------------------------------------------------------------

Int32s_t  pmcEchoReportGenerate
(
    Int32u_t                echoId,
    PmcSqlEchoDetails_p     echoIn_p,
    Char_p                  watermark_p,
    Boolean_t               viewFlag,
    Boolean_t               importFlag
)
{
    Char_t                  buf1[1024];
    Boolean_t               found;
    PmcSqlEchoDetails_p     echo_p = NIL;
    MbDateTime              dateTime;
    Int32s_t                returnCode = FALSE;
    MbPDF                   doc;
    MbCursor                cursor = MbCursor( crHourGlass );

    // Get the echo details
    if( echoIn_p )
    {
        echo_p = echoIn_p;
    }
    else
    {
        if( ( echo_p = pmcSqlEchoDetailsGet( echoId, TRUE ) ) == NIL ) goto exit;
    }

    // Set up the document template file
    if( doc.templateSet( "echo_report_", echo_p->provider.id == 0 ? 1 : echo_p->provider.id )!= MB_RET_OK )
    {
        goto exit;
    }

    found = FALSE;
    if( echo_p->indication_p )
    {
        if( strlen( echo_p->indication_p ) > 0 )
        {
            doc.subString( "_INDICATION_",  echo_p->indication_p, 80 );
            found = TRUE;
        }
    }

    if( found == FALSE )
    {
        doc.subString( "_INDICATION_",  "", 80 );
    }

    // Indicate if there is an "indication section"
    doc.subBoolean( "_IF_INDICATION_",      found );
    doc.subBoolean( "_ENDIF_INDICATION_",   found );

    doc.subString( "_PAT_NAME_LAST_"      , echo_p->patient.lastName, 40 );
    doc.subString( "_PAT_NAME_FIRST_"     , echo_p->patient.firstName, 40 );
    doc.subString( "_WATERMARK_"          , watermark_p ? watermark_p : "", 30 );
    doc.subString( "_PAT_PHN_"            , pmcFormatPhnDisplay( echo_p->patient.phn, echo_p->patient.phnProv, buf1 ), 30 );
    doc.subString( "_DR_NAME_LAST_"       , echo_p->referring.lastName, 40 );
    doc.subString( "_DR_NAME_FIRST_"      , echo_p->referring.firstName, 40 );
    doc.subString( "_DR_PHONE_"           , pmcPhoneFormatString( echo_p->referring.phone, buf1 ), 30 );
    doc.subString( "_DR_FAX_"             , pmcPhoneFormatString( echo_p->referring.fax,   buf1 ), 30 );

    if( echo_p->patient.birthDate )
    {
        dateTime.SetDate( echo_p->patient.birthDate );
        doc.subString( "_PAT_DOB_"            , dateTime.MDY_DateString( ), 0 );
    }
    else
    {
        doc.subString( "_PAT_DOB_"            , "", 0 );
    }

    if(  echo_p->studyDate )
    {
        dateTime.SetDate( echo_p->studyDate );
        doc.subString( "_STUDY_DATE_"         , dateTime.MDY_DateString( ), 0 );
    }
    else
    {
        doc.subString( "_STUDY_DATE_"         , "", 0 );
    }

    if( echo_p->readDate > PMC_ECHO_STATUS_MAX )
    {
        dateTime.SetDate( echo_p->readDate );
        doc.subString( "_READ_DATE_"         , dateTime.MDY_DateString( ), 0 );
    }
    else if( echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
    {
        doc.subString( "_READ_DATE_"         , "No Echo", 0 );
    }
    else
    {
        doc.subString( "_READ_DATE_"         , "Pending", 0 );
    }

    doc.subString( "_CD_RV_"              , pmcFloatToStr( echo_p->cd_rv    , 1 ), 0 );
    doc.subString( "_CD_AA_"              , pmcFloatToStr( echo_p->cd_aa    , 1 ), 0 );
    doc.subString( "_CD_LA_"              , pmcFloatToStr( echo_p->cd_la    , 1 ), 0 );
    doc.subString( "_CD_LVED_"            , pmcFloatToStr( echo_p->cd_lved  , 1 ), 0 );
    doc.subString( "_CD_LVES_"            , pmcFloatToStr( echo_p->cd_lves  , 1 ), 0 );
    doc.subString( "_CD_SEPT_"            , pmcFloatToStr( echo_p->cd_sept  , 1 ), 0 );
    doc.subString( "_CD_PW_"              , pmcFloatToStr( echo_p->cd_pw    , 1 ), 0 );
    doc.subString( "_CD_MI_"              , pmcFloatToStr( echo_p->cd_mi    , 1 ), 0 );
    doc.subString( "_CD_LVEF_"            , pmcFloatToStr( echo_p->cd_lvef  , 1 ), 0 );

    doc.subString( "_AV_REG_"             , echo_p->av_reg_p, 0 );
    doc.subString( "_AV_STEN_"            , echo_p->av_sten_p, 0 );

    doc.subString( "_AV_AJL_"             , pmcFloatToStr( echo_p->av_ajl     , 1 ), 0 );
    doc.subString( "_AV_APHT_"            , pmcFloatToStr( echo_p->av_apht    , 1 ), 0 );
    doc.subString( "_AV_MV_"              , pmcFloatToStr( echo_p->av_mv      , 1 ), 0 );
    doc.subString( "_AV_PG_"              , pmcFloatToStr( echo_p->av_pg      , 1 ), 0 );
    doc.subString( "_AV_MG_"              , pmcFloatToStr( echo_p->av_mg      , 1 ), 0 );
    doc.subString( "_AV_LD_"              , pmcFloatToStr( echo_p->av_ld      , 1 ), 0 );
    doc.subString( "_AV_LV_"              , pmcFloatToStr( echo_p->av_lv      , 1 ), 0 );
    doc.subString( "_AV_VTI_"             , pmcFloatToStr( echo_p->av_vti     , 1 ), 0 );
    doc.subString( "_AV_VA_"              , pmcFloatToStr( echo_p->av_va      , 1 ), 0 );

    doc.subString( "_TV_REG_"             , echo_p->tv_reg_p, 0 );
    doc.subString( "_TV_STEN_"            , echo_p->tv_sten_p, 0 );

    doc.subString( "_TV_TRJA_"            , pmcFloatToStr( echo_p->tv_trja    , 1 ), 0 );
    doc.subString( "_TV_RVSP_"            , pmcFloatToStr( echo_p->tv_rvsp    , 1 ), 0 );
    doc.subString( "_TV_EV_"              , pmcFloatToStr( echo_p->tv_ev      , 1 ), 0 );
    doc.subString( "_TV_VA_"              , pmcFloatToStr( echo_p->tv_va      , 1 ), 0 );

    doc.subString( "_MV_REG_"             , echo_p->mv_reg_p, 0 );
    doc.subString( "_MV_STEN_"            , echo_p->mv_sten_p, 0 );

    doc.subString( "_MV_MRJA_"            , pmcFloatToStr( echo_p->mv_mrja    , 1 ), 0 );
    doc.subString( "_MV_VA_"              , pmcFloatToStr( echo_p->mv_va      , 1 ), 0 );
    doc.subString( "_MV_PEV_"             , pmcFloatToStr( echo_p->mv_pev     , 1 ), 0 );
    doc.subString( "_MV_PAV_"             , pmcFloatToStr( echo_p->mv_pav     , 1 ), 0 );
    doc.subString( "_MV_MG_"              , pmcFloatToStr( echo_p->mv_mg      , 1 ), 0 );
    doc.subString( "_MV_IVRT_"            , pmcFloatToStr( echo_p->mv_ivrt    , 3 ), 0 );
    doc.subString( "_MV_EDT_"             , pmcFloatToStr( echo_p->mv_edt     , 1 ), 0 );

    doc.subString( "_PV_REG_"             , echo_p->pv_reg_p, 0 );

    doc.subString( "_PV_VEL_"             , pmcFloatToStr( echo_p->pv_vel      , 1 ), 0 );
    doc.subString( "_PV_PAT_"             , pmcFloatToStr( echo_p->pv_pat      , 3 ), 0 );
    doc.subString( "_PV_GRAD_"            , pmcFloatToStr( echo_p->pv_grad     , 1 ), 0 );

    doc.subString( "_PVF_SYS_"            , pmcFloatToStr( echo_p->pv_vel      , 1 ), 0 );
    doc.subString( "_PVF_DIA_"            , pmcFloatToStr( echo_p->pv_pat      , 1 ), 0 );
    doc.subString( "_PVF_AR_"             , pmcFloatToStr( echo_p->pv_grad     , 1 ), 0 );
    doc.subString( "_PVF_LAA_"            , pmcFloatToStr( echo_p->pv_grad     , 1 ), 0 );

    doc.subString( "_RATE_"               , pmcFloatToStr( echo_p->rate        , 1 ), 0 );
    doc.subString( "_RHYTHM_"             , echo_p->rhythm_p, 0 );
    doc.subString( "_IMAGE_QUALITY_"      , echo_p->imageQuality_p, 0 );
    doc.subString( "_NOTES_"              ,  echo_p->text_p, 0 );

     // Generate and view the PDF file
    if( doc.templateToTempSub( ) == MB_RET_OK )
    {
        if( doc.generate( 15 ) == MB_RET_OK )
        {
            found = TRUE;
            doc.view( );
        }
    }

    if( found && importFlag )
    {
        mbFileListStruct_p      file_p;
        if( pmcCfg[CFG_DATABASE_LOCAL].value == FALSE )
        {
            returnCode = FALSE;
            mbDlgInfo( "Cannot import documents when connected to a remote database.\n" );
            goto exit;
        }

        mbCalloc( file_p, sizeof(mbFileListStruct_t) );
        mbMalloc( file_p->name_p,     128 );
        mbMalloc( file_p->fullName_p, 256 );

        strcpy( file_p->fullName_p, doc.fileNameFinalGet( ) );
        pmcFilePathAndNameGet( file_p->fullName_p, NIL, file_p->name_p );
        file_p->type = PMC_DOCUMENT_TYPE_PDF;

        sprintf( buf1, "Echo report: %s", echo_p->studyName_p );

        returnCode = pmcDocProcess
        (
            file_p,                         // File details
            NIL,                            // document id - (0 = NEW)
            PMC_IMPORT_DIALOG_ALWAYS,       // Show dialog or not
            PMC_IMPORT_PROVIDER_SELECTED,
            PMC_IMPORT_PHN_NONE,
            PMC_IMPORT_DATE_SELECTED,
            TRUE,                           // TRUE = moved failed to fail dir
            echo_p->provider.id, echo_p->patient.id,
            echo_p->readDate, 0,            // date, failMask
            NIL,                            // result flag - terminate auto import
            NIL,                            // result flag - form displayed
            NIL,                            // Failed directory
            buf1,                           // description
            FALSE
        );

        if( returnCode == TRUE )
        {
            mbDlgInfo( "Document imported into database." );
        }
        else
        {
            mbDlgInfo( "Document not imported into database." );
            returnCode = FALSE;
        }
        mbFileListFreeElement( file_p );
    }

exit:

    if( echoIn_p == NIL ) pmcSqlEchoDetailsFree( echo_p );
    return returnCode;
}

//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::PageControl1Change(TObject *Sender)
{
    if( PageControl1->ActivePage == TabSheet_Notes )
    {
        Memo1->SetFocus();
    }
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_SubmitClick(TObject *Sender)
{
    Int32s_t    result;

    if( ChangeCount != 0 )
    {
        result = mbDlgYesNo( "Save changes and submit this echo for approval?\n" );
        if( result == MB_BUTTON_NO ) goto exit;
        Save( TRUE, FALSE );
        pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_READ_DATE, PMC_ECHO_STATUS_PENDING, EchoId );
        Echo_p->readDate = PMC_ECHO_STATUS_PENDING;
        Button_Submit->Enabled = FALSE;
        ControlsUpdate( );
        OkFlag = TRUE;
        Close( );
    }
    else
    {
        result = mbDlgYesNo( "Submit this echo for approval?\n" );
        if( result == MB_BUTTON_NO ) goto exit;

        pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_READ_DATE, PMC_ECHO_STATUS_PENDING, EchoId );
        Echo_p->readDate = PMC_ECHO_STATUS_PENDING;
        Button_Submit->Enabled = FALSE;
        ControlsUpdate( );
        OkFlag = TRUE;
        Close( );
    }

exit:
    return;
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::Button_ApproveClick(TObject *Sender)
{
    Int32u_t    date;
    Boolean_t   importFlag = FALSE;

    if( Echo_p->readDate == PMC_ECHO_STATUS_MAX )
    {
        mbDlgInfo( "Cannot apprve report with no Echo." );
        return;

    }

    if( Echo_p->readDate > PMC_ECHO_STATUS_MAX &&  ChangeCount == 0 )
    {
        mbDlgInfo( "This echo is already approved." );
        return;
    }

    if( ChangeCount != 0 )
    {
        if( Echo_p->readDate > PMC_ECHO_STATUS_MAX )
        {
            if( mbDlgYesNo( "Echo already approved.\n\nSave changes and re-approve this echo?" ) == MB_BUTTON_NO ) goto exit;
        }
        else
        {
            if( mbDlgYesNo( "Save changes and approve this echo?" ) == MB_BUTTON_NO ) goto exit;
        }

        if( ( date = pmcDateSelect( PMC_DATE_SELECT_PARMS, "Select Echo Read Date", mbToday( ) ) ) == 0 ) goto exit;

        OkFlag = TRUE;
        Save( TRUE, FALSE );
        Echo_p->readDate = date;
        pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_READ_DATE, date, EchoId );
        ControlsUpdate( );
        importFlag = TRUE;
    }
    else
    {
        //if( mbDlgYesNo( "Approve this echo?" ) == MB_BUTTON_NO ) goto exit;
        if( ( date = pmcDateSelect( PMC_DATE_SELECT_PARMS, "Select Echo Read Date", mbToday( ) ) ) == 0 ) goto exit;

        pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_READ_DATE, date, EchoId );
        ControlsUpdate( );
        Echo_p->readDate = date;
        OkFlag = TRUE;
        importFlag = TRUE;
    }

exit:

    if( importFlag )
    {
        if( pmcEchoReportGenerate( EchoId, NIL, "", FALSE, importFlag ) == FALSE )
        {
            mbDlgInfo( "Document import failed, echo status reverting to 'In Progress'." );
            pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_READ_DATE, PMC_ECHO_STATUS_IN_PROGRESS, EchoId );

            Echo_p->readDate = PMC_ECHO_STATUS_IN_PROGRESS;
            ControlsUpdate( );
        }
        else
        {
            Close( );
        }
    }
    return;
}
//---------------------------------------------------------------------------

void __fastcall TEchoDetailsForm::AssignPatient( PmcSqlEchoDetails_p echo_p )
{
    Boolean_t       namedFlag = FALSE;
    Char_t          firstName[128];
    Char_t          lastName[128];
    Int32s_t        i;
    Int32u_t        id;

    if( echo_p->patient.id != 0 ) return;

    namedFlag = FALSE;

    memset( firstName, 0 , 128 );
    memset( lastName, 0, 128 );

    for( i = 0 ; i < 128 ; i++ )
    {
        if( *( echo_p->studyName_p + i ) == ' ' ) break;

        firstName[i] = *( echo_p->studyName_p + i );
    }

    strncpy( lastName, ( echo_p->studyName_p + i + 1), 128 );
    mbStrClean( firstName, NIL, TRUE );
    mbStrClean( lastName, NIL, TRUE );

    mbLog( "Got name '%s' first '%s' last '%s'\n", echo_p->studyName_p, firstName, lastName );

    if( strlen( firstName ) && strlen( lastName ) )
    {
        sprintf( Cmd_p, "select id from patients where first_name = \"%s\" and last_name = \"%s\"", firstName, lastName );

        if( Sql.Query( Cmd_p ) )
        {
            if( Sql.RowCount() == 1 )
            {
                Sql.RowGet();
                id = Sql.Int32u( 0 );
                if( id !=0 )
                {
                    sprintf( Cmd_p, "update echos set patient_id = %u where id = %lu",
                            id, echo_p->echoId);
                    Sql.Update( Cmd_p );
                    echo_p->patient.id = id;
                    PatientUpdate( );
                    ChangeCount++;
                    namedFlag = TRUE;
                }
            }
        }

        if( namedFlag == FALSE )
        {
            if( pmcEchoPatSelect( echo_p->studyName_p, echo_p->studyDate, 4, &id, FALSE  ) == MB_RETURN_OK )
            {
                if( id != 0 )
                {
                    sprintf( Cmd_p, "update echos set patient_id = %u where id = %lu", id, echo_p->echoId );
                    Sql.Update( Cmd_p );
                    echo_p->patient.id = id;
                    ChangeCount++;
                    PatientUpdate( );
                }
            }
        }
    }
}
