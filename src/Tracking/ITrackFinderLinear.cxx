#include "ITrackFinderLinear.hxx"
#include "ITrackSummary.hxx"

static ITrackFinderLinear *fitObj;

ITrackFinderLinear::ITrackFinderLinear(IHit* hitCon, bool debug)
{
   fTrackCon = new ITrackContainer();
    fHitCon = new IHit("Hits in Track Finding");
    fMinuit = new TMinuit(2);
    fMinuit->SetFCN(ITrackFinderLinear::minuitFunction);

    // Initialize according to layer for pattern analysis
    MAX_SENSE_LAYER = ExperimentConfig::Get().GetNumOfLayer();
    for(int i=0;i<MAX_SENSE_LAYER;i++){
        fLayerHits_v.push_back(0);
    }
    for(int i=0;i<20;i++){
        for(int j=0;j<400;j++){
            fHitIndexlist[i][j]=-1;
        }
    }
    // Select hits from raw hits
    SelectHits(hitCon);
    fitObj = this;
    fDEBUG=debug;
}

void ITrackFinderLinear::minuitFunction(Int_t &npar, Double_t *gin, Double_t &f, Double_t *para, Int_t iflag)
{
    f = fitObj->CalculateChi2(para);
}

TString ITrackFinderLinear::GetLeftRightPattern(Double_t *par){
    TString pattern;
    // Get the hits from container
    std::vector<int> hitList = fPatternId2HitList[fCurrentPatternID];
    const int nsize = (int)hitList.size();
    std::vector<int>    v_lay       = fHitCon->GetHitLayer();
    std::vector<int>    v_cel       = fHitCon->GetHitCell();
    std::vector<double> v_hR        = fHitCon->GetHitR();
    for(int i=0;i<nsize;i++){
        // Get the hit index id
        int iHit = hitList[i];
        // Get hit information
        Int_t layer = v_lay[iHit];
        Int_t cell  = v_cel[iHit];
        // Do 2D fitting according to the scan values
        double x,y;
        ITrackingUtility::GetXYPos(layer,cell,par[2],par[3],x,y);
        double fitR=ITrackingUtility::GetFitR2D(x,y,par);
        if(fitR>0)
            pattern+="+";
        else
            pattern+="-";
    }
    return pattern;
}
Double_t ITrackFinderLinear::CalculateChi2(Double_t *par)
{
    Double_t chisq = 0;
    // Get the hits from container
    std::vector<int> hitList = fPatternId2HitList[fCurrentPatternID];
    const int nsize = (int)hitList.size();
    std::vector<int>    v_lay       = fHitCon->GetHitLayer();
    std::vector<int>    v_cel       = fHitCon->GetHitCell();
    std::vector<int>    v_leftright = fHitCon->GetLeftRight();
    std::vector<double> v_hR        = fHitCon->GetHitR();
    fCurrentNhitsUsed=nsize;
    for(int i=0;i<nsize;i++){
        // Get the hit index id
        int iHit = hitList[i];
        // Get hit information
        Int_t layer = v_lay[iHit];
        Int_t cell  = v_cel[iHit];
        double hitR = v_hR[iHit];
        // Do 2D fitting according to the scan values
        double x,y;
        ITrackingUtility::GetXYPos(layer,cell,fScan_m,fScan_c,x,y);
        double fitR=fabs(ITrackingUtility::GetFitR2D(x,y,par));
        double err=0.2;
        double delta  = (fabs(hitR)-fabs(fitR))/err;
        chisq+=(delta*delta);
    }
   return chisq;
}


ITrackFinderLinear::~ITrackFinderLinear(){
    if(fTrackCon) delete fTrackCon;
    if(fHitCon)   delete fHitCon;
    if(fMinuit)   delete fMinuit;
}

