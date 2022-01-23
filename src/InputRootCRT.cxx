#include "InputRootCRT.hxx"
#include "ExperimentConfig.hxx"

InputRootCRT::InputRootCRT(TString path, TString option, TString tree, bool useSimData)
    :fTreeName("tree"),fOption("read"),fFile(NULL),fTree(NULL)
{
    fManSet=false;
    fUseSim=useSimData;
    SetPath(path);
    SetTree(tree);
    SetOption(option);
    printf("## Initializing the input root file\n");
    Initialize();
}

InputRootCRT::~InputRootCRT(){
    fFile->Close();
}

void InputRootCRT::Initialize(void)
{
    fFile = new TFile(fRootFilePath,fOption);
    fTree = new TTree();
    fTree = (TTree*)fFile->Get(fTreeName);

    fTree->SetBranchAddress("triggerNumber",&fTriggerNumber);
    fTree->SetBranchAddress("tdcNhit",&fTDCNhit);
    fTree->SetBranchAddress("adc",&fADC);
    fTree->SetBranchAddress("driftTime",&fTDC);
    fTree->SetBranchAddress("clockNumberDriftTime",&fClkHit);
    if(fUseSim){
        fTree->SetBranchAddress("driftTime_smear",&fDriftTime_sim);
        printf("## Using simulation data, mc smeared drift Time is available in the branch now\n");
    }
}
