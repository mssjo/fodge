/* 
 * File:   Generator.cpp
 * Author: Mattias Sjo
 * 
 * Implements the group generators defined in Generator.h
 * 
 * Created on 27 June 2019, 16:11
 */

#include <numeric>
#include <vector>

#include "Generator.hpp"

namespace permute{
    
/**
 * Base constructor: sets up the identity permutation.
 * @param n the size of the permutation.
 */
Generator::Generator(int n)
: done(false), perm(n) 
{}

/**
 * Constructs a Generator for @f$Z_n$.
 * @param n the value of @f$n$.
 */
Zn_Generator::Zn_Generator(int n) 
: Generator(n), n(n), count(0)
{}

/**
 * Pre-increments th cyclic permutation by cycling it one more step.
 * @return the updated generator. 
 */
Zn_Generator& Zn_Generator::operator++(){
    for(int i = 0; i < perm.size() - 1; i++)
        perm.swap(i, i+1);
    
    if(++count >= n){
        done = true;
        count %= n;
    }
    else
        done = false;
    
    return *this;
}

///**
// * Post-increment version of @link Zn_Generator::operator++() 
// */
//Zn_Generator Zn_Generator::operator++(int dummy){
//    Zn_Generator const tmp(*this);
//    ++*this;
//    return tmp;
//}

/**
 * Constructs a Generator for @f$S_n$.
 * @param n the value of @f$n$.
 */
Sn_Generator::Sn_Generator(int n) 
: Generator(n), n(n), ctr_stack(n, 0), stack_idx(0)
{}

/**
 * @brief Visits the next permutation through the non-recursive version of 
 * Heap's algorithm.
 * 
 * Heap's algorithm visits all permutations of @f$n$ elements exactly once and
 * moves to each new permutation by only exchanging two elements. The algorithm
 * is recursive in nature and generates all permutations of the first @f$k-1$
 * elements before touching the @f$k$th. Here, we emulate the recursion by
 * maintaining a stack of counters rather than recursively nested @c for loops. 
 * 
 * @return the updated generator.
 */
Sn_Generator& Sn_Generator::operator++(){
    stack_idx = 0;
    while(stack_idx < n){
        if(ctr_stack[stack_idx] < stack_idx){
            if(stack_idx % 2)
                perm.swap(ctr_stack[stack_idx], stack_idx);
            else
                perm.swap(0, stack_idx);
            
            ctr_stack[stack_idx]++;
            
            done = false;
            return *this;
        }
        else{
            ctr_stack[stack_idx] = 0;
            stack_idx++;
        }
    }
    
    done = true;
    return *this;    
}
///**
// * Post-increment version of @link Sn_Generator::operator++() 
// */
//Sn_Generator Sn_Generator::operator ++(int dummy){
//    Sn_Generator const tmp(*this);
//    ++*this;
//    return tmp;
//}

/**
 * Constructs a generator for @f$Z_R$.
 * @param R @f$R$, an ordered sequence of integers. If the sequence is not 
 *      ordered, the Generator will not work as intended. The size of the 
 *      permutations is the sum of the elements of @p R.
 */
ZR_Generator::ZR_Generator(const std::vector<int>& R) 
: Generator(std::accumulate(R.begin(), R.end(), 0)), cycl(), swap() 
{
    //Current size, for equality comparisons
    int size = -1;
    //Number of equal-size traces in a row
    int count = 1;
    //Current index offset
    int offs = 0;
    //Offset of the beginning of a row of equal traces
    int row_begin = 0;
    
    for(int r : R){
        //Same size as previous: increment count
        if(r == size){
            count++;
        }
        //Not same size: add a block-wise swap group if this ends a row of
        //several same-size traces
        else{
            if(count > 1){
                swap.push_back(std::make_pair(
                        Sn_Generator(count), 
                        std::make_pair(row_begin, size)
                ));
            }
            size = r;
            count = 1;
            row_begin = offs;
        }
        
        //Add cyclic group if not trivial
        if(r > 1)
            cycl.push_back(std::make_pair(Zn_Generator(r), offs));
                
        offs += r;
    }
    
    //Add last row of same-size traces if non-trivial
    if(count > 1)
        swap.push_back(std::make_pair(
                Sn_Generator(count), 
                std::make_pair(row_begin, size)
        ));
}

ZR_Generator::ZR_Generator(const std::initializer_list<int>& R) 
: ZR_Generator(std::vector<int>(R)) {}

/**
 * Visits the next permutation by updating the subcomponent groups in turn.
 * @return the updated generator.
 */
ZR_Generator& ZR_Generator::operator++(){
    done = false;
    
    //Cyclings first...
    for(auto& c : cycl){
        (*(c.first)).inverse().permute(perm, c.second);
        if(++(c.first)){
            (*(c.first)).permute(perm, c.second);
            return *this;
        }        
    }
    //...then swaps.
    for(auto& s : swap){
        (*(s.first)).inverse().permute(perm, s.second.first, s.second.second);
        if(++(s.first)){
            (*(s.first)).permute(perm, s.second.first, s.second.second);
            return *this;
        }
    }
    
    //Only if all subcomponents are done are we done.
    done = true;
    return *this;
}
///**
// * Post-increment version of @link ZR_Generator::operator++() 
// */
//ZR_Generator ZR_Generator::operator ++(int dummy){
//    ZR_Generator const tmp(*this);
//    ++*this;
//    return tmp;
//}


}