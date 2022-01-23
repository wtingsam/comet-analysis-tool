#include "LayDepPreTrackHisto.hxx"

void LayDepPreTrackHisto::Initialize(){
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
    fWaveform=0;
    fWaveform1stClk=0;

    fRunNo            = IRunTimeManager::Get().GetRunNumber();
    fIter             = IRunTimeManager::Get().GetIteration();
    TRIG_CH           = ExperimentConfig::Get().GetNumOfTrigChannel();
    MAX_CH            = ExperimentConfig::Get().GetNumOfChannel();
    MAX_SENSE_LAYER   = ExperimentConfig::Get().GetNumOfLayer();
    MAX_SENSE_WIRES   = WireManager::Get().GetNumSenseWires(MAX_SENSE_LAYER-1);
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

    double ap_bin = 100;
    double ap_min = 150;
    double ap_max = 850;

    double as_bin = 100;
    double as_min = 0;
    double as_max = 1500;

    double pd_bin = 100;
    double pd_min = 200;
    double pd_max = 200;

    double pw_bin = 32;
    double pw_min = 0;
    double pw_max = 31;

    double np_bin = 32;
    double np_min = 0;
    double np_max = 31;

    double lh_bin = 20;
    double lh_min = 0;
    double lh_max = 20;

    double ce_bin = 100;
    double ce_min = 0;
    double ce_max = MAX_SENSE_WIRES;

    double en_bin = 100;
    double en_min = 0;
    double en_max = fTree->GetEntries();

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
    // Make TH1D
    this->fHistoDt=TH1DBuilder(Form("driftTime_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);
    this->fHistoAp=TH1DBuilder(Form("ADCFP_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);
    this->fHistoAs=TH1DBuilder(Form("ADCSum_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);
    this->fHistoPd=TH1DBuilder(Form("Pedestal_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);
    this->fHistoPw=TH1DBuilder(Form("PeakWidth_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);
    this->fHistoNp=TH1DBuilder(Form("NumOfPeak_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);
    this->fHistoLh=TH1DBuilder(Form("NumOfHits_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);
    this->fHistoCe=TH1DBuilder(Form("CellID_layerDep_%d",fRunNo),MAX_SENSE_LAYER,true);

    fHistoDt.MakeTH1Ds("dtl","Drift time layer",dt_bin,dt_min,dt_max,"driftTime[ns]",1,0);
    fHistoAp.MakeTH1Ds("apl","Peak's ADC layer",ap_bin,ap_min,ap_max,"Peak's ADC",1,0);
    fHistoAs.MakeTH1Ds("asl","Sum of ADC layer",as_bin,as_min,as_max,"Sum of ADC",1,0);
    fHistoPd.MakeTH1Ds("pdl","Pedestal layer",pd_bin,pd_min,pd_max,"pedestal",1,0);
    fHistoPw.MakeTH1Ds("pwl","Peak width layer",pw_bin,pw_min,pw_max,"peak width",1,0);
    fHistoNp.MakeTH1Ds("npl","Number of peaks layer",np_bin,np_min,np_max,"number of peaks",1,0);
    fHistoLh.MakeTH1Ds("lhl","Number of layer hits layer",lh_bin,lh_min,lh_max,"number of hits",1,0);
    fHistoCe.MakeTH1Ds("chl","Cell ID layer",ce_bin,ce_min,ce_max,"cell ID",1,0);

    // TH2D
    this->fHistoWaves=TH2DBuilder(Form("waveform_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoDtVAs=TH2DBuilder(Form("driftTime_adcsum_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoDtVAp=TH2DBuilder(Form("driftTime_adcpeak_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoDtVPd=TH2DBuilder(Form("driftTime_pedestal_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoDtVPw=TH2DBuilder(Form("driftTime_peakwidth_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoApVAs=TH2DBuilder(Form("adcpeak_adcsum_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoApVPd=TH2DBuilder(Form("adcpeak_pedestal_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoApVPw=TH2DBuilder(Form("adcpeak_peakwidth_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoApVNp=TH2DBuilder(Form("adcpeak_npeaks_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoAsVPd=TH2DBuilder(Form("adcsum_pedestal_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoAsVPw=TH2DBuilder(Form("adcsum_sumwidth_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoAsVNp=TH2DBuilder(Form("adcsum_nsums_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoEnVAs=TH2DBuilder(Form("Entry_adcsum_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoEnVAp=TH2DBuilder(Form("Entry_adcpeak_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoEnVPd=TH2DBuilder(Form("Entry_pedestal_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoEnVPw=TH2DBuilder(Form("Entry_peakwidth_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoEnVDt=TH2DBuilder(Form("Entry_driftTime_layerDep_%d",fRunNo),MAX_SENSE_LAYER);
    this->fHistoEnVNp=TH2DBuilder(Form("Entry_npeaks_layerDep_%d",fRunNo),MAX_SENSE_LAYER);

    fHistoWaves.MakeTH2Ds("waves","Waveform layer",64,-32,32,ap_bin,-200,500,"Arbitary","ADC");
    fHistoDtVAs.MakeTH2Ds("dtasl","Drift time vs Sum of ADC layer",dt_bin,dt_min,dt_max,as_bin,as_min,as_max,"driftTime[ns]","Sum of ADC");
    fHistoDtVAp.MakeTH2Ds("dtapl","Drift time vs Peak's ADC layer",dt_bin,dt_min,dt_max,ap_bin,ap_min,ap_max,"driftTime[ns]","Peak's ADC");
    fHistoDtVPd.MakeTH2Ds("dtpdl","Drift time vs pedestal layer",dt_bin,dt_min,dt_max,pd_bin,pd_min,pd_max,"driftTime[ns]","pedestal");
    fHistoDtVPw.MakeTH2Ds("dtpwl","Drift time vs peak width layer",dt_bin,dt_min,dt_max,pw_bin,pw_min,pw_max,"driftTime[ns]","peak width");
    fHistoApVAs.MakeTH2Ds("apasl","Peak's ADC vs Sum of ADC layer",ap_bin,ap_min,ap_max,as_bin,as_min,ap_max,"Peak's ADC","Sum of ADC");
    fHistoApVPd.MakeTH2Ds("appdl","Peak's ADC vs pedestal layer",ap_bin,ap_min,ap_max,pd_bin,pd_min,pd_max,"Peak's ADC","pedestal");
    fHistoApVPw.MakeTH2Ds("appwl","Peak's ADC vs peak width layer",ap_bin,ap_min,ap_max,pw_bin,pw_min,pw_max,"Peak's ADC","peak width");
    fHistoApVNp.MakeTH2Ds("apnpl","Peak's ADC vs number of peaks layer",ap_bin,ap_min,ap_max,np_bin,np_min,np_max,"Peak's ADC","number of peaks");
    fHistoAsVPd.MakeTH2Ds("aspdl","Sum of ADC vs pedestal layer",as_bin,as_min,as_max,pd_bin,pd_min,pd_max,"Sum of ADC","pedestal");
    fHistoAsVPw.MakeTH2Ds("aspwl","Sum of ADC vs peak width layer",as_bin,as_min,as_max,pw_bin,pw_min,pw_max,"Sum of ADC","peak width");
    fHistoAsVNp.MakeTH2Ds("asnpl","Sum of ADC vs number of peaks layer",as_bin,as_min,as_max,np_bin,np_min,np_max,"Sum of ADC","number of sums");
    fHistoEnVAs.MakeTH2Ds("enasb","Entry vs Sum of ADC layer",en_bin,en_min,en_max,as_bin,as_min,as_max,"Entry number","Sum of ADC");
    fHistoEnVAp.MakeTH2Ds("enapb","Entry vs Peak's ADC layer",en_bin,en_min,en_max,ap_bin,ap_min,ap_max,"Entry number","Peak's ADC");
    fHistoEnVPd.MakeTH2Ds("enpdb","Entry vs pedestal layer",en_bin,en_min,en_max,pd_bin,pd_min,pd_max,"Entry number","pedestal");
    fHistoEnVPw.MakeTH2Ds("enpwb","Entry vs peak width layer",en_bin,en_min,en_max,pw_bin,pw_min,pw_max,"Entry number","peak width");
    fHistoEnVDt.MakeTH2Ds("endtb","Entry vs drift time layer",en_bin,en_min,en_max,dt_bin,dt_min,dt_max,"Entry number","drift time [ns]");
    fHistoEnVNp.MakeTH2Ds("ennpb","Entry vs number of peaks layer",en_bin,en_min,en_max,np_bin,np_min,np_max,"Entry number","number of peaks");
}

void LayDepPreTrackHisto::Fill(){
    for(Int_t iev=0;iev<fTree->GetEntries();iev++){
        fTree->GetEntry(iev);
        for(Int_t iHit=0;iHit<fNumOfHits;iHit++){// Hit loop
            int layer=fLayerID->at(iHit);
            int cell=fCellID->at(iHit);
            int lh = fNumOfLayHits->at(iHit);
            double dt = fDriftTimeFirstPeak->at(iHit);
            double ap = fADCFirstPeak->at(iHit);
            double as = fQ->at(iHit);
            double pd = fPedestal->at(iHit);
            double pw = fPeakWidth->at(iHit);
            double np = fNumOfPeaks->at(iHit);
            std::vector<double> adcs=fWaveform->at(iHit);
            //TH1D
            if(lh==1){
                fHistoDt.Fill(layer,dt);
                fHistoAp.Fill(layer,ap);
                fHistoAs.Fill(layer,as);
                fHistoPd.Fill(layer,pd);
                fHistoPw.Fill(layer,pw);
                fHistoNp.Fill(layer,np);
                fHistoCe.Fill(layer,cell);
                //TH2D
                fHistoDtVAs.Fill(layer,dt,as);
                fHistoDtVAp.Fill(layer,dt,ap);
                fHistoDtVPd.Fill(layer,dt,pd);
                fHistoDtVPw.Fill(layer,dt,pw);
                fHistoApVAs.Fill(layer,ap,as);
                fHistoApVPd.Fill(layer,ap,pd);
                fHistoApVPw.Fill(layer,ap,pw);
                fHistoApVNp.Fill(layer,ap,np);
                fHistoAsVPd.Fill(layer,as,pd);
                fHistoAsVPw.Fill(layer,as,pw);
                fHistoAsVNp.Fill(layer,as,np);
                for(int iSample=0;iSample<(int)adcs.size();iSample++)
                    fHistoWaves.Fill(layer,iSample-fWaveform1stClk->at(iHit),adcs[iSample]);

            }
            fHistoLh.Fill(layer,lh);

            fHistoEnVAs.Fill(layer,iev,as);
            fHistoEnVAp.Fill(layer,iev,ap);
            fHistoEnVPd.Fill(layer,iev,pd);
            fHistoEnVPw.Fill(layer,iev,pw);
            fHistoEnVDt.Fill(layer,iev,dt);
            fHistoEnVNp.Fill(layer,iev,np);
        }
    }
}

void LayDepPreTrackHisto::Draw(){

    // Draw TH1D
    TString op="";
    bool isLogX=false;
    bool isLogY=false;
    bool isLogZ=true;
    // TH1D
    fHistoDt.Draw(op,isLogX,isLogY); fHistoDt.DrawCombine(op,isLogX,isLogY);
    fHistoAp.Draw(op,isLogX,isLogY); fHistoAp.DrawCombine(op,isLogX,isLogY);
    fHistoAs.Draw(op,isLogX,isLogY); fHistoAs.DrawCombine(op,isLogX,isLogY);
    fHistoPd.Draw(op,isLogX,isLogY); fHistoPd.DrawCombine(op,isLogX,isLogY);
    fHistoPw.Draw(op,isLogX,isLogY); fHistoPw.DrawCombine(op,isLogX,isLogY);
    fHistoNp.Draw(op,isLogX,isLogY); fHistoNp.DrawCombine(op,isLogX,isLogY);
    fHistoLh.Draw(op,isLogX,isLogY); fHistoLh.DrawCombine(op,isLogX,isLogY);
    fHistoCe.Draw(op,isLogX,isLogY); fHistoCe.DrawCombine(op,isLogX,isLogY);
    // TH2D
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
    fHistoWaves.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVAs.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVAp.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVPd.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVPw.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVDt.Draw(op,isLogX,isLogY,isLogZ);
    fHistoEnVNp.Draw(op,isLogX,isLogY,isLogZ);
}
