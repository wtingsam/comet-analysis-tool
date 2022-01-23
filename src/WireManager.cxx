#include "WireManager.hxx"
#include "ExperimentConfig.hxx"

WireManager *WireManager::fWireManager = NULL;

WireManager &WireManager::Get()
{
    if ( !fWireManager ){//make sure only being created one time
        fWireManager = new WireManager();
    }
    return *fWireManager;
}


// bool WireManager::IsWire(int layer, int wire)
// {
//     //later
//     layer;
//     wire;
//     return true;
// }

bool WireManager::IsUsedWire(int ch)
{
    if(fLayerUsed[ch]==-1){
        return false;
    }else{
        return true;
    }
}

void WireManager::Ls(void)
{
    //show all wires
    for(int iLAYER=0;iLAYER<MAX_SENSE_LAYER;iLAYER++){
        for(int iWIRE=0;iWIRE<MAX_WIREpL;iWIRE++){
            printf("%d %d \n",iLAYER,iWIRE);
            //IsWire(layer,wire);
        }
    }
}

void WireManager::LsCRT(void)
{
    //show wires used in CRT
    printf("chan layer  cell  bdID bdLay bdLoc   xhv   yhv   x0   y0   xro   yro   zhv\n");
    for(int ch=TRIG_CH;ch<MAX_CH;ch++){
        Int_t    ll  = GetUsedLayer(ch);
        Int_t    cc  = GetUsedCell(ch);
        Int_t    bid = GetUsedBoardID(ch);
        Int_t    bla = GetUsedBoardLayID(ch);
        Int_t    blo = GetUsedBoardLocID(ch);

        Double_t    xhv = GetSenseWireXPosHV(ll,cc);
        Double_t    yhv = GetSenseWireYPosHV(ll,cc);
        Double_t    x0  = GetSenseWireXPosCen(ll,cc);
        Double_t    y0  = GetSenseWireYPosCen(ll,cc);
        Double_t    xro = GetSenseWireXPosRO(ll,cc);
        Double_t    yro = GetSenseWireYPosRO(ll,cc);

        Double_t zz = GetSenseWireZPosHV(ll);
        if(ll==MAX_SENSE_LAYER-3){
            printf("%5d %5d %5d %5d %5d %5d ", ch,ll,cc,bid,bla,blo);
            printf("%6.1f %6.1f %6.1f, %6.1f %6.1f,%6.1f %6.1f \n",xhv,yhv,x0,y0,xro,yro,zz);
        }
    }
}

// double WireManager::GetZfromBeta(int layer,int wire,double beta)
// {
//     layer;
//     wire;
//     return beta/fMaxBinOfBetaZ;
// }

double WireManager::GetBetafromZ(int layer,int wire,double z){
    double start_pos = GetSenseWireZPosHV(layer);
    double range = fabs(GetSenseWireZPosHV(layer) - GetSenseWireZPosRO(layer));
    int index=(int)((z - start_pos)*fMaxBinOfBetaZ/range);
    if(index<0)index=0;
    if(index>=fMaxBinOfBetaZ)index=fMaxBinOfBetaZ-1;
    return fBeta[layer][wire][index];
}
