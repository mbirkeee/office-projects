//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// Object for string manipulation
//---------------------------------------------------------------------------

#ifndef mbStringH
#define mbStringH

//---------------------------------------------------------------------------
// Simple string class.  I know there is a built-in string object but I
// want to do my own for learning purposes.
//---------------------------------------------------------------------------
class MbString
{
public:
    __fastcall  MbString( Char_p str_p );
    __fastcall  MbString( Ints_t size );
    __fastcall  MbString( void );
    __fastcall ~MbString( void );

    Char_p __fastcall set( Char_p str_p );
    Char_p __fastcall get( void ) { return this->buf_p; }
    Ints_t __fastcall size( void ) { return this->bufSize; }
    Ints_t __fastcall length( void ) { return ( this->buf_p == NIL ) ? 0 : strlen( this->buf_p ); }
    Ints_t __fastcall resize( Ints_t size );
    Ints_t __fastcall clear( void ) { if( this->buf_p ) { *this->buf_p = 0; } return 0; }
    Char_p __fastcall clean( void );
    Char_p __fastcall upper( void );
    Char_p __fastcall truncate( Intu_t pos );
    Char_p __fastcall append( Char_p string_p );
    Char_p __fastcall appendChar( Char_t c );
    Char_p __fastcall latex( void );
    Char_p __fastcall strip( void );

private:
    Ints_t      bufSize;
    Char_p      buf_p;
};

Ints_t      mbSprintf( MbString *string_p, Char_p fmt_p, ... );

#endif // mbStringH
