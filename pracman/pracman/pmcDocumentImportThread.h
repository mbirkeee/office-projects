//---------------------------------------------------------------------------
#ifndef pmcDocumentImportThreadH
#define pmcDocumentImportThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class DocumentImportThread : public TThread
{
private:
protected:
    void __fastcall Execute();
public:
    __fastcall DocumentImportThread(bool CreateSuspended);
};
//---------------------------------------------------------------------------
#endif
