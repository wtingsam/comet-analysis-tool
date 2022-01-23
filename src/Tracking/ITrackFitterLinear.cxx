#include "ITrackFitterLinear.hxx"

static ITrackFitterLinear *fitObj;
ITrackFitterLinear::ITrackFitterLinear(IHit* hitCon, ITrackContainer* trackCon, int debug)
{
    fHitConsRaw = hitCon;
    fTrackCon = new ITrackContainer();
    fTrackCon_out = new ITrackContainer();
    SelectHits(hitCon,trackCon);
    fMinuit = new TMinuit(4);
    fMinuit->SetFCN(ITrackFitterLinear::minuitFunction);
    fCurrentTrack=-1;
    fCurrentCombi=-1;
    fCurrentNhitsUsed=0;
    fDEBUG=debug;
    fitObj=this;
}

ITrackFitterLinear::~ITrackFitterLinear()
{
    delete fTrackCon;
    delete fTrackCon_out;
    delete fMinuit;
    for(int i=0;i<(int)fHitCons.size();i++)
        delete fHitCons[i];
}
void ITrackFitterLinear::minuitFunction(Int_t &npar, Double_t *gin, Double_t &f, Double_t *para, Int_t iflag)
{
    f = fitObj->CalculateChi2(para);
}

TString ITrackFitterLinear::GetLeftRightPattern(Double_t *par)
{
    TString patternS="";
    // Get the track id and hit infomration
    const int iTrk=fCurrentTrack;
    const int ic  =fCurrentCombi;
    std::vector<int>    v_lay = fTrackMapHitConsScan[iTrk][ic]->GetHitLayer();
    std::vector<int>    v_cel = fTrackMapHitConsScan[iTrk][ic]->GetHitCell();
    std::vector<double> v_dt  = fTrackMapHitConsScan[iTrk][ic]->GetDriftTime();
    std::vector<int>    v_lr  = fTrackMapHitConsScan[iTrk][ic]->GetLeftRight();
    const int nsize = v_lay.size();
    // Loop over fPatternId2HitList mapping
    for(int iHit=0;iHit<nsize;iHit++){
        // Get hit information
        int  layer = v_lay[iHit];
        int  cell  = v_cel[iHit];
        Double_t fitR  = ITrackingUtility::GetFitDistance(layer,cell,par);
        if(fitR>0)
            patternS+="+";
        else
            patternS+="-";
    }
    return patternS;
}

Double_t ITrackFitterLinear::CalculateChi2(Double_t *par)
{
    Double_t chisq = 0;
    // Get the track id and hit infomration
    const int iTrk=fCurrentTrack;
    const int ic  =fCurrentCombi;
    std::vector<int>    v_lay = fTrackMapHitConsScan[iTrk][ic]->GetHitLayer();
    std::vector<int>    v_cel = fTrackMapHitConsScan[iTrk][ic]->GetHitCell();
    std::vector<double> v_dt  = fTrackMapHitConsScan[iTrk][ic]->GetDriftTime();
    std::vector<int>    v_lr  = fTrackMapHitConsScan[iTrk][ic]->GetLeftRight();
    const int nsize = v_lay.size();
    fCurrentNhitsUsed=nsize;
    // Loop over fPatternId2HitList mapping
    for(int iHit=0;iHit<nsize;iHit++){
        // Get hit information
        int  layer = v_lay[iHit];
        int  cell  = v_cel[iHit];
        int  lr    = v_lr[iHit];
        double dt    = v_dt[iHit];
        // Get Fit R, notice that it has sign
        Double_t fitR = ITrackingUtility::GetFitDistance(layer,cell,par);
        //double   phi  = ITrackingUtility::GetIncidentAngle(layer, cell, par);
        double   phi  = (lr==1)?0:180;
        double   hitR = ICDCGas::Get().GetDriftDistance(dt,layer,cell,phi,0);
        double err=0.2;
        double delta  = (hitR-fitR)/err;
        delta*=delta;
        chisq+=delta;
    }
    return chisq;
}

