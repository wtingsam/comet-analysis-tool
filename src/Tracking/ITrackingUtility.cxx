#include "ITrackingUtility.hxx"

ITrackingUtility::ITrackingUtility()
    :fDebug(false)
{}

ITrackingUtility::~ITrackingUtility(){}

std::map<int, std::vector<int> > ITrackingUtility::GetHitPattern(const int hitIndexlist[20][400], std::vector<int> v_layHits){
    const int n=v_layHits.size();
    std::map<int, std::vector<int> > map_comb2hitList;
    // the n arrays
    int* indices = new int[n];
    // initialize with first element's index
    for (int i = 0; i < n; i++)
        indices[i] = 0;
    int iComb = 0;
    while (1) {
        // print current combination
        std::vector<int> ipattern;
        for (int i = 0; i < n; i++){
            if(v_layHits[i]>0){
                int hit_index = hitIndexlist[i][indices[i]];
                //std::cout << hit_index << " " ;
                if(hit_index>=0){
                    ipattern.push_back(hit_index);
                }
                //std::cout << std::endl;
            }
        }
        //cout << endl;
        map_comb2hitList[iComb]=ipattern;
        iComb++;
        // find the rightmost array that has more
        // elements left after the current element
        // in that array
        int next = n - 1;
        while (next >= 0 &&
               (indices[next] + 1 >= v_layHits[next]))
               //(indices[next] + 1 >= hitIndexlist[next].size()))
            next--;
        // no such array is found so no more
        // combinations left
        if (next < 0)
            break;
        // if found move to next element in that
        // array
        indices[next]++;
        // for all arrays to the right of this
        // array current index again points to
        // first element
        for (int i = next + 1; i < n; i++)
            indices[i] = 0;
    }

    return map_comb2hitList;
}

void ITrackingUtility::GetHitPosition(int LayerID, int CellID, double& x, double& y, double& z, double *trackPar, TString option)
{
    const int DIM=3;
    //L1
    std::vector<double> r1(DIM,0);
    std::vector<double> u(DIM,0);
    //L2
    std::vector<double> r2(DIM,0);
    std::vector<double> v(DIM,0);
    //vector between L1 and L2
    std::vector<double> r21(DIM,0);

    //get the shortest distance of two lines in 3 dimension
    Double_t x1 = 850;
    Double_t y1 = x1*trackPar[0]+trackPar[1];
    Double_t z1 = x1*trackPar[2]+trackPar[3];

    Double_t x2 = -850;
    Double_t y2 = x2*trackPar[0]+trackPar[1];
    Double_t z2 = x2*trackPar[2]+trackPar[3];

    //Sense wire coordinate relative to origin of prototype
    Double_t s1 = WireManager::Get().GetSenseWireXPosHV(LayerID,CellID);
    Double_t s2 = WireManager::Get().GetSenseWireYPosHV(LayerID,CellID);
    Double_t s3 = WireManager::Get().GetSenseWireZPosHV(LayerID);
    Double_t t1 = WireManager::Get().GetSenseWireXPosRO(LayerID,CellID);
    Double_t t2 = WireManager::Get().GetSenseWireYPosRO(LayerID,CellID);
    Double_t t3 = WireManager::Get().GetSenseWireZPosRO(LayerID);

    //sense wire vector
    Double_t u1 = s1-t1;
    Double_t u2 = s2-t2;
    Double_t u3 = s3-t3;

    //track vector
    Double_t v1 = x1-x2;
    Double_t v2 = y1-y2;
    Double_t v3 = z1-z2;

    //Def: x=r+ut

    // Sense wire : L1
    VectorAnalysis::SetVector(s1,s2,s3,r1);
    VectorAnalysis::SetVector(u1,u2,u3,u);

    //  Track : L2
    VectorAnalysis::SetVector(x1,y1,z1,r2);
    VectorAnalysis::SetVector(v1,v2,v3,v);

    VectorAnalysis::Subtract(r2,r1,r21);

    double s=(VectorAnalysis::Dot(r1,v)-VectorAnalysis::Dot(r2,v)-(VectorAnalysis::Dot(r1,u)-VectorAnalysis::Dot(r2,u))*VectorAnalysis::Dot(u,v)/VectorAnalysis::Dot(u,u))/(VectorAnalysis::Dot(v,v)-pow(VectorAnalysis::Dot(u,v),2)/VectorAnalysis::Dot(u,u));
    double t=-(VectorAnalysis::Dot(r1,u)-VectorAnalysis::Dot(r2,u)-(VectorAnalysis::Dot(r1,v)-VectorAnalysis::Dot(r2,v))*VectorAnalysis::Dot(v,u)/VectorAnalysis::Dot(v,v))/(VectorAnalysis::Dot(u,u)-pow(VectorAnalysis::Dot(v,u),2)/VectorAnalysis::Dot(v,v));

    //P
    double x1_u1t=s1+u1*t;
    double y1_u2t=s2+u2*t;
    double z1_u3t=s3+u3*t;
    //Q
    double x2_v1t=x1+v1*s;
    double y2_v2t=y1+v2*s;
    double z2_v3t=z1+v3*s;

    std::vector<double> PQ_tmp(3,0);

    PQ_tmp.at(0)=x1_u1t-x2_v1t;
    PQ_tmp.at(1)=y1_u2t-y2_v2t;
    PQ_tmp.at(2)=z1_u3t-z2_v3t;
    if(option=="wire"){
        x=x1_u1t;
        y=y1_u2t;
        z=z1_u3t;
    }else if(option=="track"){
        x=x2_v1t;
        y=y2_v2t;
        z=z2_v3t;
    }else{
        std::cout << "## Wrong option in void TrackFitting::GetHitPosition()" << std::endl;
        x=0;
        y=0;
        z=0;
    }
}

