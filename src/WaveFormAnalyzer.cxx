#include "WaveFormAnalyzer.hxx"


Double_t WaveFormAnalyzer::ChooseTDChit(Double_t *tdc0, Double_t *adcp, Int_t tdcNhit0, Double_t threshold, Double_t &peak_chosen, bool &cpFlag)
{
    int MAX_SAMPLE  = ExperimentConfig::Get().GetSampleRECBE();

    Double_t tdc_choosed=-99;
    if(tdcNhit0<=0){
        return -99; // No tdc information
    }
    if(tdcNhit0==1){
        return tdc0[0];
    }
    if(adcp[1]-adcp[0]>300){
        // Very rare case
        cpFlag=true;
        peak_chosen=adcp[1];
        return tdc0[1];
    }
    for(int k=0;k<MAX_SAMPLE;k++){
        if(adcp[k]>=threshold){
            tdc_choosed=tdc0[k];
            peak_chosen=adcp[k];
            if(k!=tdcNhit0){
                cpFlag=true;
            }
            return tdc_choosed;
        }
    }
    cpFlag=false;
    return tdc0[0];
}

Int_t WaveFormAnalyzer::FindPeakAdc(Int_t* adc0, Int_t clk0, Int_t clk1)
{
    MAX_SAMPLE = ExperimentConfig::Get().GetSampleRECBE();
    Int_t pre_adc = -1  ;
    //Find peak
    if(clk0<0) return -99;
    if(clk0==0) return adc0[0];
    double diff_tolerence = 10;
    for (Int_t i=clk0; i<MAX_SAMPLE; i++) {
        if(i==clk1){
            pre_adc = adc0[clk0];
            break;
        }
        if(adc0[i] < pre_adc && (pre_adc-adc0[i])>diff_tolerence){
            break;
        } else {
            pre_adc = adc0[i];
        }
    }
    return pre_adc;
}

Double_t WaveFormAnalyzer::FindPedestal(Int_t* adc1, Int_t clk0)
{
    MAX_SAMPLE = ExperimentConfig::Get().GetSampleRECBE();
    Double_t adc_avg = 0  ;
    Double_t ii=0;
    if(clk0==-9999){
        //no hit
        for(Int_t k=0;k<MAX_SAMPLE;k++)
        {
            adc_avg=adc_avg+(Double_t)adc1[k];
        }
        return adc_avg*1./MAX_SAMPLE;
    }else{
        //with hit
        if(clk0==0){
            // First sample point is hit
            // This hit will be droped in the tracking stage
            return -1;
        }else if(clk0-2<0){
            // If the first hit is at second sample point,
            // The hit will be droped in the tracking stage
            return -2;
        }else{
            // Take samples from 0 up to 2 sample before the TDC hit
            for (Int_t k = clk0-2;k>=0;k--) {
                adc_avg=adc_avg+(Double_t)adc1[k];
                ii++;
            }
        }
        return adc_avg/ii;
    }
}

Int_t WaveFormAnalyzer::FindPeakWidth(Int_t HitSample, Int_t* adc,Double_t baseline)
{
    MAX_SAMPLE = ExperimentConfig::Get().GetSampleRECBE();
    if(HitSample<0) return -99;
    if(HitSample==MAX_SAMPLE) return -99;

    Int_t peakWidth;
    Int_t start;
    Int_t end;
    bool start_bool=false;
    bool end_bool=false;
    for(int i=HitSample;i>0;i--){
        if((double)adc[i]-baseline<0){
            start=i;
            start_bool=true;
            break;
        }else{
            start=-99;
        }
    }
    for(int i=HitSample;i<MAX_SAMPLE;i++){
        if((double)adc[i]-baseline<0){
            end=i;
            end_bool=true;
            break;
        }else{
            end=1;
        }
    }
    peakWidth=end-start;
    if(!end_bool || !start_bool){
        return -99;
    }else{
        return peakWidth;
    }
}

Double_t WaveFormAnalyzer::FindADCsum(Double_t ped, Int_t* adc)
{
    MAX_SAMPLE = ExperimentConfig::Get().GetSampleRECBE();

    Double_t q_tmp=0;
    Double_t adc_fixed=0;
    for(int clk=0;clk<MAX_SAMPLE;clk++){
        adc_fixed=(Double_t)adc[clk];
        if(adc_fixed<ped)adc_fixed=ped;
        q_tmp=q_tmp+(adc_fixed-ped);
    }
    return q_tmp;
}

Double_t WaveFormAnalyzer::FindADCsumCut(TH1D* h)
{
    double cut = 0;
    if(h->Integral()>100){
        h->Fit("gaus","Q");
        Double_t        sig  = h->GetFunction("gaus")->GetParameter(2);
        cut = sig*5 ;
    }else{
        cut = 10;
        std::cerr << h->GetName() << " has too few entries " << std::endl;
    }
    return cut;
}
