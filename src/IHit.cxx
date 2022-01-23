#include "IHit.hxx"

void IHit::Show(){
    printf("=============================\n");
    printf("Hit container: %s \n",fName.Data());
    if(fTriggerNumber>=0)printf("Trigger number %d \n",fTriggerNumber);
    if(fEventNumber>=0)printf("event ID %d : \n",fEventNumber);
    std::cout << "pointer this :" <<  this << std::endl;
    for(int i=0;i<(int)fLayerID.size();i++){
        if(fLayerID.size()!=0)printf("%3d [%d][%2d] ",i, fLayerID.at(i),fCellID.at(i));
        if(fDriftTime.size()!=0)printf("dt %10.3f(%d)",fDriftTime.at(i),(int)fDriftTimeAll[i].size());
        if(fQ.size()!=0)printf("Q %10.3f ",fQ.at(i));
        if(fQCut.size()!=0)printf("Qcut %10.3f ",fQCut.at(i));
        if(fNumOfLayHits.size()!=0)printf("NLH %3d ",fNumOfLayHits.at(i));
        if(fNumOfPeaks.size()!=0)printf("NP %3d ",fNumOfPeaks.at(i));
        if(fPeakWidth.size()!=0)printf("PW %3d ",fPeakWidth.at(i));
        if(fChoosePeakFlag.size()!=0)printf("CPF %s ",(fChoosePeakFlag.at(i))?"Y":"N");
        if(fHitR.size()!=0)printf("HitR %f ",fHitR.at(i));
        printf("\n");
    }
    if((int)fLeftRight.size()){
        printf("pattern: ");
        for(int i=0;i<(int)fLeftRight.size();i++)
            printf("%d",fLeftRight[i]);
        printf("\n");
    }
}

void IHit::Clear(){
    fTriggerNumber=-1;
    fEventNumber=-1;
    fTestLayer=-1;
    fNumOfHits=-1;
    fMAXADCSUM.clear();
    fMAXADCPeak.clear();
    fLayerID.clear();
    fCellID.clear();
    fLeftRight.clear();
    fHitR.clear();
    fDriftTime.clear();
    fDriftTimeAll.clear();
    fADCPeak.clear();
    fQ.clear();
    fQCut.clear();
    fNumOfLayHits.clear();
    fNumOfPeaks.clear();
    fPeakWidth.clear();
    fChoosePeakFlag.clear();
    fHitFlag.clear();
}