bool ITrackFinderLinear::SelectHits(IHit* hitCon){
    // Add hits for track finding 's container
    std::vector<int> v_lay      = hitCon->GetHitLayer();
    std::vector<int> v_cel      = hitCon->GetHitCell();
    std::vector<int> v_lr       = hitCon->GetLeftRight();
    std::vector<double> v_q     = hitCon->GetQ();
    std::vector<double> v_p     = hitCon->GetADCPeak();
    std::vector<double> v_hitR  = hitCon->GetHitR();
    int testLayer = hitCon->GetTestLayer();
    int nHits = (int)v_lay.size();
    // Cut out hits
    double fSigToBgQRatio = 0.3;
    double fSigToBgPeakRatio = 0.3;
    int nSelectedHits = 0;
    for(int i=0;i<nHits;i++){
        int    layer = v_lay.at(i);
        int    cell = v_cel.at(i);
        double q     = v_q.at(i);
        double p     = v_p.at(i);
        double hitR  = v_hitR.at(i);
        int leftRight = v_lr.at(i);
        // Pick up hits which has big charge and also
        // Layer hits <=2
        if(testLayer==layer ||
           hitCon->GetMaxADCSUM(layer)*fSigToBgQRatio>q ||
           hitCon->GetMaxADCPeak(layer)*fSigToBgPeakRatio>p){
            continue;
        }
        fHitCon->AddHitWire(layer,cell);
        fHitCon->AddQ(q);
        fHitCon->AddADCPeak(p);
        fHitCon->AddHitR(hitR);
        fHitCon->AddLeftRight(leftRight);
        nSelectedHits++;
    }
    fHitCon->SetNumOfHits(nSelectedHits);
    // Set variables for pattern analysis
    if(fDEBUG) fHitCon->Show();
    v_lay = fHitCon->GetHitLayer();
    nHits = (int)v_lay.size();
    for(int i=0;i<nHits;i++){
        int layer = v_lay[i];
        double q     = v_q.at(i);
        double p     = v_p.at(i);
        fHitIndexlist[layer][fLayerHits_v[layer]]=i;
        fLayerHits_v[layer]++;
    }
    return true;
}

void ITrackFinderLinear::Process(){
    fPatternId2HitList=ITrackingUtility::GetHitPattern(fHitIndexlist,fLayerHits_v);
    // Loop over patterns always pick up 1 hit in each layer (maximum 2)
    std::map<int,std::vector<int> >::iterator it;
    if(fPatternId2HitList.size()>20)return ;
    for(it=fPatternId2HitList.begin();
        it!=fPatternId2HitList.end();
        it++){
        // Get information from mapping
        int ipatId = it->first;
        std::vector<int> pat = it->second;
        if((int)pat.size()<=4)continue;
        // Set the current pattern id, very important for calling fcn function
        fCurrentPatternID = ipatId;
        std::vector<ITrackSummary> tmp_tracks;
        ScanZ(tmp_tracks,
              -150,150,35,
              -150,150,35,
              // -300,300,150,
              // -300,300,150,
              "position",
              4
              );
        if(fDEBUG)printf("## Pattern Id %d Saved tracks \n",ipatId);
        for(int i=0;i<tmp_tracks.size();i++){
            //if(fDEBUG)tmp_tracks[i].Show();
            if(fDEBUG)printf(" ITrackFinderLinear nHits %d DOF %d\n",(int)pat.size(),(int)pat.size()-4);
            double chi2 = tmp_tracks[i].GetChi2();
            if(chi2<500 && chi2>0){
                fTrackCon->AddTracks(tmp_tracks[i]);
            }
        }
    }
}
void ITrackFinderLinear::Call2DFitter(double* par, double *par_err,
                                      double *output, int *status)
{
    fScan_m = par[2];
    fScan_c = par[3];
    Double_t arglist[10]={0};
    Int_t ierflg = 0;
    fMinuit->SetPrintLevel(-1);
    //fMinuit->SetPrintLevel(1);
    fMinuit->mnexcm("SET NOW", arglist ,1,ierflg);
    arglist[0] = 1;
    fMinuit->mnexcm("SET ERR", arglist ,1,ierflg);

    //Set initial value
    Double_t vstart_m = 0;
    Double_t vstart_c = 0;
    Double_t step[2] = {1e-3 , 1e-1};
    fMinuit->mnparm(0, "a1", vstart_m, step[0],0,0,ierflg);
    fMinuit->mnparm(1, "a2", vstart_c, step[1],0,0,ierflg);

    // Fix the parameters that is related to scanning
    arglist[0] = 1000;
    arglist[1] = 1.;

    // Do minimisation
    fMinuit->mnexcm("SIMPLEX", arglist ,2,ierflg);
    fMinuit->mnexcm("MIGRAD" , arglist ,2,ierflg);

    // Get results
    for(int i=0;i<2;i++){
        fMinuit->GetParameter(i,par[i], par_err[i]) ;
    }
    Double_t fmin,fedm,errdef;
    Int_t npari,nparx,istat;
    fMinuit->mnstat (fmin, fedm, errdef, npari, nparx, istat);
    output[0] = fmin;
    output[1] = fedm;
    output[2] = errdef;
    status[0] = npari;
    status[1] = nparx;
    status[2] = istat;

}

