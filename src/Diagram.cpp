/* 
 * File:   Diagram.cpp
 * Author: Mattias Sjo
 * 
 * Implements Diagram.h
 * 
 * Created on 12 June 2019, 15:37
 */

#include "Diagram.h"

/** 
 * @brief Default constructor.
 * 
 * Creates an O(p^2) 4-point diagram, the simplest possible diagram.
 */
Diagram::Diagram() : Diagram(2, std::vector<int>(1,4)) {};

/**
 * @brief Single-vertex diagram constructor.
 * 
 * @param order  the order of the diagram.
 * @param fsplit the flavour split of the diagram. Does not have to be sorted.
 * 
 * Creates the unique single-vertex diagram with the given properties. 
 * No constructor (other than the copy constructor) exists for other diagrams;
 * these are instead created form other diagrams via the @link extend @endlink
 * method.
 */
Diagram::Diagram(int order, const std::vector<int>& fsplit)
: order(order), flav_split(fsplit), root(order, fsplit), labelings(),
        n_legs(std::accumulate(fsplit.begin(), fsplit.end(), 0))
{
    std::sort(flav_split.begin(), flav_split.end());
    index();
    labelings.push_back(Labeling(root, n_legs));
}

/**
 * @brief Checks if a diagram vanishes identically. 
 * 
 * This happens due to singlet propagators "cutting off" flavour structures into
 * single-index traces.
 * 
 * @return @c true if the diagram vanishes.
 */
bool Diagram::is_zero(){
    if(flav_split[0] == 1)
        return true;
    if(order < 6)
        return false;
    
    return root.is_zero();
}

/**
 * @brief Generates all distinct flavour-ordered diagrams with the given
 * properties.
 * 
 * @param order         the order of the diagrams.
 * @param n_legs        the number of legs on the diagrams.
 * @param singlets      whether to include singlet diagrams.
 * @param remove_zero   whether to remove diagrams that are identically zero -
 *                      should normally only be @c false when the method calls
 *                      itself.
 * @return  a sorted vector containing the diagrams.
 * 
 * This is the main method for creating diagrams. It works by generating all
 * single-vertex diagrams of the given order and size, and then recursively
 * generating smaller and lower-order diagrams and extending them to the target
 * size and order. Finally, redundant diagrams are trimmed and the list of
 * diagrams is sorted.
 */
std::vector<Diagram> Diagram::generate(int order, int n_legs, 
            bool singlets, bool remove_zero){
    //TODO: catch invalid input
    
    auto diagrs = std::vector<Diagram>();
    //Generates single-vertex diagrams to seed the recursion.
    for(auto& flav_split : valid_flav_splits(order, n_legs)){
        
//        std::cout << "Generating diagram with flavour split " 
//                << flav_split << std::endl;
        
        diagrs.push_back(Diagram(order, flav_split));
    }
    
    //Recurses over all necessary smaller (size n) and lower-order (order o) 
    //diagrams and extends them.  
    //The extension is never by more orders than the order of the extended 
    //diagram. This cuts the number of o's in half.
    //The same can applied to n only when extension and extended diagram
    //are of the same order. Otherwise, all n must be covered.
    //Identically zero diagrams are not removed when recursing, since they may
    //be rendered nonzero by the extensions.
    for(int o = order; o > order/2; o -= 2){
        int n_min = (n_legs <= 8 || 2*o != 2+order) ? 4 : n_legs/2;
        for(int n = n_legs - 2; n >= n_min; n -= 2){
            for(Diagram& d : generate(o, n, singlets, false)){
                std::cout << "Extending " << d;
                
                auto d_ext 
                    = d.extend(valid_vertices(2 + order - o, 2 + n_legs - n), 
                        singlets && (o > 2) && (order > 4));
                diagrs.insert(diagrs.end(), d_ext.begin(), d_ext.end());
            }
        }
    }
    
    //Sorts and removes redundant diagrams.
    std::sort(diagrs.begin(), diagrs.end());
    std::vector<Diagram>::iterator last 
            = std::unique(diagrs.begin(), diagrs.end());
    diagrs.resize(std::distance(diagrs.begin(), last));
    
    //Removes identically zero diagrams
    if(remove_zero){
        auto nonzero = std::vector<Diagram>();
        for(Diagram& d : diagrs){
            if(!d.is_zero())
                nonzero.push_back(d);
        }
        return nonzero;
    }
    
    return diagrs;
}

/**
 * @brief Comparison operator for sorting diagram lists.
 * 
 * @param d1    a diagram.
 * @param d2    another diagram.
 * @return  @c true if @c d1 should come before @c d2 in a list.
 * 
 * The comparison is made as follows:
 * <ol>
 *  <li> Smaller diagrams before larger diagrams.
 *  <li> Lower-order diagrams before higher-order diagrams.
 *  <li> Simpler flavour splits before more complicated ones.
 *  <li> With everythin else equal, comparisons are made lexicographically
 *       on the labelings. This puts more symmetric diagrams before less 
 *       symmetric ones.
 * </ol>
 * The particular order of precedence is mainly for aesthetic reasons when 
 * displaying lists of diagrams.
 */
bool operator<(const Diagram& d1, const Diagram& d2){
    if(d1.n_legs != d2.n_legs)
        return d1.n_legs < d2.n_legs;
    if(d1.order != d2.order)
        return d1.order < d2.order;
    //Note reverse lexicographic ordering based on flavour splits --
    //we want unsplit diagrams first for aesthetical reasons, 
    //and single-index traces last for easy removal when needed.
    if(d1.flav_split != d2.flav_split)
        return d1.flav_split > d2.flav_split;
    
    return d1.labelings < d2.labelings;
}

