/* 
 * File:   Labelling.cpp
 * Author: Mattias
 * 
 * Created on 13 June 2019, 17:55
 */

#include <vector>

#include "Labelling.hpp"
#include "Permutation.hpp"

/**
 * @brief Labels a diagram.
 * 
 * @param root the root node of the diagram.
 * @param n_legs the number of legs on the diagram.
 * 
 * The diagram must have its flavour structre determined and ints external legs
 * indexed before this method is called.
 */
Labelling::Labelling(DiagramNode& root, int n_legs) 
: perm(n_legs), props()
{
    root.set_momenta();
    root.label(props, n_legs);
    normalise();
}

/**
 * @brief Creates a permutation of a labelling.
 * 
 * @param orig the original labelling. It is left unchanged.
 * @param cycl the permutation.
 */
Labelling::Labelling(const Labelling& orig, const permute::Permutation& cycl)
: perm(cycl), props() 
{    
    for(const Propagator& p : orig.props)
        props.push_back(Propagator(p, cycl));
    
    normalise();
}

/**
 * @brief Normalises a labelling by lexicographically sorting its propagators.
 */
void Labelling::normalise(){
    std::sort(props.begin(), props.end());
    std::vector<Propagator>::iterator last 
        = std::unique(props.begin(), props.end());
    props.resize(std::distance(props.begin(), last));
}

/**
 * @brief Creates a permutation mapping the indices in this labelling to their
 * location on the diagram in the identity labelling.
 * 
 * @return the inverse of this labelling's internal permutation.
 */
permute::Permutation Labelling::index_locations() const {
    return perm.inverse();
}

/**
 * @brief Compares two labellings lexicographically 
 * by their list of propagators.
 * 
 * @param l1 a labelling.
 * @param l2 another labelling.
 * @return @c true if @p l1 is lexicographically less than @p l2, @c false
 *      otherwise.
 * 
 * The permutation does not matter, only the propagators 
 * (i.e. the kinematic factor).
 */
bool operator<(const Labelling& l1, const Labelling& l2){
    if(l1.props.size() != l2.props.size())
        return l1.props.size() < l2.props.size();
    return l1.props < l2.props;
}

/**
 * @brief Compares two labellings for equality.
 * 
 * @param l1 a labelling.
 * @param l2 another labelling.
 * @return @c true if @p l1 and @p l2 are identical for physical purposes, 
 *      @c false otherwise.
 * 
 * The permutation does not matter, only the propagators 
 * (i.e. the kinematic factor).
 */
bool operator==(const Labelling& l1, const Labelling& l2){
    return l1.props == l2.props;
}

/**
 * @brief Prints a summary of a labelling.
 * 
 * @param out   the stream to which the labelling should be printed.
 * @param l     the labelling.
 * @return  the stream.
 * 
 * Prints the permutation using the standard format for a 
 * @link permute::Permutation @endlink, followed by the propagator list separated by
 * vertical bars, using the standard format for a @link Propagator @endlink.
 */
std::ostream& operator<<(std::ostream& out, const Labelling& l){
    out << l.perm;
    for(const Propagator& p : l.props)
        out << " | " << p;
    if(l.props.empty())
        out << " | [no propagators]";
    
    return out;
}

/**
 * @brief Prints a header matching the printouts as an aid when reading them.
 * 
 * @param out the stream to which the header should be printed.
 */
void Labelling::print_header(std::ostream& out) const{
    out << permute::Permutation::identity(perm.size());
    for(const Propagator& p : props){
        out << std::string(strlen(" | "), ' ');
        p.print_header(out);
    }
    if(props.empty())
        out << std::string(strlen(" | [no propagators]"), ' ');
}

