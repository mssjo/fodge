/* 
 * File:   Labeling.h
 * Author: Mattias
 *
 * Created on 13 June 2019, 17:55
 */

#ifndef LABELING_H
#define	LABELING_H

#include "mf_fodge.h"

#include "Propagator.h"
#include "DiagramNode.h"
#include "Permutation.h"

class Labeling {
public:
    Labeling();
    Labeling(DiagramNode& root, int n_legs);
    Labeling(const Labeling& orig) = default;
    Labeling(const Labeling& orig, const permute::Permutation& cycl);
    virtual ~Labeling() = default;
    
    friend bool operator<(const Labeling& l1, const Labeling& l2);
    friend bool operator==(const Labeling& l1, const Labeling& l2);
    
    friend std::ostream& operator<<(std::ostream& out, const Labeling& l);
    void print_header(std::ostream& out) const;
    
    permute::Permutation index_locations() const;

private:
    void normalise();
    std::vector<Propagator> props;
    permute::Permutation perm;
};


#endif	/* LABELING_H */

