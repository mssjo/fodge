
#include "fodge.hpp"
#include "fsp_map.hpp"

diagram* _make_blank_diagram(size_t ngons, size_t order){
    diagram* diagr = salloc(sizeof(diagram));
    diagr->ngons = ngons;
    diagr->order = order; 
    diagr->sym = 1;
    
    diagr->gons    = salloc(ngons * sizeof(gon_t));
    diagr->gon_idx = salloc(ngons * sizeof(size_t));
    diagr->edges   = salloc(ngons * sizeof(size_t));
    
    diagr->rep  = NULL;
    diagr->next = NULL;
    
    diagr->diagram_id = DIAGRAM_ID;
    
    return diagr;
}

diagram* make_contact_diagram(size_t ngons, size_t order){
    
    diagram* diagr = _make_blank_diagram(ngons, order);
    
    for(size_t i = 0; i < ngons; i++){
        diagr->gons   [i] = i;
        diagr->gon_idx[i] = i;
        diagr->edges  [i] = 0;
    }
    
    diagr->npolys = 1;
    diagr->polys = salloc(1 * sizeof(polygon*));
    diagr->polys[0] = make_polygon(diagr->gons, NULL, ngons, order);
    
    diagr->rep = represent_diagram(diagr);
    diagr->sym = get_symmetry(diagr);
/*
    printf("Created O(p^%d) %d-point contact diagram.\n", 2*(order+1), ngons);
*/
    return diagr;
}

diagram* copy_diagram(const diagram* orig, int recursive){
    if(!orig)
        return NULL;
    
    diagram* copy = _make_blank_diagram(orig->ngons, orig->order);
    
    memcpy(copy->gons,    orig->gons,    orig->ngons * sizeof(gon_t));
    memcpy(copy->gon_idx, orig->gon_idx, orig->ngons * sizeof(size_t));
    memcpy(copy->edges,   orig->edges,   orig->ngons * sizeof(size_t));
    
    copy->npolys = orig->npolys;
    copy->polys = copy_polygons(orig->polys, NULL, copy->npolys);
    
    if(recursive)
        copy->next = copy_diagram(orig->next, TRUE);
    else
        copy->next = NULL;
    
    return copy;
}

diagram* _cut_edge(const diagram* base, gon_t edge, size_t ngons, size_t order){
    diagram* cut = _make_blank_diagram(base->ngons+ngons, base->order+order);
       
    gon_t*    newpoly  = salloc((ngons + 2) * sizeof(gon_t));
    poly_ptr* newedges = salloc((ngons + 2) * sizeof(poly_ptr));
    for(size_t i = 0; i < ngons; i++){
        cut->gons [edge + i + 1] = base->ngons + i;
        cut->edges[edge + i + 1] = base->npolys;
        newpoly [i + 1] = base->ngons + i;
        newedges[i + 1].type = EXT_LEG;
    }
    cut->edges[edge] = base->npolys;
    newpoly [0] = base->gons[edge];
    newedges[0].type = EXT_LEG;
    newpoly [ngons + 1] = base->gons[(edge + 1) % base->ngons];
    newedges[ngons + 1].type = PROPGTR;
    newedges[ngons + 1].idx  = base->edges[edge];
    
    memcpy(cut->gons, base->gons, (edge + 1) * sizeof(gon_t));
    memcpy(cut->gons + edge+1 + ngons, base->gons + ((edge+1) % base->ngons), 
            (base->ngons - (edge + 1)) * sizeof(gon_t));
    memcpy(cut->edges, base->edges, edge * sizeof(size_t));
    memcpy(cut->edges + edge+1 + ngons, base->edges + ((edge+1) % base->ngons),
            (base->ngons - (edge + 1)) * sizeof(size_t));
        

/* DEBUG STUFF */
/*
    for(idx_t i = 0; i < ngons+2; i++){
        printf("%d    ", newpoly[i]);
    }
    printf("(newpoly)\n");
    for(idx_t i = 0; i < ngons+2; i++){
        printf("%d(%d) ", newedges[i].idx, newedges[i].type);
    }
    printf("(newedges)\n");
    for(idx_t i = 0; i < cut->ngons; i++){
        printf("%d ", cut->gons[i]);
    }
    printf("(cut diagram)\n");
    for(idx_t i = 0; i < cut->ngons; i++){
        printf("%d ", cut->edges[i]);
    }
    printf("(cut edges)\n");
*/

    
    for(size_t i = 0; i < cut->ngons; i++)
        cut->gon_idx[cut->gons[i]] = i;
    
    cut->npolys = base->npolys + 1;
    cut->polys = copy_polygons(base->polys, 
            make_polygon(newpoly, newedges, ngons+2, order), base->npolys);
    free(newpoly);
    free(newedges);
    
    polygon* cutpoly = cut->polys[base->edges[edge]];
    for(size_t i = 0; /*i < cutpoly->ngons*/; i++){
        if(cutpoly->gons[i] == cut->gons[edge]){
            cutpoly->edges[i].type = PROPGTR;
            cutpoly->edges[i].idx  = base->npolys;
            break;
        }
    }
        
    cut->rep = represent_diagram(cut);
    cut->sym = get_symmetry(cut);
        
    return cut;
}

