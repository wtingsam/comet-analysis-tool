#include "EventLoop.hxx"

EventLoop::EventLoop(){}

EventLoop::~EventLoop(){}

void EventLoop::ShowProgress(int iev, int numOfEvents)
{
    if(iev%1000==0){
        std::cout << "Progress: "<< iev*1./numOfEvents*100 << "%" << std::endl;
    }
}
