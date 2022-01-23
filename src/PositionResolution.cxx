#include "PositionResolution.hxx"
#include "TPaveStats.h"

PositionResolution::~PositionResolution(){
    fFileErr->Close();
}

void PositionResolution::Initialize(bool useOwnFunction){
    fNameFittingFunction = "position_resolution";
    fDefaultIntrSReso    = 0.2; //mm
    fFileErr             = NULL;
    fFittingFunction = NULL;
    fIsUseErrFun = useOwnFunction;
}


void PositionResolution::Initialize(TString path, TString functionName, bool useOwnFunction){
    fNameFittingFunction = functionName;
    fDefaultIntrSReso    = 0.2; //mm
    fFileErr             = NULL;
    fFittingFunction     = NULL;
    fIsUseErrFun = useOwnFunction;
    ReadFunctionFrom(path,fNameFittingFunction);
    if(fFittingFunction)fFittingFunction->SetTitle("position_resolution");
}

void PositionResolution::ReadDefault(){
    // Try to read default function from CDC CRT
    char* WORK_DIR = getenv ("CCWORKING_DIR");
    if(WORK_DIR==NULL){
        cerr << "@@@@ No parameters root file :" << WORK_DIR << endl;
    }
    TString defaultPath = Form("%s/info/positionResolutionDefaultCDCCRT.root",WORK_DIR);
    fFileErr = new TFile(defaultPath);
    if(!fFileErr->IsOpen()){
        std::cerr << "@@@@ Very strange, no default function please find the default function" << std::endl;
        std::cerr << "@@@@ No position resolution root file, error = 200μm" << std::endl;
    }else{
        std::cout << "## Reading the default error function for tracking: " << defaultPath << std::endl;
        fFittingFunction = (TF1*)fFileErr->Get("default");
    }
}


void PositionResolution::ReadFunctionFrom(TString path, TString funcName){
    fFileErr = new TFile(path,"read");
    if(!fFileErr->IsOpen()){
        delete fFileErr;
        ReadDefault();
    }else{
        if((TF1*)fFileErr->Get(funcName)){
            fFittingFunction = (TF1*)fFileErr->Get(funcName);
            std::cout << "## Reading resolution Vs Drift distance function: " << std::endl;
            std::cout << "   ----     " << funcName << std::endl;
            std::cout << "   ----     " << path << std::endl;
            std::cout << "   ----     " << fFittingFunction << std::endl;
        }
        else{
            ReadDefault();
        }
    }
}