void _decrement_split_budget(const diagram* diagr, polygon* poly, 
        const polygon* src, size_t decr){
    if(decr > poly->split_budget){
        printf("\nERROR: exceeding split budget in polygon!");
        exit(EXIT_FAILURE);
    }
    
    poly->split_budget -= decr;
    
    polygon* dest;
    for(size_t g_idx = 0; g_idx < poly->ngons; g_idx++){
        if(poly->edges[g_idx].type == FLSPLIT){
            dest = diagr->polys[poly->edges[g_idx].idx];
            if(dest == src)
                continue;
            
            _decrement_split_budget(diagr, dest, poly, decr);
        }
    }    
}

diagram* _split_poly(const diagram* base, size_t p_idx){
    polygon* poly = base->polys[p_idx];
    if(!poly->split_budget || poly->ngons < 4)
        return NULL;
        
    polygon* left;
    polygon* right;
    
    diagram* root = NULL;
    diagram* split;
    
    size_t l_degen, r_degen;
    
    for(size_t i = 0; i < poly->ngons/2; i++){
        for(size_t j = i+2; j < poly->ngons; j++){
            //We can only do odd splits with a budget of at least 2, and
            //without loss of generality, we can make our lives simpler by only
            //doing odd splits when the budget is exactly 2.
            if((j - i) % 2 && poly->split_budget != 2)
                continue;
            //This catches a fringe case where we would create a 1-gon.
            if(j == poly->ngons - 1 && i == 0)
                continue;
            
            l_degen = 0;
            r_degen = 0;
            for(size_t k = i; k < j; k++){
                if(CARRIES_FLAVIDX(poly->edges[k]))
                    l_degen++;
            }
            for(size_t k = j; k < i + poly->ngons; k++){
                if(CARRIES_FLAVIDX(poly->edges[k % poly->ngons]))
                    r_degen++;
            }
            
            if(l_degen < 2 || r_degen < 2)
                continue;
                                    
            left  = make_polygon(NULL, NULL, 
                                  (j-i) + 1, poly->order);
            right = make_polygon(NULL, NULL, 
                    poly->ngons - (j-i) + 1, poly->order);
            
            split = _make_blank_diagram(base->ngons, base->order);
            memcpy(split->gons   , base->gons   , base->ngons * sizeof(gon_t));
            memcpy(split->gon_idx, base->gon_idx, base->ngons * sizeof(size_t));
            memcpy(split->edges  , base->edges  , base->ngons * sizeof(size_t));
            
            split->npolys = base->npolys + 1;
            split->polys = salloc(split->npolys * sizeof(polygon*));
            for(size_t k = 0; k < base->npolys; k++){
                if(k == p_idx)
                    continue;
                
                split->polys[k] = copy_polygon(base->polys[k]);
            }
            
            split->polys[p_idx] = left;
            split->polys[base->npolys] = right;
            
            for(size_t k = 0, ki; k < left->ngons - 1; k++){
                ki = (k+i) % poly->ngons;
                left->gons[k] = poly->gons[ki];
                
                left->edges[k].type = poly->edges[ki].type;
                left->edges[k].idx  = poly->edges[ki].idx;
            }
            left->gons [left->ngons-1] = poly->gons[j];
            left->edges[left->ngons-1].type = FLSPLIT;
            left->edges[left->ngons-1].idx  = base->npolys;
            //left->split_budget = poly->split_budget - ((j-i)%2 ? 2:1);
            
            for(size_t k = 0, kj; k < right->ngons - 1; k++){
                kj = (k+j) % poly->ngons;
                right->gons[k] = poly->gons[kj];
                
                if(poly->edges[kj].type != EXT_LEG){
                    polygon* other = split->polys[poly->edges[kj].idx];
                    for(size_t oi = 0; oi < other->ngons; oi++){
                        if(other->edges[oi].type == poly->edges[kj].type 
                                && other->edges[oi].idx == p_idx){
                            other->edges[oi].idx = base->npolys;
                            break;
                        }
                    }
                }
                else{
                    split->edges[kj] = base->npolys;
                }
                right->edges[k].type = poly->edges[kj].type;
                right->edges[k].idx  = poly->edges[kj].idx;
            }
            right->gons [right->ngons-1] = poly->gons[i];
            right->edges[right->ngons-1].type = FLSPLIT;
            right->edges[right->ngons-1].idx  = p_idx;
            //right->split_budget = 0;
            
            if(left->ngons > right->ngons){
                left->split_budget = poly->split_budget - ((j-i)%2 ? 2:1);
                right->split_budget = 0;
            }
            else{
                right->split_budget = poly->split_budget - ((j-i)%2 ? 2:1);
                left->split_budget = 0;
            }
            
            //decrement_split_budget(split, left, NULL, (j-i)%2 ? 2 : 1);
                        
            split->rep = represent_diagram(split);
            split->sym = get_symmetry(split);
                        
            root = insert_diagram(root, split);
        }
    }
    
    return root;    
}

