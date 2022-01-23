#ifndef _TRACKINGANALYZER_HH_
#define _TRACKINGANALYZER_HH_

#include "TH1D.h"
#include "TF1.h"
#include "TVector3.h"
//#include "Iteration.hxx"
#include "WireManager.hxx"
#include "AnalyzerBase.hxx"
#include "VectorAnalysis.hxx"

class ITrackingUtility : public AnalyzerBase{

    public:
        ITrackingUtility();
        ~ITrackingUtility();

        /// Return std::map of patterns of hit list with the layer size
        static std::map<int, std::vector<int> > GetHitPattern(const int hitIndexlist[20][400], std::vector<int> v_layHits);

        /// Check the hit region
        static Int_t CheckHitCellRegion(std::vector<double> hitPosWire, std::vector<double> hitPosTrack);

        /// Get Hit position information on sense wire or track
        /// option : Wire or track
        static void GetHitPosition(int LayerID, int CellID, double& x, double& y, double& z, double *trackPar, TString option);

        /// Get cell parameter must use the information of z after tracking
        /// The cell parameter is by defintion beta, which describes the change of upper part of the cell along the z diretion of CDC
        static Double_t GetCellParameter(int LayerID, int CellID, double hitZonWire);

        /// Get the azimuth phi of the incident tracks
        /// Let the vector V be a vector penpendicular to track and connected to sense wire
        /// phi is the angle between V and vector of sense wire from CDC origin
        static Double_t GetIncidentAngle(int layerID, int cellID, double *trackPar);
        static Double_t GetIncidentAngle(int layerID, int cellID, std::vector<double>trackPar){
            double par_tmp[4];
            for(int i=0;i<4;i++)par_tmp[i] = trackPar[i];
            return GetIncidentAngle(layerID,cellID,par_tmp);
        }

        /// Get Fit distance
        static Double_t GetFitR2D(double x, double y, double *par);
        static Double_t GetFitDistance(Int_t LayerID, Int_t CellID, Double_t* par);
        static Double_t GetFitDistance(Int_t LayerID, Int_t CellID, std::vector<Double_t>par){
            double par_tmp[4];
            for(int i=0;i<4;i++)par_tmp[i] = par[i];
            return GetFitDistance(LayerID,CellID,par_tmp);
        }

        /// Find the intersection point of projection of line 1 and line 2 on XY
        static TVector2 GetIntersectPoint(int l1, int w1, int l2, int w2, TString option);

        /// Find the cross Z at of line 1 and line 2
        static Double_t GetZCrossPoint(int l1, int w1, int l2, int w2){
            return GetCrossPoint(l1,w1,l2,w2).Z();
         }
        /// Find the cross point at of line 1 and line 2
        static TVector3 GetCrossPoint(int l1, int w1, int l2, int w2);

        /// Get x-y position on the CDC given a y-z direction track
        static void GetXYPos(int LayerID, int WireID, double m_track, double c_track, double& x, double& y);

    private:
        /// Get angle of wire global to local
        static bool IsOnAxis(double x,double y, double &angle);
        bool fDebug;
};

#endif
