#include "ITrackSummary.hxx"

ITrackSummary::ITrackSummary(TString name, int track_type):
    fName(name),fTrackType(track_type)
{
    fUsedHits=0;
    // Setup type
    switch(track_type){
    case 0:
        fTrackDOF=4;
        break;
    case 1:
        fTrackDOF=5;
        break;
    default:
        fTrackDOF=4;
        break;
    }
}

void ITrackSummary::Fill(const double *val_par,
                         const double *val_par_err,
                         const double *val_output,
                         const int *val_status,
                         const TString val_pattern,
                         int usedHits
                         ){
    fUsedHits=usedHits;
    fPattern=val_pattern;
    for(int i=0;i<fTrackDOF;i++){
        fPar[i]=val_par[i];
        fPar_err[i]=val_par_err[i];
    }
    for(int i=0;i<3;i++){
        fOutput[i]=val_output[i];
        fStatus[i]=val_status[i];
    }
}

void ITrackSummary::Fill(const double *val_par,
                         const double *val_par_err,
                         const double *val_output,
                         const int *val_status,
                         const TString val_pattern
                         ){
    fPattern=val_pattern;
    for(int i=0;i<fTrackDOF;i++){
        fPar[i]=val_par[i];
        fPar_err[i]=val_par_err[i];
    }
    for(int i=0;i<3;i++){
        fOutput[i]=val_output[i];
        fStatus[i]=val_status[i];
    }
}

void ITrackSummary::Show(){
    printf("=====================\n");
    printf(" Pattern:\n");
    printf(" This track, we used %d hits \n",fUsedHits);
    printf("     %s\n",fPattern.Data());
    printf(" Parameters:\n");
    for(int i=0;i<fTrackDOF;i++){
        printf("     %d | %5.3f+-%5.3f\n",i,fPar[i],fPar_err[i]);
    }
    printf(" Output status\n");
    for(int i=0;i<3;i++){
        printf("     %d | %5.3f %d\n",i,fOutput[i],fStatus[i]);
    }
}
