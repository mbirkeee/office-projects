//---------------------------------------------------------------------------
// File:    pmcUtils.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb. 21, 2001
//---------------------------------------------------------------------------
// Description:
//
// Utility functions
//---------------------------------------------------------------------------

// Platform includes
#include <stdio.h>
#include <vcl.h>
#include <time.h>
#include <dos.h>
#include <dir.h>
#include <process.h>
#include <Filectrl.hpp>
#pragma hdrstop

// Library includes
#include "mbUtils.h"
#include "splashNames.h"

// Program includes
#include "pmcUtils.h"
#include "pmcInitialize.h"
#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcAppListForm.h"
#include "pmcAppLettersForm.h"
#include "pmcAppHistoryForm.h"
#include "pmcEchoPatSelectForm.h"
#include "pmcDateSelectForm.h"

#pragma package(smart_init)

#if 0
Int32s_t            mbFileDirCheckCreate
(
    Char_p          name_p,
    qHead_p         dirCache_q,
    Boolean_t       fileFlag,
    Boolean_t       createFlag,
    Boolean_t       promptFlag
)
#endif


#define PMC_TEMPLATE_KEY_TYPE_NONE      0
#define PMC_TEMPLATE_KEY_TYPE_IF        1
#define PMC_TEMPLATE_KEY_TYPE_ELSE      2
#define PMC_TEMPLATE_KEY_TYPE_ENDIF     3

Int32s_t    pmcTemplateConditional( Char_p str_p, Char_p key_p );
Boolean_t   pmcTemplateKeyCheck( Char_p key_p, qHead_p q_p );

#define PMC_TEX_HEADING "\\myheading"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

__fastcall MbPhone::MbPhone( Char_p str_p )
{
    this->set( str_p );
}

__fastcall MbPhone::~MbPhone( void )
{

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void __fastcall MbPhone::set( Char_p str_p )
{
    memset( this->raw, 0, 36 );
    strncpy( this->raw, str_p, 35 );

    Char_t      d[10];
    Ints_t      i, digitCount = 0;

    for( i = 0 ; i < (Ints_t)strlen( raw ); i++ )
    {
        if( raw[i] < '0' || raw [i] > '9' )  continue;

        d[digitCount] = raw[i];
        digitCount++;
        if( digitCount == 10 ) break;
    }

    switch( digitCount )
    {
        case 0:
            sprintf( this->formatted, "" );
            sprintf( this->digits, "" );
            break;
        case 1:
            sprintf( this->formatted, "%c", d[0] );
            sprintf( this->digits,    "%c", d[0] );
            break;
        case 2:
            sprintf( this->formatted, "%c%c", d[0], d[1] );
            sprintf( this->digits,    "%c%c", d[0], d[1] );
            break;
        case 3:
            sprintf( this->formatted, "%c%c%c-", d[0], d[1], d[2] );
            sprintf( this->digits,    "%c%c%c-", d[0], d[1], d[2] );
            break;
        case 4:
            sprintf( this->formatted, "%c%c%c-%c", d[0], d[1], d[2], d[3] );
            sprintf( this->digits,    "%c%c%c%c",  d[0], d[1], d[2], d[3] );
            break;
        case 5:
            sprintf( this->formatted, "%c%c%c-%c%c", d[0], d[1], d[2], d[3], d[4] );
            sprintf( this->digits,    "%c%c%c%c%c",  d[0], d[1], d[2], d[3], d[4] );
            break;
        case 6:
            sprintf( this->formatted, "%c%c%c-%c%c%c", d[0], d[1], d[2], d[3], d[4], d[5] );
            sprintf( this->digits,    "%c%c%c%c%c%c",  d[0], d[1], d[2], d[3], d[4], d[5] );
           break;
        case 7:
            sprintf( this->formatted, "%c%c%c-%c%c%c%c", d[0], d[1], d[2], d[3], d[4], d[5], d[6] );
            sprintf( this->digits,    "%c%c%c%c%c%c%c",  d[0], d[1], d[2], d[3], d[4], d[5], d[6] );
            break;
        case 8:
            sprintf( this->formatted, "(%c) %c%c%c-%c%c%c%c", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7] );
            sprintf( this->digits,    "%c%c%c%c%c%c%c%c",     d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7] );
            break;
        case 9:
            sprintf( this->formatted, "(%c%c) %c%c%c-%c%c%c%c", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8] );
            sprintf( this->digits,    "%c%c%c%c%c%c%c%c%c",     d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8] );
            break;
        case 10:
            sprintf( this->formatted, "(%c%c%c) %c%c%c-%c%c%c%c", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9] );
            sprintf( this->digits,    "%c%c%c%c%c%c%c%c%c%c",     d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9] );
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall MbQueue::MbQueue( void )
{
    mbObjectCountInc();
    mbLog( "Queue object called\n" );
    this->queue_p = qInitialize( &this->queueHead );
}

__fastcall MbQueue::~MbQueue( void )
{
    mbObjectCountDec();
    mbLog( "Queue object destructor called" );
}

//---------------------------------------------------------------------------
// Initialize a simple registry.  Can differentiate by hostname/username.
// Not meant for gigantic amounts of data.
//---------------------------------------------------------------------------

Int32s_t    pmcRegInit( Char_p hostname_p, Char_p username_p )
{
    Int32s_t    result = MB_RET_ERR;

    if( gPmcRegUsername_p != NIL || gPmcRegHostname_p != NIL )
    {
        mbDlgError( "Registry already initialized" );
        goto exit;
    }

    if( hostname_p == NIL || username_p == NIL )
    {
        mbDlgError( "Failed to initialize registry" );
        goto exit;
    }

    // Store the username/hostname in global variables
    mbMallocStr( gPmcRegUsername_p, username_p );
    mbMallocStr( gPmcRegHostname_p, hostname_p );

    result = MB_RET_OK;

exit:
    return result;
}

//---------------------------------------------------------------------------
// Close a simple registry
//---------------------------------------------------------------------------

