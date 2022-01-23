#ifndef _ITRACKFITTERLINEAR_HXX_
#define _ITRACKFITTERLINEAR_HXX_

#include "TMinuit.h"

#include "WireManager.hxx"
#include "ITrackingUtility.hxx"
#include "ICDCGas.hxx"
#include "IHit.hxx"
#include "ITrackContainer.hxx"
#include "ITrackSummary.hxx"

class ITrackFitterLinear{
    private:
        // Hits corresponding to all tracks
        IHit* fHitConsRaw;
        std::vector<IHit*> fHitCons;
        std::map<int, std::vector<IHit*> >fTrackMapHitConsScan;
        ITrackContainer *fTrackCon;
        ITrackContainer *fTrackCon_out;
        int fCurrentTrack;
        int fCurrentCombi;
        int fCurrentNhitsUsed;
        bool fDEBUG;
        // Select hits by using track container
        void SelectHits(IHit* hitCon, ITrackContainer* trackCon);
    public:
        TMinuit* fMinuit;
        // Constructor
        ITrackFitterLinear(IHit* hitCon, ITrackContainer* trackCon, int debug=false);
        virtual ~ITrackFitterLinear();
        /// A function for doing minuit
        static void minuitFunction(Int_t &npar, Double_t *gin, Double_t &f, Double_t *para, Int_t iflag);

        /// Calculate chi2
        Double_t CalculateChi2(Double_t *par);

        /// Process 2 modes,
        /// 1: Simple fitting
        /// 2: Scan and fit
        void Process(int mode=0);
        void SimpleFit(ITrackSummary &tracks);
        void SimpleFit(double* par, double *par_err,
                       double *output, int *status);
        void ScanFit(std::vector<IHit*> hitCons);

        // Getters
        ITrackContainer *GetTracks(){   return fTrackCon_out;   }
        TString GetLeftRightPattern(Double_t *par);
};
#endif
