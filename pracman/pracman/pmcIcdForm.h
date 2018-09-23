//---------------------------------------------------------------------------
// File:    pmcIcdForm.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    April 21, 2002
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcIcdFormH
#define pmcIcdFormH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>

#include "pmcInitialize.h"

//---------------------------------------------------------------------------

typedef struct pmcIcdListInfo_s
{
    Int32u_t    returnCode;
    Char_t      code[8];
    Char_t      codeIn[8];
    Int32u_t    mode;
} pmcIcdListInfo_t, *pmcIcdListInfo_p;

class TIcdForm : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TButton *CancelButton;
    TListView *IcdListView;
    TGroupBox *GroupBox1;
    TEdit *SearchEdit;
    TButton *OkButton;
    TGroupBox *GroupBox2;
    TLabel *SelectedLabel;
    TGroupBox *GroupBox3;
    TLabel *Label1;
    TLabel *EntriesCountLabel;
    TLabel *Label2;
    TLabel *EntriesShownLabel;
    void __fastcall CancelButtonClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall SearchEditChange(TObject *Sender);
    void __fastcall IcdListViewColumnClick(TObject *Sender,
          TListColumn *Column);
    void __fastcall IcdListViewSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
    void __fastcall OkButtonClick(TObject *Sender);
    void __fastcall IcdListViewDblClick(TObject *Sender);
    void __fastcall SearchEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);

private:	// User declarations
    void __fastcall ListUpdate( void );
    void __fastcall ListSort( void );
    void __fastcall ListFree( void );
    bool __fastcall SortCode
    (
        Int32u_t        sortMode,
        qHead_p         temp_q,
        pmcIcdStruct_p  icd_p,
        pmcIcdStruct_p  tempIcd_p,
        Int32u_t        descSort
    );
    bool __fastcall SortDesc
    (
        Int32u_t        sortMode,
        qHead_p         temp_q,
        pmcIcdStruct_p  icd_p,
        pmcIcdStruct_p  tempIcd_p,
        Int32u_t        codeSort
    );

    pmcIcdListInfo_p    Info_p;
    pmcIcdStruct_p      SelectedIcd_p;
    bool                Active;
    qHead_t             IcdQueueHead;
    qHead_p             Icd_q;
    qHead_t             UnusedQueueHead;
    qHead_p             Unused_q;

    Int32u_t            SortMode;
    Int32u_t            SortCodeMode;
    Int32u_t            SortDescMode;

public:		// User declarations
    __fastcall TIcdForm(TComponent* Owner);
    __fastcall TIcdForm(TComponent* Owner, pmcIcdListInfo_p info_p );
};


//---------------------------------------------------------------------------

#endif
