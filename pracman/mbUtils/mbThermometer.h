//---------------------------------------------------------------------------
#ifndef pmcThermometerH
#define pmcThermometerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "CGAUGES.h"
#include <ExtCtrls.hpp>
#include <Buttons.hpp>

#include "mbTypes.h"

//---------------------------------------------------------------------------
class TThermometer : public TForm
{
__published:	// IDE-managed Components
    TCGauge *TheGauge;
    TSpeedButton *CancelButton;
    void __fastcall FormDeactivate(TObject *Sender);

private:	// User declarations
    Int32u_t    ButtonDown;
    TCursor     OrigCursor;
    Boolean_t   ShowCancelButton;
    Void_p      SQLCursorHandle;
    Void_p      DefaultCursorHandle;
    Boolean_t   Ready;

public:		// User declarations
     __fastcall TThermometer
    (
        Char_p      caption_p,
        Int32u_t    minValue,
        Int32u_t    maxValue,
        Boolean_t   showCancelButton
    );

    __fastcall ~TThermometer( );

    Int32s_t __fastcall  Increment( void );
    void     __fastcall  Set( Int32u_t value );
    Int32s_t __fastcall  CheckCancel( void );
    void     __fastcall  SetCaption( Char_p caption_p ) { if( caption_p ) { Caption = caption_p; } }
};

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------
#endif
