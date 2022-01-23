#ifndef __BoardDepPreTrackHisto_is_seen__
#define __BoardDepPreTrackHisto_is_seen__

#include "PreTrackHisto.hxx"
#include "IRunTimeManager.hxx"

/***
    This class draw the histograms for
    ana_<run number>.root files.
    It draws the layer dependence
***/
class BoardDepPreTrackHisto: public PreTrackHisto{
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
            std::cout << "## This is a Board dependent histogram class " << std::endl;
        }

    private:
        TFile *fFile;
        TTree *fTree;

        Int_t fTriggerNumber;
        Int_t  fNumOfHits;

        Int_t fRunNo;
        Int_t fTestLayer;
        Int_t fIter;

        Int_t TRIG_CH;
        Int_t MAX_CH;
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
        std::vector<std::vector<double> >*fWaveform;
        std::vector<int>*fWaveform1stClk;

        TH1DBuilder fHistoDt;
        TH1DBuilder fHistoDr;
        TH1DBuilder fHistoAp;
        TH1DBuilder fHistoAs;
        TH1DBuilder fHistoPd;
        TH1DBuilder fHistoPw;
        TH1DBuilder fHistoNp;
        TH1DBuilder fHistoLh;
        TH1DBuilder fHistoCe;
        // Draw TH2D
        TH2DBuilder fHistoWaves;

        TH2DBuilder fHistoDtVAs;
        TH2DBuilder fHistoDtVAp;
        TH2DBuilder fHistoDtVPd;
        TH2DBuilder fHistoDtVPw;

        TH2DBuilder fHistoApVAs;
        TH2DBuilder fHistoApVPd;
        TH2DBuilder fHistoApVPw;
        TH2DBuilder fHistoApVNp;

        TH2DBuilder fHistoAsVPd;
        TH2DBuilder fHistoAsVPw;
        TH2DBuilder fHistoAsVNp;

        TH2DBuilder fHistoEnVAs;
        TH2DBuilder fHistoEnVAp;
        TH2DBuilder fHistoEnVPd;
        TH2DBuilder fHistoEnVPw;
        TH2DBuilder fHistoEnVDt;
        TH2DBuilder fHistoEnVNp;
};



#endif
