//==================================================================
//    This app only did the following things
//    1. Read previous data's track parameters
//    2. Generate randomly gaussian distribution
//    3. Produce root file and fill
//-----------------------------------------------------------------
//** Data format
//
//    Unit: mm/ns
//
//
//-----------------------------------------------------------------
//  Author: Sam Wong
//    Date: 26/01/2017
//==================================================================
#include <stdlib.h>
#include <unistd.h> // for getopt
#include <string>
#include <algorithm>   // for tolower
#include <iostream>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom1.h>
#include <TRandom2.h>
#include <TRandom3.h>

#include "IRunTimeManager.hxx"

#include "InputRootCRT.hxx"
#include "WaveFormAnalyzer.hxx"
#include "EventLoop.hxx"
#include "DriftTimeAnalyzer.hxx"
#include "ITrackingUtility.hxx"

//Global
#define MAX_CH 96  // The max should be 4986
#define MAX_SAMPLING 32

using namespace std;

template <typename T>
bool
my_isnan(const T x)
{
#if __cplusplus >= 201103L
    using std::isnan;
#endif
    return isnan(x);
}

class MyAnalysisCode{
    private :
        // Class
        bool fUseZero;
        bool fUseUniformRes;
        // Config variable
        int TRIG_CH ;
        int MAX_SENSE_LAYER;
        int MAX_CELL;
        // Run info_
        Int_t fFileID;
        Int_t fRunNo;
        TString fRootFileName;
        // Variables
        TRandom1  fRandomSeed1;
        TRandom3  fRandomSeed2;
        int fNum_of_Event;
        int fPrintLevel;
        // Inputs
        int fTriggerNumber;
        int fNumOfHits;
        std::vector<bool>fGoodHitFlag;
        std::vector<bool>fChoosePeakFlag;
        std::vector<int>fNumOfLayHits;
        std::vector<int>fNumOfPeaks;
        std::vector<int>fPeakWidth;
        std::vector<int>fLayerID;
        std::vector<int>fCellID;
        std::vector<double>fDriftTime1Peak;
        std::vector< std::vector<double> > fDriftTimeAll;
        std::vector< std::vector<double> > fWaveform_in;
        std::vector< std::vector<double> > fWaveformClk_in;
        std::vector<double>fTestLayerDriftTAll; //all drift times of test layers
        std::vector<double>fADCFirstPeak;
        std::vector<double>fPedestal;
        std::vector<double>fQ;
        std::vector<double>fT0;
        std::vector<double>fT0Offset;
        std::vector<double>fADCCut;
        TFile *fFile;
        TTree *fTree;
        // mc outoput
        std::vector<double>fMCDriftDistance;
        std::vector<double>fMCDriftDistanceSmear;
        std::vector<double>fMCtrackPar;
        double fDriftVelocity;
        double fDistance_cut;
        int fNumberHitCuts;
        double fSigSmear;

        TString fSmearFunctionPath;
        // Histogram

        // Graph

        // Canvas

        // Function
        std::vector<TF1*> fAllSigFuns;

        /// Get the intrinsic error for smearing
        double GetIntrinsicResolution(double x){
            if(fAllSigFuns[0])
                if(x<8)
                    return fAllSigFuns[0]->Eval(x);
                else
                    return 0.2;
        }
    public:
        // Constructor
        MyAnalysisCode(Int_t runNo)
            :fUseZero(false),fUseUniformRes(false),
             fRootFileName("sim"),
             fFile(NULL),fTree(NULL),
             fNumberHitCuts(0),fSigSmear(0.2)
        {
            fRunNo=runNo;
            MAX_SENSE_LAYER    = ExperimentConfig::Get().GetNumOfLayer();
            MAX_CELL          = ExperimentConfig::Get().GetNumOfWirePerLayer();
            TRIG_CH           = ExperimentConfig::Get().GetNumOfTrigChannel();
        }
        // Distructor
        virtual ~MyAnalysisCode(){}

        bool ReadAllFunctions(TString dataResult){
            /// Read final data result as the first input function
            /// If not just read the default
            TString posResName = "position_resolution_l4";
            TFile *tmpFile;
            if(dataResult!=""){
                tmpFile = new TFile(dataResult);
                if(tmpFile->IsOpen()){
                    TF1 *tmpTF1;
                    if(tmpFile->Get(posResName))
                        tmpTF1 = (TF1*) tmpFile->Get(posResName);
                    fAllSigFuns.push_back(tmpTF1);
                }else{
                    return false;
                }
            }else{
                fUseUniformRes=true;
                printf("@@@@ WARNING , use no smearing \n");
            }
            return true;
        }


