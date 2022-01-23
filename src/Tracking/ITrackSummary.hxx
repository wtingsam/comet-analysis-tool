#ifndef _ITRACKSummary_HXX_
#define _ITRACKSummary_HXX_
#include "TString.h"
class ITrackSummary{
    private:
        double fPar[5]; // Track parameters
        double fPar_err[5]; // err
        double fOutput[3]; // 0: Chi2 1: EDM 2: ?
        int fStatus[3]; // Check Root CERN webpage
        TString fPattern;

        TString fName;
        int fTrackType; // 0: linear, 1: Helix
        int fTrackDOF;
        int fUsedHits;
    public:
        ITrackSummary(){fTrackType=0;}
        ITrackSummary(TString name, int track_type);
        void Fill(const double *val_par,
                  const double *val_par_err,
                  const double *val_output,
                  const int *val_status,
                  const TString val_pattern
                  );

        void Fill(const double *val_par,
                  const double *val_par_err,
                  const double *val_output,
                  const int *val_status,
                  const TString val_pattern,
                  int usedHits
                  );
        void Show();

        /// Getters
        /// @{
        int GetDOF(){   return fUsedHits-fTrackDOF;   }
        double GetChi2(){   return fOutput[0];   }
        void GetTrackPar(double *par){   for(int i=0;i<fTrackDOF;i++) par[i] = fPar[i];   }
        void GetTrackParError(double *par_err){   for(int i=0;i<fTrackDOF;i++) par_err[i] = fPar_err[i];   }
        TString GetPattern(){   return fPattern;   }
        // }
};

#endif
