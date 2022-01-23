#include "SimModeAnalysisStrategy.hxx"

void SimModeAnalysisStrategy::GetPreTrackingParameters()
{
    Int_t numOfEvents=fInput->GetEntries();
    if(fIsForceEvent)numOfEvents=fForcedMaxEvent;
    //event loop
    std::cout<< "## You have chosen to use SimModeAnalysisStrategy for pre-tracking " << std::endl;
    std::cout<< "## Doing" << std::endl;
    std::cout<< "## 1. Draw Histogram" << std::endl;
    std::cout<< "## 2. Find t0" << std::endl;
    std::cout<< "## 3. Find baseline" << std::endl;
    std::cout<< "## 4. Find adc cut" << std::endl;
    for(int iev=0;iev<numOfEvents;iev++){
        fInput->GetEntry(iev);
        // Get trigger time
        int NLayerHit[MAX_SENSE_LAYER];
        for(int l=0;l<MAX_SENSE_LAYER;l++)
            NLayerHit[l]=0;

        // Counting the layer hits
        for(int ch=TRIG_CH;ch<MAX_CH;ch++){
            Int_t    tdcNhit = fInput->GetTDCNhit(ch);
            Int_t    layer    = WireManager::Get().GetUsedLayer(ch);
            if(tdcNhit) NLayerHit[layer]++;
        }

        for(int ch=TRIG_CH;ch<MAX_CH;ch++){
            Int_t adc[MAX_SAMPLE];
            Int_t hitSample[MAX_SAMPLE];
            Int_t    tdcNhit = fInput->GetTDCNhit(ch);
            Int_t    layer    = WireManager::Get().GetUsedLayer(ch);
            if(layer<0)continue;
            //first peak
            Double_t driftTime1 = fInput->GetDriftTimeSim(ch,0);
            for(int clk=0;clk<MAX_SAMPLE;clk++){
                adc[clk] = fInput->GetADC(ch,clk);
                hitSample[clk] = fInput->GetClkHit(ch,clk);
            }

            Double_t pedestal = waveFormAnalyzer.FindPedestal(adc,hitSample[0]);
            fHistoPed[ch]->Fill(pedestal);
            if(tdcNhit>0 && driftTime1>0 && NLayerHit[layer]==1){
                fHistoFindT0[ch/48-NUM_OF_BOARDS_TRI]->Fill(driftTime1);
            }

        }
    }

    //Fit to find t0
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        if(fHistoFindT0[iBD]->Integral()>100){
            //Fit t0
            fFitT0[iBD]   = new TF1(Form("fitf_t0_b%d",iBD),DriftTimeAnalyzer::FitFunction_findt0,-10,10,6);
            fTDCRisingEdge[iBD]=driftTimeAnalyzer.FitT0(fHistoFindT0[iBD],fFitT0[iBD]);
            printf("%f\n", fTDCRisingEdge[iBD]);
        }
    }
    for(int ch=TRIG_CH;ch<MAX_CH;ch++){
        fCut_p[ch]=10;
        fCut_q[ch]=10;
    }

    // Print out the results of fitting
    Print();
}

void SimModeAnalysisStrategy::Print(){
    TCanvas *canvas = new TCanvas("canvas","DriftTime Fit Result",1024,712);
    int size;
    size=(int)(sqrt(NUM_OF_BOARDS_CDC*1.)+1);
    canvas->Divide(size,size);
    gStyle->SetOptFit(1);
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        canvas->cd(iBD+1);
        fHistoFindT0[iBD]->Draw();
        fHistoFindT0[iBD]->SetFillColor(3);
        fHistoFindT0[iBD]->SetLineColor(1);
        if(fHistoFindT0[iBD]->Integral()>100){
            fFitT0[iBD]->Draw("same");
        }
    }
    canvas->Modified();
    char *DIR = getenv ("CCGRAPH_DIR");
    int fRunNo = IRunTimeManager::Get().GetRunNumber();
    canvas->SaveAs(Form("%s/pre-track-dt_RisingEdge_r%d.pdf",DIR,fRunNo));
    canvas->Clear();
}