        void SetPrintLevel(int printLevel){  fPrintLevel = printLevel; }

        void SetOutputName(TString name){   fRootFileName = name;       }

        void SetNumberEvent(int n){
            fNum_of_Event = n;
            if(n==0){
                fNum_of_Event = 1000;
            }
        }

        void SetSeed(int i){
            fFileID = i;
            fRandomSeed1.SetSeed(i);
            fRandomSeed2.SetSeed(i);
        }

        void ClearEvent(){
            fGoodHitFlag.clear();
            fChoosePeakFlag.clear();
            fPeakWidth.clear(); //peak width
            fLayerID.clear(); //LayerID
            fCellID.clear(); //CellID
            fDriftTime1Peak.clear(); //DriftTime (1st peak)
            fDriftTimeAll.clear();
            fWaveform_in.clear();
            fWaveformClk_in.clear();

            fMCDriftDistance.clear();
            fMCDriftDistanceSmear.clear();
            fMCtrackPar.clear();
            fPedestal.clear(); //Pedestal
            fQ.clear(); //ADCSUM
            fT0.clear(); //t0
            fT0Offset.clear(); //t0 offset
            fADCCut.clear(); //fADCcuta
            fNumOfLayHits.clear();
            fNumOfHits=0;
            fTriggerNumber = 0;
        }

        void Initialize(TString smearFunctionPath=""){
            char* SIM_DIR = getenv ("CCSIMROOT_DIR");
            // Initial the resolution function to improve the simulation
            // Adopted this one as the intrinsic position resolution
            std::cout << "fPrintLevel " << fPrintLevel  << std::endl;

            fFile = new TFile(Form("%s/",SIM_DIR) + fRootFileName + Form("_%d_%d.root",fFileID,fRunNo),"RECREATE");
            if(!fFile){
                std::cerr<<"@@@@ No root created " <<std::endl;
            }
            std::cout << "## The root file is created at " <<
                Form("%s/",SIM_DIR) + fRootFileName + Form("_%d_%d.root",fFileID,fRunNo) << std::endl;

            // Set Branch
            fTree = new TTree("t","pre-track-sim");
            fTree->Branch("goodHitFlag", &fGoodHitFlag);
            fTree->Branch("choosePeakFlag", &fChoosePeakFlag);
            fTree->Branch("triggerNumber", &fTriggerNumber);
            fTree->Branch("nHit", &fNumOfHits);
            fTree->Branch("nHitLayer", &fNumOfLayHits);
            fTree->Branch("nPeaks", &fNumOfPeaks);
            fTree->Branch("peakWidth", &fPeakWidth);
            fTree->Branch("layerID", &fLayerID);
            fTree->Branch("cellID", &fCellID);
            fTree->Branch("driftTime", &fDriftTime1Peak);
            fTree->Branch("driftTAll", &fDriftTimeAll);
            fTree->Branch("waveform",&fWaveform_in);
            fTree->Branch("waveformClk",&fWaveformClk_in);

            fTree->Branch("adcPeak", &fADCFirstPeak);
            fTree->Branch("q", &fQ);
            fTree->Branch("t0", &fT0);
            fTree->Branch("adccut", &fADCCut);
            fTree->Branch("t0offset", &fT0Offset);
            fTree->Branch("ped", &fPedestal);
            // mc
            fTree->Branch("trackPar_mc",&fMCtrackPar);
            fTree->Branch("driftD_mc",&fMCDriftDistance);
            fTree->Branch("driftD_smear_mc",&fMCDriftDistanceSmear);

            fNum_of_Event = 1000;
            fDistance_cut = 10;
            //fDriftVelocity = ICDCGas::Get().GetDriftVelocity();
            fDriftVelocity = 0.023;
            printf("fDriftVelocity %f \n",fDriftVelocity);
            printf("Smear function %s \n",smearFunctionPath.Data());
        }

