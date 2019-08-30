/* 
 * File:   DiagramNode.hpp
 * Author: Mattias
 *
 * Created on 13 June 2019, 14:59
 */

#ifndef DIAGRAMNODE_H
#define	DIAGRAMNODE_H

#include <utility>
#include <unordered_set>

#include "mf_fodge.hpp"
#include "Propagator.hpp"
#include "Point.hpp"

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
        const Diagram& original, bool singlet);
    void attach(
        const vertex& new_vert, int split_idx,
        const std::vector<std::pair<int,int> >& where, int depth, bool singlet);
       
    
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
    
    void FORM(std::ostream& form, std::map<vertex, int>& verts, 
        int depth, const Propagator& prop) const;
    static void vertex_name_FORM(std::ostream& form, vertex vert, 
        int index);
                
private:
    int order;
    int n_legs;
    
    mmask momenta;
    
    bool is_leaf;
    bool is_root;
    bool is_singlet;
    
    class FlavourTrace{
    public:
        FlavourTrace(int n_legs = 0, bool connected = false);
        FlavourTrace(const FlavourTrace& other) = default;
        ~FlavourTrace() = default;
        
        std::vector<DiagramNode> legs;
        int n_idcs;
        bool connected;
        mmask momenta;
    };
    
    int connect_idx;
    std::vector<FlavourTrace> traces;
};

#endif	/* DIAGRAMNODE_H */

