//---------------------------------------------------------------------------
// File:    pmcMspValidrpt.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef pmcMspValidrptH
#define pmcMspValidrptH

Int32s_t pmcMspValidrptFileHandle(  Boolean_t internetFlag );
Int32s_t pmcMspValidrptFileProcess( Char_p fileName_p );
Int32s_t pmcMspValidrptFileCheckEmpty( Char_p fileName_p );

#endif