Double_t ITrackingUtility::GetIncidentAngle(int layerID, int cellID, double *trackPar)
{
    const int DIM=3;
    double xt,yt,zt; // Point Q
    GetHitPosition(layerID,cellID,xt,yt,zt,trackPar,"track");
    double xw,yw,zw; // Point P
    GetHitPosition(layerID,cellID,xw,yw,zw,trackPar,"wire");
    // Check hit region
    std::vector<double>hitPosWire;
    hitPosWire.push_back(xw);
    hitPosWire.push_back(yw);
    std::vector<double>hitPosTrack;
    hitPosTrack.push_back(xt);
    hitPosTrack.push_back(yt);
    int region=CheckHitCellRegion(hitPosWire,hitPosTrack);
    // At plane X-Y, build a vector from track to sense wire
    // Let vector PQ be pointing from wire to track
    double vx = xt - xw;
    double vy = yt - yw;
    double vz = 0;

    // Get unit vector
    double normPQ = sqrt( vx*vx + vy*vy );
    double normOP = sqrt( xw*xw + yw*yw );

    vx/=normPQ;
    vy/=normPQ;
    xw/=normOP;
    yw/=normOP;

    // Get the penpendicular vector of vector OP, which is namely OPp, small p for penpendicular
    double x =  xw*0 + yw*1;
    double y = -xw*1 + yw*0;

    std::vector<double> PQ(DIM,0);
    std::vector<double> OPp(DIM,0);

    VectorAnalysis::SetVector(vx,vy,vz,PQ);
    VectorAnalysis::SetVector(x,y,0,OPp);

    // Dot product of PQ and OPp gives u angle phi
    float phi=VectorAnalysis::Dot(PQ,OPp);

    const double PI=3.14159265;
    phi = fabs(acos (phi) * 180.0 / PI);
    //printf("HitPos Wire (%f %f)->(%f %f %f), Track(%f %f)->(%f %f), region %d normPQ %f normOP %f, %f \n",xw,yw,vx,vy,vz,xt,yt,x,y,region,normPQ,normOP,phi);

    if(region==1 || region==2){
        return phi;
    }else if(region==3 || region==4){
        return 360 - phi;
    }else{
        return -1;
    }
    return -2;

}

bool ITrackingUtility::IsOnAxis(double x,double y, double &angle){
    // First check 0 condition
    if(x==0 && y>0){
        angle=0;
        return true;
    }else if(x==0 && y<0){
        angle=180;
        return true;
    }else if(x>0 && y==0){
        angle=-90;
        return true;
    }else if(x<0 && y==0){
        angle=90;
        return true;
    }
    return false;
}

Int_t ITrackingUtility::CheckHitCellRegion(std::vector<double> hitPosWire, std::vector<double> hitPosTrack)
{
    /**
       Check cell region
       ________
       \       \
        \ 2 . 1 \
        |       |
        |_3___4_|

    **/

    // Since the cell has its tilted angle ,
    // lets rotate it back to the same as the figure shown above
    // Anti-clockwise : - , clockwise : +

    double xw = hitPosWire.at(0);
    double yw = hitPosWire.at(1);
    double xt = hitPosTrack.at(0);
    double yt = hitPosTrack.at(1);

    double RotAng;
    const double PI=3.14159265;

    if(!IsOnAxis(xw,yw,RotAng)){
        if(yw>0 && xw>0){
            RotAng = -1*atan(xw/yw);
        }else if(yw>0 && xw<0){
            RotAng = atan(-xw/yw);
        }else if(yw<0 && xw<0){
            RotAng = atan(yw/xw)+PI/2;
        }else if(yw<0 && xw>0){
            RotAng = -(atan(-yw/xw)+PI/2);
        }
    }

    // Check the position of xt and yt after rotation
    double x = xt*cos(RotAng)+yt*sin(RotAng);
    double y =-xt*sin(RotAng)+yt*cos(RotAng);
    //double xd= xw*cos(RotAng)+yw*sin(RotAng);
    double yd=-xw*sin(RotAng)+yw*cos(RotAng);

    // Translation back to origin
    y=y-yd;

    if(y>0 && x>0){
        return 1;
    }else if(y>0 && x<0){
        return 2;
    }else if(y<0 && x<0){
        return 3;
    }else if(y<0 && x>0){
        return 4;
    }else{
        return -1; // Unknown
    }
    return -2;
}

