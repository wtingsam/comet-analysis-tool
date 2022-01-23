#ifndef __PreTrackHisto_is_seen__
#define __PreTrackHisto_is_seen__

#include <iostream>

#include "TH1DBuilder.hxx"
#include "TH2DBuilder.hxx"

/***
    This is abstract class for making pre-track histograms
    The input root file is e.g. ana_<runNo>.root, by default
    If you want to create your own histogram booklet,
    please follow the same format
***/
class PreTrackHisto{

    public:
        /***
            The Excute function is the function to summarize what you want to do.
            E.g. you can do something like this in the sub concrete class
            void Excute(){
               Initialize();
               Fill();
               Draw();
               Finalize();
             }
        ***/
        virtual void Excute() = 0;

        /// To initialize you class, e.g. set root file and branches
        virtual void Initialize() = 0;

        /// To fill the histogram, this is the main event loop
        virtual void Fill() = 0;

        /// To draw the figures
        virtual void Draw() = 0;

        /// To close the files
        virtual void Finalize() = 0;

        /// To print out the message about the class
        virtual void Print() = 0;
};



#endif