void ITrackFinderLinear::ScanZ(std::vector<ITrackSummary> &tracks,
                               double min_x, double max_x, int step_x,
                               double min_z, double max_z, int step_z,
                               TString option, int ntracks_limit){
    // Get size of the scintillator
    double range_z = fabs(min_z - max_z); //Always the chamber axis
    double range_x = fabs(min_x - max_x); //Always the chamber axis
    double z_up,z_down;
    double x_up = max_x;
    double x_down = min_x;

    //  loop over the parameter/position space

    int itrkId = 0;
    std::map<int, ITrackSummary> map_index_results;
    std::vector<std::pair<double,int> >v_index_chi2;
    for(int k=0;k<step_x;k++){
        for(int l=0;l<step_z;l++){
            double tmp_par[4];
            double tmp_par_err[4];
            double tmp_output[3];
            int tmp_status[3];

            // Select situation
            if(option.Contains("position")){
                z_up = min_z + (range_z)*l/(step_z-1);
                z_down = min_z + (range_z)*k/(step_z-1);
                tmp_par[2] = (z_up-z_down)/(x_up-x_down);
                tmp_par[3] = z_up - tmp_par[2]*x_up ;
            }else if(option.Contains("parameter")){
                tmp_par[2] = min_x + (range_x)*k/(step_x-1);
                tmp_par[3] = min_z + (range_z)*l/(step_z-1);
            }
            // Call 2D fitting
            Call2DFitter(tmp_par,tmp_par_err,
                         tmp_output,tmp_status);
            // Get left right Pattern, make sure they dont have the same pattern
            TString patternS=GetLeftRightPattern(tmp_par);
            // Fill summary
            ITrackSummary tmp_summary(Form("scan%d",itrkId),0);
            tmp_summary.Fill(tmp_par,tmp_par_err,tmp_output,tmp_status,patternS,fCurrentNhitsUsed);
            // Select minimum chi2
            map_index_results[itrkId]=tmp_summary;
            v_index_chi2.push_back(std::make_pair(tmp_output[0],itrkId));
            itrkId++;
        }
    }
    // Sort out chi2
    std::sort(v_index_chi2.begin(),v_index_chi2.end(),std::less<std::pair<double,int> >() );

    // Choose smallest chi2 track with pattern that is not found before
    std::vector<TString> found_pattern;
    const int save_ntracks = ntracks_limit;
    int ntracks_saved = 0;
    std::vector<ITrackSummary> saved_tracks;
    for(int i=0;i<v_index_chi2.size();i++){
        // Get track id
        int itrk=v_index_chi2[i].second;
        // Get Pattern
        TString pattern = map_index_results[itrk].GetPattern();

        // Check if this track is found before or not
        bool save=true;
        for(int w=0;w<found_pattern.size();w++){
            if(found_pattern[w]==pattern){
                save=false;
                break;
            }
        }
        // Save if not found
        if(save){
            found_pattern.push_back(pattern);
            saved_tracks.push_back(map_index_results[itrk]);
            tracks.push_back(map_index_results[itrk]);
            ntracks_saved++;
        }
        if(ntracks_saved==save_ntracks)
            break;
    }
}
