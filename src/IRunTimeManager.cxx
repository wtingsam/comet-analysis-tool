#include "IRunTimeManager.hxx"

IRunTimeManager *IRunTimeManager::fIRunTimeManager = NULL;

IRunTimeManager &IRunTimeManager::Get()
{
    if ( !fIRunTimeManager ){//make sure only being created one time
        fIRunTimeManager = new IRunTimeManager();
    }
    return *fIRunTimeManager;
}

void IRunTimeManager::Initialize(int runNo, std::string config, std::string experiment){
    fRunNo=runNo;
    fSimpleMode = true;
    Initialize(runNo,999,0,config,experiment,"","");
}

void IRunTimeManager::Initialize(int runNo, int testLayer,int iter,
                                 string config, string experiment, string xtPath, string resPath){
    //fForceReadConstantDv = forceReadXt ;
    fRunNo=runNo;
    fTestLayer=testLayer;
    fIteration=iter;

    ExperimentConfig::Get().Initialize(config, experiment);
    double CDC_TILTED_ANGLE = ExperimentConfig::Get().GetSetUpAng();
    RunLogManager::Get().Initialize(runNo);
    RunLogManager::Get().Ls();
    printf("## The applied angle for rotation to make the setup horizontal is %f \n",CDC_TILTED_ANGLE+90.0);
    WireManager::Get().Initialize(CDC_TILTED_ANGLE+90.0);
    WireManager::Get().LsCRT();
    // Skilp if it is a simple mode
    if(fSimpleMode) return;
    bool forceReadLayer = true;
    ICDCGas::Get().Initialize(kUseLayPhiBetaXtTable,xtPath,
                              RunLogManager::Get().GetGasName(),
                              forceReadLayer
                              );

    // Initialize position resolution manager
    char* PAR_DIR = getenv ("CCPARAMETER_DIR");
    if(PAR_DIR==NULL){
        cerr << "## No parameters root file :" << PAR_DIR << endl;
    }
    TString path = Form("%s/Fitting_ResVsDist_r%di%d_0um.root",PAR_DIR,fRunNo,fIteration-1);
    TString funcName = Form("position_resolution_l%d",10);
    if(iter==0){
        IPosResManager::Get().Initialize(path,funcName);
    }else{
        bool usePosErrFunction = true;
        IPosResManager::Get().Initialize(path,funcName,usePosErrFunction);
    }
    TString pathRes = resPath;
    return ;
    // Initialize x-t manager
    // char* PAR_DIR = getenv ("CCPARAMETER_DIR");
    // if(PAR_DIR==NULL){
    //     cerr << "## No parameters root file :" << PAR_DIR << endl;
    // }
    // TString xtPath=Form("%s/xtf_run%d_i%d.root",PAR_DIR,runNo,iter-1);
    // //TODO   Should fix this soon...
    // TString gasType = "isobutane"; //default
    // if(ExperimentConfig::Get().GetExperimentName().Contains("spring8")){
    //     if(fRunNo<1000)gasType = "ethane";
    //     else if(fRunNo<2000)gasType = "isobutane";
    //     else if(fRunNo<3000)gasType = "methane";
    // }
    // if(iter==0){
    //     if(fForceReadConstantDv){
    //         ICDCGas::Get().Initialize(kUseConstantVelocity,xtPath,gasType);
    //     }else{
    //         ICDCGas::Get().Initialize(kUseAvgXtFunction,xtPath,gasType);
    //     }
    // }else{
    //     if(fForceReadConstantDv){
    //         ICDCGas::Get().Initialize(kUseConstantVelocity,xtPath,gasType,true);
    //     }else{
    //         ICDCGas::Get().Initialize(kUseLayPhiBetaXtTable,xtPath,gasType,true);
    //     }
    // }
    // ICDCGas::Get().Ls();

    // // Initialize position resolution manager
    // TString path = Form("%s/Fitting_ResVsDist_r%di%d_0um.root",PAR_DIR,fRunNo,fIteration-1);
    // TString funcName = Form("position_resolution_l%d",10);
    // if(iter==0){
    //     IPosResManager::Get().Initialize(path,funcName);
    // }else{
    //     bool usePosErrFunction = true;
    //     IPosResManager::Get().Initialize(path,funcName,usePosErrFunction);
    // }
}
