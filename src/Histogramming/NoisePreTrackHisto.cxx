#include "NoisePreTrackHisto.hxx"
#include "TStyle.h"

void NoisePreTrackHisto::Initialize(){
    fNumOfLayHits=0;
    fNumOfPeaks=0;
    fPeakWidth=0;
    fLayerID=0;
    fCellID=0;
    fX0=0; //x at z=0
    fY0=0; //y at z=0
    fDriftTimeFirstPeak=0;
    fADCFirstPeak=0;
    fPedestal=0;
    fHardBoardID=0;
    fQ=0;
    fT0=0;
    fT0Offset=0;
    fADCCut=0;

    fRunNo      = IRunTimeManager::Get().GetRunNumber();

    TRIG_CH           = ExperimentConfig::Get().GetNumOfTrigChannel();
    MAX_CH            = ExperimentConfig::Get().GetNumOfChannel();
    MAX_SENSE_LAYER   = ExperimentConfig::Get().GetNumOfLayer();
    NUM_OF_BOARDS_CDC = ExperimentConfig::Get().GetNumOfCDCBoard();
    MAX_CH_PER_BOARD  = int(MAX_CH/NUM_OF_BOARDS_CDC);

    //Create root file
    char* ANADIR = getenv ("CCPRE_ANALYSIS_DIR");
    if(ANADIR==NULL){
        std::cerr << "## No input root file :" << ANADIR << std::endl;
        return ;
    }

    fFile = TFile::Open(Form("%s/ana_%d.root",ANADIR,fRunNo));
    if(!fFile->IsOpen()){
        std::cerr << "## Do not have root file" << std::endl;
        return ;
    }
    fTree = (TTree*)fFile->Get("t");

    //Setbranch
    fTree->SetBranchAddress("triggerNumber", &fTriggerNumber);
    fTree->SetBranchAddress("nHit", &fNumOfHits);
    fTree->SetBranchAddress("nHitLayer", &fNumOfLayHits);
    fTree->SetBranchAddress("nPeaks", &fNumOfPeaks);
    fTree->SetBranchAddress("peakWidth", &fPeakWidth);
    fTree->SetBranchAddress("hitCh", &fHitCh);
    fTree->SetBranchAddress("layerID", &fLayerID);
    fTree->SetBranchAddress("cellID", &fCellID);
    fTree->SetBranchAddress("driftTime", &fDriftTimeFirstPeak);
    fTree->SetBranchAddress("adcPeak", &fADCFirstPeak);
    fTree->SetBranchAddress("q", &fQ);
    fTree->SetBranchAddress("t0", &fT0);
    fTree->SetBranchAddress("adccut", &fADCCut);
    fTree->SetBranchAddress("t0offset", &fT0Offset);
    fTree->SetBranchAddress("ped", &fPedestal);
    fTree->SetBranchAddress("hardwareBoardID", &fHardBoardID);

    double pd_bin = 100;
    double pd_min = 200;
    double pd_max = 300;

    // Make TH1D
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        this->fHistoPd[iBD]=TH1DBuilder(Form("Pedestal_board%d_%d",iBD,fRunNo),MAX_CH_PER_BOARD,true);
        fHistoPd[iBD].ForceColor(1);
        fHistoPd[iBD].MakeTH1Ds(Form("pdb%d",iBD),Form("Pedestal board %d ch",iBD),pd_bin,pd_min,pd_max,"pedestal",1,0);
    }

    // Make TGraphErrors
    fGraphMean = new TGraphErrors(MAX_CH);
    fGraphRMS  = new TGraphErrors(MAX_CH);
}

void NoisePreTrackHisto::Fill(){
    for(Int_t iev=0;iev<fTree->GetEntries();iev++){
        fTree->GetEntry(iev);
        for(Int_t iHit=0;iHit<fNumOfHits;iHit++){// Hit loop
            int board=fHardBoardID->at(iHit);
            if(board<0)continue;
            double pd = fPedestal->at(iHit);
            int bdCh = fHitCh->at(iHit)%MAX_CH_PER_BOARD;
            fHistoPd[board].Fill(bdCh,pd);
        }
    }

    double maxRMS=-1e5;
    double minRMS=1e5;
    double maxMean=-1e5;
    double minMean=1e5;

    for(int iCH=TRIG_CH;iCH<MAX_CH;iCH++){
        int bdID = iCH/MAX_CH_PER_BOARD;
        int ch = iCH%MAX_CH_PER_BOARD;

        fMean[iCH]    =         fHistoPd[bdID].GetHisto(ch)->GetMean();
        fMeanErr[iCH] =         fHistoPd[bdID].GetHisto(ch)->GetMeanError();
        fGraphMean->SetPoint(iCH,(double)iCH,fMean[iCH]);
        fGraphMean->SetPointError(iCH,0,fMeanErr[iCH]);
        if(fMean[iCH]>maxMean) maxMean = fMean[iCH];
        if(fMean[iCH]<minMean) minMean = fMean[iCH];

        fRMS[iCH]     =         fHistoPd[bdID].GetHisto(ch)->GetRMS();
        fRMSErr[iCH]  =         fHistoPd[bdID].GetHisto(ch)->GetRMSError();
        fGraphRMS->SetPoint(iCH,(double)iCH,fRMS[iCH]);
        fGraphRMS->SetPointError(iCH,0,fRMSErr[iCH]);
        if(fRMS[iCH]>maxRMS) maxRMS = fRMS[iCH];
        if(fRMS[iCH]<minRMS) minRMS = fRMS[iCH];
    }
    // build frame
    fFrameMean = new TH2D("frameMean","Mean VS channel",100,TRIG_CH,MAX_CH+1,100,minMean-5,maxMean+5);
    fFrameRMS = new TH2D("frameRMS","RMS VS channel",100,TRIG_CH,MAX_CH+1,100,minRMS-5,maxRMS+5);
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        fBoardSepMean[iBD] = new TLine((iBD+1)*MAX_CH_PER_BOARD,minMean-5,(iBD+1)*MAX_CH_PER_BOARD,maxMean+5);
        fBoardSepRMS[iBD] = new TLine((iBD+1)*MAX_CH_PER_BOARD,minRMS-5,(iBD+1)*MAX_CH_PER_BOARD,maxRMS+5);
    }
}

void NoisePreTrackHisto::Draw(){
    // Draw TH1D
    TString op="";
    bool isLogX=false;
    bool isLogY=false;
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        fHistoPd[iBD].Draw(op,isLogX,isLogY);
    }
    TCanvas *canvas = new TCanvas("c","canvas",720,512);
    gStyle->SetOptStat(0);
    canvas->Divide(1,2);
    canvas->cd(1);
    fFrameMean->Draw();
    fGraphMean->Draw("same p");
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        fBoardSepMean[iBD] -> SetLineColor(2);
        fBoardSepMean[iBD] -> Draw("same");
    }

    canvas->cd(2);
    fFrameRMS->Draw();
    fGraphRMS->Draw("same p");
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        fBoardSepRMS[iBD] -> SetLineColor(2);
        fBoardSepRMS[iBD] -> Draw("same");
    }

    char *DIR = getenv ("CCGRAPH_DIR");
    canvas->SaveAs(Form("%s/",DIR)+fHistoPd[0].GetHeader()+"_MeanAndRMS.pdf");
}
