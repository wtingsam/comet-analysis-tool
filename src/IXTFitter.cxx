#include "IXTFitter.hxx"
#include "TCanvas.h"

IXTFitter::IXTFitter(TF1 *lowXt, TF1 *upXt):
    fUpXt(upXt),fLowXt(lowXt)
{
    fNameResultXt="";

    for(int i=0;i<(int)fLowXt->GetNpar();i++){
        initPar.push_back(fLowXt->GetParameter(i));
    }
    for(int i=0;i<(int)fUpXt->GetNpar();i++){
        initPar.push_back(fUpXt->GetParameter(i));
    }
    initPar.push_back(370);
    fNumOfFittingPar = fLowXt->GetNpar() + fUpXt->GetNpar() + 1;
    // 1 for the turning point

}

IXTFitter::~IXTFitter(){}
TF1 *IXTFitter::Combined3TF1(TF1 *f0, TF1 *f1, TF1 *f2, double *turnpt_x, double min_x, double max_x){
    // Fill in all parameters
    std::vector<double> input_parameters;
    int nPars = 
	(int)f0->GetNpar()+
	(int)f1->GetNpar()+
	(int)f2->GetNpar();	
    for(int i=0;i<(int)f0->GetNpar();i++){
        input_parameters.push_back(f0->GetParameter(i));
    }
    for(int i=0;i<(int)f1->GetNpar();i++){
        input_parameters.push_back(f1->GetParameter(i));
    }
    for(int i=0;i<(int)f2->GetNpar();i++){
        input_parameters.push_back(f2->GetParameter(i));
    }
    // Make string for formula
    TString name0 = MakePolyString(f0->GetNpar());
    TString name1 = MakePolyString(f1->GetNpar(),f0->GetNpar());
    TString name2 = MakePolyString(f2->GetNpar(),f0->GetNpar()+f1->GetNpar());
    int iTurnPar0 = nPars;
    int iTurnPar1 = nPars+1;
    int iTurnPar2 = nPars+2;

    TString formula = Form("(x<=[%d])*0 + (x>[%d] && x<=[%d])*(%s) + (x>[%d] && x<=[%d])*(%s) + (x>[%d])*(%s)",
			   iTurnPar0,
			   iTurnPar0,iTurnPar1,
			   name0.Data(),
			   iTurnPar1,iTurnPar2,
			   name1.Data(),
			   iTurnPar2,
			   name2.Data());
    printf("  Formula for f0  %s \n",name0.Data());
    printf("  Formula for f1  %s \n",name1.Data());
    printf("  Formula for f2  %s \n",name2.Data());
    printf("%s \n",formula.Data());
    fResultXt = new TF1(fNameResultXt,formula,min_x,max_x);
    fResultXt->SetTitle(Form("pol%d + pol%d + pol%d",
    			     (int)f0->GetNpar(),
    			     (int)f1->GetNpar(),
    			     (int)f2->GetNpar()));
    for(int i=0;i<(int)input_parameters.size();i++){
        fResultXt->FixParameter(i,input_parameters[i]);
    }
    fResultXt->FixParameter(iTurnPar0,turnpt_x[0]);
    fResultXt->FixParameter(iTurnPar1,turnpt_x[1]);
    fResultXt->FixParameter(iTurnPar2,turnpt_x[2]);
    // Refix the zero points
    double zero=fResultXt->GetX(0,-5,5);
    printf("zero %f \n",zero);
    fResultXt->FixParameter(iTurnPar0,zero);
    return fResultXt;
}

TF1 *IXTFitter::Combined2TF1(TF1 *lowXt, TF1 *upXt, double turnpt_x, double min_x, double max_x){
    // Fill in all parameters
    std::vector<double> input_parameters;
    int nPars = (int)lowXt->GetNpar()+(int)upXt->GetNpar();
    for(int i=0;i<(int)lowXt->GetNpar();i++){
        input_parameters.push_back(lowXt->GetParameter(i));
    }
    for(int i=0;i<(int)upXt->GetNpar();i++){
        input_parameters.push_back(upXt->GetParameter(i));
    }
    // Make string for formula
    TString name1 = MakePolyString(lowXt->GetNpar());
    TString name2 = MakePolyString(upXt->GetNpar(),lowXt->GetNpar());
    int iTurnPar = nPars;
    TString formula = Form("(x<=[%d])*(%s) + (x>[%d])*(%s)",iTurnPar,name1.Data(),iTurnPar,name2.Data());
    printf("  Formula for lowXt %s upXt %s \n     --- %s\n",lowXt->GetName(),upXt->GetName(),formula.Data());
    // Fill parameters to combined x-t and fix all parameters
    fResultXt = new TF1(fNameResultXt,formula,min_x,max_x);
    for(int i=0;i<(int)input_parameters.size();i++){
        fResultXt->FixParameter(i,input_parameters[i]);
    }
    fResultXt->FixParameter(iTurnPar,turnpt_x);
    //fResultXt->SetParameter(iTurnPar,turnpt_x);
    // double ratio = 0.1;
    // double fit_min_x = turnpt_x*(1-ratio);
    // double fit_max_x = turnpt_x*(1+ratio);
    // fResultXt->SetParLimits(iTurnPar,fit_min_x,fit_max_x);
    return fResultXt;
}

void IXTFitter::Print(){
    if(fResultXt){
        for(int i=0;i<fResultXt->GetNpar();i++){
            printf("par%d =  %f \n",i,fResultXt->GetParameter(i));
        }
    }
}

void IXTFitter::Save(TString name){
    TCanvas *C = new TCanvas("720,512");
    fTempXtGraph->Draw("ap");
    fResultXt->Draw("same");
    C->SaveAs(name);
    C->Close();
}

void IXTFitter::Clear(){
    fTempXtGraph->Clear();
    fResultXt->Clear();
}
