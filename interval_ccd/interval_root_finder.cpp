// A root finder using interval arithmetic.
#include "interval_root_finder.hpp"

#include <stack>
#include<igl/Timer.h>
#include<iostream>
#include<interval_ccd/Rational.hpp>
namespace intervalccd {
double time20=0,time21=0,time22=0, time23=0,time24=0,time25=0;;
bool interval_root_finder(
    const std::function<Interval(const Interval&)>& f,
    const Interval& x0,
    double tol,
    Interval& x)
{
    return interval_root_finder(
        f, [](const Interval&) { return true; }, x0, tol, x);
}

bool interval_root_finder(
    const std::function<Interval(const Interval&)>& f,
    const std::function<bool(const Interval&)>& constraint_predicate,
    const Interval& x0,
    double tol,
    Interval& x)
{
    Eigen::VectorX3I x0_vec = Eigen::VectorX3I::Constant(1, x0), x_vec;
    Eigen::VectorX3d tol_vec = Eigen::VectorX3d::Constant(1, tol);
    bool found_root = interval_root_finder(
        [&](const Eigen::VectorX3I& x) {
            assert(x.size() == 1);
            return Eigen::VectorX3I::Constant(1, f(x(0)));
        },
        [&](const Eigen::VectorX3I& x) {
            assert(x.size() == 1);
            return constraint_predicate(x(0));
        },
        x0_vec, tol_vec, x_vec);
    if (found_root) {
        assert(x_vec.size() == 1);
        x = x_vec(0);
    }
    return found_root;
}

bool interval_root_finder(
    const std::function<Eigen::VectorX3I(const Eigen::VectorX3I&)>& f,
    const Eigen::VectorX3I& x0,
    const Eigen::VectorX3d& tol,
    Eigen::VectorX3I& x, const bool check_vf)
{
    return interval_root_finder(
        f, [](const Eigen::VectorX3I&) { return true; }, x0, tol, x,check_vf);
}

inline Eigen::VectorX3d width(const Eigen::VectorX3I& x)
{
    Eigen::VectorX3d w(x.size());
    for (int i = 0; i < x.size(); i++) {
        w(i) = width(x(i));
    }
    return w;
}
// convert Numccd to double number
double Numccd2double(const Numccd& n){
    double r=double(n.first)/pow(2,n.second);
    return r;
}
Eigen::VectorX3d width(const Interval3&x){
    Eigen::VectorX3d w;
    w.resize(3);
    for(int i=0;i<3;i++){
        w[i]=Numccd2double(x[i].second)-Numccd2double(x[i].first);
        assert(w[i]>=0);
    }
    return w;
    
}
template <int dim, int max_dim = dim>
inline bool zero_in(Eigen::Vector<Interval, dim, max_dim> X)
{
    // Check if the origin is in the n-dimensional interval
    for (int i = 0; i < X.size(); i++) {
        if (!boost::numeric::zero_in(X(i))) {
            return false;
        }
    }
    return true;
}
// check if (i1,i2) overlaps {(u,v)|u+v<1,u>=0,v>=0}
// by checking if i1.lower()+i2.lower()<=1
bool interval_satisfy_constrain(const Interval &i1, const Interval &i2){
    Interval l1(i1.lower(),i1.lower());
    Interval l2(i2.lower(),i2.lower());
    Interval sum=l1+l2;
    if(sum.lower()>1)
    return false;
    else return true;
    }


bool interval_root_finder(
    const std::function<Eigen::VectorX3I(const Eigen::VectorX3I&)>& f,
    const std::function<bool(const Eigen::VectorX3I&)>& constraint_predicate,
    const Eigen::VectorX3I& x0,
    const Eigen::VectorX3d& tol,
    Eigen::VectorX3I& x,const bool check_vf)
{
    // Stack of intervals and the last split dimension
    std::stack<std::pair<Eigen::VectorX3I, int>> xs;
    xs.emplace(x0, -1);
    while (!xs.empty()) {
        x = xs.top().first;
        int last_split = xs.top().second;
        xs.pop();

        Eigen::VectorX3I y = f(x);

        if (!zero_in(y)) {
            continue;
        }

        Eigen::VectorX3d widths = width(x);
        if ((widths.array() <= tol.array()).all()) {
            if (constraint_predicate(x)) {
                return true;
            }
            continue;
        }

        // Bisect the next dimension that is greater than its tolerance
        int split_i;
        for (int i = 1; i <= x.size(); i++) {
            split_i = (last_split + i) % x.size();
            if (widths(split_i) > tol(split_i)) {
                break;
            }
        }
        std::pair<Interval, Interval> halves = bisect(x(split_i));
        Eigen::VectorX3I x1 = x;
        // Push the second half on first so it is examined after the first half
        if(check_vf){
            if(split_i==1){
                if(interval_satisfy_constrain(halves.second,x(2))){
                    x(split_i) = halves.second;
                    xs.emplace(x, split_i);
                }
                if(interval_satisfy_constrain(halves.first,x(2))){
                    x(split_i) = halves.first;
                    xs.emplace(x, split_i);
                }
            }
            if(split_i==2){
                if(interval_satisfy_constrain(halves.second,x(1))){
                    x(split_i) = halves.second;
                    xs.emplace(x, split_i);
                }
                if(interval_satisfy_constrain(halves.first,x(1))){
                    x(split_i) = halves.first;
                    xs.emplace(x, split_i);
                }
            }
            if(split_i==0){
                x(split_i) = halves.second;
                xs.emplace(x, split_i);
                x(split_i) = halves.first;
                xs.emplace(x, split_i);
            }
        }
        else{
            x(split_i) = halves.second;
            xs.emplace(x, split_i);
            x(split_i) = halves.first;
            xs.emplace(x, split_i);
        }
        
        
    }
    return false;
}
bool interval_root_finder(
    const std::function<Eigen::VectorX3I(const Eigen::VectorX3I&)>& f,
    const std::function<bool(const Eigen::VectorX3I&)>& constraint_predicate,
    const Eigen::VectorX3I& x0,
    const Eigen::VectorX3d& tol,
    Eigen::VectorX3I& x){
        return interval_root_finder(f,constraint_predicate,x0,tol,x,false);
    }

// check if 0 is in interval
bool interval_bounding_box_check(const Eigen::Vector3I&in, std::array<bool,6>& flag){
    
    for(int i=0;i<3;i++){
        if(!flag[2*i]){
            if(in(i).lower()<=0){
                flag[2*i]=true;
            }
        }
        if(!flag[2*i+1]){
            if(in(i).upper()>=0){
                flag[2*i+1]=true;
            }
        }
    }
    if(flag[0]&&flag[1]&&flag[2]&&flag[3]&&flag[4]&&flag[5]) 
        return true;
    else return false;
}

template<typename T>
bool evaluate_bbox_one_dimension(
    const std::array<Numccd,2>& t,
    const std::array<Numccd,2>& u,
    const std::array<Numccd,2>& v,
    const Eigen::Vector3d& a0s,
    const Eigen::Vector3d& a1s,
    const Eigen::Vector3d& b0s,
    const Eigen::Vector3d& b1s,
    const Eigen::Vector3d& a0e,
    const Eigen::Vector3d& a1e,
    const Eigen::Vector3d& b0e,
    const Eigen::Vector3d& b1e,
    const int dimension,T tp){
    
    
    bool flag0=false, flag1=false;
    for(int i=0;i<2;i++){
        for(int j=0;j<2;j++){
            for(int k=0;k<2;k++){
                int eva=function_f(t[i],u[j],v[k],tp,dimension,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e);
                if(eva<=0){
                    flag0=true;
                }
                if(eva>=0){
                    flag1=true;
                }
                if(flag0&&flag1){
                    return true;
                }
            }
        }
    }
    if(flag0&&flag1)
    return true;
    return false;
        
}
bool Origin_in_function_bounding_box(
    const Interval3& paras,
    const std::function<Eigen::VectorX3I(const Numccd&, const Numccd&, const Numccd&)>& f){
    igl::Timer timer;
    std::array<Numccd,2> t,u,v;
    
    t[0]=paras[0].first;
    t[1]=paras[0].second;
    u[0]=paras[1].first;
    u[1]=paras[1].second;
    v[0]=paras[2].first;
    v[1]=paras[2].second;
    Eigen::Vector3I result;
    std::array<bool,6> flag;// when all flags are true, return true;
    for(int i=0;i<6;i++){
        flag[i]=false;
    }
    
    for(int i=0;i<2;i++){
        for(int j=0;j<2;j++){
            for(int k=0;k<2;k++){
                timer.start();
                result=f(t[i],u[j],v[k]);
                timer.stop();
                time23+=timer.getElapsedTimeInMicroSec();
                timer.start();
                bool check=interval_bounding_box_check(result,flag);
                timer.stop();
                time24+=timer.getElapsedTimeInMicroSec();
                if(check)
                    return true;
            }
        }
    }
    return false;
}

bool Origin_in_function_bounding_box_double(
    const Interval3& paras,
    const Eigen::Vector3d& a0s,
    const Eigen::Vector3d& a1s,
    const Eigen::Vector3d& b0s,
    const Eigen::Vector3d& b1s,
    const Eigen::Vector3d& a0e,
    const Eigen::Vector3d& a1e,
    const Eigen::Vector3d& b0e,
    const Eigen::Vector3d& b1e){
    //igl::Timer timer;
    std::array<Numccd,2> t,u,v;
    
    t[0]=paras[0].first;
    t[1]=paras[0].second;
    u[0]=paras[1].first;
    u[1]=paras[1].second;
    v[0]=paras[2].first;
    v[1]=paras[2].second;
    //bool zero_0=false, zer0_1=false, zero_2=false;
    double input_type;
    if(!evaluate_bbox_one_dimension(t,u,v,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e,0,input_type))
        return false;
    if(!evaluate_bbox_one_dimension(t,u,v,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e,1,input_type))
        return false;
    if(!evaluate_bbox_one_dimension(t,u,v,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e,2,input_type))
        return false;
    return true;
    
}
bool Origin_in_function_bounding_box_Rational(xx
    const Interval3& paras,
    const Eigen::Vector3d& a0s,
    const Eigen::Vector3d& a1s,
    const Eigen::Vector3d& b0s,
    const Eigen::Vector3d& b1s,
    const Eigen::Vector3d& a0e,
    const Eigen::Vector3d& a1e,
    const Eigen::Vector3d& b0e,
    const Eigen::Vector3d& b1e){
    //igl::Timer timer;
    std::array<Numccd,2> t,u,v;
    
    t[0]=paras[0].first;
    t[1]=paras[0].second;
    u[0]=paras[1].first;
    u[1]=paras[1].second;
    v[0]=paras[2].first;
    v[1]=paras[2].second;
    //bool zero_0=false, zer0_1=false, zero_2=false;
    Rational input_type;
    if(!evaluate_bbox_one_dimension(t,u,v,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e,0,input_type))
        return false;
    if(!evaluate_bbox_one_dimension(t,u,v,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e,1,input_type))
        return false;
    if(!evaluate_bbox_one_dimension(t,u,v,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e,2,input_type))
        return false;
    return true;
    
}
// return power t. n=result*2^t
int reduction(const int n, int& result){
    int t=0;
    int newn=n;
    while (newn%2==0){
        newn=newn/2;
        t++;
    }
    result=newn;
    return t;
}
std::pair<Singleinterval, Singleinterval> bisect(const Singleinterval& inter){
    Numccd low=inter.first;
    Numccd up=inter.second;

    // interval is [k1/pow(2,n1), k2/pow(2,n2)], k1,k2,n1,n2 are all not negative
    int k1=low.first;
    int n1=low.second;
    int k2=up.first;
    int n2=up.second;

    assert(k1 >= 0 && k2 >= 0 && n1 >= 0 && n2 >= 0);

    std::pair<Singleinterval, Singleinterval> result;
    int k,n, p,r;
    if(n2==n1){
        p=reduction(k1+k2,r);
        k=r;
        n=n2-p+1;
    }
    if(n2>n1){
        k=k1*pow(2,n2-n1)+k2; assert(k%2==1);
        n=n2+1;
    }
    if(n2<n1){
        k=k1+k2*pow(2,n1-n2); assert(k%2==1);
        n=n1+1;
    }
    Numccd newnum(k,n);
    Singleinterval i1(low,newnum),i2(newnum,up);
    result.first=i1;result.second=i2;
    return result;
}
bool interval_root_finder_opt(
    const std::function<Eigen::VectorX3I(const Numccd&, const Numccd&, const Numccd&)>& f,
    //const std::function<bool(const Eigen::VectorX3I&)>& constraint_predicate,
    //const Eigen::VectorX3I& x0,// initial interval, must be [0,1]x[0,1]x[0,1]
    const Eigen::VectorX3d& tol,
    //Eigen::VectorX3I& x,// result interval
    Interval3& final,
    const bool check_vf){
    
    Numccd low_number; low_number.first=0; low_number.second=0;// low_number=0;
    Numccd up_number; up_number.first=1; up_number.second=0;// up_number=1;
    // initial interval [0,1]
    Singleinterval init_interval;init_interval.first=low_number;init_interval.second=up_number;
    //build interval set [0,1]x[0,1]x[0,1]
    Interval3 iset;
    iset[0]=init_interval;iset[1]=init_interval;iset[2]=init_interval;
    // Stack of intervals and the last split dimension
    std::stack<std::pair<Interval3,int>> istack;
    istack.emplace(iset,-1);

    // current intervals
    Interval3 current;
    while(!istack.empty()){
        current=istack.top().first;
        int last_split=istack.top().second;
        istack.pop();
        igl::Timer timer;
        timer.start();
        bool zero_in = Origin_in_function_bounding_box(current,f);
        timer.stop();
        time20+=timer.getElapsedTimeInMicroSec();
        if(!zero_in) continue;
        timer.start();
        Eigen::VectorX3d widths = width(current);
        timer.stop();
        time21+=timer.getElapsedTimeInMicroSec();
        if ((widths.array() <= tol.array()).all()) {
            final=current;
                return true;
        }

        // Bisect the next dimension that is greater than its tolerance
        int split_i;
        for (int i = 1; i <= 3; i++) {
            split_i = (last_split + i) % 3;
            if (widths(split_i) > tol(split_i)) {
                break;
            }
        }
        timer.start();
        std::pair<Singleinterval, Singleinterval> halves = bisect(current[split_i]);
        current[split_i] = halves.second;
        istack.emplace(current, split_i);
        current[split_i] = halves.first;
        istack.emplace(current, split_i);
        timer.stop();
        time22+=timer.getElapsedTimeInMicroSec();

    }
    return false;
    
}
// calculate the sign of f. dim is the dimension we are evaluating.
template<typename T>
int function_f (
const Numccd&tpara, const Numccd&upara, const Numccd&vpara,const T& type, const int dim,
const Eigen::Vector3d& a0s,
    const Eigen::Vector3d& a1s,
    const Eigen::Vector3d& b0s,
    const Eigen::Vector3d& b1s,
    const Eigen::Vector3d& a0e,
    const Eigen::Vector3d& a1e,
    const Eigen::Vector3d& b0e,
    const Eigen::Vector3d& b1e ) {
       
       int tu = tpara.first, td=tpara.second;// t=tu/(2^td)
       int uu = upara.first, ud=upara.second;
       int vu = vpara.first, vd=vpara.second;

       T edge0_vertex0
           = (T(a0e[dim]) - T(a0s[dim])) * tu/int(pow(2,td))
           + T(a0s[dim]);
       T edge0_vertex1
           = (T(a1e[dim]) - T(a1s[dim])) * tu/int(pow(2,td))
           + T(a1s[dim]);
       T edge0_vertex
           = (edge0_vertex1 - edge0_vertex0) * uu/int(pow(2,ud)) + edge0_vertex0;

       T edge1_vertex0
           = (T(b0e[dim]) - T(b0s[dim])) * tu/int(pow(2,td))
           + T(b0s[dim]);
       T edge1_vertex1
           = (T(b1e[dim]) - T(b1s[dim])) * tu/int(pow(2,td))
           + T(b1s[dim]);
       T edge1_vertex
           = (edge1_vertex1 - edge1_vertex0) * vu/int(pow(2,vd)) + edge1_vertex0;

       
       if(edge1_vertex>edge0_vertex) return 1;
       if(edge1_vertex<edge0_vertex) return -1;
       return 0;
};
bool interval_root_finder_double(
    const Eigen::VectorX3d& tol,
    //Eigen::VectorX3I& x,// result interval
    Interval3& final,
    const bool check_vf,
    const Eigen::Vector3d& a0s,
    const Eigen::Vector3d& a1s,
    const Eigen::Vector3d& b0s,
    const Eigen::Vector3d& b1s,
    const Eigen::Vector3d& a0e,
    const Eigen::Vector3d& a1e,
    const Eigen::Vector3d& b0e,
    const Eigen::Vector3d& b1e){
    
    Numccd low_number; low_number.first=0; low_number.second=0;// low_number=0;
    Numccd up_number; up_number.first=1; up_number.second=0;// up_number=1;
    // initial interval [0,1]
    Singleinterval init_interval;init_interval.first=low_number;init_interval.second=up_number;
    //build interval set [0,1]x[0,1]x[0,1]
    Interval3 iset;
    iset[0]=init_interval;iset[1]=init_interval;iset[2]=init_interval;
    // Stack of intervals and the last split dimension
    std::stack<std::pair<Interval3,int>> istack;
    istack.emplace(iset,-1);

    // current intervals
    Interval3 current;
    while(!istack.empty()){
        current=istack.top().first;
        int last_split=istack.top().second;
        istack.pop();
        igl::Timer timer;
        timer.start();
        bool zero_in = Origin_in_function_bounding_box_double(current,a0s,a1s,b0s,b1s,a0e,a1e,b0e,b1e);
        timer.stop();
        time20+=timer.getElapsedTimeInMicroSec();
        if(!zero_in) continue;
        timer.start();
        Eigen::VectorX3d widths = width(current);
        timer.stop();
        time21+=timer.getElapsedTimeInMicroSec();
        if ((widths.array() <= tol.array()).all()) {
            final=current;
                return true;
        }

        // Bisect the next dimension that is greater than its tolerance
        int split_i;
        for (int i = 1; i <= 3; i++) {
            split_i = (last_split + i) % 3;
            if (widths(split_i) > tol(split_i)) {
                break;
            }
        }
        timer.start();
        std::pair<Singleinterval, Singleinterval> halves = bisect(current[split_i]);
        current[split_i] = halves.second;
        istack.emplace(current, split_i);
        current[split_i] = halves.first;
        istack.emplace(current, split_i);
        timer.stop();
        time22+=timer.getElapsedTimeInMicroSec();

    }
    return false;
    
}
void print_time_2(){
    std::cout<<"origin predicates, "<<time20<<std::endl;
    std::cout<<"width, "<<time21<<std::endl;
    std::cout<<"bisect, "<<time22<<std::endl;
    std::cout<<"origin part1, "<<time23<<std::endl;
    std::cout<<"origin part2, "<<time24<<std::endl;
}
} // namespace ccd
