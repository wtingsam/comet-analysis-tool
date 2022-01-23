#ifndef _ANALYSISSTRATEGY_HH_
#define _ANALYSISSTRATEGY_HH_

#include "InputRootCRT.hxx"
#include "ExperimentConfig.hxx"
#include "TH1D.h"
#include "WaveFormAnalyzer.hxx"
#include "DriftTimeAnalyzer.hxx"
#include "IRunTimeManager.hxx"
/***
    A strategy interface class for calling different strategies
    for preTracking procedures
 ***/
class AnalysisStrategy{
    protected:
        WaveFormAnalyzer waveFormAnalyzer;
        DriftTimeAnalyzer driftTimeAnalyzer;
        InputRootCRT *fInput;

        int fForcedMaxEvent;
        bool fIsForceEvent;

        int MAX_CH;
        int MAX_SENSE_LAYER;
        int TRIG_CH;
        int MAX_SAMPLE;
        int NUM_OF_BOARDS_CDC;
        int NUM_OF_BOARDS_TRI;
        bool fTriggerDelay;

        std::vector<double> fCut_q;
        std::vector<double> fCut_p;
        std::vector<double> fTDCRisingEdge;
        std::vector <TH1D*>fHistoFindT0;
        std::vector <TH1D*>fHistoPed;
        // Fitting function
        std::vector <TF1*> fFitT0;

        //output variable
        TFile* fPreRootFile;
        TTree* fPreRootTree;

        Int_t fTriggerNumber; //TriggerNumber
        Int_t fNHit; //# of hits
        Int_t fNHitAll; //# of peaks in total, this includes multiple hits in the same channel
        std::vector<bool>fGoodHitFlag; //Good Hit Flag
        std::vector<bool>fChoosePeakFlag; //Choose TDC peak
        std::vector<bool>fLargfPeakWidthFlag; //First Peak width is large
        std::vector<int>fNHitLayer; //# of layer hits
        std::vector<int>fNPeaks; //# of peaks
        std::vector<int>fPeakWidth; //peak width
        std::vector<int>fHitCh; //Hit channel
        std::vector<int>fLayerID; //LayerID
        std::vector<int>fCellID; //CellID
        std::vector<int>fBoardLayID; //BoardLayerID
        std::vector<int>fBoardLocID; //BoardLocID
        std::vector<int>fHardwareBoardID; //Hardware Board ID
        std::vector<double>fXCentre; //x at z=0
        std::vector<double>fYCentre; //y at z=0
        std::vector<double>fDriftTime; //DriftTime (1st peak)
        std::vector< std::vector<double> > fDriftTAll; // Multiple peaks
        std::vector< std::vector<double> > fADCPeakAll; // Multiple peaks
        std::vector< std::vector<double> > fWaveform; // 32 sampling
        std::vector< std::vector<double> > fWaveformClk; // maximum 32 sampling
        std::vector<int> fWaveform1stClk;
        std::vector<double>fADCpeak; //ADC (1st peak)
        std::vector<double>fPed; //Pedestal
        std::vector<double>fQ; //ADCSUM
        std::vector<double>fT0; //t0
        std::vector<double>fT0offset; //t0 offset
        std::vector<double>fADCcut; //fADCcut

