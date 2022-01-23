#include "P4SPring8AnalysisStrategy.hxx"

void P4SPring8AnalysisStrategy::GetPreTrackingParameters()
{
    //Int_t numOfEvents=fInput->GetEntries();
    Int_t numOfEvents=fInput->GetEntries()/10;

    //event loop
    std::cout<< "## You have chosen to use P4SPring8AnalysisStrategy for pre-tracking " << std::endl;
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
            Int_t    layer   = WireManager::Get().GetUsedLayer(ch);
            if(tdcNhit) NLayerHit[layer]++;
        }

        for(int ch=TRIG_CH;ch<MAX_CH;ch++){
            Int_t adc[MAX_SAMPLE];
            Int_t hitSample[MAX_SAMPLE];
            Int_t    tdcNhit = fInput->GetTDCNhit(ch);
            Int_t    layer    = WireManager::Get().GetUsedLayer(ch);
            if(layer<0)continue;
            //first peak
            Double_t driftTime1 = driftTimeAnalyzer.FixTDC(fInput->GetTDC(ch,0));
            //printf("ch. %d lay %d tdcH %d dt %f \n",ch,layer,tdcNhit,driftTime1);
            for(int clk=0;clk<MAX_SAMPLE;clk++){
                adc[clk] = fInput->GetADC(ch,clk);
                hitSample[clk] = fInput->GetClkHit(ch,clk);
            }
            // Find the pedestal
            Double_t pedestal = waveFormAnalyzer.FindPedestal(adc,hitSample[0]);
            fHistoPed[ch]->Fill(pedestal);
            // Just for getting clear hits
            if(tdcNhit>0 && ( NLayerHit[layer]==1 || NLayerHit[layer]==2 )){
                fHistoFindT0[ch/48-NUM_OF_BOARDS_TRI]->Fill(driftTime1);
            }
        }
    }

    //Fit to find t0
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        if(fHistoFindT0[iBD]->Integral()>100){
            //Fit t0
            fFitT0.push_back(new TF1(Form("fitf_t0_b%d",iBD),DriftTimeAnalyzer::FitFunction_findt0,-880,-830,6));
            fTDCRisingEdge[iBD] = driftTimeAnalyzer.FitT0(fHistoFindT0[iBD],fFitT0[iBD],"Q0",-950,-830);
            printf("## Fitting of time 0 : %f ", fTDCRisingEdge[iBD]);
            if(fTDCRisingEdge[iBD]>-800 || fTDCRisingEdge[iBD]<-1000){
                fTDCRisingEdge[iBD]=-850;
                printf("Failed  --> -850ns \n");
            }else{
                printf("\n");
            }
        }
    }

    for(int ch=TRIG_CH;ch<MAX_CH;ch++){
        if(fHistoPed[ch]->Integral()>10){
            fCut_p[ch]=waveFormAnalyzer.FindADCsumCut(fHistoPed[ch]);
            fCut_q[ch]=waveFormAnalyzer.FindADCsumCut(fHistoPed[ch]);
        }else{
            fCut_p[ch]=-1;
            fCut_q[ch]=-1;
        }
    }
    // Print out the results of fitting
    Print();
}

