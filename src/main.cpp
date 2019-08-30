/* 
 * File:   main.cpp
 * Author: Mattias
 *
 * Created on 12 June 2019, 15:34
 */

#include "mf_fodge.h"
#include "Diagram.h"
#include "permute.h"

#include <fstream>

using namespace std;
using namespace permute;

/*
 * 
 */
int main(int argc, char** argv) {

//    for(ZR_Generator zr({2,3,3,3}); zr; ++zr)
//        std::cout << *zr << std::endl;
//    
//    Permutation p24130({2,4,1,3,0});
//    Permutation p01243({0,1,2,4,3});
//    Permutation c51 = Permutation::cyclic(5, 1);
//    Permutation i5 = Permutation::identity(5);
//    
//    std::cout << p24130 << " * " << p01243 << " = " << (p24130 * p01243) << std::endl;
//    std::cout << p01243 << " * " << p24130 << " = " << (p01243 * p24130) << std::endl;
//    std::cout << p24130 << " % " << c51 << " = " << (p24130 % c51) << std::endl;
//    std::cout << "order" << p24130 << " = " << p24130.order() << std::endl;
//    for(int i = 2; i <= p24130.order(); i++)
//        std::cout << p24130 << " ^ " << i << " = " << (p24130 ^ i) << std::endl;
//    std::cout << p24130 << " * " << p24130.inverse() << " = " << p24130 * p24130.inverse() << " -> " << (p24130 * p24130.inverse()).is_identity() << std::endl;
    
    int order = 8;
    int n_legs = 10;
    
    std::ofstream tikz;
    
    tikz.open("mf_fodge.tex");
    
    int count = 0;
    for(Diagram& d : Diagram::generate(order, n_legs, true)){
        cout << "[" << ++count << "] " << d << endl;
        d.TikZ(tikz, 0, count);
    }
    cout << count << endl;
    
    tikz.close();
    
    return 0;
}

