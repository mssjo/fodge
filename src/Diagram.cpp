/* 
 * File:   Diagram.cpp
 * Author: Mattias Sjo
 * 
 * Implements Diagram.h
 * 
 * Created on 12 June 2019, 15:37
 */

#include "Diagram.hpp"

#include <sstream>

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
 * these are instead created form other diagrams via the 
 * @link Diagram::extend @endlink method.
 */
Diagram::Diagram(int order, const std::vector<int>& fsplit)
: order(order), flav_split(fsplit), singlet_diagram(false), root(order, fsplit), labellings(),
        n_legs(std::accumulate(fsplit.begin(), fsplit.end(), 0))
{
    std::sort(flav_split.begin(), flav_split.end());
    index();
    labellings.push_back(Labelling(root, n_legs));
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
 * @param traceless_generators 
 *                      whether to remove diagrams that are identically zero 
 *                      due to traceless generators --
 *                      should normally only be @c false when the method calls
 *                      itself.
 * @param debug         enables debug printouts.
 * @return  a sorted vector containing the diagrams.
 * 
 * This is the main method for creating diagrams. It works by generating all
 * single-vertex diagrams of the given order and size, and then recursively
 * generating smaller and lower-order diagrams and extending them to the target
 * size and order. Finally, redundant diagrams are trimmed and the list of
 * diagrams is sorted.
 */
std::vector< Diagram > Diagram::generate ( int order, int n_legs, 
                        bool singlets, bool traceless_generators, bool debug )
{
    
    auto diagrs = std::vector<Diagram>();
    //Generates single-vertex diagrams to seed the recursion.
    for(auto& flav_split : valid_flav_splits(order, n_legs)){
        
        if(debug){
            std::cout << "Generating diagram with flavour split " 
                    << flav_split << std::endl;
        }
        
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
            for(Diagram& d : generate(o, n, singlets, false, debug)){
                if(debug)
                    std::cout << "Extending " << d;
                
                auto d_ext 
                    = d.extend(valid_vertices(2 + order - o, 2 + n_legs - n), 
                        singlets && (o > 2) && (order > 4), debug);
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
    if( traceless_generators ){
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
    
    return d1.labellings < d2.labellings;
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
            && (d1.labellings == d2.labellings );
}


/**
 * @brief Left-shift print operator for diagrams.
 * 
 * @param out the stream to which the diagram is printed.
 * @param d   the diagram to print.
 * @return the stream.
 * 
 * The diagram is presented as 
 * <tt> O(p^<i>m</i>) <i>n</i>-point diagram, flavour split <i>fsp</i>, <i>k</i> distinct labellings </tt>,
 * with values inserted to represent the diagram. A list of labellings then follows.
 */
std::ostream& operator<<(std::ostream& out, const Diagram& d){
    out     << "O(p^" << d.order << ") " 
            << d.n_legs << "-point diagram"
            << ", flavour split " << d.flav_split
            << ", " << d.labellings.size() << " distinct labellings"
            << ":\n\t";
    
    d.labellings.front().print_header(out);
    for(Labelling lbl : d.labellings )
        out << "\n\t" << lbl;
    
    out << std::endl;
    
    return out;
}

/**
 * @brief Makes a table summarising a list of diagrams.
 * 
 * @param out the stream to which the table is printed.
 * @param diagrs the diagrams.
 * 
 * The table lists the number of diagrams per flavour structure,
 * and the number of singlet diagrams if such are present.
 */
void Diagram::summarise(std::ostream& out, const std::vector<Diagram>& diagrs)
{
    if(diagrs.empty())
        return;
    
    std::map<std::vector<int>, std::pair<size_t, size_t> > counts = {};
        
    bool any_singlets = false;
    size_t max_count = 0, max_fsp_len = 0;
    
    //Counts the number of diagrams in (non-singlet, singlet) pairs
    //mapped over flavour structures
    //Also keeps track of maximum lengths for formatting purposes
    auto flav_split = diagrs.front().flav_split;
    auto count = std::make_pair(0, 0);
    for(size_t i = 0;; i++){
        if(i >= diagrs.size() || diagrs[i].flav_split != flav_split){
            size_t fsp_len = flav_split.size() + 3;
            for(int r : flav_split)
                fsp_len += std::to_string(r).length();
            if(fsp_len > max_fsp_len)
                max_fsp_len = fsp_len;
            
            counts.insert({flav_split, count});
            
            if(i < diagrs.size()){
                flav_split = diagrs[i].flav_split;
                count = std::make_pair(0, 0);
            }
            else
                break;
        }
        
        if(diagrs[i].singlet_diagram){
            count.second++;
            any_singlets = true;
        }
        else{
            count.first++;
            if(count.first > max_count)
                max_count = count.first;
        }
    }
    
    std::string col1("Flavour split"), col2("Diagrams"), col3("Singlets");
    size_t w1 = std::max(col1.length(), max_fsp_len);
    size_t w2 = std::max(col2.length(), std::to_string(max_count).length());
    
    //Horizontal line in the table -- three are needed so we make a macro
#define TABLE_HLINE     out << std::string(w1, '-') << "-+-" << std::string(w2, '-'); \
                        if(any_singlets) out << "-+-" << std::string(w2, '-');        \
                        out << "-\n";
                        
    TABLE_HLINE
    
    out << std::setw(w1) << col1 << " | " 
        << std::setw(w2) << col2;
    if(any_singlets)
        out << " | " << col3;
    out << "\n";
    
    TABLE_HLINE
    
    for(auto& key_val : counts){
        std::ostringstream fsp_str;
        fsp_str << key_val.first;        
        
        out << std::setw(w1) << fsp_str.str() << std::setw(0) << " | "
            << std::setw(w2) << key_val.second.first + key_val.second.second << std::setw(0);
        if(any_singlets)
            out << " | " << std::setw(w2) << key_val.second.second << std::setw(0);
        out << "\n";
    }
    
    TABLE_HLINE
}


/**
 * @brief Determines the flavour split of a diagram.
 * 
 * This method is necessary for a diagram to know its own
 * number of legs and flavour split after being generated.
 */
void Diagram::find_flav_split(){
    flav_split.clear();
    
    root.find_flav_split(flav_split);
    std::sort(flav_split.begin(), flav_split.end());
    
    n_legs = std::accumulate(flav_split.begin(), flav_split.end(), 0);
}

/**
 * @brief Places flavour indices on the legs of a diagram.
 * 
 * The indices will be placed in an arbitrary flavour-ordered
 * way. This is necessary for @link Diagram::label @endlink
 * to work properly, and requires @link Diagram::find_flav_split @endlink
 * in turn.
 */
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

/**
 * @brief Generates all distinct labellings on a diagram.
 * 
 * After calling this method, a diagram is complete. It
 * requires @link Diagram::index @endlink to work correctly.
 */
void Diagram::label(){
    labellings.clear();
    labellings.push_back(Labelling(root, n_legs));
        
    for(permute::ZR_Generator zr(flav_split); zr; ++zr)        
        labellings.push_back(Labelling( labellings.front(), *zr));
        
    std::sort( labellings.begin(), labellings.end());
    std::vector<Labelling>::iterator last 
            = std::unique( labellings.begin(), labellings.end());
    labellings.resize(std::distance( labellings.begin(), last));
}

/**
 * @brief Extends a diagram by attaching vertices to its external legs.
 * 
 * @param new_verts a list of vertices to be attached.
 * @param singlets enables singlet propagators.
 * @param debug enables debug messages.
 * @return a vector containing diagrams representing all ways to attach 
 * the new vertices to legs of the diagram.
 * 
 * This method is central to the diagram generation process. In order to
 * reduce the number of redundant diagrams, only legs that, in some
 * distinct labelling of the diagram, carry a label that is a coset
 * representative under @f$  Z_R,@f$ are extended. The generated
 * diagrams are completely set up and labelled.
 */
std::vector<Diagram> Diagram::extend(
    const std::vector<vertex>& new_verts, bool singlets, bool debug)
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
    for(Labelling& lbl : labellings ){
        auto idx_loc = lbl.index_locations();
        for(int rep : idx_reps)
            rep_locs.insert(idx_loc[rep]);
    }
    
    if(debug)
        std::cout << "\tAttaching extension to legs " << rep_locs << std::endl;
    
    //Traverses the diagram and attaches all new vertices at all marked 
    //locations
    auto diagrs = std::vector<Diagram>();
    auto traversal = std::vector<std::pair<int,int>>();
    root.extend(diagrs, new_verts, rep_locs, traversal, *this, singlets, debug);
        
    return diagrs;
}

/**
 * @brief Attaches a vertex to a leg of a diagram in all distinct ways.
 * 
 * @param new_vert the vertex to be attached.
 * @param where the location in the diagram of the leg. It is given as a
 * vector of (trace, index) pairs specifying a traversal down the tree of nodes.
 * @param diagrs the list of diagrams to which the new diagrams are added.
 * @param singlet enables attaching the leg via a singlet propagator.
 * @param debug enables debug printouts.
 *
 * This method serves as an auxiliary to @link Diagram::attach @endlink.
 * Several diagrams are generated: different choices of vertex leg to attach,
 * and singlet/ordinary propagator. The generated diagrams are completely set
 * up and labelled.
 */
void Diagram::attach(
    const vertex& new_vert,
    const std::vector<std::pair<int,int> >& where, 
    std::vector<Diagram>& diagrs, 
    bool singlet, bool debug)
const {
    for(int i = 0; i < new_vert.second.size(); i++){
        if(i > 0 && new_vert.second[i] == new_vert.second[i-1])
            continue;
        
        Diagram d(*this);
        d.order += new_vert.first - 2;
        
        if(debug){
            std::cout   << "\tAttaching O(p^" << new_vert.first 
                        << ") vertex with flavour split " << new_vert.second
                        << " at location " << where << std::endl;
        }
        d.root.attach(new_vert, i, where, 0, false, debug);
        d.singlet_diagram = this->singlet_diagram;
        
        d.find_flav_split();
        d.index();
        d.label();
        
        
        diagrs.push_back(d);
        
        if(singlet && new_vert.second[i] > 2){
            Diagram s(*this);
            s.order += new_vert.first - 2;
        
            if(debug){
                std::cout   << "\tSinglet-attaching O(p^" << new_vert.first 
                            << ") vertex with flavour split " << new_vert.second
                            << " at location " << where << std::endl;
            }
            s.root.attach(new_vert, i, where, 0, true, debug);
            s.singlet_diagram = true;
            
            s.find_flav_split();
            s.index();
            s.label();
            
            diagrs.push_back(s);
        }
    }
}

/**
 * @brief Filters a list of diagrams based on their flavour structure.
 * 
 * @param diagrs    the diagrams, all of which should have the same number of legs. 
 * @param filter    a vector of flavour splits: ascendingly sorted vectors of 
 *                  integers larger than 1 that sum to the number of legs on the
 *                  diagrams.
 * @param include   if @c true, all diagrams that match the filter are kept, and
 *                  all others are discarded. If @c false, the opposite happens.
 * @return the number of diagrams removed. 
 */
size_t Diagram::filter_flav_split(std::vector<Diagram>& diagrs, 
                                    const std::vector<std::vector<int> >& filter, 
                                    bool include)
{
    auto tmp = std::vector<Diagram>();
    size_t init_size = diagrs.size();
    
    for(Diagram& d : diagrs){
        for(const std::vector<int>& flav_split : filter){
            if(flav_split == d.flav_split){
                if(include)            
                    tmp.push_back(d);
                
                break;
            }
        }
        if(!include)
            tmp.push_back(d);
    }
    
    diagrs.clear();
    diagrs.insert(diagrs.cend(), tmp.begin(), tmp.end());
    
    return init_size - diagrs.size();
}


/**
 * @brief Generates a list of all valid flavour splits of a given vertex.
 * 
 * @param order the order of the vertex.
 * @param n_legs the number of legs on the vertex.
 * @param smallest_split the smallest split to be allowed. Should only be used
 * when the method calls itself recursively.
 * @return a vector of flavour splits, i.e. sorted vectors of integers that
 * add up to @p n_legs.
 * 
 * The splittings are generated recursively: for each possible first entry
 * in the split, all valid splits of the remainder are generated. To keep
 * the splits sorted, @p smallest_split is used to ensure that no integer
 * is smaller than one preceding it.
 * 
 * The rules for flavour splits, as read from the NLSM Lagrangian, is that
 * each additional entry in a split "costs" @f$ \\mathcal O(p^2)@f$, and each 
 * pair of odd splits cost an additional @f$ \\mathcal O(p^2)@f$.
 */
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
        //Extra cost for odd split is only deducted when n_legs is even,
        //to ensure that it is only deducted once for each pair.
        for(auto& flav_split : valid_flav_splits(
                order - (split%2 && !(n_legs%2) ? 4 : 2), n_legs - split, split))
        {
            flav_split.push_back(split);
            flav_splits.push_back(flav_split);
        }
    }
    
    return flav_splits;
}

/**
 * @brief Generates all valid vertices of a given order and size.
 * 
 * @param order the order of the vertices.
 * @param n_legs the number of legs on the vertices.
 * @return a vector containing all valid vertices according to the rules
 * described in @link Diagram::valid_flav_splits @endlink.
 */
std::vector<vertex> Diagram::valid_vertices(int order, int n_legs){
    auto vertices = std::vector<vertex>();
    
    for(auto& flav_split : valid_flav_splits(order, n_legs))
        vertices.push_back(vertex(order, flav_split));
    
    return vertices;
}


