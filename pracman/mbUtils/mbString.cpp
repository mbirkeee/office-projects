//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// Object for string manipulation
//---------------------------------------------------------------------------

// System Include Files
#include <stdio.h>
#include <dos.h>
#include <dir.h>
#pragma hdrstop

#include "mbUtils.h"

//---------------------------------------------------------------------------
// Constructor.  Allocate lots of space.  The thinking here is that if the
// string is specified, it is unlikely to change, and a small buffer
// conserves space.  But if the string is not spcified, it may be used
// dynamically, hence the large size reduced the likelihood of having
// to resize it.
//---------------------------------------------------------------------------
__fastcall MbString::MbString( void )
{
    mbObjectCountInc();

    Intu_t size = 1024;
    mbMalloc( this->buf_p, size );
    if( this->buf_p == NIL )
    {
        mbDlgError( "Memory allocation error in MbString" );
        this->bufSize = 0;
    }
    else
    {
        this->bufSize = size;
    }
    this->buf_p[0] = 0;
}

//---------------------------------------------------------------------------
// Constructor.  Allocate a string object with buffer of the spcified size.
//---------------------------------------------------------------------------

__fastcall MbString::MbString( Ints_t size )
{
    mbObjectCountInc();

    if( size <= 0 ) size = 1024;
    mbMalloc( this->buf_p, size );
    if( this->buf_p == NIL )
    {
        mbDlgError( "Memory allocation error in MbString" );
        this->bufSize = 0;
    }
    else
    {
        this->bufSize = size;
    }
    this->buf_p[0] = 0;
}

//---------------------------------------------------------------------------
// Constructor.  If the string is specified, allocate only enough space to
// accomodate it.  Otherwise allocate lots of space.  The thinking here is
// that if the string is specified, it is unlikely to change, and a small
// buffer conserves space.  But if the string is not spcified, it may be
// used dynamically, hence the large size reduced the likelihood of having
// to resize it.
//---------------------------------------------------------------------------
__fastcall MbString::MbString( Char_p str_p )
{
    Ints_t  size;

    mbObjectCountInc();

    if( str_p == NIL )
    {
        size = 1024;
    }
    else
    {
        size = strlen( str_p ) + 1;
    }

    mbMalloc( this->buf_p, size );

    if( this->buf_p == NIL )
    {
        mbDlgError( "Memory allocation error in MbString" );
        this->bufSize = 0;
    }
    else
    {
        this->bufSize = size;
    }
    if( str_p )
    {
        strcpy( this->buf_p, str_p );
    }
    else
    {
        this->buf_p[0] = 0;
    }
}

//---------------------------------------------------------------------------
// Copy the specified string into the string object's buffer
//---------------------------------------------------------------------------
Char_p __fastcall MbString::set( Char_p str_p )
{
    Ints_t size;

    if( str_p == NIL ) return NIL;

    size = strlen( str_p ) + 1;

    if( this->bufSize < size )
    {
        this->resize( size + 256 );
    }

    if( this->bufSize < size )
    {
        // Error
        mbDlgError( "Error setting string\n" );
        return NIL;
    }

    strcpy( this->buf_p, str_p );

    return this->buf_p;
}

//---------------------------------------------------------------------------
// Clean the string
//---------------------------------------------------------------------------
Char_p __fastcall MbString::clean( void )
{
    Ints_t  len;
    Char_p  new_p;

    len = this->length();
    mbMalloc( new_p, len+1 );
    mbStrClean( this->get(), new_p, TRUE );
    this->set( new_p );
    mbFree( new_p );

    return this->buf_p;
}

//---------------------------------------------------------------------------
// Strip - remove leading and trailing white space
//---------------------------------------------------------------------------
Char_p __fastcall MbString::strip( void )
{
    mbStrStrip( this->get( ) );
    return this->get( );
}

//---------------------------------------------------------------------------
// Upper case the string in place
//---------------------------------------------------------------------------
Char_p __fastcall MbString::upper( void )
{
    mbStrToUpper( this->get( ) );
    return this->get( );
}

//---------------------------------------------------------------------------
// Truncate string
//---------------------------------------------------------------------------
Char_p __fastcall MbString::truncate( Intu_t pos )
{
    if( this->size() > 1 )
    {
        if( (Ints_t)pos < ( this->size() - 1 ) )
        {
            this->buf_p[pos] = 0;
        }
    }
    return this->get( );
}

//---------------------------------------------------------------------------
// Resize the buffer; mantain contents
//---------------------------------------------------------------------------
Ints_t __fastcall MbString::resize( Ints_t size )
{
    Char_p  new_p;

    mbMalloc( new_p, size );

    if( new_p == NIL )
    {
       mbDlgError( "Memory allocation error in MbString" );
       this->bufSize = 0;
    }
    else
    {
        strcpy( new_p, this->buf_p );
        mbFree( this->buf_p );
        this->buf_p = new_p;
        this->bufSize = size;
    }
    return this->bufSize;
}

//---------------------------------------------------------------------------
// Append the string to the buffer
//---------------------------------------------------------------------------
Char_p __fastcall MbString::append( Char_p string_p )
{
    Ints_t  needSize;

    if( string_p == NIL ) goto exit;

    needSize = strlen( string_p ) + this->length( ) + 1;

    // See if there is enough room in the buffer
    if( needSize > this->bufSize )
    {
        // Not enough room, resize
        this->resize( needSize + 256 );
    }

    if( needSize > this->bufSize )
    {
        // Error
        mbDlgError( "Error resizing string\n" );
        return NIL;
    }

    // Append the string
    strcat( this->buf_p, string_p );

exit:
    return this->get();
}

