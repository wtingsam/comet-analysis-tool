#ifndef _WIREMANAGER_HXX_
#define _WIREMANAGER_HXX_

#include "WireConfig.hxx"

class WireManager : public WireConfig {
    public:
        ~WireManager(){}

        static WireManager &Get();

        void Ls(void);
        void LsCRT(void);

        /// Get Z-Beta mapping, Z is the direction of beam in CDC
        //double GetZfromBeta(int layer,int wire,double beta);
        /// Get Beta-Z mapping
        double GetBetafromZ(int layer,int wire,double z);
        /// Get Maximum beta at sense layer and wire
        double GetMaxBeta(int layer,int wire){   return fBeta_max[layer][wire];   }
        /// Get Minimum beta at sense layer and wire
        double GetMinBeta(int layer,int wire){   return fBeta_min[layer][wire];   }
        /// Get Max bin of beta-z mapping
        int GetBetaZBin(){  return fMaxBinOfBetaZ;  }

        /// Get Wire position at Endplates
        double GetSenseWireXPosHV(int layer, int wire){if (CheckLayer(layer)||CheckWire(wire)) return -1; else return fXhv[layer][wire]; }
        double GetSenseWireYPosHV(int layer, int wire){if (CheckLayer(layer)||CheckWire(wire)) return -1; else return fYhv[layer][wire]; }
        double GetSenseWireZPosHV(int layer){ if (CheckLayer(layer)) return -1; else return -fCDClength[layer]/2; }
        double GetSenseWireXPosCen(int layer, int wire){if (CheckLayer(layer)||CheckWire(wire)) return -1; else return fXc[layer][wire]; }
        double GetSenseWireYPosCen(int layer, int wire){if (CheckLayer(layer)||CheckWire(wire)) return -1; else return fYc[layer][wire]; }
        double GetSenseWireXPosRO(int layer, int wire){if (CheckLayer(layer)||CheckWire(wire)) return -1; else return fXro[layer][wire]; }
        double GetSenseWireYPosRO(int layer, int wire){if (CheckLayer(layer)||CheckWire(wire)) return -1; else return fYro[layer][wire]; }
        double GetSenseWireZPosRO(int layer){ if (CheckLayer(layer)) return -1; else return  fCDClength[layer]/2; }

        int GetNumUsedSenseWire(int layer){  if(CheckLayer(layer)) return -1; else return fNumSenseWireUsed[layer]; }

        /// Geometry
        int GetNumSenseWires(int layer){ return fMaxSenseWirePerLayer[layer]; }
        int GetNumSenseWires(void){ return fNumSenseWire; }
        int GetNumFieldWires(void){ return fNumFieldWire; }

       //bool IsWire(int layer, int wire);
        /// For Cosmic Ray Test
        bool IsUsedWire(int ch);

        int GetUsedLayer(int ch){ if (CheckCh(ch)) return -1; else return fLayerUsed[ch]; }
        int GetUsedCell(int ch){ if (CheckCh(ch)) return -1; else return fCellUsed[ch]; }
        int GetUsedBoardID(int ch){ if (CheckCh(ch)) return -1; else return fBdIDUsed[ch]; }
        int GetUsedBoardLayID(int ch){ if (CheckCh(ch)) return -1; else return fBdLayIDUsed[ch]; }
        int GetUsedBoardLocID(int ch){ if (CheckCh(ch)) return -1; else return fBdLocIDUsed[ch]; }

        int GetUsedChannel(int layer, int wire){
            if (CheckLayer(layer)) return -1;
            else if(CheckWire(wire)) return -1;
            else return fChannelUsed[layer][wire];
        }

        int CheckCh(int ch){
            if (ch>=MAX_CH){
                if(fDebug)fprintf(stderr,"ChID %d is larger than MAX_CH = %d\n",ch,MAX_CH);
                return 1;
            }
            else if (ch<0){
                if(fDebug)fprintf(stderr,"Invalid ChID %d < 0\n", ch);
                return -1;
            }
            else{
                return 0;
            }
        }

        int CheckLayer(int ly){
            if (ly>=MAX_SENSE_LAYER){
                if(fDebug)fprintf(stderr,"layerID %d is larger than MAX_SENSE_LAYER = %d\n",ly,MAX_SENSE_LAYER);
                return 1;
            }
            else if (ly<0){
                if(fDebug)fprintf(stderr,"Invalid layerID %d < 0\n", ly);
                return -1;
            }
            else{
                return 0;
            }
        }

        int CheckWire(int wi){
            if (wi>=MAX_WIREpL){
                if(fDebug)fprintf(stderr,"wireID %d is larger than MAX_WIREpL = %d\n",wi,MAX_WIREpL);
                return 1;
            }
            else if (wi<0){
                if(fDebug)fprintf(stderr,"Invalid wireID %d < 0\n", wi);
                return -1;
            }
            else{
                return 0;
            }
        }

    private:
        static WireManager *fWireManager;
        WireManager(){}; //Don't Implenment
        WireManager(WireManager const&); //Don't Implenment
        void operator=(WireManager const&); //Don't Implenment


};




#endif
