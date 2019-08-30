/* 
 * File:   Propagator.hpp
 * Author: Mattias
 *
 * Created on 19 June 2019, 16:09
 */

#ifndef PROPAGATOR_H
#define	PROPAGATOR_H

#include "mf_fodge.hpp"

class Propagator {
public:
    Propagator();
    Propagator(mmask momenta, int n_mom, 
        int src_order, 
        int dst_order);
    Propagator(mmask momenta, int n_mom,
        int src_order, mmask src_prev, 
        int dst_order, mmask dst_prev);
    Propagator(const Propagator& orig) = default;
    Propagator(const Propagator& orig, const permute::Permutation& cycl);
    virtual ~Propagator() = default;
    
    friend bool operator<(const Propagator& p1, const Propagator& p2);
    friend bool operator==(const Propagator& p1, const Propagator& p2);
    
    friend std::ostream& operator<<(std::ostream& out, const Propagator& p);
    void print_header(std::ostream& out) const;
    
    void FORM(std::ostream& form, mmask prop) const;

private:    
    void normalise();
    mmask normalise_mmask(mmask m, mmask last_mask, mmask all_mask) const;

    mmask momenta;
    int n_mom;
    
    int src_order;
    mmask src_prev;
    int dst_order;
    mmask dst_prev;
};

#endif	/* PROPAGATOR_H */