Int32s_t pmcRegClose( void )
{
    mbFree( gPmcRegUsername_p );
    mbFree( gPmcRegHostname_p );

    gPmcRegUsername_p = NIL;
    gPmcRegHostname_p = NIL;

    return MB_RET_OK;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

void pmcPatIdStore( Int32u_t id )
{
    pmcStoredPatientId = id;
    return;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32u_t pmcPatIdRetrieve( Boolean_t clearFlag )
{
    Int32u_t returnId = pmcStoredPatientId;
    if( clearFlag == TRUE )
    {
        pmcStoredPatientId = 0;
    }
    return returnId;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//
//---------------------------------------------------------------------------

Char_p  pmcDocumentDirFromName( Char_p documentName_p )
{
    static Char_t   dir[1024];
    Char_t          total[1024];

    if( documentName_p == NIL ) return "";

    strcpy( dir, documentName_p );

    if( dir[8] != '-' ) return "";

    dir[6] = 0;

    sprintf( total, "%s\\%s", pmcCfg[CFG_DOC_IMPORT_TO_DIR_NEW].str_p, &dir[0] );

    // Create the dir if it does not exist 
    mbFileDirCheckCreate( total, NIL, FALSE, TRUE, FALSE );

    return &dir[0];
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

Int32u_t pmcDateSelect( Int32u_t mode, Char_p string_p, Int32u_t startDate )
{
    TDateSelectForm    *dateSelectForm_p;
    pmcDateSelectInfo_t dateSelectInfo;
    Int32u_t            date = 0;
    Char_t              resultString[64];

    dateSelectInfo.mode = mode;
    dateSelectInfo.string_p = resultString;
    dateSelectInfo.dateIn = startDate;
    dateSelectInfo.caption_p = string_p;

    dateSelectForm_p = new TDateSelectForm( NULL, &dateSelectInfo );
    dateSelectForm_p->ShowModal( );
    delete dateSelectForm_p;

    if( dateSelectInfo.returnCode != MB_BUTTON_OK ) goto exit;

    if( dateSelectInfo.dateOut > mbToday() )
    {
        if( mbDlgYesNo( "The selected date is in the future.  Use this date anyway?" ) == MB_BUTTON_NO ) goto exit;
    }

    date = dateSelectInfo.dateOut;

exit:
    return date;
}

//---------------------------------------------------------------------------
// Try to guess if this is a simple, clean string
//---------------------------------------------------------------------------
Boolean_t mbStrTexLooksClean( Char_p in_p )
{
    Boolean_t       result = FALSE;
    Char_p          temp_p;

    if( in_p == NIL ) goto exit;

    if( strlen( in_p ) == 0 ) goto exit;

    for( temp_p = in_p; *temp_p != 0 ; temp_p++ )
    {
        if( *temp_p == MB_CHAR_BACKSLASH ) goto exit;
    }

    // If we made it here, then this string looks clean
    result = TRUE;

exit:
    return result;
}

//---------------------------------------------------------------------------
// Determine if this line is part of an enum
//---------------------------------------------------------------------------
Int32s_t mbStrTexEnum( Char_p in_p )
{
    Boolean_t       found = FALSE;
    Char_p          temp_p;
    Int32s_t        result = 0;
    Int32s_t        pos = 0;
    Int32s_t        numCount = 0;

    if( in_p == NIL ) goto exit;

    if( strlen( in_p ) == 0 ) goto exit;

    for( temp_p = in_p; *temp_p != 0 ; temp_p++ )
    {
        pos++;
        if( *temp_p == '.' || *temp_p == ':' || *temp_p == ')' )
        {
            if( numCount > 0 )
            {
                found = TRUE;
                break;
            }
        }
        else if( *temp_p >= '0' && *temp_p <= '9' )
        {
            numCount++;
            continue;
        }
        break;
    }

    if( found == TRUE ) result = pos;
    
exit:
    return result;
}


//---------------------------------------------------------------------------
// Want to convert the string to individual lines
//---------------------------------------------------------------------------
Int32s_t mbStrToLines( Char_p in_p, qHead_p q_p )
{
    Char_t          c;
    MbString        buf( 4096 );

    // Loop through the chars
    for( ; ; )
    {
        c = *in_p++;

        if( c == 0 ) break;

        if( c == MB_CHAR_LF )
        {
            buf.clean();
            mbStrListAdd( q_p, buf.get() );
            buf.clear();
        }
        else
        {
            buf.appendChar( c );
        }
    }

    if( buf.length() )
    {
        buf.clean();
        mbStrListAdd( q_p, buf.get() );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Clean a string such that is can be output in Latex.  The string is
// limited to the specified length.  If the specified length is 0, the
// string length may be unlimited (1 MB).  Also, in this case, this function
// might attempt to detect points (first char of a line is '-') and output
// the contents as bullets.
//---------------------------------------------------------------------------
Char_p  mbStrTex( Char_p inNotClean_p, Char_p out_p, Int32u_t maxLength )
{
    Char_p          in_p = NIL;
    Boolean_t       truncatedFlag = FALSE;
    mbStrList_p     str_p;
    mbStrList_p     prev_p;
    qHead_t         queueIn;
    qHead_p         in_q = NIL;
    qHead_t         queueOut;
    qHead_p         out_q = NIL;
    MbString        strIn;
    MbString        strOut;
    MbString        strTmp;
    Boolean_t       foundHeading;
    Boolean_t       gotBullet = FALSE;
    Boolean_t       inBullet = FALSE;
    Boolean_t       gotList = FALSE;
    Boolean_t       inList = FALSE;
    Int32s_t        pos;
    Char_p          temp_p;
    Int32u_t        chunks;

    chunks = mbMallocChunks();

    in_q = qInitialize( &queueIn );
    out_q = qInitialize( &queueOut );

    // Must clear the output before any chance of an error
    if( out_p ) *out_p = 0;

    if( inNotClean_p == NIL || out_p == NIL ) goto exit;

    mbMalloc( in_p, strlen( inNotClean_p ) + 1 );

    if( in_p == NIL ) goto exit;

    mbStrClean( inNotClean_p, in_p, FALSE );

    // This section removes CRs... only LFs remain
    {
        Char_p  temp1_p;
        Char_p  temp2_p;

        for( temp1_p = in_p, temp2_p = in_p ; ; temp1_p++ )
        {
            if( *temp1_p != MB_CHAR_CR )
            {
                *temp2_p++ = *temp1_p;
            }
            if( *temp1_p == 0 ) break;
        }
    }

    if( maxLength > 0 )
    {
        if( strlen( in_p ) > maxLength )
        {
            // Truncate the output string
            *( in_p + maxLength ) = 0;
            truncatedFlag = TRUE;
        }
    }

    // Convert the text to a series of lines in a queue
    mbStrToLines( in_p, in_q );

    // LaTex-ize each line in the queue
    qWalk( str_p, in_q, mbStrList_p )
    {
        if( pmcCfg[CFG_LOG_TEX].value ) mbLog( "Got TEX line: '%s'\n", str_p->str_p );

        strIn.set( str_p->str_p );
        strIn.latex( );
        mbRemallocStr( str_p->str_p, strIn.get() );
    }

    if( in_q->size == 1 )
    {
        // There is just one line,
        str_p = (mbStrList_p)qRemoveFirst( in_q );
        strOut.append( str_p->str_p );
        mbStrListItemFree( str_p );
    }
    else
    {
        foundHeading = FALSE;

        // Copy the reults to strOut
        while( !qEmpty( in_q ) )
        {
            str_p = (mbStrList_p)qRemoveFirst( in_q );
            if( pmcCfg[CFG_LOG_TEX].value ) mbLog( "Considering line: '%s'\n", str_p->str_p );

            // This section skips blank lines after a heading
            if( strlen( str_p->str_p ) == 0 )
            {
                if( foundHeading )
                {
                    mbStrListItemFree( str_p );
                    continue;
                }
            }
            else
            {
                foundHeading = FALSE;
            }

            // Look for a heading
            if( mbStrPos( str_p->str_p, "---" ) == 0 )
            {
                if( pmcCfg[CFG_LOG_TEX].value ) mbLog( "This appears to be a heading indicator" );
                if( out_q->size > 0 )
                {
                    // Is there a line before this?
                    prev_p = qLast( out_q, mbStrList_p );
                    if( mbStrTexLooksClean( prev_p->str_p ) )
                    {
                        // Appears to be text before this, so reformat it
                        mbSprintf( &strTmp, "%s{%s}", PMC_TEX_HEADING, prev_p->str_p );
                        mbRemallocStr( prev_p->str_p, strTmp.get() );
                        foundHeading = TRUE;
                    }
                }
                // Punt this line in all cases
                mbStrListItemFree( str_p );
                continue;
            }

            gotBullet = FALSE;
            gotList = FALSE;

            if( mbStrPos( str_p->str_p, "-" ) == 0 )
            {
                pos = 1;
                gotBullet = TRUE;
            }
            else if( ( pos = mbStrTexEnum( str_p->str_p ) ) > 0 ) gotList = TRUE;

            if( gotBullet == FALSE && inBullet == TRUE )
            {
                mbStrListAdd( out_q, "\\end{myitemize}" );
                inBullet = FALSE;
            }

            if( gotList == FALSE && inList == TRUE )
            {
                mbStrListAdd( out_q, "\\end{myenumerate}" );
                inList = FALSE;
            }

            if( gotBullet == TRUE && inBullet == FALSE )
            {
                mbStrListAdd( out_q, "\\begin{myitemize}" );
                inBullet = TRUE;
            }

            if( gotList == TRUE && inList == FALSE )
            {
                mbStrListAdd( out_q, "\\begin{myenumerate}" );
                inList = TRUE;
            }

            if( gotBullet == TRUE || gotList == TRUE )
            {
                mbMalloc( temp_p, strlen( str_p->str_p ) + 256 );
                mbStrClean( str_p->str_p + pos, temp_p, FALSE );
                if( strlen( temp_p ) > 0 )
                {
                    mbSprintf( &strTmp, "\\item %s", mbStrClean( str_p->str_p + pos, NIL, FALSE ) );
                    mbStrListAdd( out_q, strTmp.get() );
                }
                mbStrListItemFree( str_p );
                mbFree( temp_p );
            }
            else
            {
                // Put the string into the output queue
                qInsertLast( out_q, str_p );
            }
        }

        if( inList == TRUE )
        {
            mbStrListAdd( out_q, "\\end{myenumerate}" );
        }

        if( inBullet == TRUE )
        {
            mbStrListAdd( out_q, "\\end{myitemize}" );
        }

        // All done, must copy output queue
        qWalk( str_p, out_q, mbStrList_p )
        {
            strOut.append( str_p->str_p );
            strOut.append( "\n" );
        }
        mbStrListFree( out_q );
    }

    // If line was truncated, append dots to it
    if( truncatedFlag ) strOut.append( "\\ldots" );

    // Copy to caller supplied buffer
    strcpy( out_p, strOut.get( ) );

exit:

    mbFree( in_p );
    mbStrListFree( in_q );
    mbStrListFree( out_q );

    if( pmcCfg[CFG_LOG_TEX].value ) mbLog( "Returning '%s'\n", out_p );
    {
        Int32u_t chunks2 = mbMallocChunks();
        if( chunks != chunks2 )
        {
            mbDlgInfo( "Malloc chunk mismatch: %lu != %lu", chunks, chunks2 );
        }
    }
    return out_p;
}

//---------------------------------------------------------------------------
// Copy the input file to the output file one line at a time, and make
// any substitutions from from substitution queue.
//---------------------------------------------------------------------------

Int32s_t    pmcTemplateSub
(
    Char_p          inName_p,
    Char_p          outName_p,
    qHead_p         sub_q
)
{
    FILE           *in_p = NIL;
    FILE           *out_p = NIL;
    Char_p          buf_p = NIL;
    mbStrList_p     str_p;
    Char_p          temp_p;
    Boolean_t       subFlag;
    Int32u_t        failedCount = 0;
    Int32s_t        returnCode = MB_RET_ERR;
    Ints_t          pos;
    Int32u_t        lineCount = 0;
    Boolean_t       skipMode = FALSE;
    Int32s_t        result;
    Char_t          compareKey[128];
    MbString        logString = MbString( );
    Int32s_t        type;

    mbMalloc( buf_p, 2048 );
    if( inName_p == NIL || outName_p == NIL || sub_q == NIL ) goto exit;

    // Open input file
    if( ( in_p = fopen( inName_p, "r" ) ) == NIL )
    {
        mbDlgError( "Failed to open input file '%s'\n", inName_p );
        goto exit;
    }

    // Open output file
    if( ( out_p = fopen( outName_p, "w" ) ) == NIL )
    {
        mbDlgError( "Failed to open output file '%s'\n", outName_p );
        goto exit;
    }

    /* This is for debugging */
    qWalk( str_p, sub_q, mbStrList_p )
    {
        logString.set( str_p->str2_p );
        mbLog( "Substitution list: '%s' -> '%s'\n", str_p->str_p, logString.truncate( 30 ) );
    }

    while( fgets( buf_p, 2040, in_p ) != 0 )
    {
        lineCount++;
        //mbLog( "Got string '%s'\n", buf_p );

        if( buf_p[0] == '%' )
        {
            // Found a comment line... ignore.
            continue;
        }
        for( pos = 0 , temp_p = buf_p ; ; )
        {
            if( ( pos = mbStrPos( temp_p, "[_" ) ) >= 0 )
            {
                *( temp_p + pos ) = 0;

                // mbLog( "Found key: out_p: '%p' temp_p: '%p' must output '%s'\n", out_p, temp_p, temp_p );

                if( skipMode == FALSE )
                {
                    fprintf( out_p, "%s", temp_p );
                }

                temp_p += pos;
                temp_p++;

                // What are we doing this? its removing new line at the end of lines with subs
                //mbStrClean( temp_p, NIL, FALSE );

                subFlag = FALSE;
                qWalk( str_p, sub_q, mbStrList_p )
                {
                    sprintf( compareKey, "%s]", str_p->str_p );

                    //mbLog( "Comparing: '%s' to '%s'\n", temp_p, compareKey );
                    if( mbStrPos( temp_p, compareKey ) == 0 )
                    {
                        // mbLog( "Found a match , str_p '%s' , str2_p '%s'\n", str_p->str_p, str_p->str2_p );
                        if( strcmp( str_p->str2_p, "_BOOLEAN_" ) == 0 )
                        {
                            // Simple latex scripting...
                            if( mbStrPos( str_p->str_p, "_IF_" ) == 0 )
                            {
                                // Detected an if statement in the LATEX template
                                if( str_p->handle == FALSE ) skipMode = TRUE;
                            }
                            if( mbStrPos( str_p->str_p, "_ENDIF_" ) == 0 )
                            {
                                skipMode = FALSE;
                            }
                        }
                        else if( strcmp( str_p->str2_p, "_CALLBACK_" ) == 0 && str_p->handle != 0 )
                        {
                            // Function pointer variable
                            Int32s_t (*func_p)( Int32u_t, FILE *, Char_p );

                            // The callback function pointer is stored in the
                            // string items's handle member
                            func_p = (Int32s_t (*)( Int32u_t, FILE *, Char_p ))str_p->handle;

                            // Call the callback function with first argument
                            // the value in the string item's handle2 member,
                            // second arg is a file pointer to the output
                            // file, and third argument is the key that
                            // was matched.
                            result = (*func_p)( str_p->handle2, out_p, str_p->str_p );

                            // Simple latex scripting...
                            if( mbStrPos( str_p->str_p, "_IF_" ) == 0 )
                            {
                                // Detected an if statement in the LATEX template
                                if( result == FALSE ) skipMode = TRUE;
                            }
                            if( mbStrPos( str_p->str_p, "_ENDIF_" ) == 0 )
                            {
                                skipMode = FALSE;
                            }
                        }
                        else
                        {
                            if( skipMode == FALSE )
                            {
                                // mbLog( "temp_p: '%s' outputting:  '%s'\n", temp_p, str_p->str2_p );
                                fprintf( out_p, "%s", str_p->str2_p );
                            }
                        }
                        subFlag = TRUE;
                        break;
                    }
                }

                if( subFlag == FALSE )
                {
                    // Here, look for keys that start with _IF_.  If found,
                    // look for _IF_<KEY>_ in the sub queue
                    type = pmcTemplateConditional( temp_p, compareKey );

                    if( type == PMC_TEMPLATE_KEY_TYPE_IF )
                    {
                        if( pmcTemplateKeyCheck( compareKey, sub_q ) )
                        {
                            // The key is present
                            skipMode = FALSE;
                        }
                        else
                        {
                            skipMode = TRUE;
                        }
                    }
                    else if( type == PMC_TEMPLATE_KEY_TYPE_ELSE )
                    {
                        skipMode = ( skipMode == TRUE ) ? FALSE : TRUE;
                    }
                    else if( type == PMC_TEMPLATE_KEY_TYPE_ENDIF )
                    {
                        skipMode = FALSE;
                    }

                    if( type != PMC_TEMPLATE_KEY_TYPE_ENDIF ) subFlag = TRUE;

                    // To prevent logging below
                    str_p = NIL;
                }

                if( subFlag == FALSE )
                {
                    logString.set( temp_p );

                    if( mbStrPos( temp_p, "_ENDIF" ) < 0 )
                    {
                        failedCount++;

                        // Only log if not an ENDIF
                        mbLog( "File '%s' Line: %d: Failed substitution at:\n\n[%s\n",
                            inName_p, lineCount, logString.clean() );
                    }
                }
                else
                {
                    if( str_p )
                    {
                        if( pmcCfg[CFG_LOG_TEX].value )
                        {
                            logString.set( str_p->str2_p );
                            mbLog( "Substitute string: '%s'\n", logString.truncate( 30 ) );
                        }
                    }
                }

                if( ( pos = mbStrPos( temp_p, "_]" )  ) < 0 )
                {
                    mbDlgError( "detected an error in the file\n" );
                    goto exit;
                }
                temp_p += pos;
                temp_p += 2;
            }
            else
            {
                if( skipMode == FALSE )
                {
                    // mbLog( "skipMode = FALSE; outputting: '%s'\n", temp_p );
                    fprintf( out_p, "%s", temp_p );
                }
                break;
            }
        }
    }

    returnCode = MB_RET_OK;

    if( failedCount > 0 )
    {
        mbLog( "Warning: Some parts of this document were not inserted into the document template." );
    }

exit:

    mbFree( buf_p );
    if( in_p ) fclose( in_p );
    if( out_p ) fclose( out_p );
    return returnCode;
}

//-----------------------------------------------------------------------------
// This function searches the queue for the specified key, returning TRUE
// if found, false otherwise
//-----------------------------------------------------------------------------
Boolean_t pmcTemplateKeyCheck( Char_p key_p, qHead_p q_p )
{
    Boolean_t       found = FALSE;
    mbStrList_p     str_p;

    if( key_p == NIL || q_p == NIL ) return FALSE;

    qWalk( str_p, q_p, mbStrList_p )
    {
        if( strcmp( str_p->str_p, key_p ) == 0 )
        {
            found = TRUE;
            break;
        }
    }
    return found;
}

//-----------------------------------------------------------------------------
// This function looks for keys of the format:
// _IF_KEY_, _ELSE_KEY_, ENDIF_KEY, and if found, returns _KEY_ in the
// supplied buffer, and returns TRUE.
//-----------------------------------------------------------------------------
Int32s_t pmcTemplateConditional( Char_p str_p, Char_p key_p )
{
    Ints_t      startPos = 0;
    Ints_t      endPos = 0;
    Char_p      result_p;
    Int32s_t    type = PMC_TEMPLATE_KEY_TYPE_NONE;

    if( str_p == NIL ) return PMC_TEMPLATE_KEY_TYPE_NONE;

    if( mbStrPos( str_p, "_IF_" ) == 0 )
    {
        startPos = 2;
        type = PMC_TEMPLATE_KEY_TYPE_IF;
    }
    else if( mbStrPos( str_p, "_ELSE_" ) == 0 )
    {
        startPos = 4;
        type = PMC_TEMPLATE_KEY_TYPE_ELSE;
    }
    else if( mbStrPos( str_p, "_ENDIF_" ) == 0 )
    {
        startPos = 5;
        type = PMC_TEMPLATE_KEY_TYPE_ENDIF;
    }

    if( startPos == 0 ) return PMC_TEMPLATE_KEY_TYPE_NONE;

    endPos = mbStrPos( str_p, "_]" );

    if( endPos == -1 ) return PMC_TEMPLATE_KEY_TYPE_NONE;

    // Make a copy of the input string
    mbMallocStr( result_p, str_p );

    // Remove trailing "]"
    *(result_p + endPos + 1) = 0;

    // Remove leading _IF, _ELSE, ENDIF
    strcpy( key_p, (result_p + startPos + 1) );
    mbFree( result_p );
    return type;
}

//-----------------------------------------------------------------------------
Char_p pmcFloatToStr( Float_t value, Int32u_t decimals )
{
    static Char_t   result[32];

    if( value < PMC_MIN_FLOAT ) return "";

    switch( decimals )
    {
        case 0:  sprintf( result, "%d", (Int32s_t)value ); break;
        case 1:  sprintf( result, "%.1f", value ); break;
        case 2:  sprintf( result, "%.2f", value ); break;
        case 3:  sprintf( result, "%.3f", value ); break;
        case 4:  sprintf( result, "%.4f", value ); break;
        case 5:  sprintf( result, "%.5f", value ); break;
        case 6:  sprintf( result, "%.6f", value ); break;
        case 7:  sprintf( result, "%.7f", value ); break;
        case 8:  sprintf( result, "%.8f", value ); break;
        case 9:  sprintf( result, "%.9f", value ); break;
        default: sprintf( result, "%f",   value ); break;
    }
    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void pmcSuspendPollIncDebug( Char_p function_p, Int32u_t line )
{
    // Int32u_t before = gSuspendPoll;
    gSuspendPoll++;

    //mbLog( "%s:%u: Incrementing gSuspendPoll: %u -> %u\n",
    //    function_p, line, before, gSuspendPoll );

    while( pmcInPoll ) { Sleep( 100 ); }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void pmcSuspendPollDecDebug( Char_p function_p, Int32u_t line )
{
    //Int32u_t before = gSuspendPoll;

    if( gSuspendPoll > 0 )
    {
        gSuspendPoll--;
        //mbLog( "%s:%u: Decrementing gSuspendPoll: %u -> %u\n",
        //  function_p, line, before, gSuspendPoll );
    }
    else
    {
        mbLog( "%s:%u: Error: gSuspendPoll already 0\n", function_p, line );
    }
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Char_p          pmcIntToRunCode
(
    Int32u_t    valueIn,
    Char_p      runCode_p
)
{
    Int32s_t    value;
    Ints_t      val1, val2;

    if( runCode_p == NIL ) goto exit;

    value = (Int32s_t)valueIn;
    if( value >= PMC_MSP_RUN_CODE_WRAP ) goto exit;

    value -= ( PMC_MSP_RUN_CODE_WRAP - PMC_MSP_RUN_CODE_SHIFT );

    if( value < 0 ) value +=  PMC_MSP_RUN_CODE_WRAP;

    // FIX:  This should really be turned into a loop
    val1 = value / 26;
    val2 = value - ( val1 * 26 );

    *(runCode_p)     = (Char_t)(val1 + 'A');
    *(runCode_p + 1) = (Char_t)(val2 + 'A');
    *(runCode_p + 2) = 0;

    mbLog( "value: %lu -> RunCode: '%s'\n", valueIn, runCode_p );
 exit:
    return runCode_p;
}

//---------------------------------------------------------------------------
// Function: pmcRunCodeToInt
//---------------------------------------------------------------------------
// Description:
//
// Converts an MSP run code, which is basically a base 26 number, to a
// decimal integer.
//---------------------------------------------------------------------------

Int32u_t        pmcRunCodeToInt
(
    Char_p      runCode_p
)
{
    Int32u_t    valid = FALSE;
    Int32u_t    returnValue = 0;
    Char_p      temp_p;
    Int32u_t    value = 0;
    Ints_t      i, len;
    Char_t      val;

    if( runCode_p == 0 ) goto exit;
    temp_p = runCode_p;

    if( ( len = strlen( runCode_p ) ) != 2 ) goto exit;

    for( i = 0 ; i < len ; i++ )
    {
        value *= 26;
        val = *temp_p++;

        if( val >= 'a' && val <= 'z' )
        {
            val -= 'a';
        }
        else if( val >= 'A' && val <= 'Z' )
        {
            val -= 'A';
        }
        else
        {
            // Invalid character in the run code
            goto exit;
        }

        value += (Int32u_t)val;
    }

    value += ( PMC_MSP_RUN_CODE_WRAP - PMC_MSP_RUN_CODE_SHIFT );
    if( value >= PMC_MSP_RUN_CODE_WRAP ) value -= ( PMC_MSP_RUN_CODE_WRAP );

    returnValue = value;
    valid = TRUE;
exit:
    if( valid )
    {
        mbLog( "Run Code '%s' -> value: %lu\n", runCode_p, returnValue );
    }
    else
    {
        if( runCode_p )
        {
            mbDlgExclaim( "Invalid runCode: '%s'\n", runCode_p );
        }
        else
        {
            mbDlgExclaim( "Invalid runCode: NIL pointer\n" );
        }
    }
    return returnValue;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------                              
// Description:
//---------------------------------------------------------------------------

Int32s_t        pmcTokenInit
(
    qHead_p     token_q,
    Char_p      bufIn_p
)
{
    Ints_t      returnCode = -1;
    Ints_t      i, len;
    Char_t      c;
    Char_p      buf_p = NIL;
    Char_p      buf2_p = NIL;
    Char_p      temp_p;
    bool        inToken = FALSE;
    pmcTokenStruct_p    token_p;

    // Initialize the queue
    qInitialize( token_q );

    if( bufIn_p == NIL ) goto exit;

    len = strlen( bufIn_p );
    if( len == 0 ) goto exit;

    mbCalloc( buf_p, len + 1 );
    mbCalloc( buf2_p, len + 1 );

    strcpy( buf_p, bufIn_p );

    len = strlen( buf_p );

    temp_p = buf_p;
    for( i = 0 ; i < len ; i++ )
    {
        c = *(buf_p + i);
        if( c == ' ' || c == '\t' )
        {
            if( inToken )
            {
                *(buf_p + i) = 0;
                // Clean the token
                mbStrClean( temp_p, buf2_p, FALSE );

                if( strlen( buf2_p ) )
                {
                    mbCalloc( token_p, sizeof( pmcTokenStruct_t ) );
                    mbCalloc( token_p->string_p, strlen( buf2_p ) + 1 );
                    token_p->returned = FALSE;
                    strcpy( token_p->string_p, buf2_p );
                    qInsertLast( token_q, token_p );
                }

                temp_p = buf_p + i + 1;
                inToken = FALSE;
            }
        }
        else
        {
            inToken = TRUE;
        }
    }

    if( inToken )
    {
        mbStrClean( temp_p, buf2_p, FALSE );

        if( strlen( buf2_p ) )
        {
            mbCalloc( token_p, sizeof( pmcTokenStruct_t ) );
            mbCalloc( token_p->string_p, strlen( buf2_p ) + 1 );
            token_p->returned = FALSE;
            strcpy( token_p->string_p, buf2_p );
            qInsertLast( token_q, token_p );
        }
    }

exit:
    if( buf_p )  mbFree( buf_p );
    if( buf2_p ) mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

pmcTokenStruct_p    pmcTokenNext
(
    qHead_p         token_q
)
{
    pmcTokenStruct_p    return_p = NIL;
    pmcTokenStruct_p    token_p;

    if( pmcTokenDone( token_q, FALSE ) == TRUE ) goto exit;

    token_p = (pmcTokenStruct_p)qRemoveFirst( token_q );
    qInsertLast( token_q, token_p );

    // Sanity check
    if( token_p->returned != FALSE ) mbDlgDebug(( "Token list error" ));

    return_p = token_p;
    token_p->returned = TRUE;

exit:
    return return_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

bool    pmcTokenDone
(
    qHead_p     token_q,
    bool        forceDone
)
{
    bool                returnCode = TRUE;
    pmcTokenStruct_p    token_p;

    if( forceDone == FALSE )
    {
        if( !qEmpty( token_q ) )
        {
            token_p = (pmcTokenStruct_p)qRemoveFirst( token_q );
            qInsertFirst( token_q, token_p );

            if( token_p->returned == FALSE )
            {
                // This token has not been returned yet
                returnCode = FALSE;
            }
            else
            {
                // All tokens returned; clean list
                forceDone = TRUE;
            }
        }
        else
        {
            // Queue is empty
        }
    }

    if( forceDone == TRUE )
    {
        for( ; ; )
        {
            if( qEmpty( token_q ) ) break;
            token_p = (pmcTokenStruct_p)qRemoveFirst( token_q );
            mbFree( token_p->string_p );
            mbFree( token_p );
        }
    }

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p          pmcFormatName
(
    Char_p      titleIn_p,
    Char_p      firstIn_p,
    Char_p      middleIn_p,
    Char_p      lastIn_p,
    Char_p      result_p,
    Int32u_t    style
)
{
    Char_p  first_p;
    Char_p  middle_p;
    Char_p  last_p;
    Char_p  title_p;
    Char_p  buf1_p;
    Char_p  buf2_p;

    Ints_t  titleLen;
    Ints_t  firstLen;
    Ints_t  middleLen;
    Ints_t  lastLen;

    mbMalloc( first_p,  128 );
    mbMalloc( middle_p, 128 );
    mbMalloc( last_p,   128 );
    mbMalloc( title_p,  128 );
    mbMalloc( buf1_p,   256 );
    mbMalloc( buf2_p,   256 );

    sprintf( first_p,  "" );
    sprintf( middle_p, "" );
    sprintf( last_p,   "" );
    sprintf( title_p,  "" );
    sprintf( buf1_p,   "" );
    sprintf( buf2_p,   "" );

    if( titleIn_p )  mbStrClean( titleIn_p,  title_p,   TRUE );
    if( firstIn_p )  mbStrClean( firstIn_p,  first_p,   TRUE );
    if( middleIn_p ) mbStrClean( middleIn_p, middle_p,  TRUE );
    if( lastIn_p )   mbStrClean( lastIn_p,   last_p,    TRUE );

    titleLen    = strlen( title_p );
    firstLen    = strlen( first_p );
    middleLen   = strlen( middle_p );
    lastLen     = strlen( last_p );

    switch( style )
    {
      // John Q. Smith (Mr.)
      case PMC_FORMAT_NAME_STYLE_TFML:
        if( lastLen )
        {
            if( titleLen )
            {
                sprintf( buf2_p, "%s ", title_p );
                strcat( buf1_p, buf2_p );
            }
            if( firstLen )
            {
                sprintf( buf2_p, "%s ", first_p );
                strcat( buf1_p, buf2_p );
            }
            if( middleLen )
            {
                sprintf( buf2_p, "%s ", middle_p );
                strcat( buf1_p, buf2_p );
            }
            sprintf( buf2_p, "%s ", last_p );
            strcat( buf1_p, buf2_p );
        }
        break;

      case PMC_FORMAT_NAME_STYLE_FMLT:
        if( lastLen )
        {
            if( firstLen )
            {
                sprintf( buf2_p, "%s ", first_p );
                strcat( buf1_p, buf2_p );
            }
            if( middleLen )
            {
                sprintf( buf2_p, "%s ", middle_p );
                strcat( buf1_p, buf2_p );
            }
            sprintf( buf2_p, "%s ", last_p );
            strcat( buf1_p, buf2_p );
            if( titleLen )
            {
                sprintf( buf2_p, " (%s)", title_p );
                strcat( buf1_p, buf2_p );
            }
        }
        break;
        default:
            mbDlgDebug(( "Invalid name format style" ));
            break;
    }

    if( result_p ) mbStrClean( buf1_p, result_p, TRUE );

    mbFree( first_p  );
    mbFree( middle_p );
    mbFree( last_p   );
    mbFree( title_p  );
    mbFree( buf1_p   );
    mbFree( buf2_p   );

    return result_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p pmcPhoneFormatString
(
    Char_p      in_p,
    Char_p      out_p
)
{
    Char_t      area[16];
    Char_t      number[32];
    Boolean_t   localFlag;

    if( in_p == NIL || out_p == NIL ) return "N/A";

    pmcPhoneFormat( in_p, area, number, &localFlag );

    if( strlen( in_p ) == 0 )
    {
        sprintf( out_p, "" );
    }
    else if( localFlag || strlen( area ) == 0 )
    {
        sprintf( out_p, "%s", number );
    }
    else
    {
        sprintf( out_p, "(%s) %s", area, number );
    }
    return out_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcPhoneFormat
(
    Char_p      in_p,
    Char_p      areaOut_p,
    Char_p      phoneOut_p,
    bool       *isLocal_p
)
{
    Char_p              buf_p;
    Char_p              str_p;
    Char_t              areaCode[8];
    Char_p              phone_p, p1, p2;
    bool                found;
    pmcLocalExchange_p  ex_p;

    mbMalloc( buf_p, 256 );
    mbMalloc( str_p, 256 );

    if( areaOut_p )  *areaOut_p = 0;
    if( phoneOut_p ) *phoneOut_p = 0;
    if( isLocal_p )  *isLocal_p = FALSE;

    if( in_p == NIL ) goto exit;

    // Make copy of phone number string
    strcpy( buf_p, in_p );

    // Remove all non digits from phone number and add area code if required
    pmcPhoneFix( buf_p, NIL );

    phone_p = buf_p;

    if( strlen( phone_p ) == 0 ) goto exit;

    if( strlen( phone_p ) != 10 )
    {
        nbDlgDebug(( "Invalid phone number '%s'", phone_p ));
        goto exit;
    }

    if( isLocal_p )
    {
        // Check to see if this is a local number
        mbLockAcquire( gLocalExchange_q->lock );
        found = FALSE;
  
        qWalk( ex_p, gLocalExchange_q, pmcLocalExchange_p )
        {
            if( mbStrPos( phone_p, ex_p->areaExchange ) == 0 )
            {
                found = TRUE;
                break;
            }
        }
        mbLockRelease( gLocalExchange_q->lock );
        *isLocal_p = found;
    }

    // Copy area code to output
    // Area code is first three digits
    areaCode[0] = *phone_p++;
    areaCode[1] = *phone_p++;
    areaCode[2] = *phone_p++;
    areaCode[3] = 0;

    if( areaOut_p ) strcpy( areaOut_p, areaCode );


    p1 = phone_p;
    p2 = str_p;
    if( strlen( phone_p ) == 7 )
    {
        for( Ints_t i = 0 ; ; i++ )
        {
            if( *p1 == NULL ) break;

            if( i == 3 )
            {
                *p2++ = '-';
            }
            *p2++ = *p1++;
        }
        *p2 = (Char_t)NULL;

        if( phoneOut_p ) strcpy( phoneOut_p, str_p );
    }
    else
    {
        if( phoneOut_p ) strcpy( phoneOut_p, phone_p );

    }
    nbDlgDebug(( "in '%s' area '%s' phone '%s' local: %d", in_p, areaOut_p, phoneOut_p, *isLocal_p ));

exit:
    if( buf_p ) mbFree( buf_p );
    if( str_p ) mbFree( str_p );
    return;
}

//---------------------------------------------------------------------------
// Function: pmcPhoneFix
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Ints_t         pmcPhoneFix
(
    Char_p      in_p,
    bool       *modifiedFlag_p
)
{
    Char_p      p1;
    Char_t      buf[64];
    Char_p      orig_p;
    Ints_t      returnCode;
    Int32u_t    letterCount = 0;
    Int32u_t    numberCount = 0;

    mbMalloc( orig_p, 32 );

    strcpy( buf,  in_p );
    strcpy( orig_p, in_p );

    // Next make alphanumeric.  This gets rid of all white space
    mbStrDigitsOnly( buf );

    // Next count numbers and letters to see if if is a postal code.
    // It could be a zip code.  If it looks like a postal code attempt
    // to format it as one, and generate return code indicating success
    for( p1 = buf ; *p1 != (Char_t)NULL ; p1++ )
    {
        if( *p1 >= 'A' && *p1 <= 'Z' )
        {
            letterCount++;
        }
        else if( *p1 >= '0' && *p1 <= '9' )
        {
            numberCount++;
        }
    }

    if( numberCount == 7 )
    {
        // Valid - no area code
        sprintf( in_p, "%03ld%s", pmcCfg[CFG_AREA_CODE].value, buf );
        returnCode = TRUE;
    }
    else if( numberCount == 10 )
    {
        // Valid - with area code
        sprintf( in_p, "%s", buf );
        returnCode = TRUE;
    }
    else if( numberCount == 0 && letterCount == 0 )
    {
        returnCode = TRUE;
        sprintf( in_p, "" );
    }
    else
    {
        // Invalid
        returnCode = FALSE;
    }

    if( modifiedFlag_p )
    {
        if( strcmp( in_p, orig_p ) == 0 )
        {
            // We did not change the
            *modifiedFlag_p = FALSE;
        }
        else
        {
            *modifiedFlag_p = TRUE;
        }
    }

    nbDlgDebug(( "in '%s' out '%s' valid: %ld modified: %d", orig_p, in_p, returnCode, *modifiedFlag_p ));

    mbFree( orig_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcViewAppointmentsConfirmation
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcViewAppointmentConfirmation( Int32u_t appointId )
{
    Char_p              cmd_p = NIL;
    PmcSqlPatient_p     patient_p;
    Char_p              buf_p = NIL;
    Char_p              provider_p = NIL;
    Int32u_t            confLetterDay;
    Int32u_t            confLetterTime;
    Int32u_t            confLetterId;
    Int32u_t            confPhoneDay;
    Int32u_t            confPhoneTime;
    Int32u_t            confPhoneId;
    Int32u_t            appointDay;
    Int32u_t            appointTime;
    Int32u_t            patientId;
    Int32u_t            providerId;
    Ints_t              l;
    Int32s_t            result = FALSE;
    MbSQL               sql;
    MbDateTime          dateTime;

    mbMalloc( cmd_p, 1024 );
    mbMalloc( buf_p, 64 );
    mbMalloc( provider_p, 64 );
    mbMalloc( patient_p, sizeof(PmcSqlPatient_t) );

    if( appointId == 0 ) goto exit;

    // Get Patient Name for display
    patientId = pmcAppointPatientId( appointId );
    pmcSqlPatientDetailsGet( patientId, patient_p );

    // Format SQL command
    sprintf( cmd_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_APPS_FIELD_CONF_LETTER_DATE,    // 0
        PMC_SQL_APPS_FIELD_CONF_LETTER_TIME,    // 1
        PMC_SQL_APPS_FIELD_CONF_LETTER_ID,      // 2
        PMC_SQL_APPS_FIELD_CONF_PHONE_DATE,     // 3
        PMC_SQL_APPS_FIELD_CONF_PHONE_TIME,     // 4
        PMC_SQL_APPS_FIELD_CONF_PHONE_ID,       // 5
        PMC_SQL_FIELD_DATE,                     // 6
        PMC_SQL_APPS_FIELD_START_TIME,          // 7
        PMC_SQL_FIELD_PROVIDER_ID,              // 8

        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_ID, appointId );


    if( sql.Query( cmd_p ) == FALSE ) goto exit;
    if( sql.RowCount() != 1 ) goto exit;
    if( sql.RowGet() == FALSE ) goto exit;

    confLetterDay   = sql.Int32u(0);
    confLetterTime  = sql.Int32u(1);
    confLetterId    = sql.Int32u(2);
    confPhoneDay    = sql.Int32u(3);
    confPhoneTime   = sql.Int32u(4);
    confPhoneId     = sql.Int32u(5);
    appointDay      = sql.Int32u(6);
    appointTime     = sql.Int32u(7);
    providerId      = sql.Int32u(8);

    dateTime.SetDateTime( appointDay, appointTime );

    l = 0;
    pmcProviderDescGet( providerId, provider_p );

    if( strlen( patient_p->firstName ) || strlen( patient_p->lastName ) )
    {
        l += sprintf( cmd_p+l, "Appointment for %s %s ", patient_p->firstName, patient_p->lastName );
    }
    else
    {
        l += sprintf( cmd_p+l, "Appointment " );
    }

    l += sprintf( cmd_p+l, "scheduled for:\n\n    %s %s  with %s\n\n",
        dateTime.MDY_DateString( ), dateTime.HM_TimeString( ), provider_p );

    if( confPhoneDay && confPhoneTime )
    {
        dateTime.SetDate( confPhoneDay );
        dateTime.SetTime( confPhoneTime );

        pmcProviderDescGet( confPhoneId, provider_p );

        sprintf( buf_p, "" );
        if( confPhoneDay != appointDay  ||  confPhoneTime != appointTime || confPhoneId != providerId  )
        {
            if( confPhoneDay != appointDay  ||  confPhoneTime != appointTime )
            {
                if( confPhoneId != providerId )
                {
                    sprintf( buf_p, "(Does not match scheduled time or provider)" );
                }
                else
                {
                    sprintf( buf_p, "(Does not match scheduled time)" );
                }
            }
            else
            {
                sprintf( buf_p, "(Does not match scheduled provider)" );
            }
        }

        l += sprintf( cmd_p+l, "Confirmed by telephone for:\n\n    %s %s  with %s    %s\n\n",
            dateTime.MDY_DateString( ), dateTime.HM_TimeString( ), provider_p, buf_p );
    }
    else
    {
        l += sprintf( cmd_p+l, "Not confirmed by telephone.\n\n" );
    }

    if( confLetterTime && confLetterDay )
    {
        dateTime.SetDate( confLetterDay );
        dateTime.SetTime( confLetterTime );

        pmcProviderDescGet( confLetterId, provider_p );

        sprintf( buf_p, "" );
        if( confLetterDay != appointDay || confLetterTime != appointTime || confLetterId != providerId )
        {
            if( confLetterDay != appointDay  ||  confLetterTime != appointTime )
            {
                if( confLetterId != providerId )
                {
                    sprintf( buf_p, "(Does not match scheduled time or provider)" );
                }
                else
                {
                    sprintf( buf_p, "(Does not match scheduled time)" );
                }
            }
            else
            {
                sprintf( buf_p, "(Does not match scheduled provider)" );
            }
        }

        sprintf( cmd_p+l, "Confirmed by letter for:\n\n    %s %s  with %s    %s\n\n",
            dateTime.MDY_DateString( ), dateTime.HM_TimeString( ), provider_p, buf_p );
    }
    else
    {
        sprintf( cmd_p+l, "Not confirmed by letter.\n\n" );
    }

    mbDlgInfo( cmd_p );
    result = TRUE;

exit:

    if( result == FALSE )
    {
        mbLog( "pmcViewAppointmentConfirmation() failed\n" );
    }

    mbFree( cmd_p );
    mbFree( buf_p );
    mbFree( patient_p );
    mbFree( provider_p );

    return;
}

//---------------------------------------------------------------------------
// Function:  pmcAppointmentConfirmLetter
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcAppointmentConfirmLetter
(
    Int32u_t            appointId,
    Int32u_t            confirmFlag
)
{
    Char_p              cmd_p = NIL;
    pmcPatAppLetter_p   app_p= NIL;
    MbSQL               sql;
    Int32s_t            result = FALSE;

    mbMalloc( cmd_p, 512 );
    mbCalloc( app_p, sizeof( pmcPatAppLetter_t ) );

    if( appointId == 0 ) goto exit;

    if( confirmFlag == FALSE )
    {
        sprintf( cmd_p, "update %s set %s=0,%s=0,%s=0 where %s=%ld and %s=1",
            PMC_SQL_TABLE_APPS,
            PMC_SQL_APPS_FIELD_CONF_LETTER_DATE,
            PMC_SQL_APPS_FIELD_CONF_LETTER_TIME,
            PMC_SQL_APPS_FIELD_CONF_LETTER_ID,
            PMC_SQL_FIELD_ID, appointId,
            PMC_SQL_FIELD_NOT_DELETED );

        result = pmcSqlExec( cmd_p );

        pmcAppHistory( appointId, PMC_APP_ACTION_CONFIRM_LETTER_CANCEL, 0, 0, 0, 0, NIL );
        mbLog( "Cancelled letter confirmation for appointment %d\n", appointId );
        goto exit;
    }

    // Format SQL command    0  1  2  3  4  5  6  7
    sprintf( cmd_p, "select %s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld and %s=1",
        PMC_SQL_FIELD_ID,                           // 0
        PMC_SQL_FIELD_PATIENT_ID,                   // 1
        PMC_SQL_APPS_FIELD_START_TIME,              // 2
        PMC_SQL_FIELD_DATE,                         // 3
        PMC_SQL_FIELD_PROVIDER_ID,                  // 4
        PMC_SQL_APPS_FIELD_CONF_LETTER_DATE,        // 5
        PMC_SQL_APPS_FIELD_CONF_LETTER_TIME,        // 6
        PMC_SQL_APPS_FIELD_REFERRING_DR_ID,         // 7

        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_ID, appointId,
        PMC_SQL_FIELD_NOT_DELETED );

    if( sql.Query( cmd_p ) == FALSE ) goto exit;
    if( sql.RowCount() != 1 ) goto exit;
    if( sql.RowGet() == FALSE ) goto exit;

    app_p->id               = sql.Int32u( 0 );
    app_p->patientId        = sql.Int32u( 1 );
    app_p->startTime        = sql.Int32u( 2 );
    app_p->day              = sql.Int32u( 3 );
    app_p->providerId       = sql.Int32u( 4 );
    app_p->confLetterDay    = sql.Int32u( 5 );
    app_p->confLetterTime   = sql.Int32u( 6 );
    app_p->referringId      = sql.Int32u( 7 );

    // Compute the appointment time as a 64 bit integer for sorting
    app_p->dateTimeInt64 = (unsigned __int64)app_p->day;
    app_p->dateTimeInt64 *= 1000000;
    app_p->dateTimeInt64 += (unsigned __int64)app_p->startTime;

    pmcPatAppLetterMake( app_p, TRUE, FALSE );

    if( app_p->result != PMC_APP_LETTER_RESULT_SUCCESS )
    {
         sprintf( cmd_p, "Appointment letter for %s %s could not be printed.\nReason: %s",
            app_p->subStr[PMC_SUB_STR_FIRST_NAME], app_p->subStr[PMC_SUB_STR_LAST_NAME],
            pmcPatAppLetterResultStrings[ app_p->result ] );
    }
    else
    {
        sprintf( cmd_p, "Appointment letter for %s %s sent to printer.",
            app_p->subStr[PMC_SUB_STR_FIRST_NAME], app_p->subStr[PMC_SUB_STR_LAST_NAME] );
    }
    mbDlgInfo( cmd_p );

    mbLog( "Letter confirmed appointment %d\n", appointId );
    result = TRUE;
exit:

    mbFree( cmd_p );
    if( app_p ) pmcPatAppLetterFree( app_p );

    if( result == FALSE )
    {
        mbDlgDebug(( "Error" ));
    }

    return;
}

//---------------------------------------------------------------------------
// Function:  pmcAppointmentConfirmPhone
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcAppointmentConfirmPhone
(
    Int32u_t    appointId,
    Int32u_t    providerId,
    Int32u_t    confirmFlag
)
{
    Char_p      cmd_p;
    Int32u_t    date, time;


    mbMalloc( cmd_p, 256 );
    if( appointId == 0 ) goto exit;

    pmcSuspendPollInc( );

    if( confirmFlag == TRUE )
    {
        // Set the confirmend phone time to the appointment time
        sprintf( cmd_p, "update %s set %s=%s,%s=%s,%s=%ld where %s=%ld",
            PMC_SQL_TABLE_APPS,
            PMC_SQL_APPS_FIELD_CONF_PHONE_DATE, PMC_SQL_FIELD_DATE,
            PMC_SQL_APPS_FIELD_CONF_PHONE_TIME, PMC_SQL_APPS_FIELD_START_TIME,
            PMC_SQL_APPS_FIELD_CONF_PHONE_ID, providerId,
            PMC_SQL_FIELD_ID, appointId );

        pmcSqlExec( cmd_p );

        mbLog( "Phone confirmation for appointment %d\n", appointId );

        sprintf( cmd_p, "select %s from %s where %s=%ld",
            PMC_SQL_APPS_FIELD_CONF_PHONE_DATE,
            PMC_SQL_TABLE_APPS,
            PMC_SQL_FIELD_ID, appointId );

        date = pmcSqlSelectInt( cmd_p, NIL );

        sprintf( cmd_p, "select %s from %s where %s=%ld",
            PMC_SQL_APPS_FIELD_CONF_PHONE_TIME,
            PMC_SQL_TABLE_APPS,
            PMC_SQL_FIELD_ID, appointId );

        time = pmcSqlSelectInt( cmd_p, NIL );

        pmcAppHistory( appointId, PMC_APP_ACTION_CONFIRM_PHONE, date, time, providerId, 0, NIL );
    }
    else
    {
        sprintf( cmd_p, "update %s set %s=0,%s=0,%s=0 where %s=%ld and %s=1",
            PMC_SQL_TABLE_APPS,
            PMC_SQL_APPS_FIELD_CONF_PHONE_DATE,
            PMC_SQL_APPS_FIELD_CONF_PHONE_TIME,
            PMC_SQL_APPS_FIELD_CONF_PHONE_ID,
            PMC_SQL_FIELD_ID, appointId,
            PMC_SQL_FIELD_NOT_DELETED );

        pmcSqlExec( cmd_p );
        mbLog( "Cancelled phone confirmation for appointment %d\n", appointId );
        pmcAppHistory( appointId, PMC_APP_ACTION_CONFIRM_PHONE_CANCEL, 0, 0, 0, 0, NIL );
    }
    pmcSqlExec( cmd_p );

    pmcSuspendPollDec( );
 
exit:
    mbFree( cmd_p );
}

//---------------------------------------------------------------------------
// Function:  pmcViewAppointments
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#define PMC_VIEW_APPOINTMENTS_IBOX  (0)

#if PMC_VIEW_APPOINTMENTS_IBOX
void pmcViewAppointments( Int32u_t patientId )
{
    Char_p      b;
    Char_p      cmd_p;
    Char_p      result_p;
    Ints_t      l = 0;
    Int32u_t    startTime;
    Int32u_t    day;
    Int32u_t    providerId;
    Int32u_t    duration;
    Int32u_t    rowCount = 0;
    MDateTime  *dateTime_p;

    mbMalloc( cmd_p, 256 );
    mbMalloc( result_p, 256 );
    mbMalloc( b, 2048 );

    sprintf( cmd_p, "select %s from %s where %s=%ld",
        PMC_SQL_PATIENTS_FIELD_FIRST_NAME,
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_PATIENTS_FIELD_ID,
        patientId );

    l = sprintf( b+l, "Appointments for %s ", pmcSqlSelectString( cmd_p, result_p, 256 ) );

    sprintf( cmd_p, "select %s from %s where %s=%ld",
        PMC_SQL_PATIENTS_FIELD_LAST_NAME,
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_PATIENTS_FIELD_ID,
        patientId );

    l += sprintf( b+l, "%s:\n\n", pmcSqlSelectString( cmd_p, result_p, 256 ) );

    // Query the appointents table
    sprintf( cmd_p, "select %s,%s,%s,%s from %s where %s=%ld and %s=1",
        PMC_SQL_APPS_FIELD_START_TIME,
        PMC_SQL_APPS_FIELD_DAY,
        PMC_SQL_APPS_FIELD_PROVIDER_ID,
        PMC_SQL_APPS_FIELD_DURATION,
        PMC_SQL_TABLE_APPS,
        PMC_SQL_APPS_FIELD_PATIENT_ID, patientId,
        PMC_SQL_FIELD_NOT_DELETED );

    PMC_GENERAL_QUERY_SETUP( cmd_p );
    while( !pmcDataSet_p->Eof )
    {
        startTime = pmcDataSet_p->FieldValues[ PMC_SQL_APPS_FIELD_START_TIME ];
        day = pmcDataSet_p->FieldValues[ PMC_SQL_APPS_FIELD_DAY ];
        providerId = pmcDataSet_p->FieldValues[ PMC_SQL_APPS_FIELD_PROVIDER_ID ];
        duration =  pmcDataSet_p->FieldValues[ PMC_SQL_APPS_FIELD_DURATION ];

        dateTime_p = new MDateTime( day, startTime );

        l += sprintf( b+l, "%s ", dateTime_p->DMY_DateString( result_p ) );
        l += sprintf( b+l, "%s ", dateTime_p->HM_TimeString( result_p ) );
        l += sprintf( b+l, "Duration: %ld min. ", duration );
        l += sprintf( b+l, "Provider: %s\n", pmcProviderDescGet( providerId, result_p ) );

        delete dateTime_p;

        rowCount++;
        pmcDataSet_p->Next( );
    }
    PMC_GENERAL_QUERY_CLEANUP( );

    l += sprintf( b+l, "\nNumber of appointments: %ld\n", rowCount );

    PMC_INFO_BOX(( b ));

    mbFree( b );
    mbFree( cmd_p );
    mbFree( result_p );

    return;

}

#else

Int32u_t pmcViewAppointments
(
    Int32u_t            patientId,
    bool                showFuture,
    bool                showPast,
    bool                allowGoto,
    Int32u_p            gotoProviderId_p,
    Int32u_p            gotoDate_p,
    bool                allowPatientSelect,
    Int32u_t            mode
)
{
    TAppListForm       *form_p;
    pmcAppListInfo_t    formInfo;
    Int32u_t            providerId = 0;
    Int32u_t            date = 0;
    Int32u_t            returnId = 0;

    formInfo.mode       = mode;
    formInfo.patientId  = patientId;
    formInfo.showFuture = showFuture;
    formInfo.showPast   = showPast;
    formInfo.allowGoto  = allowGoto;
    formInfo.allowPatientSelect = allowPatientSelect;

    form_p = new TAppListForm( NIL, &formInfo );
    form_p->ShowModal( );
    delete form_p;

    if( allowGoto == TRUE && formInfo.gotGoto == TRUE )
    {
        providerId = formInfo.providerId;
        date       = formInfo.date;
    }

    if( gotoProviderId_p ) *gotoProviderId_p = providerId;
    if( gotoDate_p       ) *gotoDate_p       = date;

    if( formInfo.returnCode == MB_BUTTON_OK )
    {
        returnId = formInfo.appointId;
    }
    return returnId;
}

#endif //PMC_VIEW_APPOINTMENTS_IBOX

Int32u_t pmcEchoPatSelect
(
    Char_p      echoName_p,
    Int32u_t    echoDate,
    Int32u_t    providerId,
    Int32u_p    id_p,
    Boolean_t   showCancelButton
)
{
    pmcEchoPatSelectFormInfo_t  formInfo;
    TEchoPatSelectForm         *form_p;

    formInfo.name_p = echoName_p;
    formInfo.date = echoDate;
    formInfo.providerId = providerId;

    if( id_p ) *id_p = 0;

    form_p = new TEchoPatSelectForm( NIL, &formInfo );

    if( formInfo.abortFlag == FALSE )
    {
        form_p->ShowModal( );
        if( id_p ) *id_p = formInfo.patientId;
    }
    delete form_p;

    return formInfo.returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Create a "temporary" file name.  Put the hostname first so that
// batches of files (i.e., print jobs) will get grouped by host.
//---------------------------------------------------------------------------

Char_p  pmcMakeFileName
(
    Char_p      path_p,
    Char_p      in_p
)
{
    Int32u_t    curdate;
    Int32u_t    curtime;
    Ints_t      l = 0;

    if( in_p == NIL ) goto exit;
    curdate = mbToday( );
    curtime = mbTime( );

    if( path_p )
    {
        l = sprintf( in_p, "%s\\", path_p );
    }

    sprintf( in_p+l, "%s_%08ld_%06ld_%02X",  pmcCfg[CFG_HOSTNAME].str_p, curdate, curtime,
        (Int8u_t)( pmcFileCounter & 0x000000FF ) );

    pmcFileCounter++;

exit:

    return in_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcTime( void )
{
    time_t      timer;
    struct tm  *tblock;
    Int32u_t    timeint;

    timer  = time( NULL );
    tblock = localtime( &timer );

    timeint  = tblock->tm_hour * 10000;
    timeint += tblock->tm_min * 100;
    timeint += tblock->tm_sec;

    return timeint;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p pmcFormatString
(
    Char_p  string_p,
    ...
)
{
    va_list             arg;
    Char_p              str_p;

    str_p = (Char_p)malloc( 1024 );
    if( str_p == NULL ) return str_p;

    va_start( arg, string_p );
    vsprintf( str_p, string_p, arg );
    va_end( arg );
    return str_p;
}

//---------------------------------------------------------------------------
// Function: pmcStringStripDoubleQuotes
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Char_p      pmcStringStripDoubleQuotes
(
    Char_p      in_p
)
{
    Char_p  temp1_p, temp2_p;
    Ints_t  i, len;

    len = strlen( in_p );
    temp1_p = in_p;
    temp2_p = in_p;

    for( i = 0 ; i < len ; i++ )
    {
        if( *temp1_p != '\"' )
        {
            *temp2_p++ = *temp1_p;
        }
        temp1_p++;
    }
    *temp2_p = 0;
    return in_p;
}



//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  pmcFormatPhnDisplay
(
    Char_p      phn_p,
    Char_p      phnProv_p,
    Char_p      result_p
)
{
    Int32u_t    len;
    Char_p      temp_p = NIL;
    Char_p      temp2_p = NIL;
    Char_p      p1, p2;
    Char_p      buf_p = NIL;

    mbMalloc( buf_p, 128 );

    if( result_p ) *result_p = 0;

    if( phn_p == NULL )
    {
        if( result_p ) *result_p = 0;
        goto exit;
    }
    len = strlen( phn_p );

    if( len == 0 )
    {
        if( result_p ) *result_p = 0;
        goto exit;
    }

    mbMalloc( temp_p, len + 100 );
    mbMalloc( temp2_p, len + 100 );

    strcpy( temp_p, phn_p );
    mbStrDigitsOnly( temp_p );

    p2 = temp2_p;
    p1 = temp_p;

    for( Ints_t i = 0 ;  ; i++ )
    {
        if( *p1 == NULL ) break;

        if( i == 3 || i == 6 )
        {
            *p2++ = ' ';
        }
        *p2++ = *p1++;
    }
    *p2 = (Char_t)NULL;

    if( phnProv_p )
    {
        if( strlen( phnProv_p ) )
        {
            mbStrToUpper( phnProv_p );
            if( mbStrPos( phnProv_p, PMC_PHN_DEFAULT_PROVINCE ) == -1 )
            {
                sprintf( buf_p, " (%s)", phnProv_p );
                strcat( temp2_p, buf_p );
            }
        }
    }

    nbDlgDebug(( "phn: '%s'", phn_p ));
exit:
    if( result_p && temp2_p )
    {
        if( strlen( temp2_p ) > 0 )
        {
            strcpy( result_p, temp2_p );
        }
        else
        {
            *result_p = 0;
        }
    }

    if( temp_p ) mbFree( temp_p );
    if( temp2_p ) mbFree( temp2_p );
    if( buf_p ) mbFree( buf_p );

    return result_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  pmcFormatPhnSearch
(
    Char_p      phn_p,
    Char_p      result_p
)
{
    Ints_t      len;
    Char_p      temp_p = NIL;

    if( result_p ) *result_p = 0;

    if( phn_p == NULL ) goto exit;

    len = strlen( phn_p );

    if( len == 0 ) goto exit;

    mbMalloc( temp_p, len + 100 );

    strcpy( temp_p, phn_p );
    mbStrAlphaNumericOnly( temp_p );
    mbStrToLower( temp_p );

exit:
    if( result_p && temp_p )
    {
        strcpy( result_p, temp_p );
    }

    if( temp_p ) mbFree( temp_p );

    return result_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
__int64 pmcAtoI64
(
    Char_p      string_p
)
{
    Char_p      p1;
    __int64     num = 0i64;
    Int32u_t    temp;

    if( string_p == NULL ) goto exit;
    p1 = string_p;

    for( ; ; )
    {
        if( *p1 == NULL ) break;

        if( *p1 < '0' || *p1 > '9' )
        {
            break;
        }

        num = num * 10i64;
        temp = *p1 - '0';
        num = num + (__int64)temp;
        p1++;
    }
    nbDlgDebug(( "string '%s' int64 %Ld", string_p, num ));

exit:

    return num;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcPopupItemEnable( TPopupMenu *menu_p, Char_p str_p, bool enable )
{
    Ints_t  count1, count2, count3;
    Ints_t  i, j, k;
    bool    found = FALSE;

    // Sanity check
    if( menu_p == NIL )
    {
        mbDlgDebug(( "invalid popup menu pointer" ));
        return;
    }

    count1 = menu_p->Items->Count;

    // For now, just hard code three levels of menus
    for( i = 0 ; i < count1 ; i++ )
    {
        if( mbStrPos( menu_p->Items->Items[i]->Name.c_str(), str_p ) >= 0 )
        {
            menu_p->Items->Items[i]->Enabled = enable;
            found = TRUE;
            break;
        }

        count2 = menu_p->Items->Items[i]->Count;
        for( j = 0 ; j < count2 ; j++ )
        {
            if( mbStrPos( menu_p->Items->Items[i]->Items[j]->Name.c_str(), str_p ) >= 0 )
            {
                menu_p->Items->Items[i]->Items[j]->Enabled = enable;
                found = TRUE;
                break;
            }

            count3 = menu_p->Items->Items[i]->Items[j]->Count;
            for( k = 0 ; k < count3 ; k++ )
            {
                if( mbStrPos( menu_p->Items->Items[i]->Items[j]->Items[k]->Name.c_str(), str_p ) >= 0 )
                {
                    menu_p->Items->Items[i]->Items[j]->Items[k]->Enabled = enable;
                    found = TRUE;
                    break;
                }
            }
            if( found ) break;
        }
        if( found ) break;
    }

    if( !found )
    {
        mbDlgDebug(( "Could not find item '%s'", str_p ));
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void  pmcPopupItemEnableAll( TPopupMenu *menu_p, bool enable )
{
    Ints_t  count1, count2, count3;
    Ints_t  i, j, k;

    // Sanity check
    if( menu_p == NIL )
    {
        mbDlgDebug(( "invalid popup menu pointer" ));
        return;
    }

    count1 = menu_p->Items->Count;

    // For now, just hard code three levels of menus
    for( i = 0 ; i < count1 ; i++ )
    {
        menu_p->Items->Items[i]->Enabled = enable;

        count2 = menu_p->Items->Items[i]->Count;
        for( j = 0 ; j < count2 ; j++ )
        {
            menu_p->Items->Items[i]->Items[j]->Enabled = enable;
            count3 = menu_p->Items->Items[i]->Items[j]->Count;
            for( k = 0 ; k < count3 ; k++ )
            {
                menu_p->Items->Items[i]->Items[j]->Items[k]->Enabled = enable;
            }
        }
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcExpBox( Char_p code_p )
{
    Char_t          code[8];
    Int32u_t        returnCode = FALSE;
    pmcExpStruct_p  exp_p;
    bool            found = FALSE;

    if( strlen( code_p ) > 4 )
    {
        mbDlgExclaim( "Invalid EXP code '%s'", code_p );
        goto exit;
    }

    mbStrClean( code_p, code, TRUE );

    mbLockAcquire( pmcExp_q->lock );

    qWalk( exp_p, pmcExp_q, pmcExpStruct_p )
    {
        if( strcmp( code, exp_p->code ) == 0 )
        {
            found = TRUE;
            break;
        }
    }
    mbLockRelease( pmcExp_q->lock );

    if( found )
    {
        mbDlgInfo( "Explain code:    %s \n\n\n%s", exp_p->code, exp_p->string_p );
    }

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p pmcProviderDescGet
(
    Int32u_t    providerId,
    Char_p      result_p
)
{
    pmcProviderList_p   provider_p;
    bool                found = FALSE;

    if( result_p == NIL ) goto exit;

    mbLockAcquire( pmcProvider_q->lock );

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        nbDlgDebug(( "Got provider '%s' index %ld id %ld",
            provider_p->description,
            provider_p->id,
            provider_p->listIndex ));

        if( provider_p->id == providerId )
        {
            found = TRUE;
            break;
        }
    }

    if( found )
    {
        sprintf( result_p, "%s", provider_p->description_p );
    }
    else
    {
        sprintf( result_p, "Unknown" );
    }
    mbLockRelease( pmcProvider_q->lock );

exit:

    return result_p;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t                pmcProviderIdGet
(
    Char_p              desc_p
)
{
    Int32u_t            providerId = 0;
    pmcProviderList_p   provider_p;

    mbLockAcquire( pmcProvider_q->lock );

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        if( strcmp( provider_p->description_p, desc_p ) == 0 )
        {
            providerId = provider_p->id;

            nbDlgDebug(( "Got provider '%s'", provider_p->description ));
            break;
        }
    }

    mbLockRelease( pmcProvider_q->lock );
    return providerId;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
// Thus function gets the id (i.e., primary key) of the provider with the
// specified provider number.  This whole system is braindead because the
// provider number might not be unique.  So we will say that only one
// entry in the provider table (among entries with the same provider number)
// can have a last name.
//---------------------------------------------------------------------------

Int32u_t                pmcProviderNumberToId
(
    Int32u_t            providerNumber
)
{
    Int32u_t            providerId = 0;
    pmcProviderList_p   provider_p;
    Int32u_t            count = 0;

    mbLockAcquire( pmcProvider_q->lock );

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        if( ( provider_p->providerNumber == providerNumber ) && provider_p->lastNameLen )
        {
            count++;
        }
    }

    if( count > 1 )
    {
        mbDlgExclaim( "Error: Duplicate provider numbers\n" );
    }

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        if( ( provider_p->providerNumber == providerNumber ) && provider_p->lastNameLen )
        {
            providerId = provider_p->id;
            break;
        }
    }

    mbLockRelease( pmcProvider_q->lock );
    return providerId;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t                pmcProviderIndexGet
(
    TComboBox          *comboBox_p,
    Int32u_t            providerId
)
{
    Int32s_t            index = -1;
    pmcProviderList_p   provider_p;
    int                 count, i;

    mbLockAcquire( pmcProvider_q->lock );

    count = comboBox_p->Items->Count;

    for( i = 0 ; i < count ; i++ )
    {
        mbLog( "checking '%s'\n", comboBox_p->Items->Strings[i].c_str() );

        qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
        {
            if( strcmp( provider_p->description_p, comboBox_p->Items->Strings[i].c_str() ) == 0 )
            {
                if( provider_p->id == providerId )
                {
                    index = i;
                    goto exit;
                }                    
            }
        }
    }
exit:
    mbLockRelease( pmcProvider_q->lock );
    return index;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t                pmcProviderNumberGet
(
    Int32u_t            providerId
)
{
    Int32s_t            providerNumber = 0;
    pmcProviderList_p   provider_p;

    mbLockAcquire( pmcProvider_q->lock );

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        if( provider_p->id == providerId )
        {
            providerNumber = provider_p->providerNumber;
            break;
        }
    }
    mbLockRelease( pmcProvider_q->lock );
    return providerNumber;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t                pmcProviderCounterGet
(
    Int32u_t            providerId,
    Int32u_t            counter,
    Int32u_t            mode
)
{
    Int32s_t            returnCount = -1;
    pmcProviderList_p   provider_p;
    pmcProviderList_p   providerFound_p = NIL;
    MbString            buf = MbString();

    mbLockAcquire( pmcProvider_q->lock );

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        if( provider_p->id == providerId )
        {
            providerFound_p = provider_p;
        }
        if( mode == PMC_COUNTER_UPDATE )
        {
            if( counter == PMC_PROVIDER_COUNT_PATIENTS  ) provider_p->patientCount = -1;
            if( counter == PMC_PROVIDER_COUNT_APPS      ) provider_p->appointmentCount = -1;
            if( counter == PMC_PROVIDER_COUNT_DOCUMENTS ) provider_p->documentCount = -1;
        }
    }

    if( providerFound_p == NIL ) goto exit;

    provider_p = providerFound_p;

    if( counter == PMC_PROVIDER_COUNT_PATIENTS )
    {
        if( provider_p->patientCount == -1 ) mode = PMC_COUNTER_UPDATE;
        if( mode == PMC_COUNTER_UPDATE )
        {
            mbSprintf( &buf, "select sum( %s ) from %s where %s=%ld",
                PMC_SQL_FIELD_NOT_DELETED,
                PMC_SQL_TABLE_PATIENTS,
                PMC_SQL_FIELD_PROVIDER_ID,
                providerId );
            provider_p->patientCount = pmcSqlSelectInt( buf.get(), NIL );
        }
        returnCount = provider_p->patientCount;
    }
    else if( counter == PMC_PROVIDER_COUNT_APPS )
    {
        if( provider_p->appointmentCount == -1 ) mode = PMC_COUNTER_UPDATE;
        if( mode == PMC_COUNTER_UPDATE )
        {
            mbSprintf( &buf, "select sum( %s ) from %s where %s=%ld and %s >= %ld",
                PMC_SQL_FIELD_NOT_DELETED,
                PMC_SQL_TABLE_APPS,
                PMC_SQL_FIELD_PROVIDER_ID, providerId,
                PMC_SQL_FIELD_DATE, mbToday( ) );

            provider_p->appointmentCount = pmcSqlSelectInt( buf.get(), NIL );
        }
        returnCount = provider_p->appointmentCount;
    }
    else if( counter == PMC_PROVIDER_COUNT_DOCUMENTS )
    {
        if( provider_p->documentCount == -1 ) mode = PMC_COUNTER_UPDATE;
        if( mode == PMC_COUNTER_UPDATE )
        {
            mbSprintf( &buf, "select sum( %s ) from %s where %s=%ld",
                PMC_SQL_FIELD_NOT_DELETED,
                PMC_SQL_TABLE_DOCUMENTS,
                PMC_SQL_FIELD_PROVIDER_ID,
                providerId );
            provider_p->documentCount = pmcSqlSelectInt( buf.get(), NIL );
        }
        returnCount = provider_p->documentCount;
    }

exit:
    mbLockRelease( pmcProvider_q->lock );
    return returnCount;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void  pmcProviderListFree
(
    TComboBox  *providerList_p
)
{
    providerList_p->Items->Clear();
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t pmcProviderListBuild
(
    TComboBox  *providerList_p,
    Int32u_t    providerId,
    Int32u_t    providerNumberRequiredFlag,
    Int32u_t    initializeIndexFlag
)
{
    pmcProviderList_p   provider_p;
    Ints_t              index = -1;
    Ints_t              i = 0;
    Int32u_t            providerIdOut = 0;

    mbLockAcquire( pmcProvider_q->lock );

    providerList_p->Text = "";
    providerList_p->Items->Clear();

    qWalk( provider_p, pmcProvider_q, pmcProviderList_p )
    {
        nbDlgDebug(( "Got provider '%s' index %ld id %ld",
            provider_p->description_p,
            provider_p->id,
            provider_p->listIndex ));

        if( provider_p->picklistOrder > 0 )
        {
            if(       providerNumberRequiredFlag == FALSE
                 || ( providerNumberRequiredFlag == TRUE && provider_p->providerNumber != 0 ) )
            {
                // Add description to the list box
                providerList_p->Items->Add( provider_p->description_p );

                if( provider_p->id == providerId )
                {
                    providerIdOut = providerId;
                    index = i;
                }

                i++;
            }
        }
    }

    if( index == -1 && initializeIndexFlag == TRUE )
    {
        // We may have filtered the provider list, and did not match the
        // the provider, so use the first provider in the list
        provider_p = (pmcProviderList_p)pmcProvider_q->linkage.flink;

        if( provider_p )
        {
            providerIdOut = provider_p->id;
            index = 0;
        }
    }
    mbLockRelease( pmcProvider_q->lock );

    providerList_p->ItemIndex = index;
    return providerIdOut;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void  pmcPickListFree
(
    TComboBox  *list_p
)
{
    list_p->Items->Clear();
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void  pmcPickListCfgBuild
(
    TComboBox  *list_p,
    Int32s_t    type
)
{
    pmcPickListCfg_p       pickList_p;

    mbLockAcquire( pmcPickListCfg_q->lock );
    list_p->Items->Clear();
 
    qWalk( pickList_p, pmcPickListCfg_q, pmcPickListCfg_p )
    {
        if( pickList_p->type == type )
        {
            nbDlgDebug(( "Adding item '%s' tp pick list ", pickList_p->string_p ));
            list_p->Items->Add( pickList_p->string_p );
        }
    }
    mbLockRelease( pmcPickListCfg_q->lock );
    list_p->ItemIndex = -1;
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void  pmcPickListBuildNew
(
    TComboBox          *list_p,
    Int32u_t            type
)
{
    mbStrList_p         item_p;
    Int32u_t            count = 0;

    pmcDBConfigInit( FALSE );

    list_p->Items->Clear();
    qWalk( item_p, pmcDBConfig_q, mbStrList_p )
    {
        if( item_p->handle == type )
        {
            list_p->Items->Add( item_p->str_p );
            count++;
        }
    }
    list_p->ItemIndex = 0;

    if( count > 20 )
    {
        list_p->DropDownCount = 20;
    }
    else
    {
        list_p->DropDownCount = count;
    }
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void  pmcPickListBuild
(
    TComboBox              *list_p,
    pmcPickListStruct_p     data_p,
    Int32s_t                code
)
{
    Int32s_t  i;
    Int32s_t  index = 0;

    list_p->Items->Clear();

    for( i = 0  ;  ; i++ )
    {
        if( data_p->code == PMC_PICKLIST_END_CODE ) break;

        nbDlgDebug(( "loc '%s' code %ld index %ld",
              data_p->description_p,
              data_p->code,
              data_p->index ));
         // Add description to the list box
        list_p->Items->Add( data_p->description_p );

        if( data_p->code == code ) index = i;
        data_p++;
    }
    list_p->DropDownCount = i;
    list_p->ItemIndex = index;
    return;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t        pmcPickListCodeGet
(
    pmcPickListStruct_p data_p,
    Int32s_t            index
)
{
    Int32s_t    code = -1;

    for( ; ; )
    {
        if( data_p->code == PMC_PICKLIST_END_CODE ) break;

        if( data_p->index == index )
        {
            code = data_p->code;
            break;
        }
        data_p++;
    }
    return code;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t        pmcPickListCodeVerify
(
    pmcPickListStruct_p     data_p,
    Int32s_t                code
)
{
    Int32u_t    returnCode = FALSE;

    for( ; ; )
    {
        if( data_p->code == PMC_PICKLIST_END_CODE ) break;

        if( data_p->code == code )
        {
            returnCode = TRUE;
            break;
        }
        data_p++;
    }

    return returnCode;
}


//---------------------------------------------------------------------------
// Function:  pmcLocationIndexGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t        pmcPickListIndexGet
(
    pmcPickListStruct_p     data_p,
    Int32s_t                code
)
{
    Int32s_t    index = -1;

    for( ; ; )
    {
        if( data_p->code == PMC_PICKLIST_END_CODE ) break;

        if(  data_p->code == code )
        {
            index = data_p->index;
            break;
        }
        data_p++;
    }
    return index;
}

//---------------------------------------------------------------------------
// Function:  pmcLocationcodeVerify
//---------------------------------------------------------------------------
// Description:
//
// Check to see if the specified location code is valid
//---------------------------------------------------------------------------

Int32s_t        pmcLocationCodeVerify
(
    Int32s_t                code
)
{
    Int32s_t                returnCode = FALSE;
    Int32s_t                i;

    for( i = 0 ; ; i++ )
    {
        if( pmcLocationCode[i].code == PMC_PICKLIST_END_CODE ) break;

        if( pmcLocationCode[i].code == code )
        {
            if( pmcLocationCode[i].valid == TRUE ) returnCode = TRUE;
            break;
        }
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcLocationCodeGet
//---------------------------------------------------------------------------
// Description:
//
// Check to see if the specified location code is valid
//---------------------------------------------------------------------------

Int32s_t        pmcLocationCodeGet
(
    Int32s_t                locationIndex,
    Int32s_t                premiumIndex
)
{
    Int32s_t                i, returnCode = 0;

    for( i = 0 ; ; i++ )
    {
        if( pmcLocationCode[i].code == PMC_PICKLIST_END_CODE ) break;

        if(    pmcLocationCode[i].locationIndex == locationIndex
            && pmcLocationCode[i].premiumIndex  == premiumIndex )
        {
            returnCode = pmcLocationCode[i].code;
            break;
        }
    }
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:  pmcLocationIndexesGet
//---------------------------------------------------------------------------
// Description:
//
// Check to see if the specified location code is valid
//---------------------------------------------------------------------------

Int32s_t        pmcLocationIndexesGet
(
    Int32s_t                code,
    Int32s_p                locationIndex_p,
    Int32s_p                premiumIndex_p
)
{
    Int32s_t                i, returnCode = FALSE;
    Int32s_t                locationIndex = 0;
    Int32s_t                premiumIndex = 0;
    for( i = 0 ; ; i++ )
    {
        if( pmcLocationCode[i].code == PMC_PICKLIST_END_CODE ) break;

        if( pmcLocationCode[i].code == code )
        {
            locationIndex = pmcLocationCode[i].locationIndex;
            premiumIndex  = pmcLocationCode[i].premiumIndex;

            if( pmcLocationCode[i].valid == TRUE ) returnCode = TRUE;
            break;
        }
    }
    *locationIndex_p = locationIndex;
    *premiumIndex_p  = premiumIndex;
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcDrTypeIndexGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t        pmcDrTypeIndexGet
(
    Char_p      province_p,
    Int32u_t    cancerClinic
)
{
    Int32s_t    i;
    Int32s_t    index = -1;

    if( cancerClinic == TRUE )
    {
        for( i = 0  ;  ; i++ )
        {
            if( pmcDrType[i].code == PMC_PICKLIST_END_CODE ) break;
            if( strcmp( pmcDrType[i].description_p, PMC_CANCER_CLINIC_STRING ) == 0 )
            {
                index = pmcDrType[i].index;
                goto exit;
            }
        }
    }

    if( strcmp( province_p, PMC_DEFAULT_PROVINCE ) == 0  || strlen( province_p ) == 0 )
    {
        index = 0;
        goto exit;
    }

    for( i = 0  ;  ; i++ )
    {
        if( pmcDrType[i].code == PMC_PICKLIST_END_CODE ) break;
        if( strcmp( pmcDrType[i].description_p, province_p ) == 0 )
        {
            index = pmcDrType[i].index;
            goto exit;
        }
    }

    // If we made it to this point, assume "All others"
    for( i = 0  ;  ; i++ )
    {
        if( pmcDrType[i].code == PMC_PICKLIST_END_CODE ) break;
        if( strcmp( pmcDrType[i].description_p, PMC_ALL_OTHERS_STRING ) == 0 )
        {
            index = pmcDrType[i].index;
            goto exit;
        }
    }

exit:
    return index;
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#if 0
void  pmcDrTypeListBuild
(
    TComboBox  *list_p
)
{
    Ints_t  i;
    Ints_t  index = 0;

    for( i = 0  ;  ; i++ )
    {
        if( pmcDrType[i].code == PMC_PICKLIST_END_CODE ) break;

         // Add description to the list box
        list_p->Items->Add( pmcDrType[i].description_p );

        pmcDrType[i].index = i;
    }
    list_p->ItemIndex = index;
    return;
}
#endif

//---------------------------------------------------------------------------
// Function: pmcMakeMailingAddress
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

void pmcMakeMailingAddress
(
    Char_p  title_p,
    Char_p  firstName_p,
    Char_p  lastName_p,
    Char_p  address1_p,
    Char_p  address2_p,
    Char_p  city_p,
    Char_p  province_p,
    Char_p  postalCode_p,
    Char_p  nameOut_p,
    Char_p  address1Out_p,
    Char_p  address2Out_p,
    Char_p  cityProvCodeOut_p,
    bool    allCapsFlag
)
{
    Char_p  buf1_p;
    Char_p  buf2_p;
    Char_p  buf3_p;

    mbMalloc( buf1_p, 1024 );
    mbMalloc( buf2_p, 1024 );
    mbMalloc( buf3_p, 1024 );

    mbStrClean( title_p, buf1_p, TRUE );
    if( allCapsFlag ) mbStrToUpper( buf1_p );
    mbStrClean( firstName_p, buf2_p, TRUE );
    if( allCapsFlag ) mbStrToUpper( buf2_p );
    mbStrClean( lastName_p, buf3_p, TRUE );
    if( allCapsFlag ) mbStrToUpper( buf3_p );

    if( strlen( buf1_p ) > 0 )
    {
        sprintf( nameOut_p, "%s %s %s", buf1_p, buf2_p, buf3_p );
    }
    else
    {
        sprintf( nameOut_p, "%s %s", buf2_p, buf3_p );
    }

    pmcCleanMailName( nameOut_p, allCapsFlag );

    mbStrClean( address1_p, buf1_p, TRUE );
    if( allCapsFlag ) mbStrToUpper( buf1_p );
    sprintf( address1Out_p, "%s", buf1_p );
    pmcCleanMailName( address1Out_p, allCapsFlag );

    mbStrClean( address2_p, buf1_p, TRUE );
    if( allCapsFlag ) mbStrToUpper( buf1_p );
    sprintf( address2Out_p, "%s", buf1_p );
    pmcCleanMailName( address2Out_p, allCapsFlag );

    mbStrClean( city_p, buf1_p, TRUE );
    if( allCapsFlag ) mbStrToUpper( buf1_p );
    mbStrClean( province_p, buf2_p, TRUE );
    if( allCapsFlag ) mbStrToUpper( buf2_p );

    strcpy( buf3_p, postalCode_p );
    if( pmcPostalCodeFix( buf3_p, NIL ) != TRUE )
    {
        mbDlgExclaim( "Invalid postal code detected '%s'", buf3_p );
    }

    sprintf( cityProvCodeOut_p, "%s %s  %s", buf1_p, buf2_p, buf3_p );
    pmcCleanMailName( cityProvCodeOut_p, allCapsFlag );

    if( strlen( address2Out_p ) == 0 )
    {
        strcpy( address2Out_p, cityProvCodeOut_p );
        *cityProvCodeOut_p = (Char_t)NULL;
    }

    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( buf3_p );

    return;
}

//---------------------------------------------------------------------------
// Function: pmcCleanMailName
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p  pmcCleanMailName( Char_p buf_p, bool allCapsFlag )
{
    Char_p      p1, p2;
    Char_p      buf2_p;

    if( buf_p == NULL ) goto exit;

    p1 = p2 = buf_p;

    for( ; ; )
    {
        if( *p1 == NULL ) break;
        if( *p1 >= 'A' && *p1 <= 'Z' )
        {
            *p2++ = *p1;
        }
        else if( *p1 >= 'a' && *p1 <= 'z' )
        {
            if( !allCapsFlag ) *p2++ = *p1;
        }
        else if( *p1 >= '0' && *p1 <= '9' )
        {
            *p2++ = *p1;
        }
        else if( *p1 == '-' || *p1 == ' ' || *p1 == 0x27 )
        {
             *p2++ = *p1;
        }

        p1++;
    }
    *p2 = (Char_t)NULL;

    // Must clean the string again in case leading or trailing characters were stripped
    mbMalloc( buf2_p, strlen( buf_p ) + 1 );
    strcpy( buf2_p, buf_p );
    mbStrClean( buf2_p, buf_p, TRUE );
    mbFree( buf2_p );
exit:
    return buf_p;
}

//---------------------------------------------------------------------------
// Function: pmcPostalCodeFix
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Ints_t         pmcPostalCodeFix
(
    Char_p      in_p,
    bool       *modifiedFlag_p
)
{
    Char_p      p1;
    Char_t      buf[32];
    Char_p      orig_p;
    Ints_t      returnCode = FALSE;
    Int32u_t    letterCount = 0;
    Int32u_t    numberCount = 0;

    mbMalloc( orig_p, 32 );

    strcpy( buf,  in_p );
    strcpy( orig_p, in_p );

    // Next make alphanumeric.  This gets rid of all white space
    mbStrAlphaNumericOnly( buf );
    mbStrToUpper( buf );

    // Next count numbers and letters to see if if is a postal code.
    // It could be a zip code.  If it looks like a postal code attempt
    // to format it as one, and generate return code indicating success
    for( p1 = buf ; *p1 != (Char_t)NULL  ; p1++ )
    {
        if( *p1 >= 'A' && *p1 <= 'Z' )
        {
            letterCount++;
        }
        else if( *p1 >= '0' && *p1 <= '9' )
        {
            numberCount++;
        }
    }

    // It looks like people are entering OH instead of ZERO in the postal
    // codes.  There is a special case where there could be three OHS and
    // therefore no numbers detected in the postal code

    if( ( numberCount && letterCount ) || ( numberCount == 0 && letterCount == 6 ))
    {
        // Convert ohs to zeros
        if( buf[1] == 0x4F ) buf[1] = 0x30;
        if( buf[3] == 0x4F ) buf[3] = 0x30;
        if( buf[5] == 0x4F ) buf[5] = 0x30;

        // Looks like this was an attempt at a postal code.
        if( buf[0] >= 'A' && buf[0] <= 'Z' &&
            buf[2] >= 'A' && buf[2] <= 'Z' &&
            buf[4] >= 'A' && buf[4] <= 'Z' &&
            buf[1] >= '0' && buf[1] <= '9' &&
            buf[3] >= '0' && buf[3] <= '9' &&
            buf[5] >= '0' && buf[5] <= '9' )
        {
            // Looks OK,  put one space in
            buf[7] = 0;
            buf[6] = buf[5];
            buf[5] = buf[4];
            buf[4] = buf[3];
            buf[3] = 0x20;
            returnCode = TRUE;
        }
        else
        {
            // Not sure whats going on
        }
    }
    else if( numberCount == 5 )
    {
        // Assume this is a US zip code.
        returnCode = TRUE;
    }
    else if( letterCount )
    {
        // Letters only... not sure what this is
    }
    else
    {
        // This must be an empty string.
        returnCode = TRUE;
    }

    // Whatever we've got, copy it to the original string
    strcpy( in_p, buf );
    if( modifiedFlag_p )
    {
        if( strcmp( in_p, orig_p ) == 0 )
        {
            // We did not change the
            *modifiedFlag_p = FALSE;
        }
        else
        {
            *modifiedFlag_p = TRUE;
        }
    }

    mbFree( orig_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcProvinceFix
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Ints_t         pmcProvinceFix
(
    Char_p      in_p,
    bool       *modifiedFlag_p
)
{
    Char_p      p1;
    Char_t      buf[32];
    Char_p      orig_p;
    Ints_t      returnCode = FALSE;
    Int32u_t    letterCount = 0;
    Int32u_t    numberCount = 0;

    mbMalloc( orig_p, 32 );

    strcpy( buf,  in_p );
    strcpy( orig_p, in_p );

    // Next make alphanumeric.  This gets rid of all white space
    mbStrAlphaNumericOnly( buf );
    mbStrToUpper( buf );

    // Next count numbers and letters to see if if is a postal code.
    // It could be a zip code.  If it looks like a postal code attempt
    // to format it as one, and generate return code indicating success
    for( p1 = buf ; *p1 != (Char_t)NULL  ; p1++ )
    {
        if( *p1 >= 'A' && *p1 <= 'Z' )
        {
            letterCount++;
        }
        else if( *p1 >= '0' && *p1 <= '9' )
        {
            numberCount++;
        }
    }

    if( letterCount == 2 && numberCount == 0 )
    {
        returnCode = TRUE;
    }

    // Whatever we've got, copy it to the original string
    strcpy( in_p, buf );
    if( modifiedFlag_p )
    {
        if( strcmp( in_p, orig_p ) == 0 )
        {
            // We did not change the
            *modifiedFlag_p = FALSE;
        }
        else
        {
            *modifiedFlag_p = TRUE;
        }
    }

    mbFree( orig_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcFixCapitalization
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Char_p          pmcFixCapitalization
(
    Char_p      in_p
)
{
    Ints_t      len;
    Char_p      temp_p;
    Ints_t      i;
    Int32u_t    bigCount = 0;
    Int32u_t    smallCount = 0;
    Int32u_t    spaceCount =0;
    Int32u_t    hyphenCount = 0;

    len = strlen( in_p );

    if( len <= 1 )  goto exit;

    for( temp_p = in_p, i = 0 ; i < len ; i++, temp_p++ )
    {
        if( *temp_p >= 'a' && *temp_p <= 'z' ) smallCount++;
        if( *temp_p >= 'A' && *temp_p <= 'Z' ) smallCount++;
        if( *temp_p == ' ' ) spaceCount++;
        if( *temp_p == '-' ) hyphenCount++;
    }

    if( bigCount == 0 )
    {
        // looks like there was no capitalization
        if( spaceCount == 0 )
        {
            temp_p = in_p;
            if( *temp_p >= 'a' && *temp_p <= 'z' ) *temp_p -= (Char_t)0x20;

            for( temp_p = in_p + 1, i = 1 ; i < len ; i++, temp_p++ )
            {
                if( *(temp_p - 1 ) == '\'' || *(temp_p - 1 ) == '-' )
                {
                    if( *temp_p >= 'a' && *temp_p <= 'z' ) *temp_p -= (Char_t)0x20;
                }
            }
        }
        else
        {
            // There were spaces in the name.  This could be some kind of
            // funky name.  Do nothing for now
        }
    }

#if 0
    if( mbStrPos( in_p, "Mac" ) == 0 && strlen( in_p ) <= 4  ) goto exit;

    if( mbStrPos( in_p, "Mc" ) == 0 )
    {
        if( *(in_p+2) >= 'a' && *(in_p+2) <= 'z' ) *(in_p+2) -= (Char_t)0x20;
    }

    if( mbStrPos( in_p, "Mac" ) == 0  && strlen( in_p ) )
    {
        if( *(in_p+3) >= 'a' && *(in_p+3) <= 'z' ) *(in_p+3) -= (Char_t)0x20;
    }

    if( ( pos = mbStrPos( in_p, "Mc" ) )  > 0  )
    {
        if( strlen( in_p+pos ) > 2 )
        {
            if( *(in_p+pos+2) >= 'a' && *(in_p+pos+2) <= 'z' ) *(in_p+pos+2) -= (Char_t)0x20;
        }
    }

    if( ( pos = mbStrPos( in_p, "Mac" ) ) > 0  )
    {
        if( strlen( in_p+pos ) > 2 )
        {
            if( *(in_p+pos+3) >= 'a' && *(in_p+pos+3) <= 'z' ) *(in_p+pos+3) -= (Char_t)0x20;
        }
    }
#endif

#if 0
    else
    {
        // There are already some capital letters
    }

    for( i = 0 ; i <= len ; i++ )
    {
        if( i > 0 )
        {
            if( buf2[i] >= 'A' && buf2[i] <= 'Z' ) buf2[i] += 0x20;
        }
    }

    if( mbStrPos( buf2, "Mack" ) == 0 && strlen( buf2 ) == 4 ) goto done;

    if( mbStrPos( buf2, "Mach" ) == 0 && strlen( buf2 ) == 4 ) goto done;

    if( mbStrPos( buf2, "Mc" ) == 0 )
    {
        if( buf2[2] >= 'a' && buf2[2] <= 'z' ) buf2[2] -= 0x20;
    }

    if( mbStrPos( buf2, "Mac" ) == 0 )
    {
        if( buf2[3] >= 'a' && buf2[3] <= 'z' ) buf2[3] -= 0x20;
    }

    len = strlen( buf2 );

    for( i = 0 ; i < len - 1 ; i++ )
    {
        if( buf2[i] == '-' || buf2[i] == ' ' || buf2[i] == '\'' )
        {
            buf2[i+1] -= 0x20;
        }
    }

    i = mbStrPos( buf2, " Der " );

    if( i >= 0 ) buf2[i+1] += 0x20;

    // printf(  "Replace '%s' with '%s'\n", buf1, buf2 );
done:

    strcpy( in_p, buf2 );
#endif

exit:
    return in_p;
}

//---------------------------------------------------------------------------
// Given an appointment, return the associated patient id
//---------------------------------------------------------------------------
Int32u_t        pmcAppointPatientId
(
    Int32u_t    appointId
)
{
    MbString    cmd = MbString();

    mbSprintf( &cmd, "select %s from %s where %s=%ld",
             PMC_SQL_FIELD_PATIENT_ID,
             PMC_SQL_TABLE_APPS,
             PMC_SQL_FIELD_ID, appointId );

    return pmcSqlSelectInt( cmd.get(), NIL );
}

//---------------------------------------------------------------------------
// Function: pmcAppointDoctorId
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcAppointDoctorId
(
    Int32u_t    appointId
)
{
    Char_p      cmd_p;
    Int32u_t    doctorId;

    mbMalloc( cmd_p, 256 );

    // Must read patient Id from database to be safe
    sprintf( cmd_p, "select %s from %s where %s=%ld and %s=%d",
             PMC_SQL_APPS_FIELD_REFERRING_DR_ID,
             PMC_SQL_TABLE_APPS,
             PMC_SQL_FIELD_ID,
             appointId,
             PMC_SQL_FIELD_NOT_DELETED,
             PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    doctorId = pmcSqlSelectInt( cmd_p, NIL );

    mbFree( cmd_p );
    return doctorId;
}

/******************************************************************************
 * Procedure: pmcPhnVerifyString( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

#define PMC_PHN_LEN 9

Int32s_t        pmcPhnVerifyString
(
    Char_p      phn_p
)
{
    Int32s_t    resultCode = FALSE;
    Int32u_t    digit, sum, remainder, result, check;
    Int32u_t    i;
    Char_p      temp_p;

    if( strlen( phn_p ) != PMC_PHN_LEN ) goto exit;

    sum = 0;
    for( temp_p = phn_p, i = 0 ; i < PMC_PHN_LEN - 1 ; i++, temp_p++ )
    {
        if( *temp_p < '0' || *temp_p > '9' ) goto exit;
        digit = (Int32u_t)*temp_p - 0x30;
        digit *= ( PMC_PHN_LEN - i );
        sum += digit;
    }
    check = *temp_p - 0x30;
    result = sum/11;
    remainder = sum - ( result * 11 );

    if( remainder ) remainder = 11 - remainder;

    if( remainder == check ) resultCode = TRUE;

exit:
   return resultCode;
}

/******************************************************************************
 * Procedure: pmcIcdVerify( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t            pmcIcdVerify
(
    Char_p          code_p,
    Char_p          desc_p,
    bool            overwrightFlag
)
{
    Char_p          temp_p;
    Char_p          codeClean_p;
    Int32s_t        returnCode = FALSE;
    pmcIcdStruct_p  icd_p;
    Ints_t          len, i;

    mbMalloc( temp_p, 256 );
    mbMalloc( codeClean_p, 32 );

    sprintf( temp_p, "Invalid" );

    mbStrClean( code_p, codeClean_p, TRUE );

    len = strlen( codeClean_p );
    if( len == 0 )
    {
        sprintf( temp_p, "" );
        goto exit;
    }

    if( len != 3 ) goto exit;

    // Fix up the text a little
    for( i = 0 ; i < len ; i++ )
    {
        if( !pmcIcdCaseSensitive ) mbStrToUpper( codeClean_p );

        if( !pmcIcdContainsOhs )
        {
            // If there are no "ohs" in the codes then we can convert ohs to zeros
            if( *(codeClean_p+i) == 0x4F ) *(codeClean_p+i) = 0x30;
        }
    }

    mbLockAcquire( pmcIcd_q->lock );

    qWalk( icd_p, pmcIcd_q, pmcIcdStruct_p )
    {
        if( mbStrPos( icd_p->code, codeClean_p ) == 0 )
        {
            // Found a match

            strncpy( temp_p, icd_p->description_p, 255 );
            *(temp_p + 255 ) = 0;
            returnCode = TRUE;
            break;
        }
    }

    mbLockRelease( pmcIcd_q->lock );

exit:

    if( desc_p ) strcpy( desc_p, temp_p );

    if( returnCode == TRUE && overwrightFlag == TRUE )
    {
        strcpy( code_p, codeClean_p );
    }

    mbFree( temp_p );
    mbFree( codeClean_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcFeeCodeFormatDisplay
//---------------------------------------------------------------------------
// Description:
//
// This function removes leading 0s from the fee code
//---------------------------------------------------------------------------

Char_p      pmcFeeCodeFormatDisplay
(
    Char_p      in_p,
    Char_p      out_p
)
{
    Char_p      temp1_p;
    Ints_t      len, i;
    Char_t      buf[32];

    if( out_p == NIL ) goto exit;

    *out_p = 0;

    if( in_p == 0 )  goto exit;

    len = strlen( in_p );

    if( len == 0 ) goto exit;

    mbStrClean( in_p, buf, TRUE );
    temp1_p = &buf[0];

    len = strlen( buf );

    // Strip leading zeros
    for( i = 0 ; i < len ; i++ )
    {
        if( *temp1_p == '0' )
        {
            temp1_p++;
        }
        else
        {
            break;
        }
    }
    if( strlen( temp1_p ) < 1 )
    {
        mbDlgDebug(( "Error cleaning fee code '%s'\n", in_p ));
    }
    else
    {
        strcpy( out_p, temp1_p );
    }
exit:
    return out_p;
}

/******************************************************************************
 * Procedure: pmcFeeCodeFormatDatabase( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t            pmcFeeCodeFormatDatabase
(
    Char_p          code_p,
    Char_p          out_p
)
{
    Char_t          buf[32];
    Char_t          tempCode[8];
    Ints_t          len;
    Int32s_t        returnCode = FALSE;
    Int32s_t        i;

    // Initialize output with nothing
    if( out_p ) sprintf( out_p, "" );

    mbStrClean( code_p, buf, TRUE );
    len = strlen( buf );
    if( len == 0 )
    {
        returnCode = TRUE;
        goto exit;
    }

    if( len > 4 || len == 0 ) goto exit;

    // Add leading zeros
    if( len == 1 ) sprintf( tempCode, "000%s", buf );
    if( len == 2 ) sprintf( tempCode, "00%s",  buf );
    if( len == 3 ) sprintf( tempCode, "0%s",   buf );
    if( len == 4 ) sprintf( tempCode, "%s",    buf );

    if( !pmcFeeCaseSensitive ) mbStrToUpper( tempCode );

    if( !pmcFeeContainsOhs )
    // Fix up the text a little
    {
        for( i = 0 ; i < len ; i++ )
        {
            // If there are no "ohs" in the codes then we can convert ohs to zeros
            if( tempCode[i] == 0x4F ) tempCode[i] = 0x30;
        }
    }

    if( out_p ) sprintf( out_p, "%s", tempCode );
exit:
    return returnCode;
}

/******************************************************************************
 * Procedure: pmcFeeCodeVerify( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t            pmcFeeCodeVerify
(
    Char_p          code_p,
    Int32u_t        dateOfService,
    Int32u_p        feeHigh_p,
    Int32u_p        feeLow_p,
    Int32u_p        multiple_p,
    Int32u_p        determinant_p,
    Int32u_p        index_p,
    Int32u_t        claimType
)
{
    Int32s_t        returnCode = FALSE;
    pmcFeeStruct_p  fee_p;
    Ints_t          len, i;
    Char_t          tempCode[8];
    Char_t          buf[32];
    Int32u_t        tempFeeHigh = 0;
    Int32u_t        tempFeeLow = 0;
    Int32u_t        number;
    Int32u_t        index = 0;
    bool            hospitalCode = FALSE;
    bool            multiple = FALSE;
    Int32u_t        determinant;

    mbStrClean( code_p, buf, TRUE );
    len = strlen( buf );
    if( len == 0 )
    {
        returnCode = TRUE;
        goto exit;
    }

    if( len > 4 || len == 0) goto exit;

    // Add leading zeros
    if( len == 1 ) sprintf( tempCode, "000%s", buf );
    if( len == 2 ) sprintf( tempCode, "00%s",  buf );
    if( len == 3 ) sprintf( tempCode, "0%s",   buf );
    if( len == 4 ) sprintf( tempCode, "%s",    buf );

    // Fix up the text a little
    if( !pmcFeeCaseSensitive ) mbStrToUpper( tempCode );

    if( !pmcFeeContainsOhs )
    {
        for( i = 0 ; i < len ; i++ )
        {
            // If there are no "ohs" in the codes then we can convert ohs to zeros
            if( tempCode[i] == 0x4F ) tempCode[i] = 0x30;
        }
    }

    mbLockAcquire( pmcFee_q->lock );

    qWalk( fee_p, pmcFee_q, pmcFeeStruct_p )
    {
        if( mbStrPosLen( fee_p->code, tempCode ) == 0 )
        {
            // Found a match
            returnCode = TRUE;
            tempFeeHigh = fee_p->feeHigh;
            tempFeeLow  = fee_p->feeLow;
            multiple    = fee_p->multiple;
            determinant = (Int32u_t)fee_p->determinant;
            index       = fee_p->index;

            if( dateOfService > 0 && dateOfService < pmcCfg[CFG_CUTOVER_DATE].value )
            {
                if( fee_p->oldFeeHigh == 0 )
                {
                    tempFeeHigh = 0;
                    tempFeeLow = 0;
                    // returnCode = FALSE;
                }
                else
                {
                    // Just return the "old" fee codes
                    tempFeeHigh = fee_p->oldFeeHigh;
                    tempFeeLow  = fee_p->oldFeeLow;
               }
            }
            break;
        }
    }

    if( returnCode )
    {
        // Attempt to determine if this is a "service" code or "hospital" code
        // 25B-25T, 26B-26T, 27B-27T, 28B-28T, 31B, 32B, 31C, 32C
        if( ( mbStrPos( tempCode, "31B" ) >= 0 ) ||
            ( mbStrPos( tempCode, "32B" ) >= 0 ) ||
            ( mbStrPos( tempCode, "31C" ) >= 0 ) ||
            ( mbStrPos( tempCode, "32C" ) >= 0 ) )
        {
            hospitalCode = TRUE;
        }
        else
        {
            strcpy( buf, tempCode );
            mbStrDigitsOnly( buf );
            number = atol( buf );
            if( number >= 25 && number <= 28 )
            {
                if( tempCode[3] >= 'B' && tempCode[3] <= 'T' )
                {
                    hospitalCode = TRUE;
                }
            }
        }

        if( hospitalCode == TRUE )
        {
            if( claimType != PMC_CLAIM_TYPE_HOSPITAL &&
                claimType != PMC_CLAIM_TYPE_ANY )
            {
                tempFeeHigh =  0;
                tempFeeLow = 0;
                returnCode = FALSE;
            }
        }
    }
    mbLockRelease( pmcFee_q->lock );

exit:

    if( feeHigh_p )    *feeHigh_p     = tempFeeHigh;
    if( feeLow_p  )    *feeLow_p      = tempFeeLow;
    if( multiple_p )   *multiple_p    = multiple;
    if( determinant_p) *determinant_p = determinant;
    if( index_p )      *index_p       = index;

    return returnCode;
}



//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcFileExtractSaskPhn
(
    mbFileListStruct_p      file_p,
    Char_p                  phn_p
)
{
    Int32s_t    resultCode = FALSE;
    switch( file_p->type )
    {
        case PMC_DOCUMENT_TYPE_WORD:
        case PMC_DOCUMENT_TYPE_TXT:
            resultCode = pmcFileExtractSaskPhnWord( file_p->fullName_p, phn_p );
            if( resultCode == FALSE )
            {
                resultCode = pmcFileExtractSaskShspWord( file_p->fullName_p, phn_p );
            }

            break;

        default:
            break;
    }
    return resultCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcFileExtractSaskPhnWord
(
    Char_p  fileName_p,
    Char_p  phn_p
)
{
    FILE       *fp;
    Int8u_p     buf_p;
    Ints_t      bufSize = 0;
    Ints_t      index = 0;
    Int8u_t     c;
    Int8u_t     phn[16];
    Ints_t      state = PMC_EXTRACT_PHN_STATE_WAIT_1;
    Ints_t      digitCount;
    bool        found = FALSE;
    Int32s_t    returnCode = FALSE;

    mbMalloc( buf_p, 256 );

    fp = fopen( fileName_p, "rb" );

    if( fp == NIL ) goto exit;

    for( ; ; )
    {
        if( found == TRUE ) break;

        if( index == bufSize )
        {
            index = 0;
            if( ( bufSize = fread( (Void_p)buf_p, 1, 256, fp ) ) == 0 ) break;
        }

        c = *(buf_p + index++);

        // Ignore the following characters
        if( c == 0              ||
            c == MB_CHAR_DASH  ||
            c == MB_CHAR_SPACE ||
            c == MB_CHAR_TAB   ||
            c == MB_CHAR_PERIOD ) continue;

         // Reset to start of search string if a P found
        if( state != PMC_EXTRACT_PHN_STATE_WAIT_DIGIT )
        {
            if( c == 'p' || c == 'P' ) state = PMC_EXTRACT_PHN_STATE_WAIT_1;
        }

        switch( state )
        {
            case PMC_EXTRACT_PHN_STATE_WAIT_1:
                if( c == 'p' || c == 'P' )
                {
                    state =  PMC_EXTRACT_PHN_STATE_WAIT_2;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_2:
                if( c == 'h' || c == 'H' )
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_3 ;
                }
                else
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_3:
                if( c == 'n' || c == 'N' )
                {
                     state = PMC_EXTRACT_PHN_STATE_WAIT_COLON;
                }
                else
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_COLON:
                digitCount = 0;
                if( c == MB_CHAR_COLON )
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_DIGIT;
                }
                else
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_DIGIT:
            case PMC_EXTRACT_PHN_STATE_IN_DIGITS:
                // MAB: 20011209: Really loosen up search to look for
                // first nine digits after PHN is found in the
                // document.
                if( c >= '0' && c <= '9' )
                {
                    if( state == PMC_EXTRACT_PHN_STATE_WAIT_DIGIT ) state = PMC_EXTRACT_PHN_STATE_IN_DIGITS;
                    phn[digitCount++] = c;
                    if( digitCount == 9 )
                    {
                        phn[digitCount] = 0;
                        state = PMC_EXTRACT_PHN_STATE_WAIT_DONE;
                        break;
                    }
                }
                else
                {
                    if( state == PMC_EXTRACT_PHN_STATE_IN_DIGITS )
                    {
                        state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                    }
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_DONE:
                if( c < '0' || c > '9' )
                {
                    found = TRUE;
                    break;
                }
                break;

            default:
                mbDlgDebug(( "Error" ));
                break;
        }
    }

    if( found )
    {
        returnCode = TRUE;
        mbLog( "Found PHN '%s' in file '%s'\n", phn, fileName_p );

        if( phn_p ) strcpy( phn_p, (Char_p)&phn[0] );
    }

exit:

    if( fp ) fclose( fp );
    mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t pmcFileExtractSaskShspWord
(
    Char_p  fileName_p,
    Char_p  phn_p
)
{
    FILE       *fp;
    Int8u_p     buf_p;
    Ints_t      bufSize = 0;
    Ints_t      index = 0;
    Int8u_t     c;
    Int8u_t     phn[16];
    Ints_t      state = PMC_EXTRACT_PHN_STATE_WAIT_1;
    Ints_t      digitCount;
    bool        found = FALSE;
    Int32s_t    returnCode = FALSE;

    mbMalloc( buf_p, 256 );

    fp = fopen( fileName_p, "rb" );

    if( fp == NIL ) goto exit;

    for( ; ; )
    {
        if( found == TRUE ) break;

        if( index == bufSize )
        {
            index = 0;
            if( ( bufSize = fread( (Void_p)buf_p, 1, 256, fp ) ) == 0 ) break;
        }

        c = *(buf_p + index++);

        // Ignore the following characters
        if( c == 0              ||
            c == MB_CHAR_DASH  ||
            c == MB_CHAR_SPACE ||
            c == MB_CHAR_TAB   ||
            c == MB_CHAR_PERIOD ) continue;

         // Reset to start of search string if an S found.
         // This will not work for the string SHSHSP
        if( state != PMC_EXTRACT_PHN_STATE_WAIT_DIGIT && state != PMC_EXTRACT_PHN_STATE_WAIT_3 )
        {
            if( c == 's' || c == 'S' ) state = PMC_EXTRACT_PHN_STATE_WAIT_1;
        }

        switch( state )
        {
            case PMC_EXTRACT_PHN_STATE_WAIT_1:
                if( c == 's' || c == 'S' )
                {
                    state =  PMC_EXTRACT_PHN_STATE_WAIT_2;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_2:
                if( c == 'h' || c == 'H' )
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_3;
                }
                else
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_3:
                if( c == 's' || c == 'S' )
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_4;
                }
                else
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_4:
                if( c == 'p' || c == 'P' )
                {
                     state = PMC_EXTRACT_PHN_STATE_WAIT_COLON;
                }
                else
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_COLON:
                digitCount = 0;
                if( c == MB_CHAR_COLON )
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_DIGIT;
                }
                else
                {
                    state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_DIGIT:
            case PMC_EXTRACT_PHN_STATE_IN_DIGITS:
                // MAB: 20011209: Really loosen up search to look for
                // first nine digits after PHN is found in the
                // document.
                if( c >= '0' && c <= '9' )
                {
                    if( state == PMC_EXTRACT_PHN_STATE_WAIT_DIGIT ) state = PMC_EXTRACT_PHN_STATE_IN_DIGITS;
                    phn[digitCount++] = c;
                    if( digitCount == 9 )
                    {
                        phn[digitCount] = 0;
                        state = PMC_EXTRACT_PHN_STATE_WAIT_DONE;
                        break;
                    }
                }
                else
                {
                    if( state == PMC_EXTRACT_PHN_STATE_IN_DIGITS )
                    {
                        state = PMC_EXTRACT_PHN_STATE_WAIT_1;
                    }
                }
                break;

            case PMC_EXTRACT_PHN_STATE_WAIT_DONE:
                if( c < '0' || c > '9' )
                {
                    found = TRUE;
                    break;
                }
                break;

            default:
                mbDlgDebug(( "Error" ));
                break;
        }
    }

    if( found )
    {
        returnCode = TRUE;
        mbLog( "Found PHN '%s' in file '%s'\n", phn, fileName_p );

        if( phn_p ) strcpy( phn_p, (Char_p)&phn[0] );
    }

exit:

    if( fp ) fclose( fp );
    mbFree( buf_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t        pmcFileStringSearch
(
    Char_p      fileName_p,
    Char_p      stringIn_p,
    Int32u_t    caseMode,
    Int32u_t    skipWhiteSpace,
    Int32u_p    offset_p
)
{
    FILE       *fp;
    Int8u_p     buf_p;
    Int8u_p     string_p;
    Ints_t      bufSize = 0;
    Ints_t      index = 0;
    Int8u_t     c;
    bool        found = FALSE;
    Int32s_t    returnCode = FALSE;
    Int8u_p     check_p;
    Int32s_t    checkCount;
    Int32s_t    len;
    Int32u_t    offset = 0;
    Int32u_t    totalCount = 0;

    mbMalloc( buf_p, 4096 );

    len = strlen( stringIn_p );
    mbMalloc( string_p, len + 1 );
    strcpy( (Char_p)string_p, stringIn_p );

    if( skipWhiteSpace )
    {
        // Must string white space and recompute match length if skipping
        // white space in the search
        Int8u_p  temp1_p = string_p;
        Int8u_p  temp2_p = string_p;

        for( int i = 0 ; i < len ; i++ )
        {
            c = *temp1_p++;

            if( c == MB_CHAR_CR    ||
                c == MB_CHAR_LF    ||
                c == MB_CHAR_SPACE ||
                c == MB_CHAR_TAB   ||
                c == 0  ) continue;

            *temp2_p++ = c;
        }
        *temp2_p = 0;
        len = strlen( (Char_p)string_p );
    }

    if( caseMode == PMC_IGNORE_CASE ) mbStrToUpper( (Char_p)string_p );

    fp = fopen( fileName_p, "rb" );

    if( fp == NIL ) goto exit;

    check_p = string_p;
    checkCount = 0;

    for( ; ; )
    {
        if( index == bufSize )
        {
            index = 0;
            if( ( bufSize = fread( (Void_p)buf_p, 1, 4096, fp ) ) == 0 ) break;
        }

        c = *(buf_p + index++);
        totalCount++;

        if( skipWhiteSpace )
        {
            if( c == MB_CHAR_CR    ||
                c == MB_CHAR_LF    ||
                c == MB_CHAR_SPACE ||
                c == MB_CHAR_TAB   ||
                c == 0  ) continue;
        }

        // Convert to upper case if ignore case
        if( caseMode == PMC_IGNORE_CASE )
        {
            if( c >= (Int8u_t)0x61 && c <= (Int8u_t)0x7A ) c -= (Int8u_t)0x20;
        }

        if( c == *check_p )
        {
            // 20021230: Check for starting position of potential match
            if( check_p == string_p ) offset = ( totalCount - 1 );

            checkCount++;
            check_p++;
            if( checkCount == len )
            {
                found = TRUE;
                break;
            }

        }
        else
        {
            check_p = string_p;
            checkCount = 0;
        }
    }

    if( found )
    {
        returnCode = TRUE;
        //mbLog( "Found string '%s' in file '%s'\n", string_p, fileName_p );
        if( offset_p ) *offset_p = offset;
    }

exit:

    if( fp ) fclose( fp );
    mbFree( buf_p );
    mbFree( string_p );
    return returnCode;
}

/******************************************************************************
 * Procedure: pmcDirDriveAndPathGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t        pmcDirDriveAndPathGet
(
    Char_p      dirIn_p,
    Char_p      drive_p,
    Char_p      path_p

)
{
    Int32s_t    returnCode;

    returnCode = FALSE;

    if( dirIn_p == NIL ) goto exit;

    if( *(dirIn_p + 1) != MB_CHAR_COLON ) goto exit;

    if( drive_p ) *drive_p = *dirIn_p;

    if( path_p ) strcpy( path_p, dirIn_p + 2 );

    returnCode = TRUE;

exit:
    if( returnCode == FALSE )
    {
        if( drive_p ) *drive_p = 0;
        if( path_p )  *path_p = 0;
    }
    return returnCode;
}

/******************************************************************************
 * Procedure: pmcFileGetPathAndName( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t        pmcFilePathAndNameGet
(
    Char_p      fullName_p,
    Char_p      path_p,
    Char_p      name_p
)
{
    Int32s_t    returnCode = FALSE;
    Char_t      buf[256];
    Int32s_t    len, i;
    bool        found = FALSE;

    if( fullName_p == NIL ) goto exit;

    if( ( len = strlen( fullName_p ) ) > 255 ) goto exit;

    // Make a copy of the input
    strcpy( &buf[0], fullName_p );

    for( i = len - 1 ; i >= 0 ; i-- )
    {
        if( buf[i] == MB_CHAR_BACKSLASH )
        {
            found = TRUE;
            buf[i] = 0;
            break;
        }
    }

    if( found )
    {
        if( path_p ) strcpy( path_p, &buf[0] );
        if( name_p ) strcpy( name_p, &buf[i+1] );
    }
    else
    {
        // The passed in parameter must have just been a file name
        if( path_p ) *path_p = 0;
        if( name_p ) strcpy( name_p, fullName_p );
    }

    returnCode = TRUE;

exit:
    return returnCode;
}

/******************************************************************************
 * Procedure: pmcFileNameExtractSaskPhn(  )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t        pmcFileNameExtractSaskPhn
(
    Char_p      nameIn_p,
    Char_p      phn_p,
    Int32u_p    date_p
)
{
    Int32s_t    returnCode;
    Char_p      buf_p;
    Char_p      free_p;
    Char_t      phn[16];
    bool        phnFound = FALSE;
    bool        invalidDate = FALSE;
    Ints_t      i;
    Ints_t      len;
    Int32u_t    date = 0;
    Char_t      year[8];
    Char_t      month[4];
    Char_t      day[4];
    int         y, m, d;
    int         daysThisMonth;

    mbMalloc( buf_p, 256 );
    free_p = buf_p;

    if( nameIn_p == NIL ) goto exit;

    strcpy( buf_p, nameIn_p );

    // pmcAlphaNumericOnly( buf_p );
    mbStrDigitsOnly( buf_p );

    mbLog( "Check file name (digits only) '%s' for phn and date\n", buf_p );

    if( *buf_p < '0' || *buf_p > '9' )
    {
    }
    else
    {
        for( i = 0 ; ; )
        {
            if( *buf_p >= '0' && *buf_p <= '9' )
            {
                phn[i] = *buf_p;
            }
            else
            {
                break;
            }
            buf_p++;
            if( ++i == 9 )
            {
                phn[i] = 0;
                phnFound = TRUE;
                break;
            }
        }
    }

    // At this point, buf_p should point past the phn
    mbStrDigitsOnly( buf_p );

    len = strlen( buf_p );
    if( len >= 8 )
    {
        year[0] = *(buf_p + 0);
        year[1] = *(buf_p + 1);
        year[2] = *(buf_p + 2);
        year[3] = *(buf_p + 3);
        year[4] = 0;

        month[0] = *(buf_p + 4);
        month[1] = *(buf_p + 5);
        month[2] = 0;

        day[0] = *(buf_p + 6);
        day[1] = *(buf_p + 7);
        day[2] = 0;

        y = atol( year );
        m = atol( month );
        d = atol( day );

        if( y < 1900 || y > 2025 ) invalidDate = TRUE;
        if( m < 1    || m > 12   ) invalidDate = TRUE;

        if( !invalidDate )
        {
            daysThisMonth = MainForm->Calendar->DaysPerMonth( y, m );
            if( d < 1    || d > daysThisMonth ) invalidDate = TRUE;
        }
#if 0
        // Do not look for date in reverse order
        if( invalidDate )
        {
            invalidDate = FALSE;

            day[0] = *(buf_p + 0);
            day[1] = *(buf_p + 1);
            day[2] = 0;

            month[0] = *(buf_p + 2);
            month[1] = *(buf_p + 3);
            month[2] = 0;

            year[0] = *(buf_p + 4);
            year[1] = *(buf_p + 5);
            year[2] = *(buf_p + 6);
            year[3] = *(buf_p + 7);
            year[4] = 0;

            y = atol( year );
            m = atol( month );
            d = atol( day );

            if( y < 1900 || y > 2025 ) invalidDate = TRUE;
            if( m < 1    || m > 12   ) invalidDate = TRUE;

            if( !invalidDate )
            {
                daysThisMonth = MainForm->Calendar->DaysPerMonth( y, m );
                if( d < 1    || d > daysThisMonth ) invalidDate = TRUE;
            }
        }
#endif

        if( !invalidDate )
        {
            date = y * 10000 + m * 100 + d;
        }
    }

    returnCode = TRUE;

exit:

    if( phn_p )
    {
        if( phnFound )
        {
            mbLog( "Found phn  '%s'\n", phn );
            strcpy( phn_p, &phn[0] );
        }
        else
        {
            *phn_p = 0;
        }
    }

    if( date_p )
    {
        if( date != 0 ) mbLog( "Found date %ld\n", date );
        *date_p = date;
    }

    mbFree( free_p );
    return returnCode;
}

/******************************************************************************
 * Procedure: pmcFileTypeToIndex( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t            pmcFileTypeToIndex
(
    Int32s_t        code
)
{
    Int32s_t    i, index = -1;

    for( i = 0 ; ; i++ )
    {
        if( pmcDocumentTypes[i].code == PMC_PICKLIST_END_CODE ) break;

        if( pmcDocumentTypes[i].code == code )
        {
            index = pmcDocumentTypes[i].index;
            break;
        }
    }
    return index;
}

/******************************************************************************
 * Procedure: pmcFileTypeToIndex( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

Int32s_t            pmcFileIndexToType
(
    Int32s_t        index
)
{
    Int32s_t        i, code = 0;

    for( i = 0 ;  ; i++ )
    {
        if( pmcDocumentTypes[i].code == PMC_PICKLIST_END_CODE ) break;

        if( pmcDocumentTypes[i].index == index )
        {
            code = pmcDocumentTypes[i].code;
            break;
        }
    }
    return code;
}

/******************************************************************************
 * Procedure: pmcFileTypeGet( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

Int32s_t            pmcFileTypeGet
(
    Char_p          name_p,
    Int32s_t        type
)
{
    Int32s_t        returnCode = PMC_DOCUMENT_TYPE_ANY;
    Int32s_t        pos, i;

    for( i = 0 ; i < PMC_DOCUMENT_TYPE_INVALID ; i++ )
    {
        if( ( pos = mbStrPosIgnoreCase( name_p, pmcDocumentTypeExtStrings[i] ) ) > 0 )
        {
            if( pos + strlen( pmcDocumentTypeExtStrings[i] ) == strlen( name_p ) )
            {
                returnCode = i;
                break;
            }
        }
    }

    if( type != PMC_DOCUMENT_TYPE_ANY )
    {
        // Sanity Check
        if( returnCode != type ) mbDlgExclaim( "File type mismatch: %s\n", name_p );
    }
    return returnCode;
}


/******************************************************************************
 * Procedure: pmcButtonDown( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

void pmcButtonDown( TCanvas *canvas_p, TRect *rect_p, Char_p str_p )
{
    AnsiString  str = "";

    if( pmcButtonCanvas_p )
    {
        mbDlgDebug(( "Error: button canvas already set" ));
        goto exit;
    }

    pmcButtonCanvas_p = canvas_p;
    pmcButtonRect_p = rect_p;
    pmcButtonString_p = NIL;

    canvas_p->Brush->Color = clBtnFace;
    canvas_p->FillRect(*rect_p);

    if( str_p )
    {
        str = str_p ;
        canvas_p->TextRect( *rect_p, rect_p->Left + 3, rect_p->Top + 1 , str );
        pmcButtonString_p = str_p;
    }

    canvas_p->Pen->Color = clBtnFace;
    canvas_p->MoveTo( rect_p->Left  , rect_p->Bottom - 1 );
    canvas_p->LineTo( rect_p->Right , rect_p->Bottom - 1 );

    canvas_p->MoveTo( rect_p->Right - 1 , rect_p->Top     );
    canvas_p->LineTo( rect_p->Right - 1 , rect_p->Bottom  );

    canvas_p->Pen->Color = clGray;

    canvas_p->MoveTo( rect_p->Left,  rect_p->Top  );
    canvas_p->LineTo( rect_p->Right, rect_p->Top  );

    canvas_p->MoveTo( rect_p->Left,  rect_p->Top     );
    canvas_p->LineTo( rect_p->Left,  rect_p->Bottom );

exit:
    return;
}

/******************************************************************************
 * Procedure: pmcButtonUp( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 */

void pmcButtonUp( void )
{
    TCanvas    *canvas_p;
    TRect      *rect_p;
    AnsiString  str = "";

    if( pmcButtonCanvas_p == NIL || pmcButtonRect_p == NIL )
    {
        goto exit;
    }

    canvas_p = pmcButtonCanvas_p;
    rect_p   = pmcButtonRect_p;

#if 0
    canvas_p->Pen->Color = clBlue;

    canvas_p->MoveTo( rect_p->Left  , rect_p->Bottom - 1 );
    canvas_p->LineTo( rect_p->Right , rect_p->Bottom - 1 );

    canvas_p->MoveTo( rect_p->Right - 1 , rect_p->Top     );
    canvas_p->LineTo( rect_p->Right - 1 , rect_p->Bottom  );

    canvas_p->Pen->Color = clGreen;

    canvas_p->MoveTo( rect_p->Left  , rect_p->Top    );
    canvas_p->LineTo( rect_p->Right , rect_p->Top    );

    canvas_p->MoveTo( rect_p->Left ,  rect_p->Top     );
    canvas_p->LineTo( rect_p->Left ,  rect_p->Bottom );
#endif

    if( pmcButtonString_p )
    {
        str = pmcButtonString_p;
        canvas_p->TextRect( *rect_p, rect_p->Left + 2, rect_p->Top , str );
        pmcButtonString_p = NIL;
    }


exit:
    pmcButtonCanvas_p = NIL;
    pmcButtonRect_p = NIL;

    return;
}

/******************************************************************************
 * Procedure: pmcCloseSplash( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * The appearance of this file tells the splash screen to close if it is
 * still open.
 *-----------------------------------------------------------------------------
 */

void pmcCloseSplash( void )
{
    FILE *fp;
    Char_t  fileName[256];

    sprintf( fileName, "" );
    // Change to the home directory
    if( pmcHomeDir ) sprintf( fileName, "%s\\", pmcHomeDir );

    strcat( fileName, SPLASH_FLAG_NAME );

    fp = fopen( fileName, "w" );
    fclose( fp);
}

//---------------------------------------------------------------------------
// Check to see if this deceased patient has future appointments
//---------------------------------------------------------------------------

Int32s_t        pmcCheckDeceasedApps
(
    Int32u_t    patientId,
    Int32u_t    deceasedDate
)
{
    Int32u_t    today;
    Int32u_t    count;
    Char_p      buf_p;

    mbMalloc( buf_p, 1024 );
    today = mbToday( );

    // Check to see if deceased date needs to be specified
    if( deceasedDate == 0 )
    {
        sprintf( buf_p, "select %s from %s where %s=%ld\n",
            PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH,
            PMC_SQL_TABLE_PATIENTS,
            PMC_SQL_FIELD_ID, patientId );

        deceasedDate = pmcSqlSelectInt( buf_p, &count );

        // Sanity check
        if( count != 1 )
        {
            mbDlgDebug(( "error reading deceased date for patient: %ld (count: %ld)\n", patientId, count ));
            goto exit;
        }
    }

    if( deceasedDate == 0 ) goto exit;

    if( deceasedDate > today )
    {
        mbDlgExclaim( "Deceased date is in the future\n" );
        goto exit;
    }

    sprintf( buf_p, "select %s from %s where %s=%ld and %s>=%ld and %s=%d",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_PATIENT_ID, patientId,
        PMC_SQL_FIELD_DATE, today,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    pmcSqlSelectInt( buf_p, &count );

    if( count > 0 )
    {
        if( mbDlgYesNo( "This patient has been marked as deceased, but has %d future appointment%s scheduled.\n"
                        "Would you like to delete %s appointment%s now?",
                         count,
                         ( count > 1 ) ? "s" : "",
                         ( count > 1 ) ? "those" : "that",
                         ( count > 1 ) ? "s" : "" ) == MB_BUTTON_NO )
        {
            goto exit;
        }
        sprintf( buf_p, "update %s set %s=%ld where %s=%ld and %s>=%ld",
            PMC_SQL_TABLE_APPS,
            PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE,
            PMC_SQL_FIELD_PATIENT_ID, patientId,
            PMC_SQL_FIELD_DATE, today );

        pmcSqlExec( buf_p );
    }

exit:
    mbFree( buf_p );

    return TRUE;
}

//---------------------------------------------------------------------------
// This function compares the patient's future appointments to the
// specified referring dr ID, and prompts to see if the apppointment
// referring drs should be updated to the specified referring dr.
//---------------------------------------------------------------------------

Int32s_t    pmcCheckReferringDrApps
(
    Int32u_t          patientId,
    Int32u_t          doctorId
)
{
    PmcSqlPatient_p   pat_p;
    PmcSqlDoctor_p    drNew_p;
    Char_p            buf_p;
    Int32u_t          count;
    Int32s_t          returnCode = FALSE;

    mbMalloc( pat_p,   sizeof(PmcSqlPatient_t) );
    mbMalloc( drNew_p, sizeof(PmcSqlDoctor_t) );
    mbMalloc( buf_p, 1024 );

    sprintf( buf_p, "select %s from %s where %s=%d and %s>=%d and %s!=%d and %s=%d",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_PATIENT_ID, patientId,
        PMC_SQL_FIELD_DATE, mbToday( ),
        PMC_SQL_APPS_FIELD_REFERRING_DR_ID, doctorId,
        PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    pmcSqlSelectInt( buf_p, &count );

    if( patientId && count && doctorId )
    {
        if( pmcSqlPatientDetailsGet( patientId, pat_p ) != TRUE )
        {
            mbDlgDebug(( "Error getting patient id %d details\n", patientId ));
            goto exit;
        }

        if( pmcSqlDoctorDetailsGet( doctorId, drNew_p ) != TRUE )
        {
            mbDlgDebug(( "Error getting doctor id %d details\n", doctorId ));
            goto exit;
        }

        sprintf( buf_p, "This patient has %d future appointment%s with %s\n"
                        "different (or no) referring doctor%s specified.\n\n"
                        "Should the referring doctor in the future appointments\n"
                        "belonging to %s %s be set to %s %s?",
                            count,
                            ( count == 1 ) ? "" : "s",
                            ( count == 1 ) ? "a " : "",
                            ( count == 1 ) ? "" : "s",
                            pat_p->firstName, pat_p->lastName,
                            drNew_p->firstName, drNew_p->lastName );

        if( mbDlgYesNo( buf_p ) == MB_BUTTON_YES )
        {
            sprintf( buf_p, "update %s set %s=%d where %s=%d and %s>=%d and %s!=%d and %s=%d",
                PMC_SQL_TABLE_APPS,
                PMC_SQL_APPS_FIELD_REFERRING_DR_ID, doctorId,
                PMC_SQL_FIELD_PATIENT_ID, patientId,
                PMC_SQL_FIELD_DATE, mbToday( ),
                PMC_SQL_APPS_FIELD_REFERRING_DR_ID, doctorId,
                PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

            pmcSqlExec( buf_p );
            returnCode = TRUE;
        }
    }

exit:

    mbFree( buf_p );
    mbFree( pat_p );
    mbFree( drNew_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Check the referring dr against the referring dr in the patient's record
//---------------------------------------------------------------------------

Int32s_t    pmcCheckReferringDr
(
    Int32u_t            patientId,
    Int32u_t            doctorId
)
{
    PmcSqlPatient_p     pat_p;
    PmcSqlDoctor_p      drNew_p;
    PmcSqlDoctor_p      drOld_p;
    Char_p              buf_p;
    Int32s_t            returnCode = FALSE;

    mbMalloc( pat_p,   sizeof(PmcSqlPatient_t) );
    mbMalloc( drNew_p, sizeof(PmcSqlDoctor_t) );
    mbMalloc( drOld_p, sizeof(PmcSqlDoctor_t) );
    mbMalloc( buf_p, 1024 );

    if( patientId && doctorId )
    {
        if( pmcSqlPatientDetailsGet( patientId, pat_p ) != TRUE )
        {
            mbDlgDebug(( "Error getting patient id %d details\n", patientId ));
            goto exit;
        }

        if( pat_p->refDrId == doctorId )
        {
            // Nothing to do
            goto exit;
        }

        if( pmcSqlDoctorDetailsGet( doctorId, drNew_p ) != TRUE )
        {
            mbDlgDebug(( "Error getting doctor id %d details\n", doctorId ));
            goto exit;
        }

        if( pmcSqlDoctorDetailsGet( pat_p->refDrId, drOld_p ) == TRUE )
        {
            sprintf( buf_p, "This referring doctor does not match the default\n"
                            "referring doctor in this patient's database record.\n\n"
                            "Should the default referring doctor in the patient \n"
                            "record belonging to %s %s be changed\n"
                            "from %s %s to %s %s?",
                            pat_p->firstName, pat_p->lastName,
                            drOld_p->firstName, drOld_p->lastName,
                            drNew_p->firstName, drNew_p->lastName );
        }
        else
        {
            sprintf( buf_p, "This patient's database record has no default referring doctor. \n\n"
                            "Should the default referring doctor in the patient record belonging\n"
                            "to %s %s be set to %s %s?",
                            pat_p->firstName, pat_p->lastName,
                            drNew_p->firstName, drNew_p->lastName );
        }

        if( mbDlgYesNo( buf_p ) == MB_BUTTON_YES )
        {
            if( pmcSqlRecordLock( PMC_SQL_TABLE_PATIENTS, patientId, TRUE ) )
            {
                pmcSqlExecInt( PMC_SQL_TABLE_PATIENTS,
                               PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID,
                               doctorId,
                               patientId );


                pmcSqlRecordUnlock( PMC_SQL_TABLE_PATIENTS, patientId );
                returnCode = TRUE;
                pmcCheckReferringDrApps( patientId, doctorId );
            }
        }
    }

exit:

    mbFree( buf_p );
    mbFree( pat_p );
    mbFree( drNew_p );
    mbFree( drOld_p );
    return returnCode;
}

