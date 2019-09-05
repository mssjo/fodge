/* 
 * File:   Labelling.hpp
 * Author: Mattias Sjo
 *
 * Created on 13 June 2019, 17:55
 */

#ifndef LABELLING_H
#define	LABELLING_H

#include "fodge.hpp"

#include "Propagator.hpp"
#include "DiagramNode.hpp"
#include "Permutation.hpp"


/**
 * @brief Describes labellings of flavour-ordered diagrams.
 * 
 * A labelling is a way to place indices on external legs in a flavour-ordered
 * manner. Physically, a labelling is associated with a kinematic structure.
 * For a diagram, only labellings that give distinct kinematic structures should
 * be kept. For this end, labellings are normalised and sorted based on
 * kinematic structure so that duplicates can be removed.
 *
 * The kinematic factor in a flavour-ordered labelling is uniquely determined by
 * the propagator momenta it gives, plus some auxiliary information stored together
 * with them in the @link Propagator @endlink class.
 */
class Labelling
 {
public:
    Labelling();
    Labelling (DiagramNode& root, int n_legs);
    Labelling (const Labelling& orig) = default;
    Labelling (const Labelling& orig, const permute::Permutation& cycl);
    virtual ~Labelling() = default;
    
    friend bool operator<(const Labelling& l1, const Labelling& l2);
    friend bool operator==(const Labelling& l1, const Labelling& l2);
    
    friend std::ostream& operator<<(std::ostream& out, const Labelling& l);
    void print_header(std::ostream& out) const;
    
    permute::Permutation index_locations() const;
    
    void FORM(std::ostream& form) const;

private:
    void normalise();
    /** Propagators with auxiliary information*/
    std::vector<Propagator> props;
    /** Permutation defining the labelling relative to the
     *  one generated by indexing the diagram. */
    permute::Permutation perm;
};


#endif	/* LABELLING_H */

