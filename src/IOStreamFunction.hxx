#ifndef _IOSTREAMFUNCTION_HXX_
#define _IOSTREAMFUNCTION_HXX_
#include <ctime>
#include <sstream>
#include <fstream>
#include "iostream"
#include "TString.h"

using namespace std;
class IOStreamFunction{
    public:
        /// Constructor
        IOStreamFunction(){}
        /// Destructor
        ~IOStreamFunction(){}


        /// Return true if the line is found in the file
        bool IsWritten(TString file,TString searchLine){
            bool found = false;
            ifstream stream1(file);
            string line ;
            while( getline( stream1, line ) && !found)
            {
                if(line.find(searchLine) != string::npos){ // WILL SEARCH in file
                    found = true;
                }
            }
            return found;
        }

        /// Return true if there title of the file exist
        bool IsFileExist(TString file){
            bool exist = false;
            if(std::ifstream(file)) exist = true;
            return exist;
        }

        void Write(){}

        void Read();

        void OpenAppend(TString fileName, TString header){
            if(!IsFileExist(fileName)){
                fOutputStream.open(fileName,ios::out | ios::app);
                fOutputStream << header << "\n";
            }else{
                fOutputStream.open(fileName,ios::out | ios::app);
            }
        }

        void OpenTrunc(TString fileName, TString header){
            if(!IsFileExist(fileName)){
                fOutputStream.open(fileName,ios::out | ios::trunc);
                fOutputStream << header << "\n";
            }else{
                fOutputStream.open(fileName,ios::out | ios::trunc);
            }
        }

        template <typename X>IOStreamFunction & operator<<(X s){
            fOutputStream << s ;
            return *this;
        }

    private :
        ofstream fOutputStream;
};

#endif
