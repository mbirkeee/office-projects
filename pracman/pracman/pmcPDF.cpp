//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// Utilities for PDF file generation
//---------------------------------------------------------------------------

// System Include Files
#include <stdio.h>
#include <dos.h>
#include <dir.h>
#include <process.h>
#pragma hdrstop

// Library Include Files
#include "mbUtils.h"

// Project Include Files
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcPDF.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// MbPDF Default Constuctor
//---------------------------------------------------------------------------
__fastcall MbPDF::MbPDF( void )
{
    this->init( );
}

//---------------------------------------------------------------------------
// MbPDF Constuctor
//---------------------------------------------------------------------------
__fastcall MbPDF::MbPDF( Char_p name_p, Int32u_t id )
{
    this->init( );
    this->templateSet( name_p, id );
}

//---------------------------------------------------------------------------
// Constructor helper
//---------------------------------------------------------------------------
Int32s_t    __fastcall MbPDF::init( void )
{
    mbObjectCountInc();
    this->error = MB_RET_ERROR;

    mbCalloc( this->templateName_p, MAX_PATH );
    mbCalloc( this->targetBase_p, MAX_PATH );
    mbCalloc( this->spoolName_p, MAX_PATH );
    mbCalloc( this->flagName_p, MAX_PATH );
    mbCalloc( this->pdfName_p, MAX_PATH );
    mbCalloc( this->finalName_p, MAX_PATH );
    mbCalloc( this->tempName_p, MAX_PATH );
    mbMalloc( this->bigBuf_p, 1000000 );

    // Make queue for substitution strings
    this->sub_q = qInitialize( &this->subQueue );

    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// Constructor helper
//---------------------------------------------------------------------------
Int32s_t __fastcall MbPDF::templateSet( Char_p template_p, Int32u_t id )
{
    if( template_p == NIL )
    {
        mbDlgError( "Unable to access PDF template file 'NIL'" );
        goto exit;
    }

    sprintf( templateName_p, "%s\\%s%d.tex", pmcCfg[CFG_TEX_TEMPLATE_DIR].str_p, template_p, id );

    if( FileExists( templateName_p ) == FALSE )
    {
        mbDlgError( "Unable to access PDF template file '%s'", templateName_p );
        goto exit;
    }

    // Get base filename
    sprintf( targetBase_p, "%s_%s", template_p, pmcMakeFileName( NIL, this->spoolName_p ) );

    // Get file names with path
    sprintf( spoolName_p, "%s\\spool\\%s.tex", pmcCfg[CFG_PMCDESPL_DIR].str_p, targetBase_p );
    sprintf( flagName_p,  "%s\\flags\\%s.tex", pmcCfg[CFG_PMCDESPL_DIR].str_p, targetBase_p );
    sprintf( pdfName_p,   "%s\\pdf\\%s.pdf",   pmcCfg[CFG_PMCDESPL_DIR].str_p, targetBase_p );
    sprintf( finalName_p, "%s\\%s.pdf",        pmcCfg[CFG_TEMP_DIR].str_p,     targetBase_p );
    sprintf( tempName_p,  "%s\\%s.tmp",        pmcCfg[CFG_TEMP_DIR].str_p,     targetBase_p );

    this->error = MB_RET_OK;

exit:
    return this->error;
}

//---------------------------------------------------------------------------
// Add boolean value to the substitute queue
//---------------------------------------------------------------------------
Int32s_t __fastcall MbPDF::subBoolean( Char_p key_p, Boolean_t value )
{
    return mbStrListAddNew( this->sub_q, key_p, "_BOOLEAN_", (Int32u_t)value, 0 );
}

//---------------------------------------------------------------------------
// Add callback function to the substitute queue
//---------------------------------------------------------------------------
Int32s_t __fastcall MbPDF::subCallback( Char_p key_p, Int32s_t (*func_p)( Int32u_t, FILE *, Char_p ), Int32u_t handle )
{
    return mbStrListAddNew( this->sub_q, key_p, "_CALLBACK_", (Int32u_t)func_p, handle );
}

//---------------------------------------------------------------------------
// Add string to the substitute queue
//---------------------------------------------------------------------------
Int32s_t __fastcall MbPDF::subString( Char_p key_p, Char_p value_p, Int32u_t maxLength )
{
    return mbStrListAddPair( this->sub_q, key_p, mbStrTex( value_p, this->bigBuf_p, maxLength ) );
}

//---------------------------------------------------------------------------
// Add date string to the substitute queue
//---------------------------------------------------------------------------
Int32s_t __fastcall MbPDF::subDate( Char_p key_p, Int32u_t date )
{
    MbDateTime  dateTime = MbDateTime( date, 0 );
    return this->subString( key_p, dateTime.MDY_DateString( ), 0 );
}

//---------------------------------------------------------------------------
// Copy the template file to its temp location, and make any required
// substitutions
//---------------------------------------------------------------------------

Int32s_t __fastcall MbPDF::templateToTempSub( void )
{
    Int32s_t    result;
    MbString    target;

    target.set( this->tempName_p );
    target.append( "-stage1" );

    result =  pmcTemplateSub( this->templateName_p, target.get(), this->sub_q );

    if( result == MB_RET_OK )
    {
        this->postProcessTex( target.get(), this->tempName_p );
    }
    return result;
}

//---------------------------------------------------------------------------
// Post process the file.  Make any changes after the substitutions are
// done.  Changes include
// - look for
//---------------------------------------------------------------------------

Int32s_t __fastcall MbPDF::postProcessTex( Char_p nameIn_p, Char_p nameOut_p )
{
    Int32s_t    result = MB_RET_ERR;
    MbString    target;
    MbString    buf( 2048 );
    FILE       *out_p;
    FILE       *in_p;
    qHead_t     inQueue;
    qHead_p     in_q;
    mbStrList_p line_p;
    Char_p      prevLine_p;

    in_q = qInitialize( &inQueue );

    // Open the input file
    if( ( in_p = fopen( nameIn_p, "r" ) ) == NIL )
    {
        mbDlgError( "Failed to open output file '%s'\n", nameIn_p );
        goto exit;
    }

    // Read the input file, put contents into a queue
    while( fgets( buf.get(), buf.size(), in_p ) != 0 )
    {
        buf.strip();
        mbStrListAdd( in_q, buf.get( ) );
    }

    fclose( in_p );
    in_p = NIL;

    // Open output file
    if( ( out_p = fopen( nameOut_p, "w" ) ) == NIL )
    {
        mbDlgError( "Failed to open output file '%s'\n", nameOut_p );
        goto exit;
    }

    prevLine_p = NIL;
    qWalk( line_p, in_q, mbStrList_p )
    {
        // mbLog( "Got line: %s\n", line_p->str_p );
        if( mbStrPos( line_p->str_p, "\\\\*" ) == 0 )
        {
            if( prevLine_p ) fprintf( out_p, "%s\n", prevLine_p );
            prevLine_p = line_p->str_p;
        }
        else
        {
            if( prevLine_p )
            {
                if( mbStrPos( line_p->str_p, "\\begin{myitemize}" ) > 0 )
                {
                    prevLine_p = NIL;
                }
                else if( mbStrPos( line_p->str_p, "\\begin{myenumerate}" ) > 0 )
                {
                    prevLine_p = NIL;
                }
            }
            if( prevLine_p ) fprintf( out_p, "%s\n", prevLine_p );
            fprintf( out_p, "%s\n", line_p->str_p );
            prevLine_p = NIL;
        }
    }

    fclose( out_p );
    out_p = NIL;

    result = MB_RET_OK;
exit:

    mbStrListFree( in_q );

    if( out_p ) fclose( out_p );
    if( in_p )  fclose( in_p );

    return result;
}

//---------------------------------------------------------------------------
// Copy the template file to its temp location
//---------------------------------------------------------------------------

Int32s_t __fastcall MbPDF::templateToTemp( void )
{
    Int32s_t    result = MB_RET_ERROR;

    if( this->error == TRUE ) goto exit;

    if( ( result = mbFileCopy( this->templateName_p, this->tempName_p ) ) != MB_RET_OK )
    {
        mbDlgError( "Error copying template file to temp location" );
        goto exit;
    }

    result = MB_RET_OK;
exit:
    return result;
}

//---------------------------------------------------------------------------
// Produce the PDF file by writing flag file to trigger pmcdespl
//---------------------------------------------------------------------------
Int32s_t __fastcall MbPDF::generate( Int32u_t waitSec )
{
    Int32s_t    result = MB_RET_ERR;
    FILE        *fp;
    Int32u_t    wait;
    Boolean_t   found = FALSE;
    Int32u_t    i;

    if( ( result = mbFileCopy( this->tempName_p, this->spoolName_p ) ) != MB_RET_OK )
    {
        goto exit;
    }

    unlink( this->tempName_p );

    // Write flag file to trigger despooler
    fp = fopen( this->flagName_p, "w" );
    if( fp == NIL )
    {
        mbDlgError( "Could not open file '%s'", flagName_p );
        goto exit;
    }
    else
    {
        fprintf( fp, "%s", flagName_p );
        fclose( fp );

        // Now must wait for the file generation to complete
        wait = waitSec * 10;

        for( i = 0 ; i < wait ; i++ )
        {
            if( FileExists( this->pdfName_p ) )
            {
                found = TRUE;
                break;
            }
            Sleep( 100 );
        }

        if( found )
        {
            mbFileCopy( this->pdfName_p, this->finalName_p );
            unlink( this->pdfName_p );
            result = MB_RET_OK;
        }
    }

exit:

    if( result != MB_RET_OK )
    {
        mbDlgError( "PDF document generation failed.  Please contact system administrator." );
    }

    return result;
}

//---------------------------------------------------------------------------
// View
//---------------------------------------------------------------------------
Int32s_t __fastcall MbPDF::view(  )
{
    MbString    buf = MbString();
    Int32s_t    result = MB_RET_ERR;

    mbSprintf( &buf, "\"%s\"", this->finalName_p );

    if( pmcCfg[CFG_VIEWER_PDF].str_p )
    {
        spawnle( P_NOWAITO,
            pmcCfg[CFG_VIEWER_PDF].str_p,
            pmcCfg[CFG_VIEWER_PDF].str_p,
            buf.get(),
            NULL, NULL );
        result = MB_RET_OK;
    }
    else
    {
        mbDlgInfo( "No viewer configured for PDF files\n" );
    }
    return result;
}

//---------------------------------------------------------------------------
// Class: MbPDF Destructor
//---------------------------------------------------------------------------
__fastcall MbPDF::~MbPDF( )
{
    // Free the substitution queue
    mbObjectCountDec();

    // mbStrListLog( sub_q );
    mbStrListFree( sub_q );

    mbFree( this->bigBuf_p );
    mbFree( this->templateName_p );
    mbFree( this->targetBase_p );
    mbFree( this->spoolName_p );
    mbFree( this->flagName_p );
    mbFree( this->pdfName_p );
    mbFree( this->finalName_p );
    mbFree( this->tempName_p );
    return;
}

