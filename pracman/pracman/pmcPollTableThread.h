//---------------------------------------------------------------------------
#ifndef pmcPollTableThreadH
#define pmcPollTableThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class PollTableThread : public TThread
{
private:
protected:
    void __fastcall Execute();
public:
    __fastcall PollTableThread(bool CreateSuspended);
//    void __fastcall GetTableStatus();
};
//---------------------------------------------------------------------------
#endif