void P4SPring8AnalysisStrategy::PrepareRootFile(){
    Int_t numOfEvents=fInput->GetEntries();
    if(fIsForceEvent)numOfEvents=fForcedMaxEvent;
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
    //  for(int iev=0;iev<1;iev++){
        AnalysisStrategy::ClearEvent();
        fInput->GetEntry(iev);
        if(iev%(numOfEvents/10)==0)printf("Processing %.2f\n",iev*1./numOfEvents*100);
        Int_t triggerNumber = fInput->GetTriggerNumber();
        // Get trigger time
        Double_t triggerTime=avg_t0offset;
        // You have to minus the drift time 2^15 because of counter in FPGA
        //Get number of hits
        Int_t counter_hits = 0;
        Int_t counter_peaks = 0;
        int NLayerHit[MAX_SENSE_LAYER];
        for(int l=0;l<MAX_SENSE_LAYER;l++)
            NLayerHit[l]=0;

        for(int ch=TRIG_CH;ch<MAX_CH;ch++){
            Int_t tdcNhit = fInput->GetTDCNhit(ch);
            Int_t    layer    = WireManager::Get().GetUsedLayer(ch);
            Int_t    cell    = WireManager::Get().GetUsedCell(ch);
            if(layer<0 || cell<0)continue;
            if(tdcNhit){
                counter_hits++;
                counter_peaks+=tdcNhit;
                NLayerHit[layer]++;
            }
            //if(triggerNumber==3174)printf("ch. %d [%d][%d] %d \n",ch,layer,cell,tdcNhit);
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
            if(layer<0 || cell<0)continue;
            Int_t    bdlayer = WireManager::Get().GetUsedBoardLayID(ch);
            Int_t    bdloc   = WireManager::Get().GetUsedBoardLocID(ch);

            Double_t    x_center = WireManager::Get().GetSenseWireXPosCen(layer,cell);
            Double_t    y_center = WireManager::Get().GetSenseWireYPosCen(layer,cell);

            for(int clk=0;clk<MAX_SAMPLE;clk++){
                adc[clk]                  = fInput->GetADC(ch,clk);
                hitSample[clk]            = fInput->GetClkHit(ch,clk);
                clockNumberDriftTime[clk] = fInput->GetClkHit(ch,clk);
                driftTime[clk]            = driftTimeAnalyzer.FixTDC(fInput->GetTDC(ch,clk))-triggerTime;
                driftTime[clk]            = driftTimeAnalyzer.TDC2DriftTime(driftTime[clk]);
            }
            //if(tdcNhit && iev==0) printf("%d sam Fix(%d - %f)-> =%f \n",ch,
            //                           fInput->GetTDC(ch,0),triggerTime, driftTime[0]);
            Double_t    pedestal       = waveFormAnalyzer.FindPedestal(adc,hitSample[0]);
            Double_t    q_tmp          = waveFormAnalyzer.FindADCsum(pedestal,adc);
            Int_t       firstPeakWidth = waveFormAnalyzer.FindPeakWidth(clockNumberDriftTime[0],adc,pedestal);

            Int_t numOfPeaks=0;
            std::vector<double> a_adc;
            std::vector<double> a_adc_clk;
            for(int clk=0;clk<MAX_SAMPLE;clk++){
                a_adc.push_back(adc[clk]-pedestal);
                a_adc_clk.push_back((double)clockNumberDriftTime[clk]);
                ADCpeak[clk] = -99;
                if(clk+1==MAX_SAMPLE){
                    ADCpeak[numOfPeaks] = adc[clk];
                }
                if(tdcNhit){
                    ADCpeak[numOfPeaks] = waveFormAnalyzer.FindPeakAdc(adc, clockNumberDriftTime[clk],clockNumberDriftTime[clk+1]);
                    ADCpeak[numOfPeaks] -= pedestal;
                    if(ADCpeak[numOfPeaks]<0)ADCpeak[numOfPeaks]=0;
                }
                if(triggerNumber==35115 && ch==28) printf("adc %d clkhit %d chosepeak %f\n",adc[clk],clockNumberDriftTime[clk],ADCpeak[numOfPeaks]);
                numOfPeaks++;
            }

            bool cpFlag_tmp = false;
            double ChosenDriftT = -1;
            double ChosenADCPeak = -1;
            if(tdcNhit){
                double tmp_peak=0;
                //double tmp_dt=waveFormAnalyzer.ChooseTDChit(driftTime,ADCpeak,tdcNhit,fCut_q[ch],tmp_q,cpFlag_tmp);
                //double tmp_dt=waveFormAnalyzer.ChooseTDChit(driftTime,ADCpeak,tdcNhit,30,tmp_peak,cpFlag_tmp);
                double tmp_dt=driftTime[0];
                // if(triggerNumber==3174)printf("ch. %d dtraw %f trigger  %f cp %d fix time %f adcp0 %f adcp1 %f \n",
                //                            ch,
                //                            driftTime[0],triggerTime, cpFlag_tmp, tmp_dt, ADCpeak[0],ADCpeak[1]);
                if(tmp_dt!=driftTime[0]){
                    cpFlag_tmp=true;
                    ChosenDriftT=tmp_dt;
                    ChosenADCPeak = tmp_peak;
                }else{
                    cpFlag_tmp=false;
                    ChosenDriftT=driftTime[0];
                    ChosenADCPeak=ADCpeak[0];
                }
                tmp_dt=driftTime[0];
            }

            //Fill output variable
            if(tdcNhit && layer!=-1 && cell!=-1){
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
                int ch_per_board=int(MAX_CH/NUM_OF_BOARDS_CDC);
                fHardwareBoardID.push_back((ch-TRIG_CH)/(ch_per_board));

                fDriftTime.push_back(ChosenDriftT);
                std::vector<double> driftTimeOfEachPeak;
                for(int w=0;w<tdcNhit;w++){
                    driftTimeOfEachPeak.push_back(driftTime[w]);
                }
                fDriftTAll.push_back(driftTimeOfEachPeak);
                fADCpeak.push_back(ChosenADCPeak);
                std::vector<double> adcPeakOfEachPeak;
                for(int w=0;w<tdcNhit;w++){
                    adcPeakOfEachPeak.push_back(ADCpeak[w]);
                }
                fADCPeakAll.push_back(adcPeakOfEachPeak);
                fWaveform.push_back(a_adc);
                fWaveformClk.push_back(a_adc_clk);
                fWaveform1stClk.push_back(clockNumberDriftTime[0]);
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
        fNHitAll=counter_peaks;
        if(fNHit==0){
            AnalysisStrategy::PushDummyData();
        }
        fPreRootTree->Fill();

    }
    fPreRootTree->Write();
    fPreRootFile -> Close();
}

void P4SPring8AnalysisStrategy::Print(){
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
    canvas->SaveAs(Form("%s/FittingOfRisingEdge_r%d.pdf",DIR,fRunNo));
    canvas->Clear();

    int ch_per_board=int(MAX_CH/NUM_OF_BOARDS_CDC);
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        canvas->Divide(8,6);
        for(int ich=0;ich<ch_per_board;ich++){
            canvas->cd(ich+1);
            fHistoPed[ich+iBD*ch_per_board]->Draw();
        }
        canvas->SaveAs(Form("%s/PedestalCut_r%d_ibd%d.pdf",DIR,fRunNo,iBD));
        canvas->Clear();
    }
    // Draw pedestal cut value
    TGraph *gPedestalCut = new TGraph(MAX_CH-TRIG_CH);
    gPedestalCut->SetTitle("Pedestal cut;Channel;Cut value");
    gPedestalCut->SetLineColor(2);
    gPedestalCut->SetMarkerColor(2);
    gPedestalCut->SetMarkerStyle(4);
    for(int ch=TRIG_CH;ch<MAX_CH;ch++){
        double gx = ch - TRIG_CH;
        double gy = fCut_p[ch];
        gPedestalCut->SetPoint(ch-TRIG_CH,gx,gy);
    }
    canvas->cd();
    gPedestalCut->Draw("apl");
    canvas->SaveAs(Form("%s/graphPedestalCut_r%d.pdf",DIR,fRunNo));
    canvas->Clear();
    // Draw pedestal
    TGraph *gPedestal = new TGraph(MAX_CH-TRIG_CH);
    gPedestal->SetTitle("Pedestal ;Channel; Pedestal");
    gPedestal->SetLineColor(2);
    gPedestal->SetMarkerColor(2);
    gPedestal->SetMarkerStyle(4);
    for(int ch=TRIG_CH;ch<MAX_CH;ch++){
        double gx = ch - TRIG_CH;
        double gy = (fHistoPed[ch-TRIG_CH]->GetFunction("gaus"))?
            fHistoPed[ch-TRIG_CH]->GetFunction("gaus")->GetParameter(1):-1;
        gPedestal->SetPoint(ch-TRIG_CH,gx,gy);
    }
    canvas->cd();
    gPedestal->Draw("apl");
    canvas->SaveAs(Form("%s/graphPedestal_r%d.pdf",DIR,fRunNo));
}
