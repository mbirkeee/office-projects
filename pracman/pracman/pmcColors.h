//---------------------------------------------------------------------------
// File:    pmcColors.h
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    May 12, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#ifndef H_pmcColors
#define H_pmcColors

#ifdef  External
#undef  External
#endif

#ifdef  PMC_INIT_GLOBALS
#define External
#else
#define External extern "C"
#endif  /* PMC_INIT_GLOBALS */

enum pmcColorList
{
     PMC_COLOR_AVAIL
    ,PMC_COLOR_BREAK
    ,PMC_COLOR_APP
    ,PMC_COLOR_CONFIRMED
    ,PMC_COLOR_ARRIVED
    ,PMC_COLOR_COMPLETED
    ,PMC_COLOR_NOSHOW
    ,PMC_COLOR_CONFLICT
    ,PMC_COLOR_REDUCED
    ,PMC_COLOR_COUNT
};

External Int32s_t pmcDefaultColors[]
#ifdef PMC_INIT_GLOBALS
=
{
     0x00FFFFFF             // COLOR_AVAIL
    ,0x00E1E1E1             // COLOR_BREAK
    ,0x00FFE4B7             // COLOR_APPOINT
    ,0x00FB5E59             // COLOR_WAITING
    ,clGreen                // COLOR_EXAM
    ,0x00A3FAAB             // COLOR_COMPLETE
    ,clYellow               // COLOR_NOSHOW
    ,clRed                  // COLOR_CONFLICT
    ,0x000080FF             // COLOR_REDUCED
}
#endif
;

External Char_p pmcColorLegendStrings_p[]
#ifdef PMC_INIT_GLOBALS
=
{
     "Available"            // COLOR_AVAIL
    ,"Unavailable / Break"  // COLOR_BREAK
    ,"Appointment"          // COLOR_APPOINT
    ,"Confirmed"            // COLOR_CONFIMED
    ,"Arrived"              // COLOR_ARRIVED
    ,"Complete"             // COLOR_COMPLETE
    ,"No Show"              // COLOR_NOSHOW
    ,"Attention Required"   // COLOR_CONFLICT
}
#endif
;

External Int32s_t   pmcAppLegendColor[]
#ifdef PMC_INIT_GLOBALS
=
{
     pmcDefaultColors[PMC_COLOR_APP]
    ,pmcDefaultColors[PMC_COLOR_CONFIRMED]
    ,pmcDefaultColors[PMC_COLOR_COMPLETED]
    ,pmcDefaultColors[PMC_COLOR_NOSHOW]
    ,pmcDefaultColors[PMC_COLOR_REDUCED]
    ,pmcDefaultColors[PMC_COLOR_CONFLICT]
    ,pmcDefaultColors[PMC_COLOR_CONFLICT]
}
#endif
;

External Char_p pmcAppLegendString_p[]
#ifdef PMC_INIT_GLOBALS
=
{
     "OK"
    ,"Confirmed"
    ,"Completed"
    ,"No Show"
    ,"Cancelled/Deleted"
    ,"Conflict"
    ,"Invalid"
}
#endif
;

#define PMC_COLOR_DARKEN( _parm )\
{\
    if( _parm != 0x00FFFFFF )\
    {\
    _parm = (( _parm & 0x000000FF ) > 0x00000020 ) ? _parm - 0x00000020 : _parm & 0xFFFFFF00;\
    _parm = (( _parm & 0x0000FF00 ) > 0x00002000 ) ? _parm - 0x00002000 : _parm & 0xFFFF00FF;\
    _parm = (( _parm & 0x00FF0000 ) > 0x00200000 ) ? _parm - 0x00200000 : _parm & 0xFF00FFFF;\
    }\
}

#endif /* H_pmcColors */

