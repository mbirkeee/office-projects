//---------------------------------------------------------------------------

#ifndef pmcEchoPatSelectFormH
#define pmcEchoPatSelectFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

//---------------------------------------------------------------------------

typedef struct pmcEchoPatSelectFormInfo_s
{
    Int32u_t                returnCode;
    Int32u_t                date;
    Int32u_t                patientId;
    Int32u_t                providerId;
    Char_p                  name_p;
    Boolean_t               cancelButton;
    Boolean_t               abortFlag;

} pmcEchoPatSelectFormInfo_t, *pmcEchoPatSelectFormInfo_p;

#define PMC_PAT_BUTTON_COUNT    10
//---------------------------------------------------------------------------
class TEchoPatSelectForm : public TForm
{
__published:	// IDE-managed Components
    TButton *Button0;
    TButton *Button1;
    TButton *Button2;
    TButton *Button5;
    TButton *Button6;
    TButton *Button7;
    TButton *Button8;
    TButton *Button9;
    TButton *Button4;
    TButton *ButtonCancel;
    TLabel *Label1;
    TLabel *LabelEchoDate;
    TLabel *Label3;
    TLabel *LabelEchoName;
    TLabel *LabelExplain;
    TButton *Button3;
    TButton *ButtonNoMatch;
    void __fastcall ButtonCancelClick(TObject *Sender);
    void __fastcall ButtonNoMatchClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall Button0Click(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
    void __fastcall Button2Click(TObject *Sender);
    void __fastcall Button3Click(TObject *Sender);
    void __fastcall Button4Click(TObject *Sender);
    void __fastcall Button5Click(TObject *Sender);
    void __fastcall Button6Click(TObject *Sender);
    void __fastcall Button7Click(TObject *Sender);
    void __fastcall Button8Click(TObject *Sender);
    void __fastcall Button9Click(TObject *Sender);
private:	// User declarations
    pmcEchoPatSelectFormInfo_p Info_p;

    TButton        *PatButton_p[PMC_PAT_BUTTON_COUNT];
    Int32u_t        PatId[PMC_PAT_BUTTON_COUNT];

    void __fastcall ButtonClick( Int32u_t index );
    
public:		// User declarations

    __fastcall TEchoPatSelectForm(TComponent* Owner);
    __fastcall TEchoPatSelectForm(TComponent* Owner, pmcEchoPatSelectFormInfo_p info_p );
};

#endif

