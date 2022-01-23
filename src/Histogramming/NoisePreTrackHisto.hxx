#ifndef __NoisePreTrackHisto_is_seen__
#define __NoisePreTrackHisto_is_seen__

#include "PreTrackHisto.hxx"
#include "IRunTimeManager.hxx"
#include "TGraphErrors.h"
#include "TLine.h"
/***
    This class draw the histograms for
    ana_<run number>.root files.
    It draws the layer dependence
***/
class NoisePreTrackHisto: public PreTrackHisto{
    public:
        void Excute(){
            Initialize();
            Fill();
            Draw();
            Finalize();
            Print();
        }

        void Initialize();

        void Fill();

        void Draw();

        void Finalize(){
            fFile->Close();
        }

        void Print(){
            std::cout << "## This is a histogram class for checking noise level" << std::endl;
        }

    private:
        TFile *fFile;
       TTree *fTree;

        Int_t fTriggerNumber;
        Int_t  fNumOfHits;

        Int_t fRunNo;

        Int_t TRIG_CH;
        Int_t MAX_CH;
        Int_t MAX_CH_PER_BOARD;
        Int_t MAX_SENSE_LAYER;
        Int_t NUM_OF_BOARDS_CDC;

        std::vector<int>*fNumOfLayHits;
        std::vector<int>*fNumOfPeaks;
        std::vector<int>*fPeakWidth;
        std::vector<int>*fLayerID;
        std::vector<int>*fCellID;
        std::vector<double>*fX0; //x at z=0
        std::vector<double>*fY0; //y at z=0
        std::vector<double>*fDriftTimeFirstPeak;
        std::vector<double>*fADCFirstPeak;
        std::vector<double>*fPedestal;
        std::vector<double>*fHardBoardID;
        std::vector<double>*fQ;
        std::vector<double>*fT0;
        std::vector<double>*fT0Offset;
        std::vector<double>*fADCCut;
        std::vector<int>*fHitCh;
        TH1DBuilder fHistoPd[104];

        TH2D *fFrameMean;
        TH2D *fFrameRMS;
        TGraphErrors *fGraphMean;
        TGraphErrors *fGraphRMS;
        Double_t fMean[5000];
        Double_t fRMS[5000];
        Double_t fMeanErr[5000];
        Double_t fRMSErr[5000];
        TLine *fBoardSepMean[104];
        TLine *fBoardSepRMS[104];
};



#endif