        void InitTrackPar(double *par, TString opt="uniform"){
            TString exp=ExperimentConfig::Get().GetExperimentName();
            // These parameters are roughly the one we got from the data
            double scin_x_u(0),scin_y_u(0),scin_z_u(0);
            double scin_x_d(0),scin_y_d(0),scin_z_d(0);
            if(exp.Contains("crtsetup")){
            }else if(exp.Contains("spring8")){
                if(opt.Contains("uniform")){
                    scin_x_u=600;
                    scin_x_d=500;
                    scin_y_u=fRandomSeed1.Uniform(-30,30);
                    scin_y_d=fRandomSeed1.Uniform(-30,30);
                    scin_z_u=fRandomSeed1.Uniform(-100,100);
                    scin_z_d=fRandomSeed1.Uniform(-100,100);
                }else{

                }
                // Imagine the tracks is always going from left to right
                // y = par[0]*x + par[1]
                // z= par[2]*y + par[3]
                par[0] = (scin_y_u-scin_y_d)/(scin_x_u-scin_x_d);
                par[1] = scin_y_u - par[0]*scin_x_u;
                par[2] = (scin_z_u-scin_z_d)/(scin_x_u-scin_x_d);
                par[3] = scin_z_u - par[0]*scin_x_u;
            }
        }

