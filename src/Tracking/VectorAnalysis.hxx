#ifndef _VECTORANALYSIS_HXX_
#define _VECTORANALYSIS_HXX_

#include <vector>
#include <iostream>

class VectorAnalysis{
    public:
        VectorAnalysis(){}
        ~VectorAnalysis(){}

        static double Dot(std::vector<double> x,std::vector<double> y);

        static void SetVector(double a, double b, double c,std::vector<double>& v);

        static void Subtract(std::vector<double> v,std::vector<double> u,std::vector<double>& sub);

};
#endif
