#ifndef _ICDCGAS_HXX_
#define _ICDCGAS_HXX_

#include <TROOT.h>
#include <TFile.h>
#include <TF1.h>
#include "TString.h"
#include "ExperimentConfig.hxx"
#include "CalibrationFile.hxx"
#include "WireManager.hxx"

class ICDCGas{
/***
    This class reads the x-t relations and gain relation.
***/
    public:
        static ICDCGas &Get();

        static void Reset()
            {
                delete fICDCGas; // REM : it works even if the pointer is NULL (does nothing then)
                fICDCGas = NULL; // so GetInstance will still work.
            }

        virtual ~ICDCGas();

        /// GetDriftModel
        /// Consider only two types for now, you either read the constant one
        /// or You read all other dependencies.
        Int_t StateToDriftModel(TString state){
            if(state == "constant"){
                fDriftModel = kUseConstantVelocity;
            }else {
                fDriftModel = kUseLayPhiBetaXtTable;
            }
            return fDriftModel;
        }

        /// Initialize
        void Initialize();
        void Initialize(Int_t driftModel, TString xtPath);
        void Initialize(Int_t driftModel, TString xtPath, TString gasType, bool forceReadXtList=false){
            fGasType=gasType;
            if(forceReadXtList)ReadXtMappingList();
            Initialize(driftModel,xtPath);
        }

        /// if forceReadXtList == true;
        /// Read the list of the x-t relation for mapping of which layer should use which x-t for i layer
        /// In the calibration stage, the x-t for outermost layer has high uncertainty due to the higher tracking error
        /// Therefore this function allows you to fix some layers to use certiain layers' x-t
        /// while updating some layers.
        void ReadXtMappingList();

        /// Check x-t return values for debugging
        void DebugXtValues(){
            for(int i=0;i<fPhi_bin;i++){
                double phi = i*1./fPhi_bin*360;
                double beta = -1;
                Double_t hitR  = ICDCGas::Get().GetDriftDistance(5,19,5,phi,beta);
                printf("%d phi %f beta %f R %f \n",i,phi,beta,hitR);
            }
        }

        void Ls(void);
        /// Set/get the simulation model of ionized electron drift
        /// 0 : constant drift velocity w/o B field
        /// 1 : use 1 Xt-function
        /// 2 : use Xt-function for each layer
        /// 3 : use Xt-function for each layer with angle dependence
        /// 4 : use Xt-function for each layer with angle and z dependence
        enum {
            kUseConstantVelocity  = 0,
            kUseAvgXtFunction     = 1,
            kUseLayXtTable        = 2,
            kUseLayPhiXtTable     = 3,
            kUseLayPhiBetaXtTable = 4,
            kUsePhiXtTable        = 5,
            kUsePhiBetaXtTable    = 6
        };

        /// Function to calculate the drift distance and time
        Double_t GetDriftDistance(Double_t driftT, Int_t layer, int wire, Double_t phi, Double_t beta);
        Double_t GetDriftDistance(Double_t driftT, Int_t layer, Double_t phi, Double_t beta){    return GetDriftDistance(driftT,layer,5,phi,beta);    }
        Double_t GetDriftDistance(Double_t driftT, Int_t layer, Double_t phi){    return GetDriftDistance(driftT,layer,5,phi,0);    }
        Double_t GetDriftDistance(Double_t driftT, Double_t phi, Double_t beta){     return GetDriftDistance(driftT,5,5,phi,beta);    }
        Double_t GetDriftDistance(Double_t driftT, Double_t phi){   return GetDriftDistance(driftT,5,5,phi,0);    }

        /// Get drift time using average x-t, 0guard layer is always registered as the averaged x-t relation
        Double_t GetDriftTime (double driftD){
            double driftT = -99;
            if(fXtLayPhiBetaFunction[5][0][0])
                driftT = fXtLayPhiBetaFunction[5][0][0]->GetX(driftD,0,300);
            return driftT;
        }

        /// Function to calculate the gas gain
        Double_t GetGasGain(Double_t q,Double_t *tracPar);

        /// Binning of x-t relations are pre-set, please do not change it unless it is necessary
        Int_t GetPhiBin(){ return fPhi_bin; }
        Int_t GetBetaBin(){ return fBeta_bin; }

        /// Get constant driftvelocity
        Double_t GetDriftVelocity(void) {  return fDriftVelocity;  }
        Double_t GetDriftVelocity(double t) {
            if(!fXtLayPhiBetaFunction[5][0][0]){
                //std::cerr << "@@@@ The xt function is not exist" << std::endl;
                return fDriftVelocity;
            }else{
                double par[5];
                for(int i=0;i<5;i++){
                    par[i] = fXtLayPhiBetaFunction[5][0][0]->GetParameter(i);
                }
                double velocity = fXtLayPhiBetaFunction[5][0][0]->Derivative(t);
                return velocity;
            }
        }

        /// Detele function
        void ClearXtRelations(){
            for(int l=0;l<20;l++)
                    for(int p=0;p<36;p++)
                        for(int b=0;b<10;b++)
                            if(fXtLayPhiBetaFunction[l][p][b]) delete fXtLayPhiBetaFunction[l][p][b];

        }

        /// Return xt relation of layer, beta and phi
        TF1 *GetFunction(int layer, int phi, int beta){
            return fXtLayPhiBetaFunction[layer][phi][beta];
        }

    private:
        // For default model
        void SetDriftModel(int value){   fDriftModel = value;  }
        /// Choose default constant drift velocity
        void SetDefaultDriftVelocity();
        /// Choose default xt path
        void SetDefaultXtPath();


        /// Counting number of xt being read
        bool EnsureAllXt();
        /// Set averaged xt function, Create functions for each layer with phi and beta
        void GetDefaultXtRelation(void);
        /// Set averaged xt function, Create functions for each layer with phi and beta
        void GetAveragedXtRelation(void);
        /// Set xt function, Create functions for each layer with phi and beta
        void GetLayPhiBetaXtRelation(void);

        /// Initialize x-t model to be used in the analysis.
        void InitModel(void);

        static ICDCGas *fICDCGas;
        ICDCGas(){}; //Don't Implenment
        void operator=(ICDCGas const&); //Don't Implenment

        int GetDriftModel(void) {  return fDriftModel;  }

        /// configuration variable
        int NUM_OF_BOARDS_CDC;

        ///Class
        Iteration *iteration;

        std::vector<TString> fForceXtUseLayer; // In layer accending order
        Int_t MAX_SENSE_LAYER;
        Int_t fTurningPoints_xt;
        Int_t fPhi_bin;
        Int_t fBeta_bin;
        Int_t fDriftModel;
        Double_t fDriftVelocity;
        Double_t *fBoundTimeBoard;
        Double_t fBoundTime[2]; //left right
        Double_t fBoundTimeLay[20][2]; //left right
        TString fXtFunctionPath;
        TString fXtFunctionDefaultPath;

        TFile    *fXtFunctionFile;
        TFile    *fXtDefault;

        TF1 *fXtLayPhiBetaFunction[20][36][10]; ///[layer][edge][PhiBin][BetaBin]
        TF1 *fXtAvgFunction;
        TString fXTFileName;

        TString fDefaultPath;

        TString fGasType;

        /// Gas Gain ADC to charge relation
        /// Gain adc to charge
        TF1 * fADC2ChargeFunction;
};


#endif
