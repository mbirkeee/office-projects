//---------------------------------------------------------------------------
// File:    pmcReportForm.c
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    May 9, 2001
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
#include "pmcUtils.h"
#include "pmcReportForm.h"

#pragma resource "*.dfm"

//---------------------------------------------------------------------------

__fastcall TReportForm::TReportForm(TComponent* Owner)
    : TForm(Owner)
{
    mbDlgDebug(( "Default constructor called" ));
}

__fastcall TReportForm::TReportForm
(
    TComponent             *Owner,
    pmcReportFormInfo_p     reportFormInfo_p
)
    : TForm(Owner)
{
    Char_p          buf_p;
    FILE           *fp;
    Ints_t          len, i;

    mbMalloc( buf_p, 512 );
    Memo->Lines->Clear();

    if( strlen( reportFormInfo_p->caption_p ) > 0 )
    {
        Caption = reportFormInfo_p->caption_p;
    }
    else
    {
        Caption = reportFormInfo_p->caption_p;
    }

//    sprintf( buf_p,  "1--------1---------2---------3---------4---------5---------6---------7---------8---------9--------|\n" );
//    Memo->Lines->Add( buf_p );
//    Memo->Lines->Add( buf_p );
//    Memo->Lines->Add( buf_p );
//    sprintf( buf_p, "File name '%s'\n", reportFormInfo_p->fileName_p );
//    Memo->Lines->Add( buf_p );

    fp = fopen( reportFormInfo_p->fileName_p, "r" );

    if( fp )
    {
        while( fgets( buf_p, 512, fp ) != 0 )
        {
            // Null terminate buffer (in case some kind of binary data was read)
            *(buf_p + 511) = 0;
            len = strlen( buf_p );

            for( i = 0 ; i < len ; i++ )
            {
                if( *( buf_p + i ) == '\n' )
                {
                    *( buf_p + i ) = 0;
                    break;
                }
            }
            Memo->Lines->Add( buf_p );
        }
    }
    else
    {
        sprintf( buf_p, "Could not open file '%s'\n", reportFormInfo_p->fileName_p );
        Memo->Lines->Add( buf_p );
    }

    if( fp ) fclose( fp );

    mbFree( buf_p );
}

//---------------------------------------------------------------------------

void __fastcall TReportForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    Memo->Lines->Clear();
    Action = caFree;
}
//---------------------------------------------------------------------------
void __fastcall TReportForm::OkButtonClick(TObject *Sender)
{
    Close( );
}
//---------------------------------------------------------------------------
void __fastcall TReportForm::Button1Click(TObject *Sender)
{
    Close( );

}
//---------------------------------------------------------------------------