void SimModeAnalysisStrategy::PrepareRootFile(){
    TFile* fPreRootFile;
    TTree* fPreRootTree;

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
    fPreRootTree->Branch("adcPeak",&fADCpeak);
    fPreRootTree->Branch("ped",&fPed);
    fPreRootTree->Branch("q",&fQ);
    fPreRootTree->Branch("t0",&fT0);
    fPreRootTree->Branch("adccut",&fADCcut);
    fPreRootTree->Branch("t0offset",&fT0offset);

    Int_t numOfEvents=fInput->GetEntries();
    Int_t trig0 = -1;
    std::cout<< "## Doing output the files for tracking" << std::endl;
    std::cout<< "## by grouping them into layer and cell hits" << std::endl;
    std::cout<< "## Fill tree" << std::endl;

    // Avg the t0 offset of all boards
    double avg_t0offset=0;
    for (int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        avg_t0offset+=fTDCRisingEdge[iBD];
    }
    avg_t0offset/=NUM_OF_BOARDS_CDC;
    // Event loop
    for(int iev=0;iev<numOfEvents;iev++){
        ClearEvent();
        fInput->GetEntry(iev);

        Int_t triggerNumber = fInput->GetTriggerNumber();
        // Get trigger time
        Double_t triggerTime=0;
        if(iev==0)trig0 = triggerNumber;
        //Get number of hits
        Int_t counter_hits = 0;
        int NLayerHit[MAX_SENSE_LAYER];
        for(int l=0;l<MAX_SENSE_LAYER;l++)
            NLayerHit[l]=0;

        for(int ch=TRIG_CH;ch<MAX_CH;ch++){
            Int_t tdcNhit = fInput->GetTDCNhit(ch);
            Int_t    layer    = WireManager::Get().GetUsedLayer(ch);
            if(tdcNhit){
                counter_hits++;
                NLayerHit[layer]++;
            }
        }

        for(int ch=TRIG_CH;ch<MAX_CH;ch++){
            Int_t tdcNhit = fInput->GetTDCNhit(ch);

            Double_t    driftTime[MAX_SAMPLE];
            Double_t    ADCpeak[MAX_SAMPLE];
            Int_t    adc[MAX_SAMPLE];
            Int_t    hitSample[MAX_SAMPLE];
            Int_t    clockNumberDriftTime[MAX_SAMPLE];

            Int_t    layer   = WireManager::Get().GetUsedLayer(ch);
            Int_t    cell    = WireManager::Get().GetUsedCell(ch);

            Int_t    bdlayer = WireManager::Get().GetUsedBoardLayID(ch);
            Int_t    bdloc   = WireManager::Get().GetUsedBoardLocID(ch);

            Double_t    x_center = WireManager::Get().GetSenseWireXPosCen(layer,cell);
            Double_t    y_center = WireManager::Get().GetSenseWireYPosCen(layer,cell);

            for(int clk=0;clk<MAX_SAMPLE;clk++){
                adc[clk]                  = fInput->GetADC(ch,clk);
                hitSample[clk]            = fInput->GetClkHit(ch,clk);
                clockNumberDriftTime[clk] = fInput->GetClkHit(ch,clk);
                driftTime[clk]            = driftTimeAnalyzer.FixTDC(fInput->GetDriftTimeSim(ch,clk)) - triggerTime;
                driftTime[clk]            = driftTimeAnalyzer.TDC2DriftTime(driftTime[clk]);
            }

            Int_t numOfPeaks=0;
            for(int clk=0;clk<MAX_SAMPLE;clk++){
                ADCpeak[clk] = -99;
                if(clk+1==MAX_SAMPLE){
                    ADCpeak[numOfPeaks] = adc[clk];
                }
                if(tdcNhit){
                    ADCpeak[numOfPeaks] = waveFormAnalyzer.FindPeakAdc(adc, clockNumberDriftTime[clk],clockNumberDriftTime[clk+1]);
                }
                numOfPeaks++;
            }

            Double_t    pedestal       = waveFormAnalyzer.FindPedestal(adc,hitSample[0]);
            Double_t    q_tmp          = waveFormAnalyzer.FindADCsum(pedestal,adc);
            Int_t       firstPeakWidth = waveFormAnalyzer.FindPeakWidth(clockNumberDriftTime[0],adc,pedestal);
            bool cpFlag_tmp = false;
            if(tdcNhit){
//              printf("%f-(%f+%f)=%f \n",driftTimeAnalyzer.FixDriftTime(fInput->GetDriftTimeSim(ch,0)),(t1+t2)/2.,avg_t0offset,driftTime[0]);
                // This gives you to choose the second hit using the waveform information
                double tmp_q;
                double tmp_dt=waveFormAnalyzer.ChooseTDChit(driftTime,ADCpeak,tdcNhit,fCut_q[ch],tmp_q,cpFlag_tmp);
//              printf("ch. %d %f %f %f %d fix time %f adcp0 %f adcp1 %f \n",
//                     ch,driftTimeAnalyzer.FixDriftTime(fInput->GetDriftTimeSim(ch,0)),
//                     driftTime[0],triggerTime, cpFlag_tmp, tmp_dt, ADCpeak[0],ADCpeak[1]);
                driftTime[0] = tmp_dt ;
                ADCpeak[0] = tmp_q;
            }

            //Fill output variable
            if(tdcNhit){
                if(pedestal<0) fGoodHitFlag.push_back(false); else fGoodHitFlag.push_back(true);
                if(firstPeakWidth<0)fLargfPeakWidthFlag.push_back(true); else fLargfPeakWidthFlag.push_back(false);
                fChoosePeakFlag.push_back(cpFlag_tmp);
                fNHitLayer.push_back(NLayerHit[layer]);
                fNPeaks.push_back(tdcNhit);
                fPeakWidth.push_back(firstPeakWidth);
                fHitCh.push_back(ch);
                fLayerID.push_back(layer);
                fCellID.push_back(cell);
                fXCentre.push_back(x_center);
                fYCentre.push_back(y_center);
                fBoardLayID.push_back(bdlayer);
                fBoardLocID.push_back(bdloc);
                fHardwareBoardID.push_back(ch/48);
                fDriftTime.push_back(driftTime[0]);
                fADCpeak.push_back(ADCpeak[0]);
                fPed.push_back(pedestal);
                fQ.push_back(q_tmp);
                fT0.push_back(triggerTime);
                fADCcut.push_back(fCut_q[ch]);
            }
        }
        for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
            fT0offset.push_back(fTDCRisingEdge[iBD]);
        }
        fTriggerNumber=triggerNumber;
        fNHit=counter_hits;
        if(fNHit==0){
            AnalysisStrategy::PushDummyData();
        }
        fPreRootTree->Fill();

    }
    fPreRootTree->Write();
    fPreRootFile->Close();
}


