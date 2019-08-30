/* 
 * File:   DiagramNode.cpp
 * Author: Mattias
 * 
 * Created on 13 June 2019, 14:59
 */

#include <vector>

#include "DiagramNode.hpp"
#include "Labelling.hpp"
#include "Diagram.hpp"

/**
 * Constructs a flavour trace with a given number of legs.
 * 
 * @param n_legs    the number of legs.
 * @param connected @c true if this is connected to a parent FlavourTrace.
 */
DiagramNode::FlavourTrace::FlavourTrace(int n_legs, bool connected) 
: legs(n_legs, DiagramNode()), n_idcs(n_legs), momenta(0), connected(connected)
{}

/**
 * Default constructor: constructs a leaf node representing an external leg.
 */
DiagramNode::DiagramNode() 
: order(0), n_legs(0), momenta(0), 
        is_leaf(true), is_root(false), is_singlet(false),
        traces(), connect_idx(-1)
{}

/**
 * Constructs a root node representing a single-vertex diagram.
 * @param order         the order of the vertex.
 * @param flav_split    the flavour split, must be sorted.
 */
DiagramNode::DiagramNode(int order, const std::vector<int>& flav_split) 
: order(order), momenta(0), n_legs(0), 
        is_leaf(false), is_root(true), is_singlet(false), 
        traces(), connect_idx(-1)
{    
    for(int fsp : flav_split){
        traces.push_back(FlavourTrace(fsp, false));
        n_legs += fsp;
    }
}

/**
 * Constructs a non-root non-leaf node representing a vertex in a diagram.
 * @param order         the order of the vertex.
 * @param flav_split    the flavour split, must be sorted.
 * @param split_idx     the index of the part of the split that is connected
 *                      to the parent node.
 * @param singlet       @c true to mark the propagator connecting this node to
 *                      its parent as a singlet propagator.
 */
DiagramNode::DiagramNode(
        int order, const std::vector<int>& flav_split, 
        int split_idx, bool singlet)
: order(order), momenta(0), n_legs(0),
        is_leaf(false), is_root(false), is_singlet(singlet),
        traces(), connect_idx(split_idx)
{
        
    int split_size;
    for(int i = 0; i < flav_split.size(); i++){
        split_size = flav_split[i] - (i == split_idx ? 1 : 0);
        
        traces.push_back(FlavourTrace(split_size, i == split_idx));
        n_legs += split_size;
    }
}

bool DiagramNode::is_zero(){
    if(is_leaf)
        return false;
    
    if(traces[0].legs.size() == 1 
            && (is_singlet != traces[0].legs[0].is_singlet))
        return true;
        
    for(FlavourTrace tr : traces){
        if(tr.legs.size() == 2 
                && (tr.legs[0].is_singlet != tr.legs[1].is_singlet))
            return true;
        
        for(DiagramNode leg : tr.legs){
            if(leg.is_zero())
                return true;
        }
    }
        
    return false;
}
    
/**
 * @brief Recursively determines the flavour split of a diagram, and sets all
 * @c n_idcs members to the correct value.
 * 
 * This method traverses the diagram and counts the number of child legs
 * of each node. Whenever the root of a flavour trace (or of the entire diagram)
 * is found, the number of legs is pushed to the flavour split.
 * 
 * @param flav_split    the (unsorted) flavour split that is built up by this
 *                      method.
 * @return  the number of legs that are children of this node and that are in
 *          the same flavour trace as its parent. A leaf returns 1. The return
 *          value of the root is nonsensical, but once the root returns, 
 *          @c flav_split contains the correct flavour split. 
 */
int DiagramNode::find_flav_split(std::vector<int>& flav_split){
    if(is_leaf)
        return 1;
        
    int sum;
    int con_sum;
    for(FlavourTrace& tr : traces){
        sum = 0;
        for(DiagramNode& leg : tr.legs){
            if(leg.is_singlet){
                int singlet_sum = leg.find_flav_split(flav_split);
                if(singlet_sum > 0)
                    flav_split.push_back(singlet_sum);
            }
            else
                sum += leg.find_flav_split(flav_split);
        }
        
        tr.n_idcs = sum;
        
        if(tr.connected)
            con_sum = sum;
        else if(sum > 0)
            flav_split.push_back(sum);
    }
    
    return con_sum;
}

/**
 * @brief Recursively indexes the external legs of a diagram.
 *  
 * @param flav_split_idcs   maps the size of a flavour split to the index at
 *                          which its legs should start.  
 * @param fsp_idx
 */
