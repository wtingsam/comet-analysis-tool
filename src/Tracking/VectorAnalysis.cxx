#include "VectorAnalysis.hxx"


double VectorAnalysis::Dot(std::vector<double> x,std::vector<double> y)
{
    if(x.size()!=3||y.size()!=3){
        std::cerr<<" Error : intput vector is not 3-Dimension \n"<< std::endl;
        return 0;
    }else{
        double sum=0;
        for(int i=0;i<3;i++){
            sum+=x.at(i)*y.at(i);
        }
        return sum;
    }
}

void VectorAnalysis::SetVector(double a, double b, double c,std::vector<double>& v)
{
    v.at(0)=a;
    v.at(1)=b;
    v.at(2)=c;
}

void VectorAnalysis::Subtract(std::vector<double> v,std::vector<double> u,std::vector<double>& sub)
{
    sub.at(0)=v.at(0)-u.at(0);
    sub.at(1)=v.at(1)-u.at(1);
    sub.at(2)=v.at(2)-u.at(2);
}

