#include "IPosResManager.hxx"

IPosResManager *IPosResManager::fIPosResManager = NULL;

IPosResManager &IPosResManager::Get(){
    if ( !fIPosResManager ){//make sure only being created one time
        fIPosResManager = new IPosResManager();	
    }    
    return *fIPosResManager;
}

IPosResManager::~IPosResManager(){}

double IPosResManager::GetTrackingError(){
    // Check exist of fFittingFunction
    if(fFittingFunction==NULL){
	//std::cerr <<  "@@@@ No Fitting function" << std::endl;
	return 0;
    }
    double trackErr = fFittingFunction->GetParameter(4);
    return trackErr;
}

double IPosResManager::GetIntrinsicResolution(double driftDistance){
    // Force to default default
    if(!fIsUseErrFun) return fDefaultIntrSReso;
    // Check exist of fFittingFunction
    if(fFittingFunction==NULL)
	return fDefaultIntrSReso;
    double trackErr = fFittingFunction->GetParameter(4);
    double totalErr = fFittingFunction->Eval(fabs(driftDistance));
    double intriErr = sqrt(totalErr*totalErr-trackErr*trackErr);
    if(driftDistance<=8 && driftDistance>=-8)
	return  intriErr;
    else
	return  fDefaultIntrSReso; //
}
	
void IPosResManager::GetParametersAndErrors(double *par, double *parErr){
    for(int i=0;i<5;i++){
	par[i]	  = fFittingFunction->GetParameter(i);
	parErr[i] = fFittingFunction->GetParError(i);
    }

}

void IPosResManager::GetParameters(double *par){
    for(int i=0;i<5;i++){
	par[i]	  = fFittingFunction->GetParameter(i);
    }

}
