#include "DriftTimeAnalyzer.hxx"

DriftTimeAnalyzer::DriftTimeAnalyzer(){}
DriftTimeAnalyzer::~DriftTimeAnalyzer(){}


Double_t DriftTimeAnalyzer::FixTDC(Int_t TDC)
{
    if(TDC>10000){
	TDC-=pow(2,15);
	return TDC;
    }else{
	return TDC;
    } 
} 
 
Double_t DriftTimeAnalyzer::FixTDC(Double_t TDC)
{ 
    if(TDC>10000){
	TDC-=pow(2,15);
	return TDC;
    }else{
        return TDC;
    }
}

Double_t DriftTimeAnalyzer::FitT0(TH1D *h, TF1 *fitFunction, TString option)
{
    if(h->Integral()>100){
        Double_t MaxV = h->GetMaximum();
        fitFunction->SetParLimits(0,0.1,50);
        fitFunction->SetParLimits(1,MaxV-MaxV*0.3,MaxV+MaxV*0.3);
	fitFunction->SetParLimits(2,0,2);
	fitFunction->SetParLimits(4,-100,100);
	fitFunction->SetParLimits(5,2.0,5.0);
        h->Fit(Form("%s",fitFunction->GetName()),option,"",-100,100);
        return fitFunction->GetParameter(4);
    }else{
        std::cerr << h->GetName() << " has too few entries " << std::endl;
        return -1;
    }
}

Double_t DriftTimeAnalyzer::FitT0(TH1D *h, TF1 *fitFunction, TString option, Double_t min, Double_t max)
{
    if(h->Integral()>100){
        Double_t MaxV = h->GetMaximum();
        fitFunction->SetParLimits(0,1,100);
        fitFunction->SetParLimits(1,MaxV-MaxV*0.5,MaxV+MaxV*0.5);
	fitFunction->SetParLimits(2,0,1);
	fitFunction->SetParLimits(4,min,max);
	fitFunction->SetParLimits(5,1.0,10.0);
        h->Fit(Form("%s",fitFunction->GetName()),option,"",min,max);
        return fitFunction->GetParameter(4);
    }else{
        std::cerr << h->GetName() << " has too few entries " << std::endl;
        return -1;
    }
}

Double_t DriftTimeAnalyzer::FitEdge(TH1D *h, TF1 *fitFunction, TString option)
{
    // Stop using this fitting function
    if(h->Integral()>100){
        fitFunction->SetParLimits(0,1,100);
        fitFunction->SetParLimits(1,1,1e2);
        fitFunction->SetParLimits(2,0.1,2);
        fitFunction->SetParLimits(3,100,700);
        fitFunction->SetParLimits(4,1,20);
	std::cout << fitFunction->GetName() << std::endl;
        h->Fit(Form("%s",fitFunction->GetName()),option,"",200,500);
        return fitFunction->GetParameter(3);
    }else{
        std::cerr << h->GetName() << " has too few entries " << std::endl;
        return -1;
    }
}
Double_t DriftTimeAnalyzer::FitFunction_findt0(Double_t *t,Double_t *par)
{ //From KLOE
    return par[0]+par[1]*exp(-par[2]*(t[0]-par[3]))/(1+exp(-(t[0]-par[4])/par[5]));
}

Double_t DriftTimeAnalyzer::FitFunction_findEdge(Double_t* t,Double_t *par)
{ //From Atlas
    // Stop using this fitting function
    return par[0]+(par[1]*t[0]+par[2])/(1+exp((t[0]-par[3])/par[4]));
}

Double_t DriftTimeAnalyzer::FitFunction_driftTime(Double_t* t,Double_t *par)
{ //From Atlas
    return par[0]+(par[1]+par[2]*exp(-t[0]/par[3]))/((1+exp((par[4]-t[0])/par[5]))*(1+exp((t[0]-par[6])/par[7])));
}

