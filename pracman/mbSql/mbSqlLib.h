//---------------------------------------------------------------------------
// Function: pmcDateTime.h
//---------------------------------------------------------------------------
// Date: Feb. 15, 2001
//---------------------------------------------------------------------------
// Description:
//
// Customized Date Time class for PMC application
//---------------------------------------------------------------------------

#ifndef mbSql_h
#define mbSql_h

#include "mysql.h"
#include "mbUtils.h"

#define MB_SQL_ERR_OK           0
#define MB_SQL_ERR_CONNECT      1
#define MB_SQL_ERR_ALLOC        2
#define MB_SQL_ERR_INVALID      3
#define MB_SQL_ERR_CANCEL       4

typedef struct PmcMbSql_s
{
    qLinkage_t      linkage;
    Int32s_t        index;
    Int32s_t        numFields;
    MYSQL          *mysql_p;
    MYSQL_RES      *result_p;
    MYSQL_ROW       row;
} PmcMbSql_t, *PmcMbSql_p;

class MbSQL
{
  public:
    // Constructors
    __fastcall MbSQL( );
    __fastcall MbSQL( Char_p cmd_p );

    // Destructor
   __fastcall ~MbSQL( );

    // Public Functions
    Boolean_t   __fastcall Query( Char_p cmd_p );
    Boolean_t   __fastcall Update( Char_p cmd_p );
    Boolean_t   __fastcall RowGet( Void_t );
    Int32s_t    __fastcall IndexGet( Char_p name_p );
    Int32u_t    __fastcall RowCount( Void_t );

    // These functions return data by col index
    Char_p      __fastcall String( Int32s_t index );
    Int32u_t    __fastcall Int32u( Int32s_t index );
    Int64u_t    __fastcall DateTimeInt64u( Int32s_t index );
    Float_t     __fastcall Float( Int32s_t index );

    // These functions return data by colunm name
    Char_p      __fastcall NmString( Char_p name_p );
    Int32u_t    __fastcall NmInt32u( Char_p name_p );
    Int64u_t    __fastcall NmDateTimeInt64u( Char_p name_p );
    Float_t     __fastcall NmFloat( Char_p name_p );

  private:
    // Private Functions
    Boolean_t   __fastcall Init( Void_t );
    Boolean_t   __fastcall Execute( Char_p cmd_p, Boolean_t queryFlag );

    // Private members
    PmcMbSql_p      mSql_p;
};


//---------------------------------------------------------------------------
// Function prototypes
//---------------------------------------------------------------------------

extern Int32s_t     mbSqlInit
(
    Char_p      host_p,
    Int32u_t    port,
    Char_p      database_p,
    Char_p      username_p,
    Char_p      password_p,
    Boolean_t   logFlag
);

extern Int32s_t     mbSqlTerminate
(
    Void_t
);

extern Void_t    mbSqlTerminateRequest( Void_t );

#endif // mbSql_h

