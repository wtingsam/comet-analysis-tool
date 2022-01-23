#ifndef _WAVEFORMANALYZER_HH_
#define _WAVEFORMANALYZER_HH_

#include "AnalyzerBase.hxx"

class WaveFormAnalyzer : public AnalyzerBase{

    public:
        WaveFormAnalyzer(){}
        virtual ~WaveFormAnalyzer(){}

	/// Find the peak of adc 
        Int_t FindPeakAdc(Int_t* adc0, Int_t clk0, Int_t clk1);

	/// Find the width of peak
        Int_t FindPeakWidth(Int_t HitSample, Int_t* adc,Double_t baseline);
	
	/// Find the ADC by considering also the pedestal
        Double_t FindADCsum(Double_t ped, Int_t* adc);
	
	/// Find the pedestal
        Double_t FindPedestal(Int_t* adc1, Int_t clk0);
	
	/// Find the cut of ADC sum to distinguish the noise and signal
        Double_t FindADCsumCut(TH1D* h);
	
        /// The TDC hit is chosen based on the threshold
        Double_t ChooseTDChit(Double_t *tdc0, Double_t *adcfp0, Int_t tdcNhit0, Double_t threshold, Double_t &peak_chosen, bool &cpFlag);
	
    private:
        int MAX_SAMPLE;

};

#endif