/**
 * @brief Compares two diagrams for equality.
 * @param d1    a diagram.
 * @param d2    another diagram.
 * @return  @c true only if they have the same size, order, flavour split, and
 *          labelings.
 */
bool operator==(const Diagram& d1, const Diagram& d2){
    return (d1.n_legs == d2.n_legs) && (d1.order == d2.order)
            && (d1.flav_split == d2.flav_split) 
            && (d1.labelings == d2.labelings);
}

std::ostream& operator<<(std::ostream& out, const Diagram& d){
    out     << "O(p^" << d.order << ") " 
            << d.n_legs << "-point diagram"
            << ", flavour split " << d.flav_split
            << ", " << d.labelings.size() << " distinct labelings"
            << ":\n\t";
    
    d.labelings.front().print_header(out);
    for(Labeling lbl : d.labelings)
        out << "\n\t" << lbl;
    
    out << std::endl;
    
    return out;
}

void Diagram::find_flav_split(){
    flav_split.clear();
    
    root.find_flav_split(flav_split);
    std::sort(flav_split.begin(), flav_split.end());
    
    n_legs = std::accumulate(flav_split.begin(), flav_split.end(), 0);
    
    std::cout << "\t\tDetermined flavour split: " << flav_split << std::endl;
}

void Diagram::index(){
    
    std::list<std::pair<int, int>> flav_split_idcs 
        = std::list<std::pair<int, int>>();
    int idx = 0;
    for(int split : flav_split){
        flav_split_idcs.push_back(std::make_pair(split, idx));
//        std::cout << split << " -> " << idx << "\n";
        idx += split;
    }
    
    root.index(flav_split_idcs);
}

void Diagram::label(){
    labelings.clear();
    labelings.push_back(Labeling(root, n_legs));
        
    for(permute::ZR_Generator zr(flav_split); zr; ++zr)        
        labelings.push_back(Labeling(labelings.front(), *zr));
        
    std::sort(labelings.begin(), labelings.end());
    std::vector<Labeling>::iterator last 
            = std::unique(labelings.begin(), labelings.end());
    labelings.resize(std::distance(labelings.begin(), last));
}

std::vector<Diagram> Diagram::extend(
    const std::vector<vertex>& new_verts, bool singlets)
{
    
    //A vector of representatives taken from each equivalence class of indices
    //under Z_R. The representative is the smallest index in each trace that is
    //either the first trace (index 0) or larger than the preceding trace.
    auto idx_reps = std::vector<int>();
    idx_reps.push_back(0);
    for(int i = 1, idx = flav_split[0]; 
            i < flav_split.size(); 
            i++, idx += flav_split[i])
    {
        if(flav_split[i] != flav_split[i-1])
            idx_reps.push_back(idx);
    }
    
    //All locations in the diagram where, in some labeling, an index
    //representative occurs, are marked as distinct places to put a new vertex.
    auto rep_locs = std::unordered_set<int>();
    for(Labeling& lbl : labelings){
        auto idx_loc = lbl.index_locations();
        for(int rep : idx_reps)
            rep_locs.insert(idx_loc[rep]);
    }
    
    std::cout << "\tAttaching extension to legs " << rep_locs << std::endl;
    
    //Traverses the diagram and attaches all new vertices at all marked 
    //locations
    auto diagrs = std::vector<Diagram>();
    auto traversal = std::vector<std::pair<int,int>>();
    root.extend(diagrs, new_verts, rep_locs, traversal, *this, singlets);
        
    return diagrs;
}

void Diagram::attach(
    const vertex& new_vert,
    const std::vector<std::pair<int,int> >& where, 
    std::vector<Diagram>& diagrs, bool singlet)
const {
    for(int i = 0; i < new_vert.second.size(); i++){
        if(i > 0 && new_vert.second[i] == new_vert.second[i-1])
            continue;
        
        Diagram d(*this);
        d.order += new_vert.first - 2;
        
        std::cout   << "\tAttaching O(p^" << new_vert.first 
                    << ") vertex with flavour split " << new_vert.second
                    << " at location " << where << std::endl;
        d.root.attach(new_vert, i, where, 0, false);
        
        d.find_flav_split();
        d.index();
        d.label();
        
        diagrs.push_back(d);
        
        if(singlet && new_vert.second[i] > 2){
            Diagram s(*this);
            s.order += new_vert.first - 2;
        
            std::cout   << "\tSinglet-attaching O(p^" << new_vert.first 
                        << ") vertex with flavour split " << new_vert.second
                        << " at location " << where << std::endl;
            s.root.attach(new_vert, i, where, 0, true);
            
            s.find_flav_split();
            s.index();
            s.label();
            
            diagrs.push_back(s);
        }
    }
}

std::vector<std::vector<int>> Diagram::valid_flav_splits(
    int order, int n_legs, int smallest_split)
{
    std::vector<std::vector<int>> flav_splits 
        = std::vector<std::vector<int>>();
    flav_splits.push_back( {n_legs} );
    
    if(order == 2)
        return flav_splits;
       
    int step = order > 4 ? 1 : 2;
    for(int split = smallest_split; split <= n_legs/2; split += step){
        for(auto& flav_split : valid_flav_splits(
                order - 2*(1 + split%2), n_legs - split, split))
        {
            flav_split.push_back(split);
            flav_splits.push_back(flav_split);
        }
    }
    
    return flav_splits;
}

std::vector<vertex> Diagram::valid_vertices(int order, int n_legs){
    auto vertices = std::vector<vertex>();
    
    for(auto& flav_split : valid_flav_splits(order, n_legs))
        vertices.push_back(vertex(order, flav_split));
    
    return vertices;
}


