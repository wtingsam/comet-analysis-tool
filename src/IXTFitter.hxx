#ifndef _IXTFitter_HXX_
#define _IXTFitter_HXX_

#include "iostream"
#include "TGraphErrors.h"
#include "TF1.h"

class IXTFitter{
    public:
        IXTFitter(){}

        IXTFitter(TF1 *lowXt, TF1 *upXt);

        virtual ~IXTFitter();

        TF1 *Combined2TF1(TF1 *lowXt, TF1 *upXt, double turnpt_x, double min_x, double max_x);
        
	TF1 *Combined3TF1(TF1 *f0, TF1 *f1, TF1 *f2, double *turnpt_x, double min_x, double max_x);

        TString MakePolyString(int nPar, int offset=0){
            TString name="";
            TString xstring;
            name = Form("[%d]",offset);
            TString x = "x";
            for(int i=1;i<nPar;i++){
                name += Form(" +[%d]*%s",i+offset,x.Data());
                x+="*x";
            }
            //std::cout << "MakePolyString " << nPar << " name " << name << std::endl;
            return name;
        }

        TString PolyToString(TF1* f1){
            TString name="";
            int nPar = f1->GetNpar();
            TString xstring;
            name = Form("(%.5e)",f1->GetParameter(0));
            TString x = "x";
            for(int i=1;i<nPar;i++){
                name += Form(" +(%.5e)*%s",f1->GetParameter(i),x.Data());
                x+="*x";
            }
            std::cout << "PolyToString " << nPar << " name " << name << std::endl;
            return name;
        }

        void SetName(TString value){  fNameResultXt = value;  }

        void Print();

        void Save(TString name="test.pdf");

        void Clear();
    private:

        int fNumOfFittingPar;
        TGraphErrors *fTempXtGraph;
        TF1 *fResultXt;
        TF1 *fUpXt;
        TF1 *fLowXt;
        TString fNameResultXt;
        std::vector<double> initPar;
};

#endif
