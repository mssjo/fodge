/* 
 * File:   Propagator.cpp
 * Author: Mattias
 * 
 * Created on 19 June 2019, 16:09
 */

#include "Propagator.h"

Propagator::Propagator() : momenta(0), src_order(0), dst_order(0), n_mom(0) {};

Propagator::Propagator(mmask momenta, int n_mom, 
        int src_order, 
        int dest_order) 
: Propagator(momenta, n_mom, src_order, 0, dest_order, 0)
{}

Propagator::Propagator(mmask momenta, int n_mom,
            int src_order, mmask src_prev, 
            int dst_order, mmask dst_prev)
: momenta(momenta), 
        src_order(src_order), src_prev(src_prev), 
        dst_order(dst_order), dst_prev(dst_prev), 
        n_mom(n_mom)
{
    normalise();
}

Propagator::Propagator(const Propagator& orig, 
        const permute::Permutation& cycl) 
: momenta(cycl.permute_bits(orig.momenta)), 
        src_order(orig.src_order), src_prev(cycl.permute_bits(orig.src_prev)),
        dst_order(orig.dst_order), dst_prev(cycl.permute_bits(orig.dst_prev)), 
        n_mom(orig.n_mom)
{
    assert(n_mom == cycl.size());
    normalise();
}

void Propagator::normalise(){
    mmask last_mask = 1 << (n_mom-1);
    mmask all_mask = (1 << n_mom) - 1;
    
    src_prev = normalise_mmask(src_prev, last_mask, all_mask);
    dst_prev = normalise_mmask(dst_prev, last_mask, all_mask);
    mmask norm = normalise_mmask(momenta, last_mask, all_mask);
    
    if(norm != momenta){
        std::swap(src_order, dst_order);
        std::swap(src_prev, dst_prev);
        momenta = norm;
    }
}

mmask Propagator::normalise_mmask(mmask m, mmask last_mask, mmask all_mask)
const {
    int count = bitwise::bitcount(m);
    if(count > n_mom/2 || (count == n_mom/2 && (m & last_mask)))
        m ^= all_mask;
    return m;
}

bool operator<(const Propagator& p1, const Propagator& p2){
    
    if(p1.src_order != p2.src_order)
        return p1.src_order < p2.src_order;
    if(p1.dst_order != p2.dst_order)
        return p1.dst_order < p2.dst_order;
    if(p1.src_prev != p2.src_prev)
        return p1.src_prev < p2.src_prev;
    if(p1.dst_prev != p2.dst_prev)
        return p1.dst_prev < p2.dst_prev;
    
    return p1.momenta < p2.momenta;
}

bool operator==(const Propagator& p1, const Propagator& p2){
    return (p1.momenta == p2.momenta) 
            && (p1.src_order == p2.src_order) 
            && (p1.dst_order == p2.dst_order)
            && (p1.src_prev == p2.src_prev)
            && (p1.dst_prev == p2.dst_prev);
}
#define HI_CHAR 'X'
#define LO_CHAR '.'
std::ostream& operator<<(std::ostream& out, const Propagator& p){
    bool print_prev = p.src_prev || p.dst_prev;
    
    bitwise::print_bits(p.momenta, p.n_mom, out, HI_CHAR, LO_CHAR);
    
    out << " (" << p.src_order;
    if(print_prev){
        out << "[";
        bitwise::print_bits(p.src_prev, p.n_mom, out, HI_CHAR, LO_CHAR);
        out << "]";
    }    
    out << " -> " << p.dst_order;
    if(print_prev){
        out << "[";
        bitwise::print_bits(p.dst_prev, p.n_mom, out, HI_CHAR, LO_CHAR);
        out << "]";
    }
    out << ")";
    
    return out;
}

void Propagator::print_header(std::ostream& out) const {
    bool print_prev = src_prev || dst_prev;
    int w = 0;
    for(int ord = std::max(src_order, dst_order); ord > 0; ord /= 10, w++);
    
    for(int i = 0; i < n_mom; i++)
        out << (i % 10);
    
    out << "  " << std::string(w, ' ');
    if(print_prev){
        out << " ";
        for(int i = 0; i < n_mom; i++)
            out << (i % 10);
        out << " ";
    }
    
    out << "    " << std::string(w, ' ');
    if(print_prev){
        out << " ";
        for(int i = 0; i < n_mom; i++)
            out << (i % 10);
        out << " ";
    }
    out << " ";   
}