        void Analyze(){
            int event_saved_counter = 0;
            // Start event loop
            for(int iev=0;iev<fNum_of_Event;iev++){
                ClearEvent();
                if(fPrintLevel>1)
                    if(iev%1000==0){ printf("Progress : %f \n",iev*100./fNum_of_Event); }

                // Initialize the parameters
                double par[4];
                InitTrackPar(par);
                fMCtrackPar.push_back(par[0]);
                fMCtrackPar.push_back(par[1]);
                fMCtrackPar.push_back(par[2]);
                fMCtrackPar.push_back(par[3]);
                if(fPrintLevel>10) printf("%d iev.%d %.05f %.05f %.05f %.05f \n",fPrintLevel,iev,par[0],par[1],par[2],par[3]);
                int min_index[MAX_SENSE_LAYER];
                double r_min_index[MAX_SENSE_LAYER];
                double driftD[MAX_SENSE_LAYER][MAX_CELL];
                double driftDHitFlag[MAX_SENSE_LAYER][MAX_CELL];
                for(int iLayer=0;iLayer<MAX_SENSE_LAYER;iLayer++){
                    min_index[iLayer]=-1;
                    r_min_index[iLayer]=-1;
                    double min_r = 1e5;
                    for(int iCell=0;iCell<WireManager::Get().GetNumUsedSenseWire(iLayer);iCell++){
                        // Calculate drift distance
                        double tmp_D=ITrackingUtility::GetFitDistance(iLayer,iCell,par);
                        if(my_isnan(tmp_D))continue;
                        driftD[iLayer][iCell]=tmp_D;
                        double xt,yt,zt;
                        double xw,yw,zw;
                        ITrackingUtility::GetHitPosition(iLayer,iCell,xt,yt,zt,par,"track");
                        ITrackingUtility::GetHitPosition(iLayer,iCell,xw,yw,zw,par,"wire");

                        // Select only smallest one
                        if(fabs(tmp_D)<fabs(min_r)){
                            min_r               = driftD[iLayer][iCell];
                            min_index[iLayer]   = iCell;
                            r_min_index[iLayer] = min_r;
                        }
                        if(fPrintLevel>100) printf("[%d][%d] D_true %f Track:(%f %f %f) Wire:(%f %f %f )\n",iLayer,iCell,tmp_D, xt,yt,zt,xw,yw,zw );
                    }
                }
                // Fill in hit flags
                int numberOfHit=0;

                for(int iLayer=0;iLayer<MAX_SENSE_LAYER;iLayer++){
                    for(int iCell=0;iCell<WireManager::Get().GetNumUsedSenseWire(iLayer);iCell++){
                        if(min_index[iLayer]==iCell && iLayer!=0){
                            if(fabs(driftD[iLayer][iCell])<10){
                                driftDHitFlag[iLayer][iCell] = true;
                                numberOfHit++;
                            }else{
                                driftDHitFlag[iLayer][iCell] = false;
                            }
                        }else{
                            driftDHitFlag[iLayer][iCell] = false;
                        }
                    }
                }


                // Start smearing
                double driftDSmear[MAX_SENSE_LAYER][MAX_CELL];
                double R_SUM = 0 ;
                for(int iLayer=0;iLayer<MAX_SENSE_LAYER;iLayer++){
                    for(int iCell=0;iCell<WireManager::Get().GetNumUsedSenseWire(iLayer);iCell++){
                        if(driftDHitFlag[iLayer][iCell]){
                            double R_sig = fSigSmear;
                            double d_tmp = driftD[iLayer][iCell];
                            if(fPrintLevel>5)printf("---   ");
                            if(!fUseZero){
                                if(!fUseUniformRes){
                                    R_sig = GetIntrinsicResolution(fabs(driftD[iLayer][iCell]));// smear depending on distance
                                    driftDSmear[iLayer][iCell] = fRandomSeed2.Gaus(d_tmp,R_sig);
                                }else{
                                    driftDSmear[iLayer][iCell] = fRandomSeed2.Gaus(d_tmp,R_sig);
                                }
                            }else
                                driftDSmear[iLayer][iCell] = driftD[iLayer][iCell];
                            if(fPrintLevel>5) printf("iev.%d [%3d][%3d] true_D %.03f smear_D %.03f with %.03f \n"
                                                     ,iev,iLayer,iCell,driftD[iLayer][iCell],driftDSmear[iLayer][iCell], R_sig);
                            R_SUM+=abs(driftDSmear[iLayer][iCell]);
                        }
                    }
                }

                // FillData
                fMCtrackPar.push_back(par[0]);
                fMCtrackPar.push_back(par[1]);
                fMCtrackPar.push_back(par[2]);
                fMCtrackPar.push_back(par[3]);

                for(int iLayer=0;iLayer<MAX_SENSE_LAYER;iLayer++){
                    for(int iCell=0;iCell<WireManager::Get().GetNumUsedSenseWire(iLayer);iCell++){
                        if(driftDHitFlag[iLayer][iCell]){
                            fGoodHitFlag.push_back(1);
                            fChoosePeakFlag.push_back(0);
                            fNumOfPeaks.push_back(1);
                            fPeakWidth.push_back(1); //peak width
                            fLayerID.push_back(iLayer); //LayerID
                            fCellID.push_back(iCell); //CellID
                            double dt = driftDSmear[iLayer][iCell]/fDriftVelocity;
                            //double dt = driftD[iLayer][iCell]/fDriftVelocity;
                            fDriftTime1Peak.push_back(fabs(dt)); //DriftTime (1st peak)
                            fMCDriftDistance.push_back(driftD[iLayer][iCell]);
                            fMCDriftDistanceSmear.push_back(driftDSmear[iLayer][iCell]);
                            std::vector<double> dt_peaks;
                            dt_peaks.push_back(fabs(dt));
                            fDriftTimeAll.push_back(dt_peaks);
                            fWaveform_in.push_back(dt_peaks);
                            fWaveformClk_in.push_back(dt_peaks);
                            fPedestal.push_back(500); //Pedestal
                            fADCFirstPeak.push_back(500);
                            fQ.push_back(500); //ADCSUM
                            fT0.push_back(0); //t0
                            fT0Offset.push_back(0); //t0 offset
                            fADCCut.push_back(10); //fADCcuta
                            fNumOfLayHits.push_back(1);
                            fNumOfHits=numberOfHit;
                            fTriggerNumber = iev;
                        }
                    }
                }
                // Save event
                if(fPrintLevel>5){
                    printf("Rsum %f fDistance_cut*MAX_SENSE_LAYER %f numberOfHit %d  fNumberHitCuts %d\n",
                           R_SUM,fDistance_cut*MAX_SENSE_LAYER,numberOfHit,fNumberHitCuts);
                }
                if(numberOfHit>=fNumberHitCuts){
                    event_saved_counter ++ ;
                    fTree->Fill();
                }
            }
            printf("Total number of events filled %d \n",event_saved_counter);
        }

        void Finalize(){
            fTree->Write();
            fFile->Close();
        }

};