int DiagramNode::index(
    std::list<std::pair<int, int>>& flav_split_idcs, int idx)
{
    if(is_leaf){        
        momenta = ((mmask) 1) << idx;
        return idx + 1;
    }
    
    int sub_idx = -1;
    for(FlavourTrace& tr : traces){
        
        //Finds the flavour index for each trace, except the connected one, 
        //which inherits the parent's index. (For the root, there is no 
        //connected trace, and singlet-connected traces don't inherit.)
        //Skip zero-index "traces" (vertices with only singlet legs) entriely.
        if((!tr.connected || is_singlet) && tr.n_idcs > 0){
            for(auto it = flav_split_idcs.begin();; ++it){
                if((*it).first == tr.n_idcs){
                    sub_idx = (*it).second;
                    flav_split_idcs.erase(it);
                    break;
                }
                
                if(it == flav_split_idcs.end()){
                    std::cout << "\t\tn_idcs = " << tr.n_idcs << std::endl;
                    assert(it != flav_split_idcs.end());
                }
            }
        }
        else
            sub_idx = idx;
        
        assert(sub_idx >= 0 || tr.n_idcs == 0);
        
        for(DiagramNode& leg : tr.legs){
            if(leg.is_singlet)
                leg.index(flav_split_idcs);
            else
                sub_idx = leg.index(flav_split_idcs, sub_idx);
            
            if(tr.connected)
                idx = sub_idx;
        }
    }
    
    return idx;
}

mmask DiagramNode::set_momenta(){
    if(is_leaf)
        return momenta;
    
    momenta = 0;
    for(FlavourTrace& tr : traces){
        tr.momenta = 0;
        for(DiagramNode& leg : tr.legs)
            tr.momenta |= leg.set_momenta();
        
        momenta |= tr.momenta;
    }
    
    return momenta;
}

void DiagramNode::label(std::vector<Propagator>& props, int n_idcs, 
        int parent_order, mmask parent_prev) const
{    
    if(is_leaf)
        return;
    
    for(const FlavourTrace& tr : traces){
        mmask prev;
        if(tr.connected)
            //If prev is propagator leading to this node, then we must
            //invert it to make it ingoing!
            prev = ((((mmask) 1) << n_idcs) - 1) ^ momenta;
        else
            prev = tr.legs.back().momenta;
        
        for(const DiagramNode& leg : tr.legs){
            leg.label(props, n_idcs, order, prev);
            prev = leg.momenta;
        }
    }
    
    if(!is_root && !is_singlet){
        props.push_back(Propagator(momenta, n_idcs, 
                order, 
                parent_order));
    }
    else if(is_singlet){
        mmask prev = traces[connect_idx].legs.back().momenta;
        props.push_back(Propagator(momenta, n_idcs,
                order, prev, 
                parent_order, parent_prev));
    }
}


void DiagramNode::extend(
    std::vector<Diagram>& diagrs, const std::vector<vertex>& new_verts, 
    std::unordered_set<int>& idcs, std::vector<std::pair<int, int> >& traversal, 
    const Diagram& original, bool singlet)
{
    if(is_leaf){
        int index = bitwise::unshift(momenta);
        
        if(idcs.find(index) == idcs.end())
            return;

        for(vertex v : new_verts){           
            original.attach(v, traversal, diagrs, 
                    singlet && (v.first > 2));
        }
        
        return;
    }
    
    traversal.push_back(std::make_pair(0,0));
    
    for(FlavourTrace& tr : traces){
        for(DiagramNode& leg : tr.legs){
            leg.extend(diagrs, new_verts, idcs, traversal, original, 
                    singlet && (!leg.is_leaf || order > 2)
            );
            traversal.back().second++;
        }
        traversal.back().first++;
        traversal.back().second = 0;
    }
    
    traversal.pop_back();
}

void DiagramNode::attach(
    const vertex& new_vert, int split_idx, 
    const std::vector<std::pair<int,int> >& where, int depth, bool singlet)
{
    auto wd = where[depth];
    if(depth < where.size() - 1){
        traces[wd.first].legs[wd.second]
            .attach(new_vert, split_idx, where, depth+1, singlet);
    }
    else{
        traces[wd.first].legs[wd.second] 
            = DiagramNode(new_vert.first, new_vert.second, split_idx, singlet);
    }
}