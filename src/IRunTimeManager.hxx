#ifndef _IRUNTIMEMANAGER_HXX_
#define _IRUNTIMEMANAGER_HXX_

#include <iostream>

#include "WireManager.hxx"
#include "RunLogManager.hxx"
#include "ExperimentConfig.hxx"
#include "ICDCGas.hxx"
#include "IPosResManager.hxx"

using namespace std;

/***
    This is a singleton class which sets up all the managers.
***/
class IRunTimeManager {
    public:
    ~IRunTimeManager(){}

    enum {
          kUseConstantVelocity  = 0,
          kUseAvgXtFunction     = 1,
          kUseLayXtTable        = 2,
          kUseLayPhiXtTable     = 3,
          kUseLayPhiBetaXtTable = 4,
          kUsePhiXtTable        = 5,
          kUsePhiBetaXtTable    = 6,
    };

    static IRunTimeManager &Get();

    /// Initialize this, when you dont need x-t relation
    /// and resolutionl always use constant x-t
    void Initialize(int runNo, string config, string experiment);

    /// Initialize this, when you need to do the tracking
    void Initialize(int runNo, int testLayer, int iter,
                    string config, string experiment,
                    string xtPath, string resPath);
    void Initialize(int runNo, int testLayer, int iter,
                    TString config, TString experiment,
                    string xtPath, string resPath){
        std::string conf(config.Data());
        std::string exp(experiment.Data());
        Initialize(runNo,testLayer,iter,conf,exp,xtPath,resPath);
    }

    int GetRunNumber(){  return fRunNo;  }
    int GetTestLayer(){  return fTestLayer;  }
    int GetIteration(){  return fIteration;  }

    /// Functions for adding pre/suffix
    /// {@
    void AddSuffix(TString suffix){  fSuffix = suffix; }
    void AddPrefix(TString prefix){  fPrefix = prefix; }
    /// Functions for getting pre/suffix
    TString GetSuffix(){  return fSuffix;  }
    TString GetPrefix(){  return fPrefix;  }
    /// @}

    private:
    TString fSuffix;
    TString fPrefix;

    int fRunNo;
    int fTestLayer;
    int fIteration;
    bool fForceReadConstantDv;
    bool fSimpleMode;
    string fConfigure;
    string fExperiment;
    static IRunTimeManager *fIRunTimeManager;
    IRunTimeManager(){}; //Don't Implenment
    IRunTimeManager(IRunTimeManager const&); //Don't Implenment
    void operator=(IRunTimeManager const&); //Don't Implenment
};

#endif
