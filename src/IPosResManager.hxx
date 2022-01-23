#ifndef __IPosResManager_hxx__
#define __IPosResManager_hxx__

#include "PositionResolution.hxx"

/***
    This class is dedicated for Get/Set value by using loaded position resolution relations in its parant class.
    PositionResolution.hxx
***/

class IPosResManager : public PositionResolution {
    public:
	static IPosResManager &Get();
	virtual ~IPosResManager();
	
	/// Set the function mannually
	void SetFunction(TF1* fitF){   fFittingFunction = fitF;   }
	
	/// Get the fitting results from the fitting 
	void GetParametersAndErrors(double *par,double *parErr);
	void GetParameters(double *par);
	void GetErrors(double *parErr);
	
	/// Get the fitting function
	TF1 *GetFunction(){   if(fFittingFunction) return fFittingFunction; else return NULL;  }

	/// Get the error from extrapolation of track
	double GetTrackingError();
	
	/***
	    Get the intrinsic error of wire
	    The intrinsic position resolution should minus out the tracking error
	    Tracking err is obtained from the fitting function
	***/
	double GetIntrinsicResolution(double driftDistance);	
	
    private:
	static IPosResManager *fIPosResManager;
        IPosResManager(){}; //Don't Implenment
        void operator=(IPosResManager const&); //Don't Implenment
	
};

#endif 