void ITrackFitterLinear::Process(int mode){
    // Loop over tracks
    const int nTracks = fTrackCon->GetNTracks();
    std::vector< ITrackSummary > trackSummaries = fTrackCon->GetTrackSummaries();
    std::vector<TString> found_pattern;
    for(int iTrk=0;iTrk<nTracks;iTrk++){
        ITrackSummary a_trackSummary = trackSummaries[iTrk];
        double par[4];
        // Get infomation from track container
        a_trackSummary.GetTrackPar(par);
        // Set current track ID
        fCurrentTrack = iTrk;
        // Get nTracks
        int nSelectedHits = fHitCons[iTrk]->GetNumOfHits();
        if(nSelectedHits<5){    if(fDEBUG)printf(" Skip Number of hits %d \n",nSelectedHits);  continue;   }
        // Do Simple fitting
        for(int ic = 0;ic<fTrackMapHitConsScan[iTrk].size();ic++){
            // Set current combintation id of left right
            fCurrentCombi=ic;
            //fTrackMapHitConsScan[iTrk][ic]->Show();
            // Continue if the chi2 is too big to avoid looking at all combinations
            if(CalculateChi2(par)>100)continue;
            //SimpleFit(par,par_err,output,status);
            SimpleFit(a_trackSummary);
            // Get pattern
            TString patternS = a_trackSummary.GetPattern();
            bool save = true;
            for(int w=0;w<(int)found_pattern.size();w++)
                if(patternS==found_pattern[w]){
                    save=false;
                    break;
                }
            //std::cout <<"CalculateChi2(par)>100" << CalculateChi2(par) << std::endl;
            double chi2=a_trackSummary.GetChi2();
            if(chi2<500 && save){
                found_pattern.push_back(patternS);
                // if(fDEBUG)printf("ITrackFitterLinear nHits %d DOF %d \n",
                //                  fTrackMapHitConsScan[iTrk][ic]->GetNumOfHits(),
                //                  fTrackMapHitConsScan[iTrk][ic]->GetNumOfHits()-4);
                //if(fDEBUG) a_trackSummary.Show();
                fTrackCon_out->AddTracks(a_trackSummary);
            }
        }
    }
}

