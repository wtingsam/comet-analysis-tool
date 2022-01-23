#include "ITrackFitterMultPeakLinear.hxx"

static ITrackFitterMultPeakLinear *fitObj;

ITrackFitterMultPeakLinear::ITrackFitterMultPeakLinear(IHit* hitCon, ITrackContainer* tracks, bool debug)
{
    fTrackCon = new ITrackContainer();
    fHitCon = new IHit("Hits in ITrackFitterMultPeakLinear");
    fMinuit = new TMinuit(4);
    fMinuit->SetFCN(ITrackFitterMultPeakLinear::minuitFunction);

    // Initialize according to layer for pattern analysis
    MAX_SENSE_LAYER = ExperimentConfig::Get().GetNumOfLayer();
    for(int i=0;i<MAX_SENSE_LAYER;i++){
        fLayerHits_v.push_back(0);
    }
    for(int i=0;i<20;i++){
        for(int j=0;j<32;j++){
            fHitIndexlist[i][j]=-1;
        }
    }
    // Select hits from raw hits
    SelectHits(hitCon,tracks);
    fitObj = this;
    fDEBUG=debug;
}



bool ITrackFitterMultPeakLinear::SelectHits(IHit* hitCon, ITrackContainer* tracks){
    // get nTracks
}

ITrackFitterMultPeakLinear::~ITrackFitterMultPeakLinear(){
    if(fTrackCon) delete fTrackCon;
    if(fHitCon)   delete fHitCon;
    if(fMinuit)   delete fMinuit;
}

void ITrackFitterMultPeakLinear::minuitFunction(Int_t &npar, Double_t *gin, Double_t &f, Double_t *para, Int_t iflag)
{
    f = fitObj->CalculateChi2(para);
}

Double_t ITrackFitterMultPeakLinear::CalculateChi2(Double_t *par)
{
    double chi2;
    return chi2;
}