diagram* _singlet_prop(const diagram* base, size_t p_idx){
    polygon* poly = base->polys[p_idx];
    polygon* target;
    if(poly->order < 1)
        return NULL;
    
    diagram* root = NULL;
    diagram* singlet;
    
    for(size_t g_idx = 0, t_idx; g_idx < poly->ngons; g_idx++){
        if(poly->edges[g_idx].type == PROPGTR){
            t_idx = poly->edges[g_idx].idx;
            
            if(t_idx < p_idx || base->polys[t_idx]->order < 1)
                continue;
            
            singlet = copy_diagram(base, FALSE);
            target = singlet->polys[t_idx];
            
            singlet->polys[p_idx]->edges[g_idx].type = SINGLET;
            for(size_t i = 0; i < target->ngons; i++){
                if(target->edges[i].type == PROPGTR 
                        && target->edges[i].idx == p_idx){
                    target->edges[i].type = SINGLET;
                    break;
                }  
            }
            
            singlet->rep = represent_diagram(singlet);
            singlet->sym = get_symmetry(singlet);
            
            root = insert_diagram(root, singlet);
        }
    }
    
    return root;
}

diagram* grow_diagrams(const diagram* base, size_t ngons, size_t order){
    if(!ngons)
        return NULL;
    
    diagram* grown = NULL;
    diagram* cut;
    for(; base; base = base->next){
        cut = NULL;
        for(size_t edge = 0; edge < (base->ngons/base->sym); edge++){
            cut = insert_diagram(cut, _cut_edge(base, edge, ngons, order));
        }
        
        grown = merge_diagrams(grown, cut);
    }
    
    return grown;
}

diagram* split_diagrams(diagram* base){
    
    diagram* split = NULL;
    for(diagram* diagr = base; diagr; diagr = diagr->next){
        for(size_t idx = 0; idx < diagr->npolys; idx++){
            split = merge_diagrams(split, _split_poly(diagr, idx));
        }
    }

    if(split)
        base = merge_diagrams(base, split_diagrams(split));
    else
        base = merge_diagrams(base, split);
    
    return base;
}

diagram* singlet_diagrams(diagram* base){
    
    diagram* singlet = NULL;
    for(diagram* diagr = base; diagr; diagr = diagr->next){
        for(size_t idx = 0; idx < diagr->npolys - 1; idx++){
            singlet = merge_diagrams(singlet, _singlet_prop(diagr, idx));
        }
    }

    if(singlet)
        base = merge_diagrams(base, singlet_diagrams(singlet));
    else
        base = merge_diagrams(base, singlet);
    
    return base;
}

int check_zero_fsp(diagram* diagr){
    polygon* poly;
    size_t nm, ns;
    for(size_t p_idx = 0; p_idx < diagr->npolys; p_idx++){
        poly = diagr->polys[p_idx];
        
        nm = 0, ns = 0;
        for(size_t g_idx = 0; g_idx < poly->ngons; g_idx++){
            if(poly->edges[g_idx].type == EXT_LEG 
                    || poly->edges[g_idx].type == PROPGTR)
                nm++;
            if(poly->edges[g_idx].type == SINGLET)
                ns++;
        }
        if(nm == 1 || (!nm && ns < 2))
            return TRUE;
    }
    
    return FALSE;
}

diagram* remove_zero_fsp(diagram* list){
    diagram* root = list;
    diagram* prev = NULL;
    diagram* next = list;
    
    while(next){
        if(check_zero_fsp(next)){
            if(!prev){
                root = next->next;
                delete_diagram(next, FALSE);
                next = root;
            }
            else{
                prev->next = next->next;
                delete_diagram(next, FALSE);
                next = prev->next;
            }
        }
        else{
            prev = next;
            next = next->next;
        }
    }
    
    return root;
}

int compare_diagrams(diagram* diagr_1, diagram* diagr_2){
    if(!diagr_1)
        return diagr_2 ? -1 : 0;
    if(!diagr_2)
        return +1;
    
    //TEMP
    //return -1;
    
    int comp = COMPARE(diagr_2->rep->nreps, diagr_1->rep->nreps);
    if(comp)
        return comp;
    
    comp = COMPARE(diagr_1->npolys, diagr_2->npolys);
    if(comp)
        return comp;
    
    return compare_comprep(diagr_1->rep, diagr_2->rep, NULL);
}

