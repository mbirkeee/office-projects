//---------------------------------------------------------------------------
#ifndef pmcPromptFormH
#define pmcPromptFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>

#include "mbUtils.h"

typedef struct pmcAppEditInfo_s
{
    Int32u_t    returnCode;
    Int32u_t    mode;
    Int32u_t    appointId;
    Char_t      caption[256];
} pmcAppEditInfo_t, *pmcAppEditInfo_p;


//---------------------------------------------------------------------------
class TAppCommentForm : public TForm
{
__published:	// IDE-managed Components
    TButton *OkButton;
    TButton *CancelButton;
    TBevel *Bevel1;
    TMaskEdit *CommentInEdit;
    TMaskEdit *CommentOutEdit;
    TLabel *Label2;
    TLabel *Label3;
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
    pmcAppEditInfo_p    AppEditInfo_p;
    
public:		// User declarations
    __fastcall TAppCommentForm(TComponent* Owner);
    __fastcall TAppCommentForm(TComponent* Owner, pmcAppEditInfo_p appEditInfo_p );
};
//---------------------------------------------------------------------------
// extern PACKAGE TPromptForm *PromptForm;
//---------------------------------------------------------------------------
#endif
