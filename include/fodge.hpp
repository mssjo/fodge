/* 
 * File:   fodge.hpp
 * Author: Mattias
 *
 * Created on 13 June 2019, 17:46
 */

#ifndef FODGE_H
#define	FODGE_H

#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cmath>

#include <iostream>
#include <iomanip>

#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <unordered_map>


#include <algorithm>
#include <numeric>
#include <bitset>

#include "permute.hpp"
#include "bitwise.hpp"

#define FODGE_VERSION "FODGE version 2.0"

#define PI 3.14159265358979

/** Bitmask specifying a sum of some momenta 
 *  (those whose indices correspond to 1-bits) */
typedef uint32_t mmask;
/** An order-flavour split pair specifying a vertex. */
typedef std::pair<int, std::vector<int>> vertex;

/** Returns true if the 1-bits of a is a subset of the 1-bits of b. */
#define SUBSET(a,b) (((a) & (b)) == (a))

class Diagram;
class DiagramNode;
class Labelling;
class Propagator;

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T> vec){
    out << "{ ";
    for(const T& t : vec)
        out << t << " ";
    out << "}";
    
    return out;
}
template<typename T>
std::ostream& operator<<(std::ostream& out, const std::unordered_set<T> vec){
    out << "{ ";
    for(const T& t : vec)
        out << t << " ";
    out << "}";
    
    return out;
}
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& out, const std::pair<T1, T2> pair){
    out << "(" << pair.first << " " << pair.second << ")";
    
    return out;
}

#endif	/* FODGE_H */

