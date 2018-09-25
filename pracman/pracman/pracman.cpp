//---------------------------------------------------------------------------
// File:    pracman.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 6, 2001
//---------------------------------------------------------------------------
// Description:
//
// Practice Manager entry point.
//---------------------------------------------------------------------------

#include <dos.h>
#include <dir.h>
#include <process.h>

#include <vcl.h>
#include <vcl\DBTables.hpp>
#include <vcl\DB.hpp>
#include <Db.hpp>
#pragma hdrstop

#include "pmcUtils.h"
#include "pmcInitialize.h"

USEFORM("pmcMainForm.cpp", MainForm);
USEFORM("pmcPatientListForm.cpp", PatientListForm);
USEFORM("pmcNewProviderForm.cpp", ProviderDetailsForm);
USEFORM("pmcSplashScreen.cpp", SplashScreen);
USEFORM("pmcPatientEditForm.cpp", PatientEditForm);
USEFORM("pmcDateSelectForm.cpp", DateSelectForm);
USEFORM("pmcPromptForm.cpp", AppCommentForm);
USEFORM("pmcPatAppListForm.cpp", PatAppListForm);
USEFORM("pmcDoctorListForm.cpp", DoctorListForm);
USEFORM("pmcAppLettersForm.cpp", AppLettersForm);
USEFORM("pmcDoctorEditForm.cpp", DoctorEditForm);
USEFORM("pmcAboutForm.cpp", AboutForm);
USEFORM("pmcClaimListForm.cpp", ClaimListForm);
USEFORM("pmcClaimEditForm.cpp", ClaimEditForm);
USEFORM("pmcReportForm.cpp", ReportForm);
USEFORM("pmcAppListForm.cpp", AppListForm);
USEFORM("pmcBatchImportForm.cpp", BatchImportForm);
USEFORM("pmcDocumentEditForm.cpp", DocumentEditForm);
USEFORM("pmcDocumentListForm.cpp", DocumentListForm);
USEFORM("pmcWordCreateForm.cpp", WordCreateForm);
USEFORM("pmcSelectForm.cpp", SelectForm);
USEFORM("pmcPatientForm.cpp", PatientForm);
USEFORM("pmcIcdForm.cpp", IcdForm);
USEFORM("pmcAppHistoryForm.cpp", AppHistoryForm);
USEFORM("pmcEchoImportForm.cpp", EchoImportForm);
USEFORM("pmcEchoCDContentsForm.cpp", EchoCDContentsForm);
USEFORM("pmcEchoListForm.cpp", EchoListForm);
USEFORM("pmcTextEditForm.cpp", TextEditForm);
USEFORM("pmcEchoPatSelectForm.cpp", EchoPatSelectForm);
USEFORM("pmcMedListForm.cpp", MedListForm);
USEFORM("pmcEchoDetailsForm.cpp", EchoDetailsForm);
USEFORM("pmcFormPickList.cpp", Form_PickList);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    bool failed = TRUE;

    pmcWinVersion = mbWinVersion( );

    // Run the splash screen program.  I am having trouble getting a splash
    // screen to some up within this program... it won't come up until
    // after the database is opened, which can take a lot of time on
    // a slow computer.  Therefore, launch standalone splash program.
    spawnle( P_NOWAITO, "splash.exe", "splash.exe", NULL, NULL, NULL );

    if( !pmcInitialize( ) ) goto exit;

    try
    {
         Application->Initialize();
         Application->Title = PMC_NAME;
         Application->Title = "Practice Manager";
         Application->CreateForm(__classid(TMainForm), &MainForm);
         Application->Run();
    }
    catch( Exception &exception )
    {
        goto exit;
    }

    failed = FALSE;

exit:
    pmcShutdown( );
    pmcCloseSplash( );
    if( failed )
    {
        Sleep( 1000 );

        Application->MessageBox( PMC_NAME
                                 " could not initialize and is now shutting down.\n"
                                 "Ensure the user name and password are correct.\n"
                                 "If problems persist, contact the system administrator.",
                                  PMC_NAME, MB_OK|MB_ICONERROR );
    }
    return 0;
}



