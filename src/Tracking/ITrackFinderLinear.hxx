#ifndef _ITRACKFINDERLINEAR_HXX_
#define _ITRACKFINDERLINEAR_HXX_

#include "TMinuit.h"

#include "WireManager.hxx"
#include "ITrackingUtility.hxx"
#include "ICDCGas.hxx"
#include "IHit.hxx"
#include "ITrackContainer.hxx"
#include "ITrackSummary.hxx"

class ITrackFinderLinear{
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
        int fCurrentNhitsUsed;
        // }
        // These variables are for scanZ
        // @{
        double fScan_m;
        double fScan_c;
        // }
        bool fDEBUG;
    public:
        TMinuit* fMinuit;
        // Constructor
        ITrackFinderLinear(IHit* hitCon, bool debug=false);
        // Destructor
        virtual ~ITrackFinderLinear();

        // Getters
        ITrackContainer *GetTracks(){  return fTrackCon;  }
        IHit *GetHits(){  return fHitCon;  }

        // Process track finding
        void Process();

        // A function for scanning z position/ parameters
        // and then call fitting in 2D
        void ScanZ(std::vector<ITrackSummary> &tracks,
                   double min_x, double max_x, int step_x,
                   double min_z, double max_z, int step_z,
                   TString option,int ntracks_limit=2);

        void Call2DFitter(double *par, double *par_err,
                          double *output, int *status);

        /// A function for doing minuit
        static void minuitFunction(Int_t &npar, Double_t *gin, Double_t &f, Double_t *para, Int_t iflag);

        /// Calculate chi2
        Double_t CalculateChi2(Double_t *par);

    private:
        // Select hits from the raw hit container for track finding
        bool SelectHits(IHit* hitCon);
        // Get left right pattern
        TString GetLeftRightPattern(Double_t *par);
};

#endif
