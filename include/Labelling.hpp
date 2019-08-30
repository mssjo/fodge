/* 
 * File:   Labelling.hpp
 * Author: Mattias
 *
 * Created on 13 June 2019, 17:55
 */

#ifndef LABELLING_H
#define	LABELLING_H

#include "fodge.hpp"

#include "Propagator.hpp"
#include "DiagramNode.hpp"
#include "Permutation.hpp"

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

private:
    void normalise();
    std::vector<Propagator> props;
    permute::Permutation perm;
};


#endif	/* LABELLING_H */