        /// Initialize histogram
        void Init(){
            for(int ch=0;ch<MAX_CH;ch++){
                fHistoPed.push_back(new TH1D(Form("Ped%d",ch),Form("Ped[%d]",ch),50,100,300));;
                fCut_q.push_back(0);
                fCut_p.push_back(0);

            }
            for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
                if(ExperimentConfig::Get().GetExperimentName().Contains("spring8")){
                    fHistoFindT0.push_back(new TH1D(Form("FindT0%d",iBD),Form("FindT0_BD[%d]",iBD),50,-900,-800));
                }else if(ExperimentConfig::Get().GetExperimentName().Contains("crtsetup3")){
                    fHistoFindT0.push_back(new TH1D(Form("FindT0%d",iBD),Form("FindT0_BD[%d]",iBD),200,-200,500));
                }else{
                    fHistoFindT0.push_back(new TH1D(Form("FindT0%d",iBD),Form("FindT0_BD[%d]",iBD),200,-200,500));
                }
                fTDCRisingEdge.push_back(0);
            }
        }

        /// Set save Branch
        void Branch(){
            char* ANADIR = getenv ("CCPRE_ANALYSIS_DIR");
            if(ANADIR==NULL){
                std::cerr << "## No such directory: " << ANADIR << std::endl;
            }
            int fRunNo = IRunTimeManager::Get().GetRunNumber();
            fPreRootFile = new TFile(Form("%s/ana_%d.root",ANADIR,fRunNo),"RECREATE");
            fPreRootTree = new TTree("t","t");

            // Branches
            fPreRootTree->Branch("goodHitFlag",&fGoodHitFlag);
            fPreRootTree->Branch("choosePeakFlag",&fChoosePeakFlag);
            fPreRootTree->Branch("largfPeakWidthFlag",&fLargfPeakWidthFlag);
            fPreRootTree->Branch("triggerNumber",&fTriggerNumber);
            fPreRootTree->Branch("nHitAll",&fNHitAll);
            fPreRootTree->Branch("nHit",&fNHit);
            fPreRootTree->Branch("nHitLayer",&fNHitLayer);
            fPreRootTree->Branch("nPeaks",&fNPeaks);
            fPreRootTree->Branch("peakWidth",&fPeakWidth);
            fPreRootTree->Branch("hitCh",&fHitCh);
            fPreRootTree->Branch("layerID",&fLayerID);
            fPreRootTree->Branch("cellID",&fCellID);
            fPreRootTree->Branch("boardLayID",&fBoardLayID);
            fPreRootTree->Branch("boardLocID",&fBoardLocID);
            fPreRootTree->Branch("hardwareBoardID",&fHardwareBoardID);
            fPreRootTree->Branch("xCentre",&fXCentre);
            fPreRootTree->Branch("yCentre",&fYCentre);
            fPreRootTree->Branch("driftTime",&fDriftTime);
            fPreRootTree->Branch("driftTAll",&fDriftTAll);
            fPreRootTree->Branch("adcPeak",&fADCpeak);
            fPreRootTree->Branch("adcPeakAll",&fADCPeakAll);
            fPreRootTree->Branch("waveform",&fWaveform);
            fPreRootTree->Branch("waveformClk",&fWaveformClk);
            //fPreRootTree->Branch("waveform1stClk",&fWaveform1stClk);
            fPreRootTree->Branch("ped",&fPed);
            fPreRootTree->Branch("q",&fQ);
            fPreRootTree->Branch("t0",&fT0);
            fPreRootTree->Branch("adccut",&fADCcut);
            fPreRootTree->Branch("t0offset",&fT0offset);
        }

        void PushDummyData(){
            //Fill dummy , otherwise when I read the tree, vector out of range occur
            fGoodHitFlag.push_back(false);
            fLargfPeakWidthFlag.push_back(false);
            fChoosePeakFlag.push_back(-1);
            fNHitLayer.push_back(-1);
            fNPeaks.push_back(-1);
            fPeakWidth.push_back(-1);
            fHitCh.push_back(-1);
            fLayerID.push_back(-1);
            fCellID.push_back(-1);
            fXCentre.push_back(-1);
            fYCentre.push_back(-1);
            fBoardLayID.push_back(-1);
            fBoardLocID.push_back(-1);
            fHardwareBoardID.push_back(-1);
            fDriftTime.push_back(-1);
            std::vector<double> empty;
            fDriftTAll.push_back(empty);
            fADCPeakAll.push_back(empty);
            std::vector<double> fix_sized(32);
            fWaveform.push_back(fix_sized);
            fWaveformClk.push_back(fix_sized);
            fWaveform1stClk.push_back(-1);
            fADCpeak.push_back(-1);
            fPed.push_back(-1);
            fQ.push_back(-1);
            fT0.push_back(-1);
            fADCcut.push_back(-1);
        }

        void ClearEvent(){
            fTriggerNumber=0;
            fNHitAll=0;
            fNHit=0;
            fGoodHitFlag.clear();
            fChoosePeakFlag.clear();
            fLargfPeakWidthFlag.clear();
            fNHitLayer.clear();
            fNPeaks.clear();
            fPeakWidth.clear();
            fHitCh.clear();
            fLayerID.clear();
            fCellID.clear();
            fBoardLayID.clear();
            fBoardLocID.clear();
            fHardwareBoardID.clear();
            fXCentre.clear();
            fYCentre.clear();
            fDriftTime.clear();
            fDriftTAll.clear();
            fADCPeakAll.clear();
            fWaveform.clear();
            fWaveformClk.clear();
            fWaveform1stClk.clear();
            fADCpeak.clear();
            fPed.clear();
            fQ.clear();
            fT0.clear();
            fT0offset.clear();
            fADCcut.clear();
        }

    public:
        virtual ~AnalysisStrategy(){}

        /// constructors
        AnalysisStrategy(InputRootCRT *input, bool triggerDelay=false):
            fInput(input),fIsForceEvent(false), fTriggerDelay(triggerDelay)
            {
                MAX_CH            = ExperimentConfig::Get().GetNumOfChannel();
                MAX_SENSE_LAYER   = ExperimentConfig::Get().GetNumOfLayer();
                TRIG_CH           = ExperimentConfig::Get().GetNumOfTrigChannel();
                MAX_SAMPLE        = ExperimentConfig::Get().GetSampleRECBE();
                NUM_OF_BOARDS_CDC = ExperimentConfig::Get().GetNumOfCDCBoard();
                NUM_OF_BOARDS_TRI = ExperimentConfig::Get().GetNumOfTrigBoard();
            }

        /// Force to run number of events
        void ForceMaxEvents(int num_event){
            fForcedMaxEvent = num_event;
            if(num_event>0)
                fIsForceEvent = true;
            std::cout << "## Forcing number of events for getting pre-tracking parameters at "
                      << fForcedMaxEvent
                      << " events" << std::endl;
        }

        /// Excuting the methods
        void Excute(){
            GetPreTrackingParameters();
            PrepareRootFile();
        }


    private:
        /// Method for getting pre-tracking parameters
        virtual         void GetPreTrackingParameters() = 0;

        /// Method for preparing the root file for tracking
        virtual         void PrepareRootFile() = 0;

        /// Method for print out the result
        virtual void Print() = 0;

};


#endif
