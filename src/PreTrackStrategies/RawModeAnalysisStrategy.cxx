
#include "RawModeAnalysisStrategy.hxx"

void RawModeAnalysisStrategy::GetPreTrackingParameters()
{
    Int_t numOfEvents=fInput->GetEntries();
    if(fIsForceEvent)numOfEvents=fForcedMaxEvent;

    //event loop
    std::cout<< "## You have chosen to use RawModeAnalysisStrategy for pre-tracking " << std::endl;
    std::cout<< "## Doing" << std::endl;
    std::cout<< "## 1. Draw Histogram" << std::endl;
    std::cout<< "## 2. Find t0" << std::endl;
    std::cout<< "## 3. Find baseline" << std::endl;
    std::cout<< "## 4. Find adc cut" << std::endl;
    for(int iev=0;iev<numOfEvents;iev++){
        fInput->GetEntry(iev);
        // Get trigger time
        Double_t triggerTime;
        Double_t t1=(Int_t)fInput->GetTDC(0,0);
        Double_t t2=(Int_t)fInput->GetTDC(8,0);
        if(!fTriggerDelay)
            if( (t1==0 || t2==0)) continue;
        if( fInput->GetTDCNhit(0)==0 || fInput->GetTDCNhit(8)==0 ) continue;
        triggerTime = (t1+t2)/2.;
        //printf("%d triggertime %f ch0 %d ch8 %d\n", triggerNumber,triggerTime, fInput->GetTDCNhit(0),fInput->GetTDCNhit(8)) ;

        if(fTriggerDelay) triggerTime = 0;
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
            Double_t driftTime1 = fInput->GetTDC(ch,0) - triggerTime;
            for(int clk=0;clk<MAX_SAMPLE;clk++){
                adc[clk] = fInput->GetADC(ch,clk);
                hitSample[clk] = fInput->GetClkHit(ch,clk);
            }
            Double_t pedestal = waveFormAnalyzer.FindPedestal(adc,hitSample[0]);
            fHistoPed[ch]->Fill(pedestal);
            if(tdcNhit>0 && NLayerHit[layer]==1){
                //printf("%f \n",driftTime1);
                fHistoFindT0[ch/48-NUM_OF_BOARDS_TRI]->Fill(driftTime1);
            }

        }
    }

    //Fit to find t0
    for(int iBD=0;iBD<NUM_OF_BOARDS_CDC;iBD++){
        if(fHistoFindT0[iBD]->Integral()>100){
            //Fit t0
            fFitT0.push_back(new TF1(Form("fitf_t0_b%d",iBD),DriftTimeAnalyzer::FitFunction_findt0,-150,0,6));
            fTDCRisingEdge[iBD]=driftTimeAnalyzer.FitT0(fHistoFindT0[iBD],fFitT0[iBD]);
            printf("%f\n", fTDCRisingEdge[iBD]);
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

void RawModeAnalysisStrategy::Print(){
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
}

void RawModeAnalysisStrategy::PrepareRootFile(){
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

    //std::cout << "debugsam avg_t0offset " << avg_t0offset  << " NUM_OF_BOARDS_CDC " << NUM_OF_BOARDS_CDC << std::endl;
    // Event loop
    for(int iev=0;iev<numOfEvents;iev++){
        AnalysisStrategy::ClearEvent();
        fInput->GetEntry(iev);

        Int_t triggerNumber = fInput->GetTriggerNumber();
        // Get trigger time
        Double_t triggerTime;
        // You have to minus the drift time 2^15 because of counter in FPGA
        Int_t t1=driftTimeAnalyzer.FixTDC(fInput->GetTDC(0,0));
        Int_t t2=driftTimeAnalyzer.FixTDC(fInput->GetTDC(8,0));
        if(!fTriggerDelay)
            if( t1==0 || t2==0 ) continue;
        if( fInput->GetTDCNhit(0)==0 || fInput->GetTDCNhit(8)==0 ) continue;
        triggerTime = (t1+t2)/2. + avg_t0offset;
        if(fTriggerDelay) triggerTime = 0;
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
                driftTime[clk]            = driftTimeAnalyzer.FixTDC(fInput->GetTDC(ch,clk)) - triggerTime;
                driftTime[clk]            = driftTimeAnalyzer.TDC2DriftTime(driftTime[clk]);
                //if(triggerNumber==17407 && fInput->GetTDC(ch,clk)>-9000)printf("clk [%d] tdc raw %d tdc_fix %f triggerTime %f dt %f dt_fix %f\n",clk,fInput->GetTDC(ch,clk),
                //driftTimeAnalyzer.FixTDC(fInput->GetTDC(ch,clk)),triggerTime, driftTimeAnalyzer.FixTDC(fInput->GetTDC(ch,clk)) - triggerTime ,driftTime[clk]);
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
            double picked1PeakDt = 0;
            if(tdcNhit){
                // This gives you to choose the second hit using the waveform information
                double tmp_q;
                double tmp_dt=waveFormAnalyzer.ChooseTDChit(driftTime,ADCpeak,tdcNhit,pedestal+10,tmp_q,cpFlag_tmp);
                //double tmp_dt=waveFormAnalyzer.ChooseTDChit(driftTime,ADCpeak,tdcNhit,fCut_q[ch],tmp_q,cpFlag_tmp);
                //tmp_dt = driftTime[0];// For suppress mode
                // if(triggerNumber==17407){
                //     //printf("ch. %d %f %f %f %d fix time %f pedestal %f adcp0 %f adcp1 %f \n",
                //         ch,tmp_dt,
                //         driftTime[0],triggerTime, cpFlag_tmp, tmp_dt,pedestal, ADCpeak[0],ADCpeak[1]);
                // }
                picked1PeakDt = tmp_dt ;
                if(picked1PeakDt!=driftTime[0])cpFlag_tmp=true;
                else cpFlag_tmp=false;
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

                fDriftTime.push_back(picked1PeakDt);
                std::vector<double> driftTimeOfEachPeak;
                for(int w=0;w<tdcNhit;w++){
                    driftTimeOfEachPeak.push_back(driftTime[w]);
                }
                fDriftTAll.push_back(driftTimeOfEachPeak);
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
    fPreRootFile -> Close();
}