double ITrackingUtility::GetFitR2D(double x,double y,double *par)
{
    Double_t value=(y-par[0]*x-par[1])/sqrt(1+par[0]*par[0]);
    return value;
}

Double_t ITrackingUtility::GetFitDistance(Int_t LayerID, Int_t CellID, Double_t* par)
{
    //get the shortest distance of two lines in 3 dimension
    //y=mx+c z=mx+c
    Double_t x1 = 850;
    Double_t y1 = x1*par[0]+par[1];
    Double_t z1 = x1*par[2]+par[3];
    Double_t x2 = -850;
    Double_t y2 = x2*par[0]+par[1];
    Double_t z2 = x2*par[2]+par[3];

    //Sense wire coordinate relative to origin of prototype
    Double_t r1 = WireManager::Get().GetSenseWireXPosHV(LayerID,CellID);
    Double_t s1 = WireManager::Get().GetSenseWireYPosHV(LayerID,CellID);
    Double_t t1 = WireManager::Get().GetSenseWireZPosHV(LayerID);
    Double_t r2 = WireManager::Get().GetSenseWireXPosRO(LayerID,CellID);
    Double_t s2 = WireManager::Get().GetSenseWireYPosRO(LayerID,CellID);
    Double_t t2 = WireManager::Get().GetSenseWireZPosRO(LayerID);

    TVector3 vTrack,vTrackU,vTrackD;
    TVector3 vWire,vWireU,vWireD;
    TVector3 vDist;
    TVector3 vD;

    vTrackU.SetXYZ(x1,y1,z1);
    vTrackD.SetXYZ(x2,y2,z2);
    vTrack=vTrackU-vTrackD;
    vWireU.SetXYZ(r1,s1,t1);
    vWireD.SetXYZ(r2,s2,t2);
    vWire=vWireU-vWireD;
    vDist = vWire.Cross(vTrack);
    vD=vTrackU-vWireU;
    double dist = vD.Dot(vDist)/vDist.Mag();

//    return fabs(dist);
    return dist;
}

TVector3 ITrackingUtility::GetCrossPoint(int l1, int w1, int l2, int w2){
    const int DIM=3;
    //L1
    std::vector<double> r1(DIM,0);
    std::vector<double> u(DIM,0);
    //L2
    std::vector<double> r2(DIM,0);
    std::vector<double> v(DIM,0);

    //vector between L1 and L2
    std::vector<double> r21(DIM,0);

    Double_t s1=WireManager::Get().GetSenseWireXPosHV(l1,w1);
    Double_t s2=WireManager::Get().GetSenseWireYPosHV(l1,w1);
    Double_t s3=WireManager::Get().GetSenseWireZPosHV(l1);
    Double_t t1=WireManager::Get().GetSenseWireXPosRO(l1,w1);
    Double_t t2=WireManager::Get().GetSenseWireYPosRO(l1,w1);
    Double_t t3=WireManager::Get().GetSenseWireZPosRO(l1);

    Double_t x1=WireManager::Get().GetSenseWireXPosHV(l2,w2);
    Double_t y1=WireManager::Get().GetSenseWireYPosHV(l2,w2);
    Double_t z1=WireManager::Get().GetSenseWireZPosHV(l2);
    Double_t x2=WireManager::Get().GetSenseWireXPosRO(l2,w2);
    Double_t y2=WireManager::Get().GetSenseWireYPosRO(l2,w2);
    Double_t z2=WireManager::Get().GetSenseWireZPosRO(l2);

    //sense wire vector
    Double_t u1 = -t1+s1;
    Double_t u2 = -t2+s2;
    Double_t u3 = -t3+s3;
    //sense wire2 vector
    Double_t v1 = x1-x2;
    Double_t v2 = y1-y2;
    Double_t v3 = z1-z2;

    //   //Def: x=r+ut

    // Sense wire : L1
    VectorAnalysis::SetVector(s1,s2,s3,r1);
    VectorAnalysis::SetVector(u1,u2,u3,u);

    //  Track : L2
    VectorAnalysis::SetVector(x1,y1,z1,r2);
    VectorAnalysis::SetVector(v1,v2,v3,v);

    VectorAnalysis::Subtract(r2,r1,r21);

    double s=(VectorAnalysis::Dot(r1,v)-VectorAnalysis::Dot(r2,v)-(VectorAnalysis::Dot(r1,u)-VectorAnalysis::Dot(r2,u))*VectorAnalysis::Dot(u,v)/VectorAnalysis::Dot(u,u))/(VectorAnalysis::Dot(v,v)-pow(VectorAnalysis::Dot(u,v),2)/VectorAnalysis::Dot(u,u));
    double t=-(VectorAnalysis::Dot(r1,u)-VectorAnalysis::Dot(r2,u)-(VectorAnalysis::Dot(r1,v)-VectorAnalysis::Dot(r2,v))*VectorAnalysis::Dot(v,u)/VectorAnalysis::Dot(v,v))/(VectorAnalysis::Dot(u,u)-pow(VectorAnalysis::Dot(v,u),2)/VectorAnalysis::Dot(v,v));

    //P
    double x1_u1t=s1+u1*t;
    double y1_u2t=s2+u2*t;
    double z1_u3t=s3+u3*t;
    //Q
    double x2_v1t=x1+v1*s;
    double y2_v2t=y1+v2*s;
    double z2_v3t=z1+v3*s;

    double x_cross = (x1_u1t+x2_v1t)/2;
    double y_cross = (y1_u2t+y2_v2t)/2;
    double z_cross = (z1_u3t+z2_v3t)/2;
    TVector3 vec3(x_cross,y_cross,z_cross);
    return vec3;
}

