/* 
 * File:   Labelling.cpp
 * Author: Mattias
 * 
 * Created on 13 June 2019, 17:55
 */

#include <vector>

#include "Labelling.hpp"
#include "Permutation.hpp"

Labelling::Labelling() : perm(), props() {};

Labelling::Labelling(DiagramNode& root, int n_legs) 
: perm(n_legs), props()
{
    root.set_momenta();
    root.label(props, n_legs);
    normalise();
}

Labelling::Labelling(const Labelling& orig, const permute::Permutation& cycl)
: perm(cycl), props() 
{    
    for(const Propagator& p : orig.props)
        props.push_back(Propagator(p, cycl));
    
    normalise();
}

void Labelling::normalise(){
    std::sort(props.begin(), props.end());
    std::vector<Propagator>::iterator last = std::unique(props.begin(), props.end());
    props.resize(std::distance(props.begin(), last));
}

permute::Permutation Labelling::index_locations() const {
    return perm.inverse();
}

bool operator<(const Labelling& l1, const Labelling& l2){
    if(l1.props.size() != l2.props.size())
        return l1.props.size() < l2.props.size();
    return l1.props < l2.props;
}

bool operator==(const Labelling& l1, const Labelling& l2){
    return l1.props == l2.props;
}

std::ostream& operator<<(std::ostream& out, const Labelling& l){
    out << l.perm;
    for(const Propagator& p : l.props)
        out << " | " << p;
    if(l.props.empty())
        out << " | [no propagators]";
    
    return out;
}

void Labelling::print_header(std::ostream& out) const{
    out << permute::Permutation::identity(perm.size());
    for(const Propagator& p : props){
        out << std::string(strlen(" | "), ' ');
        p.print_header(out);
    }
    if(props.empty())
        out << std::string(strlen(" | [no propagators]"), ' ');
}