//---------------------------------------------------------------------------
// Append the string to the buffer
//---------------------------------------------------------------------------
Char_p __fastcall MbString::appendChar( Char_t c )
{
    Ints_t  needSize;
    Ints_t  len;

    len = this->length( );
    needSize = len + 2;

    // See if there is enough room in the buffer
    if( needSize > this->bufSize )
    {
        // Not enough room, resize
        this->resize( needSize + 256 );
    }

    if( needSize > this->bufSize )
    {
        // Error
        mbDlgError( "Error resizing string\n" );
        return NIL;
    }

    // Append the char
    *( this->buf_p + len ) = c;
    *( this->buf_p + len + 1 ) = 0;

exit:
    return this->get();
}

//---------------------------------------------------------------------------
// Convert the string to a format that can be output in latex
//---------------------------------------------------------------------------
Char_p __fastcall MbString::latex(  )
{
    MbString    out;
    Boolean_t   include;
    Char_p      in_p;
    Int8u_t     c;

    for( in_p = this->buf_p ; ; )
    {
        include = FALSE;
        c = (Int8u_t)*in_p++;

        if( c == 0 ) break;

        if(    ( c >= 'a' && c <= 'z' )
            || ( c >= 'A' && c <= 'Z' )
            || ( c >= '0' && c <= '9' )

            || c == '!'
            || c == ','
            || c == '.'
            || c == ':'
            || c == ';'
            || c == '?'
            || c == '/'
            || c == '='
            || c == '+'
            || c == '-'
            || c == MB_CHAR_QUOTE_SINGLE )
        {
            include = TRUE;
        }
        else if(    c == MB_CHAR_SPACE
                 || c == MB_CHAR_TAB )
        {
            include = TRUE;
        }
        else if( c == MB_CHAR_LF )
        {
            include = TRUE;
        }
        else if( c == MB_CHAR_CR )
        {
            include = TRUE;
        }
        else if( c == MB_CHAR_FF )
        {
            include = TRUE;
        }
        else if( c == '_' )
        {
            out.append( "{\\_}" );
        }
        else if( c == '~' )
        {
            out.append( "{\\~{}}" );
        }
        else if( c == '^' )
        {
            out.append( "{\\textasciicircum{}}" );
        }
        else if( c == '#' )
        {
            out.append( "{\\#}" );
        }
        else if( c == '|' )
        {
            out.append( "{$|$}" );
        }
        else if( c == '%' )
        {
            out.append( "{\\%}" );
        }
        else if( c == '@' )
        {
            out.append( "{$@$}" );
        }
        else if( c == '$' )
        {
            out.append( "{\\$}" );
        }
        else if( c == '&' )
        {
            out.append( "{\\&}" );
        }
        else if( c == '*' )
        {
            out.append( "{$\\ast$} " );
        }
        else if( c == '(' )
        {
           out.append( "{(}" );
        }
        else if( c == ')' )
        {
            out.append( "{)}" );
        }
        else if( c == '{' )
        {
            out.append( "{\\{}" );
        }
        else if( c == '}' )
        {
            out.append( "{\\}}" );
        }
        else if( c == '[' )
        {
            out.append( "{[}" );
        }
        else if( c == ']' )
        {
            out.append( "{]}" );
        }
        else if( c == '<' )
        {
            out.append( "{$<$}" );
        }
        else if( c == '>' )
        {
            out.append( "{$>$}" );
        }
        else if( c == MB_CHAR_DEGREES )
        {
            out.append( "{$^{\\circ}$}" );
        }
        else if( c == MB_CHAR_ALPHA )
        {
            out.append( "{${\\alpha}$}" );
        }
        else if( c == MB_CHAR_BETA )
        {
            out.append( "{${\\beta}$}" );
        }

        if( include ) out.appendChar( c );
    }

#if 0
    if( truncatedFlag )
    {
        out.append( "\\ldots" );
    }
#endif

    this->set( out.get() );

    return this->get();
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
__fastcall MbString::~MbString( void )
{
    mbObjectCountDec();
    mbFree( this->buf_p );
}

//---------------------------------------------------------------------------
// Sprintf into a MbString object
//---------------------------------------------------------------------------
Ints_t mbSprintf( MbString *string_p, Char_p fmt_p, ... )
{
    Ints_t      length;
    va_list     arg;
    Ints_t      size;

    if( string_p->get() == NIL ) return 0;

    if( fmt_p == NIL ) return 0;

      // OK, attempt to write into this object's string buffer

    size = string_p->size();

    va_start( arg, fmt_p );
    length = vsnprintf( string_p->get(), string_p->size(), fmt_p, arg );
    va_end( arg );

    if( length < size )
    {
        // All is OK
    }
    else
    {
        size = length + 1;
        string_p->resize( size );

        if( string_p->get() == NIL )
        {
            mbDlgError( "Memory allocation error in MbString" );
            goto exit;
        }

        va_start( arg, fmt_p );
        length = vsnprintf( string_p->get(), string_p->size(), fmt_p, arg );
        va_end( arg );

        if( length < size )
        {
            // All is OK
        }
        else
        {
            mbDlgError( "Buffer overflow in MbString::sprintf()" );
            string_p->clear();
        }
    }

exit:
    return string_p->length();
}
