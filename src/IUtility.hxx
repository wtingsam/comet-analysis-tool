#ifndef _IUTILITY_HH_
#define _IUTILITY_HH_

#include "TH1D.h"
#include "TF1.h"
#include "TVector3.h"
//#include "Iteration.hxx"
#include "WireManager.hxx"
#include "AnalyzerBase.hxx"
#include "VectorAnalysis.hxx"

class IUtility : public AnalyzerBase{

    private:
        IUtility(){}
        virtual ~IUtility(){}
    public:
    static bool Repeated(std::vector<double> *v, double val, double e=1e-3){
        for(int i=0;i<(int)v->size();i++){
            if(fabs(v->at(i)-val)<e)
                return true;
        }
        return false;
    }

};

#endif
