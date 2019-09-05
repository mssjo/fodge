/* 
 * File:   Propagator.hpp
 * Author: Mattias
 *
 * Created on 19 June 2019, 16:09
 */

#ifndef PROPAGATOR_H
#define	PROPAGATOR_H

#include "fodge.hpp"

/**
 * @brief Represents a propagator for the purposes of uniquely specifying
 * kinematic structures.
 * 
 * For a labelling on a flavour-ordered diagram, the kinematic structure is
 * uniquely determined by
 * <ul>
 *  <li> for @f$\mathcal O(p^2)@f$: the list of propagator momenta.
 *  <li> for higher-order non-singlet diagrams: the list of propagator momenta,
 *       plus the vertex orders at each end of each propagator, plus knowledge
 *       of the flavour split of the diagram.
 *  <li> for singlet diagrams: the above, plus the momentum on another vertex
 *       leg adjacent to each singlet propagator. We choose the leg preceding
 *       the propagator in the list of legs on a @link DiagramNode node @endlink.
 * </ul>
 * this information is supplied by this class. Importantly, momenta are normalised
 * to a canonical form under conservation of momentum so that well-defined
 * comparisons can be made.
 */
class Propagator {
public:
    Propagator() = default;
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

    /** Bitmask storing the momentum indices carried by the propagator. */
    mmask momenta;
    /** Total number of momenta in the containing diagram. */
    int n_mom;
    
    /** Order of the "source" vertex (momentum flowing out) */
    int src_order;
    /** Momenta carried by adjacent vertex leg at source, used by singlets */
    mmask src_prev;
    /** Order of the "destination" vertex (momentum flowing in) */
    int dst_order;
    /** Momenta carried by adjacent vertex leg at dest, used by singlets */
    mmask dst_prev;
};

#endif	/* PROPAGATOR_H */

