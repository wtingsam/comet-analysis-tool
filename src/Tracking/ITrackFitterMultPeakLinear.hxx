#ifndef _ITRACKFITTERMULTPEAKLINEAR_HXX_
#define _ITRACKFITTERMULTPEAKLINEAR_HXX_

#include "TMinuit.h"

#include "WireManager.hxx"
#include "ITrackingUtility.hxx"
#include "ICDCGas.hxx"
#include "IHit.hxx"
#include "ITrackContainer.hxx"
#include "ITrackSummary.hxx"
// This class will pick up 1 hit per layer
// and then pick up all samples in the hit
// do the fitting to get the best one
// This class should be used when the hit
// list are clean and the parameters are very
// close to the real track
class ITrackFitterMultPeakLinear{
    private:
        IHit* fHitCon;
        ITrackContainer *fTrackCon;
        int MAX_SENSE_LAYER;
        // @{
        // These variables are for loop each pattern in the layer
        int fHitIndexlist[20][400];
        std::vector<int> fLayerHits_v;
        std::map<int, std::vector<int> > fPatternId2HitList;
        int fCurrentPatternID;
        // }
        bool fDEBUG;
    public:
        TMinuit* fMinuit;
        // Constructor
        ITrackFitterMultPeakLinear(IHit* hitCon, ITrackContainer* tracks, bool debug=false);
        // Destructor
        virtual ~ITrackFitterMultPeakLinear();

        /// A function for doing minuit
        static void minuitFunction(Int_t &npar, Double_t *gin, Double_t &f, Double_t *para, Int_t iflag);

        /// Calculate chi2
        Double_t CalculateChi2(Double_t *par);

        // Process track finding
        void Process();
    private:
        // Select hits from the raw hit container for track finding
        bool SelectHits(IHit* hitCon,ITrackContainer* tracks);
};
#endif
