//---------------------------------------------------------------------------
// (c) 2008 Michael A. Bree
//---------------------------------------------------------------------------
// Class for PDF file generation.  Uses pracman global config settings.
// Also uses "pmcdespl" service running on server.
//---------------------------------------------------------------------------
#ifndef pmcPDFH
#define pmcPDFH

class MbPDF
{
public:
    // Constructors
    __fastcall MbPDF( void );
    __fastcall MbPDF( Char_p name_p, Int32u_t id );

    Char_p      __fastcall nameTemp( void ) { return this->tempName_p; }
    Char_p      __fastcall nameFinal( void ) { return this->finalName_p; }

    Int32s_t    __fastcall generate( Int32u_t waitSec );
    Int32s_t    __fastcall view( void );
    Int32s_t    __fastcall subDate( Char_p key_p, Int32u_t date );
    Int32s_t    __fastcall subString( Char_p key_p, Char_p value_p, Int32u_t maxLength );
    Int32s_t    __fastcall subCallback( Char_p     key_p,
                                        Int32s_t (*func_p)( Int32u_t handle, FILE *fp, Char_p key_p ),
                                        Int32u_t   handle );
    Int32s_t    __fastcall subBoolean( Char_p key_p, Boolean_t value );
    Int32s_t    __fastcall templateToTemp( void );
    Int32s_t    __fastcall templateToTempSub( void );
    Int32s_t    __fastcall check( void ) { return this->error; }
    Int32s_t    __fastcall templateSet( Char_p template_p, Int32u_t id );
    Char_p      __fastcall fileNameFinalGet( void ) { return this->finalName_p; }

    // Destructor
    __fastcall ~MbPDF( );

private:

    Int32s_t    __fastcall init( void );
    Int32s_t    __fastcall postProcessTex( Char_p nameIn_p, Char_p nameOut_p );
    
    Char_p      targetBase_p;
    Char_p      templateName_p;
    Char_p      targetFileName_p;
    Char_p      tempName_p;
    Char_p      spoolName_p;
    Char_p      flagName_p;
    Char_p      pdfName_p;
    Char_p      finalName_p;

    Char_p      bigBuf_p;
    
    qHead_t     subQueue;
    qHead_p     sub_q;

    Boolean_t   error;
};

#endif // MbPDF
