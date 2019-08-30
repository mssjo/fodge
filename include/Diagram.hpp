/* 
 * File:   Diagram.hpp
 * Author: Mattias Sjo
 * 
 * Implemented in Diagram.cpp
 *
 * Created on 12 June 2019, 15:37
 */

#ifndef DIAGRAM_H
#define	DIAGRAM_H

#include "permute.hpp"

#include "mf_fodge.hpp"
#include "DiagramNode.hpp"
#include "Labeling.hpp"
#include "Point.hpp"

/**
 * @brief Describes flavour-ordered tree-level diagrams as trees.
 * 
 * The tree is built up from @link DiagramNode nodes @endlink. The tree stores
 * only the root node and some global information about the diagram; most of the
 * functionality is handled by the nodes.
 */
class Diagram {
public:
    Diagram();
    Diagram(int order, const std::vector<int>& flav_split);
    Diagram(const Diagram& orig) = default;
    virtual ~Diagram() = default;
    
    bool is_zero();
    
    static std::vector<Diagram> generate(int order, int n_legs, 
        bool singlets, bool traceless_generators = true);
    std::vector<Diagram> extend(
        const std::vector<vertex>& new_verts, bool singlets);
    void attach(const vertex& new_vert,
        const std::vector<std::pair<int, int> >& where,
        std::vector<Diagram>& diagrs, bool singlet) const;
    
    friend bool operator<(const Diagram& d1, const Diagram& d2);
    friend bool operator==(const Diagram& d1, const Diagram& d2);
    
    friend std::ostream& operator<<(std::ostream& out, const Diagram& d);
    
    void TikZ(std::ostream& tikz, double radius = 0, int idx = -1) const;
    void FORM(std::ostream& form, std::map<vertex, int>& verts) const;
    static void FORM(std::string filename, const std::vector<Diagram> diagrs);
    
private:
    friend class Labeling;
    
    /** The root node of the tree. */
    DiagramNode root;
    
    /** The total order (as in O(p^...) ) of the diagram. */
    int order;
    /** The total number of legs on the diagram. */
    int n_legs;
    /** The flavour split of the diagram. Represented as a sorted list of 
     *  integers, each representing the number of indices in a trace in the
     *  flavour structure. The integers must sum to @c n_legs . */
    std::vector<int> flav_split;
    
    /** All independent flavour-ordered labelings of the legs of the diagram. */
    std::vector<Labeling> labelings;
    
    void find_flav_split();
    void index();
    void label();
        
    static std::vector<Diagram> add_singlets(
            const std::vector<Diagram> diagrs, int order);
    
    static std::vector< std::vector<int> > valid_flav_splits(
        int order, int n_legs, int smallest_split = 2);
    static std::vector<vertex> valid_vertices(int order, int n_legs);
};

#endif	/* DIAGRAM_H */

