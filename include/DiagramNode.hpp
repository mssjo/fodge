/* 
 * File:   DiagramNode.hpp
 * Author: Mattias Sjo
 *
 * Created on 13 June 2019, 14:59
 */

#ifndef DIAGRAMNODE_H
#define	DIAGRAMNODE_H

#include <utility>
#include <unordered_set>

#include "fodge.hpp"
#include "Propagator.hpp"
#include "Point.hpp"


/**
 * @brief Represents a vertex or external leg in a diagram.
 * 
 * A @link Diagram @endlink is built up from these nodes, and
 * most of its functionality is recursively implemented by them.
 * 
 * The nodes form a tree with external legs as leaves. The parent
 * and children of a node represent vertices directly connected to
 * it by propagators, as well as the external legs connected to it.
 * 
 * The children of a node are grouped as "traces", with each trace
 * holding a set of subtrees that reside in a separate flavour trace.
 * One trace is marked as "connected", indicating that it is a
 * continuation of the trace that the node is part of rather than
 * starting a new flavour trace. The root does not have a connected
 * trace, and external legs have no traces at all.
 * 
 * A non-root node has responsibility over the propagator that connects
 * it to its parent, and stores a bitmask representing the momenta
 * flowing through the propagator towards the parent. It also knows its
 * order, number of legs, and whether its propagator is a singlet.
 */
class DiagramNode {
public:
    DiagramNode();
    DiagramNode(int order, const std::vector<int>& flav_split);
    DiagramNode(int order, const std::vector<int>& flav_split, 
        int split_idx, bool singlet);
    DiagramNode(const DiagramNode& other) = default;
    virtual ~DiagramNode() = default;
    
    bool is_zero();
    
    //Methods for determining properties of diagrams
    int find_flav_split(std::vector<int>& flav_split);
    int index(
        std::list<std::pair<int, int>>& flav_split_idcs, 
        int idx = -1);
    mmask set_momenta();
    void label(
        std::vector<Propagator>& props, int n_idcs, 
        int parent_order = 0, mmask parent_prev = 0) const;
    
    //Methods for making new diagrams
    void extend(
        std::vector<Diagram>& diagrs, const std::vector<vertex>& new_verts,
        std::unordered_set<int>& idcs, 
        std::vector<std::pair<int, int> >& traversal, 
        const Diagram& original, 
        bool singlet, bool debug);
    void attach(
        const vertex& new_vert, int split_idx,
        const std::vector<std::pair<int,int> >& where, int depth, 
        bool singlet, bool debug);
       
    
    //Methods for drawing diagrams (implemented in TikZ.cpp)
    bool def_TikZ(const std::vector<Point>& perimeter, int*idx,
        std::unordered_map<mmask, Point>& points, 
        mmask parent_key = 0) const;
    void adjust_TikZ(/*std::ostream& tikz,*/
        std::unordered_map<mmask, Point>& points,
        double radius, mmask parent_key = 0) const;
    Point draw_TikZ(std::ostream& tikz, 
        const std::unordered_map<mmask, Point> points, 
        mmask parent_key = 0) const;
    void vertex_order_TikZ(std::ostream& tikz, 
        const std::unordered_map<mmask, Point> points, 
        mmask parent_key = 0
    ) const;
    static void compress_points(
        std::unordered_map<mmask, Point>& points, const Point& ref,
        mmask key, mmask sub_key, bool incl_parent, 
        double mid_angle, double compression, double radius);
    static Point compress_point(const Point& ref,
        double angle, double mid_angle, double compression, double radius);
    
    //Methods for producing FORM output (implemented in FORM.cpp)
    void FORM(std::ostream& form, std::map<vertex, int>& verts, 
        int depth, const Propagator& prop) const;
    static void vertex_name_FORM(std::ostream& form, const vertex& vert, 
        int index, bool vertid);
    static void vertices_FORM(std::ostream& form, std::map<vertex, int>& verts);
    static bool heavy_vertex(const vertex& vert);
    
private:
    
    /** Marks the node as a leaf, i.e external leg. Most other members
     *  are meaningless for a leaf. */
    bool is_leaf;
    /** Marks the node as the root of the diagram tree. */
    bool is_root;
    /** Marks the node's propagator as a singlet. */
    bool is_singlet;
    
    /** The order of the vertex. */
    int order;
    /** The number of legs of the vertex. */
    int n_legs;
    
    /** Represents the momenta flowing from this node to its parent. 
     *  Has a single bit set in the case of an external leg. */
    mmask momenta;
    
    /**
     * @brief Represents a flavour trace holding a bundle of nodes. 
     * A node holds one or more flavour traces, and they in turn hold
     * its children.
     */
    class FlavourTrace{
    public:
        FlavourTrace(int n_legs = 0, bool connected = false);
        FlavourTrace(const FlavourTrace& other) = default;
        ~FlavourTrace() = default;
        
        /** The nodes that are children to the node through this trace. */
        std::vector<DiagramNode> legs;
        /** The number of flavour indices in the subtrees contained
         *  by this trace. */
        int n_idcs;
        /** Marks the trace as connected to the parent node. */
        bool connected;
        /** The momenta arriving at the node through the propagators
         *  in this trace. */
        mmask momenta;
    };
    
    /** The flavour traces held by the node. */
    std::vector<FlavourTrace> traces;
    /** The index of the connected flavour trace. 
     *  Is irrelevant for roots and leaves. */
    int connect_idx;
};

#endif	/* DIAGRAMNODE_H */

