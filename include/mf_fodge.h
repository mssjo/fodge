/* 
 * File:   mf_fodge.h
 * Author: Mattias
 *
 * Created on 13 June 2019, 17:46
 */

#ifndef MF_FODGE_H
#define	MF_FODGE_H

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

#include "permute.h"
#include "bitwise.h"

#define PI 3.14159265358979

typedef uint32_t mmask;
/** Simple container struct specifying a vertex. Could've just used a pair, 
 *  but this way we get member names! */
typedef std::pair<int, std::vector<int>> vertex;

/** Returns true if the 1-bits of a is a subset of the 1-bits of b. */
#define SUBSET(a,b) (((a) & (b)) == (a))

class Diagram;
class DiagramNode;
class Labeling;
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

#endif	/* MF_FODGE_H */

