#ifndef _EVENTLOOP_HH_
#define _EVENTLOOP_HH_

#include "TF1.h"
#include <iostream>
#include <math.h>

#include "WireManager.hxx"

class EventLoop{
    public:
        EventLoop();
        virtual ~EventLoop();

        virtual void Initialize(){}

        virtual void ClearEvent(){}

        virtual void ClearHits(void){}

        virtual void ShowProgress(int iev, int numOfEvents);

    private:

};

#endif
