#ifndef __HistoFactory_is_seen__
#define __HistoFactory_is_seen__

#include "LayDepPreTrackHisto.hxx"
#include "BoardDepPreTrackHisto.hxx"
#include "NoisePreTrackHisto.hxx"
class HistoFactory{
    public:
        PreTrackHisto *createHisto(TString type){
            if(type=="layer"){
                return new LayDepPreTrackHisto;
            }
            if(type=="board"){
                return new BoardDepPreTrackHisto;
            }
            if(type=="noise"){
                return new NoisePreTrackHisto;
            }
            else{
                std::cout << "No such type of histograms" << std::endl;
                return NULL;
            }
        }

};

#endif