void ITrackFitterLinear::SimpleFit(double *par, double *par_err,
                                   double *output, int *status)
{
    Double_t arglist[10]={0};
    Int_t ierflg = 0;
    fMinuit->SetPrintLevel(-1);
    //fMinuit->SetPrintLevel(1);
    fMinuit->mnexcm("SET NOW", arglist ,1,ierflg);
    arglist[0] = 1;
    fMinuit->mnexcm("SET ERR", arglist ,1,ierflg);
    //Set initial value
    Double_t step[4] = {0.001,0.01,0.001,0.01};
    Double_t ranges_min[4] = {-3,-300,-5,-1000};
    Double_t ranges_max[4] = { 3, 300, 5, 1000};
    for(int i=0;i<4;i++){
        double range_min = ranges_min[i];
        double range_max = ranges_max[i];
        // double range_min = par[i]-fabs(par[i])*0.1;
        // double range_max = par[i]+fabs(par[i])*0.1;
        fMinuit->mnparm(i, Form("a%d",i),
                        par[i],step[i],
                        //range_min[i],range_max[i],
                        range_min,range_max,
                        ierflg);
    }
    // Fix the parameters that is related to scanning
    arglist[0] = 5000;
    arglist[1] = 1.;

    // Do minimisation
    fMinuit->mnexcm("SIMPLEX", arglist ,4,ierflg);
    fMinuit->mnexcm("MIGRAD" , arglist ,4,ierflg);

    // Get results
    for(int i=0;i<4;i++){
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


void ITrackFitterLinear::SimpleFit(ITrackSummary &tracks)
{
    Double_t arglist[10]={0};
    Int_t ierflg = 0;
    fMinuit->SetPrintLevel(-1);
    //fMinuit->SetPrintLevel(1);
    fMinuit->mnexcm("SET NOW", arglist ,1,ierflg);
    arglist[0] = 1;
    fMinuit->mnexcm("SET ERR", arglist ,1,ierflg);
    //Set initial value
    Double_t step[4] = {0.01,0.1,0.01,0.1};
    Double_t ranges_min[4] = {-1,-300,-3,-500};
    Double_t ranges_max[4] = { 1, 300, 3, 500};

    double trackPar[4];
    tracks.GetTrackPar(trackPar);
    for(int i=0;i<4;i++){
        double range_min = ranges_min[i];
        double range_max = ranges_max[i];
        // double range_min = par[i]-fabs(par[i])*0.1;
        // double range_max = par[i]+fabs(par[i])*0.1;
        fMinuit->mnparm(i, Form("a%d",i),
                        trackPar[i],step[i],
                        //range_min[i],range_max[i],
                        range_min,range_max,
                        ierflg);
    }
    // Fix the parameters that is related to scanning
    arglist[0] = 2000;
    arglist[1] = 1.;

    // Do minimisation
    fMinuit->mnexcm("SIMPLEX", arglist ,4,ierflg);
    fMinuit->mnexcm("MIGRAD" , arglist ,4,ierflg);

    // Get results
    double par[4];
    double par_err[4];
    double output[4];
    int status[3];
    for(int i=0;i<4;i++){
        fMinuit->GetParameter(i,par[i], par_err[i]) ;
    }
    fMinuit->mnstat (output[0],output[1],output[2],
                     status[0],status[1],status[2]);
    TString patternS=GetLeftRightPattern(par);
    tracks.Fill(par,par_err,output,status,patternS,fCurrentNhitsUsed);
}

void ITrackFitterLinear::ScanFit(std::vector<IHit*>hitCons){
}

void ITrackFitterLinear::SelectHits(IHit* hitCon, ITrackContainer* trackCon){
    // get nTracks
    std::vector<ITrackSummary> trackSummaries = trackCon->GetTrackSummaries();
    int nTracks = trackCon->GetNTracks();
    for(int iTrk=0;iTrk<nTracks;iTrk++){
        fHitCons.push_back(new IHit(Form("TrackFitting trackID %d",iTrk)));
        ITrackSummary a_trackSummary = trackSummaries[iTrk];
        // Copy to the Track container in this class
        fTrackCon->AddTracks(a_trackSummary);
        double trackPar[4];
        // Get infomation from track container
        a_trackSummary.GetTrackPar(trackPar);

        // Add hits for track finding 's container
        std::vector<int> v_lay      = hitCon->GetHitLayer();
        std::vector<int> v_cel      = hitCon->GetHitCell();
        std::vector<int> v_lr       = hitCon->GetLeftRight();
        std::vector<double> v_dt    = hitCon->GetDriftTime();
        std::vector<double> v_q     = hitCon->GetQ();
        std::vector<double> v_p     = hitCon->GetADCPeak();
        std::vector<double> v_hitR  = hitCon->GetHitR();
        int testLayer = hitCon->GetTestLayer();
        int nHits = (int)v_lay.size();

        const double res_cut = 1.0;
        int nSelectedHits = 0;
        for(int iHit=0;iHit<nHits;iHit++){
            int layer = v_lay[iHit];
            if(layer==testLayer) continue;
            int cell  = v_cel[iHit];
            double dt = v_dt[iHit];
            double phi = ITrackingUtility::GetIncidentAngle(layer, cell, trackPar);
            double hitR = fabs(ICDCGas::Get().GetDriftDistance(dt,layer,cell,phi,0));
            double fitR = fabs(ITrackingUtility::GetFitDistance(layer,cell,trackPar));
            //printf("[%d][%d] %f-%f=%f  \n",layer,cell,fitR,hitR,hitR-fitR);

            // Select hits that has small residue
            if(fabs(fitR-hitR)<res_cut){
                fHitCons[iTrk]->AddHitWire(layer,cell);
                fHitCons[iTrk]->AddDriftTime(dt);
                nSelectedHits++;
            }
            // Select hits that can be used for wide range
        }

        // Continue if no hit found
        if(nSelectedHits==0) continue; 
        fHitCons[iTrk]->SetNumOfHits(nSelectedHits);

        // Setup left right for fitting
        // A lot of redundence here...TODO fix it
        const int max_lr_comb = 1<<nSelectedHits;
        for(int ic=0;ic<max_lr_comb;ic++){
            fTrackMapHitConsScan[iTrk].push_back(new IHit(Form("Scan TrackFitting trackID %d comb %d",
                                                       iTrk, ic)));
            // Fill hit con
            const double res_cut = 1.0;
            for(int iHit=0;iHit<nHits;iHit++){
                int layer = v_lay[iHit];
                if(layer==testLayer) continue;
                int cell  = v_cel[iHit];
                double dt = v_dt[iHit];
                double phi = ITrackingUtility::GetIncidentAngle(layer, cell, trackPar);
                double hitR = fabs(ICDCGas::Get().GetDriftDistance(dt,layer,cell,phi,0));
                double fitR = fabs(ITrackingUtility::GetFitDistance(layer,cell,trackPar));
                /// Make sure this is the same as above...because you are using
                //  this const int max_lr_comb = 1<<nSelectedHits;
                if(fabs(fitR-hitR)<res_cut){
                    fTrackMapHitConsScan[iTrk][ic]->AddHitWire(layer,cell);
                    fTrackMapHitConsScan[iTrk][ic]->AddDriftTime(dt);
                }
            }
            fTrackMapHitConsScan[iTrk][ic]->SetNumOfHits(nSelectedHits);
            int bit = 1<<(nSelectedHits-1);
            int count = 0;
            //Set up the left right for each hit
            std::vector<int> leftright;
            while ( bit ) {
                int lr = ic & bit ? 1 : -1;
                fTrackMapHitConsScan[iTrk][ic]->AddLeftRight(lr);
                count ++;
                //printf("%d",lr);
                if(count==max_lr_comb)count=0;
                bit >>= 1;
            }
            //printf("\n");
        }
    }
}
