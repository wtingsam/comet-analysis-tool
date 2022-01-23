#ifndef _Calibration_File_HXX_
#define _Calibration_File_HXX_
#include "TFile.h"
#include "TString.h"
#include "iostream"
#include "Iteration.hxx"

using namespace std;

class CalibrationFile{
    public:
        virtual ~CalibrationFile(){}
        void LoadXtFile(TString path);

        static CalibrationFile &Get();
        static void Reset(){
            delete fCalibrationFile;
            fCalibrationFile = NULL;
        }

        TFile *GetXtFile(){ return fXtFunctionFile; }
    private:
        TFile *fXtFunctionFile;

        static CalibrationFile *fCalibrationFile;
        CalibrationFile(){};
        void operator=(CalibrationFile const&);

};


#endif
