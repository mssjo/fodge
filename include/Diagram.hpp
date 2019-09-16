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

#include "fodge.hpp"
#include "DiagramNode.hpp"
#include "Labelling.hpp"
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
                                         bool singlets, bool traceless_generators = true, 
                                         bool debug = false);
    std::vector<Diagram> extend(const std::vector<vertex>& new_verts, 
                                bool singlets, bool debug);
    void attach(const vertex& new_vert,
                const std::vector<std::pair<int, int> >& where,
                std::vector<Diagram>& diagrs, bool singlet, bool debug) const;
    
    friend bool operator<(const Diagram& d1, const Diagram& d2);
    friend bool operator==(const Diagram& d1, const Diagram& d2);
    
   static size_t filter_flav_split(std::vector<Diagram>& diagrs, 
                                  const std::vector<std::vector<int>>& filter, 
                                  bool include);
    
    friend std::ostream& operator<<(std::ostream& out, const Diagram& d);
    static void summarise(std::ostream& out, const std::vector<Diagram>& diagrs);
    
    void TikZ(std::ostream& tikz, double radius = 0, int idx = -1, 
              bool draw_circle = false) const;
    static int TikZ(const std::string& filename, const std::vector<Diagram>& diagrs,
                    int split, double radius, bool draw_circle);
    static void balance_points(std::unordered_map<mmask, Point>& pts);
    
    void FORM(std::ostream& form, std::map<vertex, int>& verts, int index) const;
    void diagram_name_FORM(std::ostream& form, int index) const;
    static int FORM(const std::string& filename, const std::vector<Diagram>& diagrs);
    
private:
    friend class Labelling;
        
    /** The total order (as in O(p^...) ) of the diagram. */
    int order;
    /** The total number of legs on the diagram. */
    int n_legs;
    /** The flavour split of the diagram. Represented as a sorted list of 
     *  integers, each representing the number of indices in a trace in the
     *  flavour structure. The integers must sum to @c n_legs . */
    std::vector<int> flav_split;
    bool singlet_diagram;
    
    /** The root node of the tree. */
    DiagramNode root;
    
    /** All independent flavour-ordered labelings of the legs of the diagram. */
    std::vector<Labelling> labellings;
    
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

