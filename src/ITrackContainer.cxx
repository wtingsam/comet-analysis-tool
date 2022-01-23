#include "ITrackContainer.hxx"

ITrackContainer::ITrackContainer(){
    has100Chi2Track=false;
}

void ITrackContainer::Clear(){
    fTracks.clear();
    fChi2.clear();
    fDOF.clear();
    fPatterns.clear();
}