diagram* insert_diagram(diagram* list, diagram* diagr){
    if(!list)
        return diagr;
    if(!diagr)
        return list;
    
    int comp = compare_diagrams(list, diagr);
    if(comp < 0){
        diagr->next = list;
        return diagr;
    }
    else if(comp == 0){
        delete_diagram(diagr, FALSE);
        return list;
    }
    
    diagram* prev = list;
    diagram* next = list->next;
    for(; next; prev = next, next = next->next){
        comp = compare_diagrams(next, diagr);
        if(comp < 0){
            prev->next = diagr;
            diagr->next = next;
            return list;
        }
        else if(comp == 0){
            delete_diagram(diagr, FALSE);
            return list;
        }
    }
    prev->next = diagr;
    diagr->next = NULL;
    return list;
}

diagram* merge_diagrams(diagram* list_1, diagram* list_2){
    if(!list_1)
        return list_2;
    if(!list_2)
        return list_1;
    
    diagram* root = NULL;
    int comp = compare_diagrams(list_1, list_2);
    if(comp > 0){
        root = list_1;
        list_1 = list_1->next;
    }
    else if(comp < 0){
        root = list_2;
        list_2 = list_2->next;
    }
    else{
        root = list_1;
        list_1 = list_1->next;
        list_2 = delete_diagram(list_2, FALSE);
    }
    
    diagram* diagr = root;
    for(; list_1 && list_2; diagr = diagr->next){
        comp = compare_diagrams(list_1, list_2);
        if(comp > 0){
            diagr->next = list_1;
            list_1 = list_1->next;
        }
        else if(comp < 0){
            diagr->next = list_2;
            list_2 = list_2->next;
        }
        else{
            diagr->next = list_1;
            list_1 = list_1->next;
            list_2 = delete_diagram(list_2, FALSE);
        }
    }
    
    if(list_1)
        diagr->next = list_1;
    else
        diagr->next = list_2;
    
    return root;
}

void count_diagrams(const diagram* list, int count_detail){
    if(!list){
        printf("[no diagram]\n");
        return;
    }
    
    size_t tot = 0;
    fsp_map* map = NULL;
    
    for(const diagram* diagr = list; diagr; diagr = diagr->next){
        tot++;
        
        if(count_detail > 0)
            count_diagram(&map, diagr);
    }
    
    if(count_detail == 0){
        printf("O(å^%zd) %zd-point diagrams: %zd\n", 
                OP(list->order), list->ngons, tot);
    }
    else{
        printf("\nO(p^%zd) %zd-point diagrams (grand total: %zd)\n", 
                OP(list->order), list->ngons, tot);
        print_fsp_map(map, "sym", count_detail > 1);
        delete_fsp_map(map);    
    }
}

void _print_diagram_helper(const diagram* diagr, int index){
    printf("O(p^%zd) %zd-point diagram [ID %zd]", 
            OP(diagr->order), diagr->ngons, diagr->diagram_id);
    
    if(index >= 0)
        printf(" (%d)", index);
    
    printf(", symmetry factor %zd:\n", diagr->sym);
    
    for(size_t i = 0; i < diagr->npolys; i++){
        printf("poly ");
        print_polygon(diagr, i);
        printf("\n");
    }
    
    print_comprep(diagr->rep, diagr);
    printf("\n");
}

size_t get_n_flavidx(const diagram* diagr, size_t idx){
    if(idx >= diagr->rep->nreps)
        return 0;
    
    return diagr->rep->reps[idx]->n_flavidx;
}

void print_diagram(const diagram* diagr, int recursive){
    if(!diagr){
        printf("[no diagram]\n\n");
        return;
    }
    
    int index = recursive ? 0 : -1;
    _print_diagram_helper(diagr, index);
    for(diagr = diagr->next; diagr && recursive; diagr = diagr->next){
        _print_diagram_helper(diagr, ++index);
    }
}

void _delete_diagram_helper(diagram* diagr){
    delete_polygons(diagr->polys, diagr->npolys);
    free(diagr->gons);
    free(diagr->gon_idx);
    free(diagr->edges);
    delete_comprep(diagr->rep);
    
    free(diagr);
}

diagram* delete_diagram(diagram* diagr, int recursive){
    if(!diagr)
        return NULL;
    
    diagram* next = diagr->next;
    if(!recursive){
        _delete_diagram_helper(diagr);
        return next;
    }
    
    for(; diagr; diagr = next){
        next = diagr->next;
        _delete_diagram_helper(diagr);
    }
    return NULL;
}