TVector2 ITrackingUtility::GetIntersectPoint(int l1, int w1, int l2, int w2, TString option){
    Double_t x00=WireManager::Get().GetSenseWireXPosHV(l1,w1);
    Double_t y00=WireManager::Get().GetSenseWireYPosHV(l1,w1);
    Double_t z00=WireManager::Get().GetSenseWireZPosHV(l1);
    Double_t x01=WireManager::Get().GetSenseWireXPosRO(l1,w1);
    Double_t y01=WireManager::Get().GetSenseWireYPosRO(l1,w1);
    Double_t z01=WireManager::Get().GetSenseWireZPosRO(l1);
    Double_t x10=WireManager::Get().GetSenseWireXPosHV(l2,w2);
    Double_t y10=WireManager::Get().GetSenseWireYPosHV(l2,w2);
    Double_t z10=WireManager::Get().GetSenseWireZPosHV(l2);
    Double_t x11=WireManager::Get().GetSenseWireXPosRO(l2,w2);
    Double_t y11=WireManager::Get().GetSenseWireYPosRO(l2,w2);
    Double_t z11=WireManager::Get().GetSenseWireZPosRO(l2);
    if(option.Contains("zx")){
        y00=x00;
        y10=x10;
        y01=x01;
        y11=x11;
        x00=z00;
        x10=z10;
        x01=z01;
        x11=z11;
    }else if(option.Contains("zy")){
        x00=z00;
        x10=z10;
        x01=z01;
        x11=z11;
    }
    double m1,c1,m2,c2=0;
    // y=m1x+c1
    // y=m2x+c2
    m1=(y01-y00)/(x01-x00);
    c1=y00-m1*x00;
    m2=(y11-y10)/(x11-x10);
    c2=y10-m2*x10;
    double x_inter = (c2-c1)/(m1-m2);
    double y_inter = m1*x_inter+c1;
    TVector2 vec2(x_inter,y_inter);
    return vec2;
}

void ITrackingUtility::GetXYPos(int LayerID, int WireID, double m_track, double c_track, double& x, double& y)
{
    //Sense wire coordinate relative to origin of prototype
    Double_t x1=WireManager::Get().GetSenseWireXPosHV(LayerID,WireID);
    Double_t y1=WireManager::Get().GetSenseWireYPosHV(LayerID,WireID);
    Double_t z1=WireManager::Get().GetSenseWireZPosHV(LayerID);

    Double_t x2=WireManager::Get().GetSenseWireXPosRO(LayerID,WireID);
    Double_t y2=WireManager::Get().GetSenseWireYPosRO(LayerID,WireID);
    Double_t z2=WireManager::Get().GetSenseWireZPosRO(LayerID);

    //Sense wire m c
    //y=mx+c
    //z=mx+c
    double m1 = (z2-z1)/(x2-x1);
    double c1 = z1-m1*x1;
    double m2 = (y2-y1)/(x2-x1);
    double c2 = y1-m2*x1;

    //Coordinate of projected track intersecting with projected sense wires
    x = (c1-c_track)/(m_track-m1);
    y = m2*x + c2;
    //double z = m_track*(c_track-c2)/(m2-m_track) + c_track;
    //  if(debug)printf("GetXYPOS function : (%f,%f) --> (%f,%f)\n",(x1+x2)/2,(y1+y2)/2,x,y);
}
