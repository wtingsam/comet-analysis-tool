#include "BoardDepPreTrackHisto.hxx"

void BoardDepPreTrackHisto::Initialize(){
    fRunNo      = IRunTimeManager::Get().GetRunNumber();
    fTestLayer  = IRunTimeManager::Get().GetTestLayer();
    fIter       = IRunTimeManager::Get().GetIteration();

    TRIG_CH           = ExperimentConfig::Get().GetNumOfTrigChannel();
    MAX_CH            = ExperimentConfig::Get().GetNumOfChannel();
    MAX_SENSE_LAYER       = ExperimentConfig::Get().GetNumOfLayer();
    NUM_OF_BOARDS_CDC = ExperimentConfig::Get().GetNumOfCDCBoard();

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

    fNumOfLayHits=0;
    fNumOfPeaks=0;
    fPeakWidth=0;
    fLayerID=0;
    fCellID=0;
    fX0=0;
    fY0=0;
    fDriftTimeFirstPeak=0;
    fADCFirstPeak=0;
    fPedestal=0;
    fHardBoardID=0;
    fQ=0;
    fT0=0;
    fT0Offset=0;
    fADCCut=0;
    fWaveform=0;
    fWaveform1stClk=0;
    fTree = (TTree*)fFile->Get("t");
    //Setbranch
    fTree->SetBranchAddress("triggerNumber", &fTriggerNumber);
    fTree->SetBranchAddress("nHit", &fNumOfHits);
    fTree->SetBranchAddress("nHitLayer", &fNumOfLayHits);
    fTree->SetBranchAddress("nPeaks", &fNumOfPeaks);
    fTree->SetBranchAddress("peakWidth", &fPeakWidth);
    fTree->SetBranchAddress("layerID", &fLayerID);
    fTree->SetBranchAddress("cellID", &fCellID);
    fTree->SetBranchAddress("driftTime", &fDriftTimeFirstPeak);
    fTree->SetBranchAddress("adcPeak", &fADCFirstPeak);
    fTree->SetBranchAddress("waveform", &fWaveform);
    fTree->SetBranchAddress("waveform1stClk", &fWaveform1stClk);
    fTree->SetBranchAddress("q", &fQ);
    fTree->SetBranchAddress("t0", &fT0);
    fTree->SetBranchAddress("adccut", &fADCCut);
    fTree->SetBranchAddress("t0offset", &fT0Offset);
    fTree->SetBranchAddress("ped", &fPedestal);
    fTree->SetBranchAddress("hardwareBoardID", &fHardBoardID);

    double dt_bin = 100;
    double dt_min = -400;
    double dt_max = 700;

    double dr_bin = 100;
    double dr_min = 0;
    double dr_max = 10;

    double ap_bin = 100;
    double ap_min = 150;
    double ap_max = 850;

    double as_bin = 100;
    double as_min = 0;
    double as_max = 1500;

    double pd_bin = 100;
    double pd_min = 200;
    double pd_max = 300;

    double pw_bin = 32;
    double pw_min = 0;
    double pw_max = 31;

    double np_bin = 32;
    double np_min = 0;
    double np_max = 31;

    double lh_bin = 31;
    double lh_min = 0;
    double lh_max = 30;

    double ce_bin = 100;
    double ce_min = 0;
    double ce_max = 30;

    double en_bin = 100;
    double en_min = 0;
    double en_max = fTree->GetEntries();

    // Make TH1D
    this->fHistoDt=TH1DBuilder(Form("driftTime_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoDr=TH1DBuilder(Form("measuredDriftR_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoAp=TH1DBuilder(Form("ADCFP_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoAs=TH1DBuilder(Form("ADCSum_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoPd=TH1DBuilder(Form("Pedestal_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoPw=TH1DBuilder(Form("PeakWidth_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoNp=TH1DBuilder(Form("NumOfPeak_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoLh=TH1DBuilder(Form("NumOfHits_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);
    this->fHistoCe=TH1DBuilder(Form("CellID_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC,true);

    fHistoDt.MakeTH1Ds("dtb","Drift time board",dt_bin,dt_min,dt_max,"driftTime[ns]",1,0);
    fHistoDr.MakeTH1Ds("drb","Drift distance board",dr_bin,dr_min,dr_max,"measured R[mm]",1,0);
    fHistoAp.MakeTH1Ds("apb","Peak's ADC board",ap_bin,ap_min,ap_max,"Peak's ADC",1,0);
    fHistoAs.MakeTH1Ds("asb","Sum of ADC board",as_bin,as_min,as_max,"Sum of ADC",1,0);
    fHistoPd.MakeTH1Ds("pdb","Pedestal board",pd_bin,pd_min,pd_max,"pedestal",1,0);
    fHistoPw.MakeTH1Ds("pwb","Peak width board",pw_bin,pw_min,pw_max,"peak width",1,0);
    fHistoNp.MakeTH1Ds("npb","Number of peaks board",np_bin,np_min,np_max,"number of peaks",1,0);
    fHistoLh.MakeTH1Ds("lhb","Number of layer hits board",lh_bin,lh_min,lh_max,"number of hits",1,0);
    fHistoCe.MakeTH1Ds("chb","Channel board",ce_bin,ce_min,ce_max,"cell ID",1,0);

    // TH2D
    this->fHistoWaves=TH2DBuilder(Form("waveform_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoDtVAs=TH2DBuilder(Form("driftTime_adcsum_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoDtVAp=TH2DBuilder(Form("driftTime_adcpeak_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoDtVPd=TH2DBuilder(Form("driftTime_pedestal_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoDtVPw=TH2DBuilder(Form("driftTime_peakwidth_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoApVAs=TH2DBuilder(Form("adcpeak_adcsum_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoApVPd=TH2DBuilder(Form("adcpeak_pedestal_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoApVPw=TH2DBuilder(Form("adcpeak_peakwidth_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoApVNp=TH2DBuilder(Form("adcpeak_npeaks_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoAsVPd=TH2DBuilder(Form("adcsum_pedestal_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoAsVPw=TH2DBuilder(Form("adcsum_sumwidth_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoAsVNp=TH2DBuilder(Form("adcsum_nsums_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoEnVAs=TH2DBuilder(Form("Entry_adcsum_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoEnVAp=TH2DBuilder(Form("Entry_adcpeak_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoEnVPd=TH2DBuilder(Form("Entry_pedestal_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoEnVPw=TH2DBuilder(Form("Entry_peakwidth_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoEnVDt=TH2DBuilder(Form("Entry_driftTime_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);
    this->fHistoEnVNp=TH2DBuilder(Form("Entry_npeaks_boardDep_%d",fRunNo),NUM_OF_BOARDS_CDC);

    fHistoWaves.MakeTH2Ds("waves","Waveform board",64,-32,32,ap_bin,-200,500,"Arbitary","ADC");
    fHistoDtVAs.MakeTH2Ds("dtasb","Drift time vs Sum of ADC board",dt_bin,dt_min,dt_max,as_bin,as_min,as_max,"driftTime[ns]","Sum of ADC");
    fHistoDtVAp.MakeTH2Ds("dtapb","Drift time vs Peak's ADC board",dt_bin,dt_min,dt_max,ap_bin,ap_min,ap_max,"driftTime[ns]","Peak's ADC");
    fHistoDtVPd.MakeTH2Ds("dtpdb","Drift time vs pedestal board",dt_bin,dt_min,dt_max,pd_bin,pd_min,pd_max,"driftTime[ns]","pedestal");
    fHistoDtVPw.MakeTH2Ds("dtpwb","Drift time vs peak width board",dt_bin,dt_min,dt_max,pw_bin,pw_min,pw_max,"driftTime[ns]","peak width");
    fHistoApVAs.MakeTH2Ds("apasb","Peak's ADC vs Sum of ADC board",ap_bin,ap_min,ap_max,as_bin,as_min,ap_max,"Peak's ADC","Sum of ADC");
    fHistoApVPd.MakeTH2Ds("appdb","Peak's ADC vs pedestal board",ap_bin,ap_min,ap_max,pd_bin,pd_min,pd_max,"Peak's ADC","pedestal");
    fHistoApVPw.MakeTH2Ds("appwb","Peak's ADC vs peak width board",ap_bin,ap_min,ap_max,pw_bin,pw_min,pw_max,"Peak's ADC","peak width");
    fHistoApVNp.MakeTH2Ds("apnpb","Peak's ADC vs number of peaks board",ap_bin,ap_min,ap_max,np_bin,np_min,np_max,"Peak's ADC","number of peaks");
    fHistoAsVPd.MakeTH2Ds("aspdb","Sum of ADC vs pedestal board",as_bin,as_min,as_max,pd_bin,pd_min,pd_max,"Sum of ADC","pedestal");
    fHistoAsVPw.MakeTH2Ds("aspwb","Sum of ADC vs peak width board",as_bin,as_min,as_max,pw_bin,pw_min,pw_max,"Sum of ADC","peak width");
    fHistoAsVNp.MakeTH2Ds("asnpb","Sum of ADC vs number of peaks board",as_bin,as_min,as_max,np_bin,np_min,np_max,"Sum of ADC","number of sums");
    fHistoEnVAs.MakeTH2Ds("enasb","Entry vs Sum of ADC board",en_bin,en_min,en_max,as_bin,as_min,as_max,"Entry number","Sum of ADC");
    fHistoEnVAp.MakeTH2Ds("enapb","Entry vs Peak's ADC board",en_bin,en_min,en_max,ap_bin,ap_min,ap_max,"Entry number","Peak's ADC");
    fHistoEnVPd.MakeTH2Ds("enpdb","Entry vs pedestal board",en_bin,en_min,en_max,pd_bin,pd_min,pd_max,"Entry number","pedestal");
    fHistoEnVPw.MakeTH2Ds("enpwb","Entry vs peak width board",en_bin,en_min,en_max,pw_bin,pw_min,pw_max,"Entry number","peak width");
    fHistoEnVDt.MakeTH2Ds("endtb","Entry vs drift time board",en_bin,en_min,en_max,dt_bin,dt_min,dt_max,"Entry number","drift time [ns]");
    fHistoEnVNp.MakeTH2Ds("ennpb","Entry vs number of peaks board",en_bin,en_min,en_max,np_bin,np_min,np_max,"Entry number","number of peaks");
}

void BoardDepPreTrackHisto::Fill(){
    for(Int_t iev=0;iev<fTree->GetEntries();iev++){
        fTree->GetEntry(iev);
        for(Int_t iHit=0;iHit<fNumOfHits;iHit++){// Hit loop
            int board=fHardBoardID->at(iHit);
            int cell=fCellID->at(iHit);
            if(board<0)continue;
            int lh = fNumOfLayHits->at(iHit);
            double dt = fDriftTimeFirstPeak->at(iHit);
            double ap = fADCFirstPeak->at(iHit);
            double as = fQ->at(iHit);
            double pd = fPedestal->at(iHit);
            double pw = fPeakWidth->at(iHit);
            double np = fNumOfPeaks->at(iHit);
            std::vector<double> adcs=fWaveform->at(iHit);
            if(lh==1){
                fHistoDt.Fill(board,dt);
                fHistoAp.Fill(board,ap);
                fHistoAs.Fill(board,as);
                fHistoPd.Fill(board,pd);
                fHistoPw.Fill(board,pw);
                fHistoNp.Fill(board,np);
                fHistoCe.Fill(board,cell);

                //TH2D
                fHistoDtVAs.Fill(board,dt,as);
                fHistoDtVAp.Fill(board,dt,ap);
                fHistoDtVPd.Fill(board,dt,pd);
                fHistoDtVPw.Fill(board,dt,pw);
                fHistoApVAs.Fill(board,ap,as);
                fHistoApVPd.Fill(board,ap,pd);
                fHistoApVPw.Fill(board,ap,pw);
                fHistoApVNp.Fill(board,ap,np);
                fHistoAsVPd.Fill(board,as,pd);
                fHistoAsVPw.Fill(board,as,pw);
                fHistoAsVNp.Fill(board,as,np);
                for(int iSample=0;iSample<(int)adcs.size();iSample++)
                    fHistoWaves.Fill(board,iSample-fWaveform1stClk->at(iHit),adcs[iSample]);

            }


            fHistoLh.Fill(board,lh);
            // Time
            fHistoEnVAs.Fill(board,iev,as);
            fHistoEnVAp.Fill(board,iev,ap);
            fHistoEnVPd.Fill(board,iev,pd);
            fHistoEnVPw.Fill(board,iev,pw);
            fHistoEnVDt.Fill(board,iev,dt);
            fHistoEnVNp.Fill(board,iev,np);

        }

    }
}

void BoardDepPreTrackHisto::Draw(){
    // Draw TH1D
    TString op="";
    bool isLogX=false;
    bool isLogY=false;
    bool isLogZ=true;
    fHistoDt.Draw(op,isLogX,isLogY); fHistoDt.DrawCombine(op,isLogX,isLogY);
    fHistoAp.Draw(op,isLogX,isLogY); fHistoAp.DrawCombine(op,isLogX,isLogY);
    fHistoAs.Draw(op,isLogX,isLogY); fHistoAs.DrawCombine(op,isLogX,isLogY);
    fHistoPd.Draw(op,isLogX,isLogY); fHistoPd.DrawCombine(op,isLogX,isLogY);
    fHistoPw.Draw(op,isLogX,isLogY); fHistoPw.DrawCombine(op,isLogX,isLogY);
    fHistoNp.Draw(op,isLogX,isLogY); fHistoNp.DrawCombine(op,isLogX,isLogY);
    fHistoLh.Draw(op,isLogX,isLogY); fHistoLh.DrawCombine(op,isLogX,isLogY);
    fHistoCe.Draw(op,isLogX,isLogY); fHistoCe.DrawCombine(op,isLogX,isLogY);
    // // Draw TH2D
    fHistoDtVAs.Draw(op,isLogX,isLogY,isLogZ);
    fHistoDtVAp.Draw(op,isLogX,isLogY,isLogZ);
    fHistoDtVPd.Draw(op,isLogX,isLogY,isLogZ);
    fHistoDtVPw.Draw(op,isLogX,isLogY,isLogZ);
    fHistoApVAs.Draw(op,isLogX,isLogY,isLogZ);
    fHistoApVPd.Draw(op,isLogX,isLogY,isLogZ);
    fHistoApVPw.Draw(op,isLogX,isLogY,isLogZ);
    fHistoApVNp.Draw(op,isLogX,isLogY,isLogZ);
    fHistoAsVPd.Draw(op,isLogX,isLogY,isLogZ);
    fHistoAsVPw.Draw(op,isLogX,isLogY,isLogZ);
    fHistoAsVNp.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVAs.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVAp.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVPd.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVPw.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVDt.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVNp.Draw(op,isLogX,isLogY,isLogZ);
    fHistoWaves.Draw(op,isLogX,isLogY,isLogZ);
}
