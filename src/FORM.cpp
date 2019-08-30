
#include "Diagram.h"
#include "DiagramNode.h"

#define INDENT_SIZE 4

void Diagram::FORM(std::ostream& form, std::map<vertex,int>& verts) 
const {
    auto local_verts = std::map<vertex, int>();
    
    root.FORM(form, local_verts, 0, Propagator(0, n_legs, 0, 0));
    
    for(auto vert_count : local_verts){
        auto global_count = verts.find(vert_count.first);
        if(global_count == verts.end())
            verts.insert(vert_count);
        else
            (*global_count).second += vert_count.second;
    }
}

void DiagramNode::FORM(
        std::ostream& form, std::map<vertex, int>& verts, 
        int depth, const Propagator& prop) 
const {
    
    if(is_leaf){
        form << bitwise::unshift(momenta);
        return;
    }
    
    auto flav_split = std::vector<int>();
    for(const auto tr : traces)
        flav_split.push_back(tr.legs.size() + (tr.connected ? 1 : 0));
    assert(std::is_sorted(flav_split.begin(), flav_split.end()));    
    
    vertex vert = std::make_pair(order, flav_split);
    auto vert_count = verts.find(vert);
    int vert_idx;
    if(vert_count == verts.end()){
        verts.insert(std::make_pair(vert, 1));
        vert_idx = 1;
    }
    else
        vert_idx = ++((*vert_count).second);
    
    form << std::string(depth*INDENT_SIZE, ' ') << "diagram(";
    vertex_name_FORM(form, vert, vert_idx);
    
    for(const auto tr : traces){
        for(const auto leg : tr.legs){
            form << (leg.is_leaf ? ", " : ",\n");
            leg.FORM(form, verts, depth+1, prop);
        }
        if(tr.connected){
            form << (is_singlet ? "singlet(" : "prop(");
            prop.FORM(form, momenta);
        }
    }
    form << ")\n";
}

void DiagramNode::vertex_name_FORM(std::ostream& form, vertex vert, int index){
    form << "[V" << vert.second[0];
    for(int i = 1; i < vert.second.size() - 1; i++)
        form << "/" << vert.second[i];
    form << "p" << vert.first << "." << index << "]";
}

void Propagator::FORM(std::ostream& form, mmask prop) const {    
    mmask one = (mmask) 1;
    prop = normalise_mmask(prop, one << (n_mom - 1), (one << n_mom) - 1);
    
    bool first = true;
    for(int i = 0; prop < (one << i) && i < n_mom; i++){
        if(prop & (one << i)){
            if(!first)
                form << "+";
            else
                first = false;
            
            form << "p" << (i+1);
        }
    }
}
