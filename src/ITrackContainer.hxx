#ifndef _ITrackContainer_HXX_
#define _ITrackContainer_HXX_

#include <vector>
#include <iostream>
#include <stdio.h>
#include "TString.h"
#include "ITrackSummary.hxx"

class ITrackContainer{
    private:
        std::vector< std::vector<double> > fTracks;
        std::vector<double> fChi2;
        std::vector<int> fDOF;
        std::vector< TString > fPatterns;
        std::vector<ITrackSummary> fTrackSummaries;
        bool has100Chi2Track;
    public:
        // Constructor
        ITrackContainer();
        // Destructor
        virtual ~ITrackContainer(){}
        // Clear all hits
        void Clear();
        // This parameters tell you that there is at least 1 track having chi2<100 in this containaer
        bool hasGoodTracks(){   return has100Chi2Track;   }
        // Add track summary to container
        void AddTracks(ITrackSummary a_track){
            if(a_track.GetChi2()/a_track.GetDOF()<100){   has100Chi2Track=true;   }
            fTrackSummaries.push_back(a_track);
        }
        // Get summaries of tracks
        std::vector<ITrackSummary> GetTrackSummaries(){  return fTrackSummaries;  }
        // Show the Summaries of tracks
        void Show(){
            printf("\n#######################################\n\n");
            for(int i=0;i<(int)fTrackSummaries.size();i++)
                fTrackSummaries[i].Show();
            printf("\n#######################################\n\n");
        }
        // Get Size of track container
        int GetNTracks(){   return fTrackSummaries.size();   }

};

#endif