int usage(char* prog_name)
{
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n\n");
    fprintf(stderr,"[How is works]\n");
    fprintf(stderr,"1. Generate tracks smeared with gaussian distribution (Hard coded)\n");
    fprintf(stderr,"2. Find hits \n");
    fprintf(stderr,"3. Smear hits with resolution function σ(r) \n");
    fprintf(stderr,"4. Find tracks that passes through almost all layers \n");
    fprintf(stderr,"5. Fill tree \n\n");
    fprintf(stderr,"=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@=@\n");
    fprintf(stderr,"Usage %s [options (args)] [run number]\n",prog_name);
    fprintf(stderr,"[options]\n");
    fprintf(stderr,"\t -e\n");
    fprintf(stderr,"\t\t choose experiment:[crtsetup2(default), crtsetup1]\n");
    fprintf(stderr,"\t -c\n");
    fprintf(stderr,"\t\t choose configure file for experiments: [experiemnt-config.txt(default)]\n");
    fprintf(stderr,"\t -n\n");
    fprintf(stderr,"\t\t set maximum event number [0 as default (no limit)]\n");
    fprintf(stderr,"\t -p\n");
    fprintf(stderr,"\t\t set event level information print modulo [1000 as default]\n");
    fprintf(stderr,"\t -s\n");
    fprintf(stderr,"\t\t set stamp of the simulation output\n");
    fprintf(stderr,"\t -S\n");
    fprintf(stderr,"\t\t set smearing function name from the data result\n");
    fprintf(stderr,"\t -t\n");
    fprintf(stderr,"\t\t set seed number of the random generator for simulation \n");
    fprintf(stderr,"[example]\n");
    fprintf(stderr,"\t\t You probably wanna start run number from 1000x , so that you do not mess up with your data run number \n");
    fprintf(stderr,"\t\t%s 61 -e crtsetup2 -n 1000 -p 10 -c experiemnt-config.txt -s test -S Fitting_ResVsDist_r61i20.root \n",prog_name);
    return 0;
}

int main(int argc, char* argv[]){
    // set run options
    std::string fExperiment = "crtsetup2";
    std::string fConfigure = "experiemnt-config.txt";
    Long64_t fPrintModulo = 0; // Print event level information every fPrintModulo events
    Long64_t fNumEvents = 0; // Maximum  number of events to be processed. 0 means no limit;
    std::string fSimVersion = "sim";
    TString fRDCASmearFunctionPath = "";
    int seedNumber = 0;
    int fRunNo = 0;
    int optquery;
    while((optquery=getopt(argc,argv,"he:p:c:s:S:t:r:n:"))!=-1){
        printf("##   ");
        switch(optquery){
        case 'p':
            fPrintModulo = atol(optarg);
            printf("Set event print modulo to : %lld\n",fPrintModulo);
            break;
        case 'e':
            fExperiment = optarg;
            std::transform(fExperiment.begin(),fExperiment.end(),fExperiment.begin(),::tolower);
            printf("Set experiment to : %s\n",fExperiment.c_str());
            break;
        case 'c':
            fConfigure = optarg;
            printf("Set configure file to : %s\n",fConfigure.c_str());
            break;
        case 's':
            fSimVersion = optarg;
            printf("Set stamp of the simulation : %s\n",fSimVersion.c_str());
            break;
        case 'S':
            fRDCASmearFunctionPath = optarg;
            printf("Set smearing function path : %s \n",fRDCASmearFunctionPath.Data());
            break;
        case 't':
            seedNumber = atoi(optarg);
            printf("Set seed number : %d\n",seedNumber);
            break;
        case 'r':
            fRunNo = atoi(optarg);
            printf("Set run number to : %d\n",fRunNo);
            break;
        case 'n':
            fNumEvents = atol(optarg);
            printf("Set maximum event number to : %lld\n",fNumEvents);
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
        default:
            usage(argv[0]);
            return 1;
        }
    }
    if(optind<2){
        usage(argv[0]);
        return 1;
    }
    if(fRDCASmearFunctionPath==""){
        std::cerr << "@@@@ No input data result for smearing the drift distance" << std::endl;
    }else{
        std::cerr << "## Use zero smearing " << std::endl;
    }
    // Initialize
    IRunTimeManager::Get().Initialize(fRunNo,fConfigure, fExperiment);

    MyAnalysisCode  *userCode = new MyAnalysisCode(fRunNo);
    userCode->SetSeed(seedNumber);
    userCode->SetOutputName(fSimVersion);
    userCode->SetPrintLevel(fPrintModulo);
    if(!userCode->ReadAllFunctions(fRDCASmearFunctionPath)){
        std::cerr << "@@@@ Error, Try to load " << fRDCASmearFunctionPath << " , but failed" << std::endl;
        //return 0;
    }
    userCode->Initialize("");
    userCode->SetNumberEvent(fNumEvents);
    userCode->Analyze();
    userCode->Finalize();
    return 0;
}
