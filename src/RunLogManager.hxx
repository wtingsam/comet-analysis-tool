#ifndef _RUNLOGMANAGER_HXX_
#define _RUNLOGMANAGER_HXX_

#include <sstream>
#include <fstream>

#include "TString.h"

#include "ExperimentConfig.hxx"
/***
    This is a singleton class.
    Only call it one time when you need it at the beginning of the main function
***/

class RunLogManager{

    public:

        ~RunLogManager(){}

        static RunLogManager &Get();

        void Initialize();
        void Initialize(int runNo);

        void Ls(void);

        /// Set Experiment name
        void SetExperimentName(TString experimentName){  fExperimentName=experimentName;  }

        /// Get run information
        TString GetGasName(){ return fGasMixture;   }
        Int_t GetRunNumber(){  return fRunNumber;  }
        Int_t GetFlowRate(){  return fFlowRate;  }
        Int_t GetRunGrade(){  return fRunGrade;  }
        Int_t GetTime(){  return fTime;  }
        Int_t GetHighVoltage(){  return fHighVoltage;  }
        Int_t GetThreshold(){  return fThreshold;  }

        virtual void CheckPath(void);

        virtual void SetPath(TString path){ fRunLogPath=path; }

        /// Read run log root format
        void ReadRunLog(Int_t runInput);//Run a root file in specific format

        /// Read run log text format
        void ReadRunLogText(Int_t runInput);//Run a text file in specific format

    private:
        /// Singleton
        RunLogManager();
        RunLogManager(RunLogManager const&);
        void operator=(RunLogManager const&);

        static RunLogManager *fRunLogManager;
        //  static *RunLogManager=fRunLogManager;

        TString fExperimentName;
        TString fRunLogPath;
        TString fGasMixture;
        Int_t fRunNumber;
        Int_t fRunGrade;
        Int_t fTime;
        Int_t fHighVoltage;
        Int_t fHighVoltage_guard;
        Double_t fMagneticField;
        Double_t fThreshold;
        Double_t fFlowRate;

};

#endif
