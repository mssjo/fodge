
#include "fodge_FORM.h"

void _FORM_indent(FILE* form, size_t depth){
    fprintf(form, "        ");
    for(size_t i = 0; i < depth; i++)
        fprintf(form, "  ");
}

/**
 * Prints the standardised name of a vertex.
 * 
 * @param form      the file to which the vertex name should be printed.
 * @param order     the order of the vertex: 0 for O(p^2), 1 for O(p^4), etc.
 * @param fsp       the flavour splitting of the vertex.
 * @param fsp_len   the number of flavour splits.
 * @param vert_idx  the index of the vertex, i.e. how many identical ones have
 *                  already been named.
 */
void _FORM_vertex_name(FILE* form, size_t order, 
        const size_t* fsp, size_t fsp_len, size_t vert_idx){
    
    fprintf(form, "[V%zd", fsp[0]);
    for(size_t i = 1; i < fsp_len; i++)
        fprintf(form, "/%zd", fsp[i]);
    
    /* Only prints order if it is larger than minimum for this split */
    if(order >= fsp_len)
        fprintf(form, ".p%zd", OP(order));
    
    /* Only prints index if it is greater than zero 
     * (i.e. vertex is not unique) */
    if(vert_idx > 0)
        fprintf(form, ".%zd", vert_idx);
    
    fprintf(form, "]");
}

/**
 * Determines the flavour splitting of a vertex, and finds all polygons 
 * contained in it.
 * 
 * @param diagr     the diagram containing the vertex.
 * @param p_idx     the index of a polygon in the vertex.
 * @param fsp       an array of length at least (diagr->order + 1). It will be
 *                  filled with an the ordered flavour splitting of the vertex,
 *                  e.g. {2,4,4}.
 * @param fsp_poly  an array of length at least (diagr->order + 1). fsp_poly[i]
 *                  will be the index of the polygon corresponding to fsp[i].
 * @param fsp_len   a pointer to a value that will be set to the number of
 *                  used elements in fsp and fsp_poly. Elements beyond this are
 *                  left undefined.
 * @param visited   all polygons visited by this method will be marked here.
 */
void _explore_vertex(const diagram* diagr, size_t p_idx, 
        size_t* fsp, size_t* fsp_poly, size_t* fsp_len, int* visited){
    
    /* For all indices p of polygons in the vertex, fsp[p] will be the number 
     * of vertex legs represented by that polygon.*/
    size_t* poly_fsp = scalloc(diagr->npolys, sizeof(size_t));
    
    size_t g_idx = 0;
    size_t g_mark = g_idx, p_mark = p_idx;
    polygon* poly = diagr->polys[p_idx];
    gon_t gon;
    
    visited[p_idx] = TRUE;
    for(;;){
        switch(poly->edges[g_idx].type){
            case PROPGTR:
            case SINGLET:                
            case EXT_LEG:
                
                poly_fsp[p_idx]++;
                
                g_idx = (g_idx + 1) % poly->ngons;
                break;
                
            case FLSPLIT: 
                                
                gon = poly->gons[g_idx];
                p_idx = poly->edges[g_idx].idx;
                poly = diagr->polys[p_idx];
                
                for(g_idx = 0; poly->gons[g_idx] != gon; g_idx++);
                
                visited[p_idx] = TRUE;
                
                break;
             
        }
        
        if(p_idx == p_mark && g_idx == g_mark)
            break;
    }
    
    size_t fsp_idx = 0;
    for(p_idx = 0; p_idx < diagr->npolys; p_idx++){
        if(poly_fsp[p_idx]){
            fsp[fsp_idx] = poly_fsp[p_idx];
            fsp_poly[fsp_idx] = p_idx;
            fsp_idx++;
        }
    }
    
    /* Sorts fsp and rearranges fsp_poly accordingly. */
    /* poly_fsp has served its role and is reused as a whither array. */
    size_t* whither = poly_fsp;
    info_sort(fsp, fsp_idx, sizeof(size_t), integer_comp, 
            NULL, whither, NULL, NULL);
    permute(fsp_poly, fsp_idx, sizeof(size_t), whither);
   
    *fsp_len = fsp_idx;
    
    free(whither);    
} 

/**
 * Prints a list of diagrams to a FORM-readable format.
 * 
 * @param filename  the filename to use. Three files will be produced:
 *                  filename_vert.hf  holding all vertex factor definitions,
 *                  filename_diagr.hf holding all diagram definitions, and
 *                  filename_ampl.hf  holding the summation of the diagrams
 *                  into an amplitude.
 * @param diagr     the root of the list of diagrams. All must be of equal size
 *                  and order.
 * @return  0 if everything went well, 1 if some error was encountered while
 *          writing the files. If 1 is returned, the global variable errno may 
 *          be consulted.
 */
int fodge_FORM(const char* filename, const diagram* diagr){
    if(!diagr)
        return 0;
    
    size_t name_len = strlen(filename)
        + decimal_width(diagr->ngons) 
        + decimal_width(OP(diagr->order))
        + strlen("_p");
    FILE* form;
    
    char* name = salloc(name_len + strlen("_diagr.hf") + 1);
    sprintf(name, "%s_%zdp%zd", filename, diagr->ngons, OP(diagr->order));
    
    sprintf(name + name_len, "_diagr.hf");
    printf("FORMing O(p^%zd) %zd-point diagrams > %s ...", 
            OP(diagr->order), diagr->ngons, name);
    if(!(form = fopen(name, "w")))
        goto abort;
    
    
    fsp_map* vert_list = NULL;
    size_t n_diagr = FORM_diagrams(form, diagr, &vert_list);
    
    printf("[done]\n");
    if(fclose(form))
        goto abort;
    
    sprintf(name + name_len, "_vert.hf");
    printf("FORMing all necessary vertex factors > %s ...", name);
    if(!(form = fopen(name, "w")))
        goto abort;
    
    FORM_vertices(form, vert_list, diagr->order);
    
    printf("[done]\n");
    if(fclose(form))
        goto abort;
    
    sprintf(name + name_len, "_ampl.hf");
    printf("FORMing the resulting amplitude > %s ...", name);
    if(!(form = fopen(name, "w")))
        goto abort;
    
    FORM_amplitude(form, diagr, n_diagr);
    
    printf("[done]\n");
    if(fclose(form))
        goto abort;
    
    free(name);
    return 0;
    
    abort:\
    free(name);
    return 1;
}

/**
 * Prints FORM representations of a list of diagrams.
 * 
 * @param form      the file to which the diagrams should be printed.
 * @param diagr     the root of the list.
 * @param vert_list a pointer to a (initially NULL) list of vertices, ordered
 *                  by flavour splitting and order. 
 * @return  the number of diagrams printed.
 */
size_t FORM_diagrams(FILE* form, const diagram* diagr, fsp_map** vert_list){
    
    size_t* start_idcs = salloc(diagr->npolys * sizeof(size_t));
    size_t* momenta = salloc(diagr->ngons * sizeof(size_t));
    int* visited = salloc(diagr->npolys * sizeof(int));
    
    fsp_map* diagr_vert_list;
    size_t start_idx;
    
    size_t count = 0;
    for(; diagr; diagr = diagr->next, count++){
        memset(momenta, 0, diagr->ngons * sizeof(size_t));
        memset(visited, 0, diagr->npolys * sizeof(int));
        diagr_vert_list = NULL;
        
        start_idx = 1;
        for(size_t r = 0; r < diagr->rep->nreps; r++){
            for(size_t p = 0; p < diagr->npolys; p++){
                if(diagr->rep->poly_reps[p] == diagr->rep->reps[r])
                    start_idcs[p] = start_idx;
            }
            start_idx += diagr->rep->reps[r]->n_flavidx;
        }
        
/*
        for(size_t r = 0; r < diagr->rep->nreps; r++){
            print_cycrep(diagr->rep->reps[r], diagr);
            printf("\n");
        }
        
        for(size_t p = 0; p < diagr->npolys; p++){
            print_cycrep(diagr->rep->poly_reps[p], diagr);
            printf("\nstart_idx: %zd\n", start_idcs[p]);
        }
*/
        
        fprintf(form, "\nglobal [D%zd.p%zd.%zd] =", 
                diagr->ngons, OP(diagr->order), count + 1);
        
        FORM_poly(form, diagr, 0, start_idcs[0], 
                start_idcs, momenta, visited, 0,
                &diagr_vert_list, vert_list);
                
        FORM_cycl(form, diagr, start_idcs);
        
        fprintf(form, ";\n");
        
        delete_fsp_map(diagr_vert_list);
    }

    return count;
}

/**
 * Increments the vertex lists in response to a new vertex.
 * 
 * @param vert_list         The local vertex list - always incremented.
 * @param main_vert_list    The global vertex list - only incremented is the
 *                          local one exceeds it.
 * @param fsp               The flavour splitting of the vertex.
 * @param fsp_len           The number of entries in fsp.
 * @param order             The order of the vertex.
 * @return  The new value of the count that was incremented.
 */
size_t _count_vertex(fsp_map** vert_list, fsp_map** main_vert_list, 
        const size_t* fsp, size_t fsp_len, size_t order){
    
    size_t main_vert_count = get_count(*main_vert_list, fsp, fsp_len, order);
    size_t vert_count = count(vert_list, fsp, fsp_len, order);
    if(vert_count > main_vert_count)
        count(main_vert_list, fsp, fsp_len, order);
    
    return vert_count;
}

/**
 * Prints the momentum content of a propagator. 
 * 
 * @param form      The file to which the propagator is printed.
 * @param momenta   All momenta p such that momenta[p] >= depth are included
 *                  in the propagator. They are set to (depth-1) to prevent
 *                  inclusion in future calls at this or deeper levels.
 * @param max_f_idx The maximum flavour index in the diagram, and the length
 *                  of momenta.
 * @param depth     The depth of recursion from which this method is called.
 */
void _FORM_propagator(FILE* form, size_t* momenta, 
        size_t max_f_idx, size_t depth, int is_singlet){
    
    fprintf(form, is_singlet ? "singlet(" : "prop(");
    
    size_t f_idx;
    
    /* If more than half the momenta are included in the propagator,
     * use the complementary half (by COM) to reduce computation time
     * (result in Mandelstams is unchanged, but many momenta take longer
     * to simplify) */
    size_t n_mom = 0;
    for(f_idx = 1; f_idx <= max_f_idx; f_idx++){
        if(momenta[f_idx-1] >= depth)
            n_mom++;
    }
    int inv_mom = (n_mom > max_f_idx / 2);
    
    for(f_idx = 1; inv_mom != (momenta[f_idx-1] < depth); f_idx++);
    fprintf(form, "p%zd", f_idx);
    momenta[f_idx-1] = depth-1;
    for(f_idx++; f_idx <= max_f_idx; f_idx++){
        if(inv_mom != (momenta[f_idx-1] >= depth)){
            fprintf(form, "+p%zd", f_idx);
            momenta[f_idx-1] = depth - 1;
        }
    }
    fprintf(form, ")");
}

/**
 * Recursively builds a diagram polygon by polygon (i.e. vertex by vertex, or
 * part by part for split vertices).
 * 
 * It works by traversing the polygon, marking external legs as it goes. When a
 * propagator is encountered, a new vertex is added based on the polygon that is
 * entered, and the method calls itself on the new polygon. If the vertex is 
 * split, its parts are filled by recursing on them in ascending order.
 * 
 * This method can terminate in two ways. If it passes a propagator back into a
 * polygon that was already visited, it prints the momentum content of that
 * propagator and terminates. If it reaches its point of origin in the diagram,
 * it simply terminates; this happens to the "root" polygon of each part.
 * 
 * @param form              The file to which the FORM representation should be
 *                          written.
 * @param diagr             The diagram to be represented.
 * @param p_idx             The index of the polygon to be represented.
 * @param f_idx             The current flavour index.
 * @param start_idcs        For each polygon index p, start_idcs[p] is the first
 *                          flavour index to be used by the part containing
 *                          polygon p.
 * @param momenta           For each flavour index f, momenta[f-1] (the offset 
 *                          being due to f starting at 1, not 0) is marked with
 *                          the depth of recursion at which it features as an
 *                          external leg. This is used to resolve propagator
 *                          contents.
 * @param visited           Polygons are marked here as they are visited by
 *                          the method.
 * @param depth             the depth of recursion, starting at 0.
 * @param vert_list         the list of vertices used by this diagram. It will 
 *                          be extended whenever a new vertex is added.
 * @param main_vert_list    the global list of vertices. It will be extended
 *                          whenever a vertex is added to the local vert_list
 *                          that is not already included here. 
 * @return  the next flavour index to be used. This return value is only used
 *          internally in the recursion; 0 is returned by the top-level call.
 */
size_t FORM_poly(FILE* form, const diagram* diagr, size_t p_idx, size_t f_idx, 
        const size_t* start_idcs, size_t* momenta, int* visited, size_t depth,
        fsp_map** vert_list, fsp_map** main_vert_list){
    
    size_t* fsp = salloc((diagr->order + 1) * sizeof(size_t));
    size_t* fsp_poly = salloc((diagr->order + 1) * sizeof(size_t));
    size_t fsp_len, vert_count;
    
    polygon* poly = diagr->polys[p_idx];
    
    if(depth == 0){
        visited[p_idx] = TRUE;

        /* Determines the properties of the new vertex */
        _explore_vertex(diagr, p_idx, 
                fsp, fsp_poly, &fsp_len, visited);

        /* Adds the vertex to the lists */
        vert_count = _count_vertex(vert_list, main_vert_list, 
                fsp, fsp_len, poly->order);

        /* Prints the new vertex in preparation for recursion */
        fprintf(form, "\n");
        _FORM_indent(form, 0);
        fprintf(form, "diagram(");
        _FORM_vertex_name(form, poly->order, fsp, fsp_len, vert_count);

        /* Recurses on all parts of the vertex in order */
        for(size_t i = 0; i < fsp_len; i++){
            if(fsp_poly[i] == p_idx){
                FORM_poly(form, diagr, fsp_poly[i], f_idx,
                        start_idcs, momenta, visited, 1,
                        vert_list, main_vert_list);
            }
            else{
                FORM_poly(form, diagr, fsp_poly[i], start_idcs[fsp_poly[i]], 
                        start_idcs, momenta, visited, 1,
                        vert_list, main_vert_list);
            }
        }
        
        fprintf(form, ")");
        return 0;
    }
    
    size_t g_idx = 0, next_p_idx;
    size_t g_mark = g_idx, p_mark = p_idx;
    
    for(;;){
              
        /* Traverses the polyon */
        switch(poly->edges[g_idx].type){              
            case EXT_LEG:
                
                if(f_idx == start_idcs[p_idx])
                    fprintf(form, ", %zd", f_idx);
                else
                    fprintf(form, ",%zd", f_idx);
                
                momenta[f_idx-1] = depth;
                f_idx++;
                /* Intentional fall-through */

            case FLSPLIT:

                g_idx = (g_idx + 1) % poly->ngons;

                /* This marks that no new polygon was entered */
                next_p_idx = diagr->npolys;
                break;

            case PROPGTR:
            case SINGLET:

                next_p_idx = poly->edges[g_idx].idx;

                g_idx = (g_idx + 1) % poly->ngons;
                break;
        } 
        
        if(next_p_idx >= diagr->npolys){
            /* Just continue; this is placed here to reduce indentation... */
        }
        else if(!visited[next_p_idx]){
            visited[next_p_idx] = TRUE;
            
            /* Determines the properties of the new vertex */
            _explore_vertex(diagr, next_p_idx, 
                    fsp, fsp_poly, &fsp_len, visited);

            /* Adds the vertex to the lists */
            vert_count = _count_vertex(vert_list, main_vert_list, 
                    fsp, fsp_len, diagr->polys[next_p_idx]->order);
            
            /* Prints the new vertex in preparation for recursion */
            fprintf(form, ",\n");
            _FORM_indent(form, depth);
            fprintf(form, "diagram(");
            _FORM_vertex_name(form, diagr->polys[next_p_idx]->order, 
                    fsp, fsp_len, vert_count);

            /* Recurses on all parts of the vertex in order */
            for(size_t i = 0; i < fsp_len; i++){
                if(fsp_poly[i] == next_p_idx){
                    f_idx = FORM_poly(form, diagr, fsp_poly[i], f_idx,
                            start_idcs, momenta, visited, depth + 1,
                            vert_list, main_vert_list);
                    
                    fprintf(form, "\n");
                    _FORM_indent(form, depth-1);
                }
                else{
                    FORM_poly(form, diagr, fsp_poly[i], start_idcs[fsp_poly[i]], 
                            start_idcs, momenta, visited, depth + 1,
                            vert_list, main_vert_list);
                }
            }
        }
        else{
            /* Prints the (singlet) propagator back to the origin */
            /* A propagator is a singlet iff the polygons it separates are
             * in different flavour traces, which is equivalent to having 
             * different starting flavour index. */
            fprintf(form, ", ");
            _FORM_propagator(form, momenta, diagr->ngons, depth, 
                    start_idcs[p_idx] != start_idcs[next_p_idx]);
            fprintf(form, ")");
            
            /* Terminates and returns the next flavour index */
            free(fsp);
            free(fsp_poly);
            return f_idx;
        }
        
        /* Terminates if the entire polygon has been traversed */
        /* This means all indices in the trace have been used, 
         * so we return 0. */
        if(p_idx == p_mark && g_idx == g_mark){
            free(fsp);
            free(fsp_poly);
            return 0;
        }
    }
}

/**
 * Prints the FORM code for "identity, plus permutation that swaps block
 * L and block R". Block L must come before R, and they must be of equal length.
 * By printing this for all pairs of index blocks that should be swapped, all 
 * allowed permutations of blocks are executed exactly once.
 * 
 * @param form          the file to which the permutations are printed.
 * @param max_flavidx   the largest flavour index in the diagram.
 * @param block_len     the number of flavour indices in each block.
 * @param start_L       the first flavour index in block L.
 * @param start_R       the first flavour index in block R.
 */
void FORM_perm(FILE* form, size_t max_flavidx, size_t block_len,
        size_t start_L, size_t start_R){
        
    fprintf(form, "\n");
    _FORM_indent(form, 0);
    
    /* Identity plus permutation...*/
    fprintf(form, " * (1 + permute(");
    
    /* Indices before L*/
    if(start_L > 1)
        fprintf(form, "1,...,%zd, ", start_L - 1);
    
    /* L replaced by R */
    fprintf(form, "%zd,...,%zd, ", start_R, start_R + block_len - 1);
    
    /* Indices between L and R */
    if(start_R > start_L + block_len)
        fprintf(form, "%zd,...,%zd, ", start_L + block_len, start_R - 1);
    
    /* R replaced by L */
    fprintf(form, "%zd,...,%zd", start_L, start_L + block_len - 1);
    
    /* Indices after R */
    if(start_R + block_len < max_flavidx)
        fprintf(form, ", %zd,...,%zd", start_R + block_len, max_flavidx);
    
    fprintf(form, "))");
}

/**
 * Prints all necessary cyclings and permutations that should be applied to a
 * diagram.
 * 
 * @param form          The file to which the cyclings should be printed.
 * @param diagr         The diagram.
 * @param start_idcs    For each polygon index p, start_idcs[p] is the first
 *                      flavour index to be used by the part containing
 *                      polygon p.
 */
void FORM_cycl(FILE* form, const diagram* diagr, const size_t* start_idcs){
    
    cycrep** reps = diagr->rep->reps;
    
    /* Translates start_idcs into an array holding the start index of each
     * polygon instead */
    size_t* start_idx_rep = salloc(diagr->rep->nreps * sizeof(size_t));
    for(size_t i = 0; i < diagr->rep->nreps; i++){
        for(size_t p = 0; p < diagr->npolys; p++){
            if(diagr->rep->poly_reps[p] == reps[i]){
                start_idx_rep[i] = start_idcs[p];
                break;
            }
        }
    }
    
    for(size_t i = 0; i < diagr->rep->nreps; i++){
        /* Skips empty parts */
        if(!reps[i]->n_flavidx)
            continue;
        
        /* Prints cycling of less-than-completely symmetric parts */
        if(reps[i]->period > 1){
            fprintf(form, "\n");
            _FORM_indent(form, 0);
            fprintf(form, " * cycle(%zd, %zd,...,%zd)", 
                    reps[i]->period, 
                    start_idx_rep[i], 
                    start_idx_rep[i] + reps[i]->n_flavidx - 1);
        }
        
        /* Prints swaps with distinct equal-size parts of LARGER index
         * (this avoids double-counting) */
        for(size_t j = i+1; j < diagr->rep->nreps; j++){
            if(reps[j]->n_flavidx != reps[i]->n_flavidx)
                break;
            
            if(diagr->rep->eq_reps[j] != diagr->rep->eq_reps[i])
                FORM_perm(form, diagr->ngons, reps[i]->n_flavidx,
                        start_idx_rep[i], start_idx_rep[j]);
        }
    }
    
    free(start_idx_rep);
}

/**
 * Recursive auxiliary to FORM_vertices. Traverses the vertex list depth-first
 * and prints all vertices encountered.
 * 
 * @param form      the file to which the vertices are printed.
 * @param vert_list the current sub-list of vertices being traversed.
 * @param fsp_list  the flavour split at the current point in the traversal.
 * @param n_legs    the number of legs on the current vertex; the sum over 
 *                  fsp_list up to fsp_list[depth-1].
 * @param depth     the current depth of recursion.
 */
void _FORM_vertices(FILE* form, const fsp_map* vert_list, 
        size_t* fsp_list, size_t n_legs, size_t depth){
    
    if(!vert_list)
        return;

    int already_unsplit = FALSE;
    
    if(vert_list->n_count){
        /* Sets up the correct splittings, not needed if vertex is unsplit */
        if(already_unsplit){}
        else if(depth < 1){
	    fprintf(form, "\n#redefine SPLIT \"unsplit\"\n");
	    already_unsplit = TRUE;
	}
	else {
	    fprintf(form, "\n#redefine SPLIT \"split(%zd", fsp_list[0]);
            for(size_t i = 1; i < depth; i++)
                fprintf(form, ",%zd", fsp_list[i]);
            fprintf(form, ")\"\n");
	    already_unsplit = FALSE;
        }
        
        /* Prints all vertices with this splitting */
        for(size_t ord = 0; ord < vert_list->n_count; ord++){
            for(size_t idx = 0; idx < vert_list->counts[ord]; idx++){
                fprintf(form, "#call sfrule(%zd,%zd,", n_legs, OP(ord));
                _FORM_vertex_name(form, ord, fsp_list, depth, idx+1);
                fprintf(form, ")\n");
            }
        }
    }
    
    if(vert_list->n_child){
        /* Edits the flavour splitting as needed and recurses */
        /* Iteration is done backwards for more natural ordering */
        for(size_t fsp = vert_list->n_child - 1; fsp+1 > 0; fsp--){
            if(!vert_list->children[fsp])
                continue;
                
            fsp_list[depth] = fsp;
            _FORM_vertices(form, vert_list->children[fsp], 
                    fsp_list, n_legs + fsp, depth + 1);
        }
    }    
}

/**
 * Prints the definitions of all vertices required by an amplitude, as 
 * determined through a call to FORM_diagrams.
 * 
 * @param form      the file to which the vertex definitions should be printed.
 * @param vert_list the list of vertices, ordered by flavour splitting and 
 *                  order.
 * @param order     the maximum order of any vertex.
 */
void FORM_vertices(FILE* form, const fsp_map* vert_list, size_t order){
    size_t* fsp = salloc((order + 1) * sizeof(size_t));
    
    _FORM_vertices(form, vert_list, fsp, 0, 0);
    
    free(fsp);
}

/**
 * Prints the summation of all diagrams into an amplitude.
 * 
 * @param form      the file to which the summation is printed. 
 * @param diagr     an example diagram.
 * @param n_diagr   the number of diagrams to print.
 */
void FORM_amplitude(FILE* form, const diagram* diagr, size_t n_diagr){
    fprintf(form, "global [M%zdp%zd] =\n   ", diagr->ngons, OP(diagr->order));
    
    for(size_t idx = 0; idx < n_diagr; idx++){
        if(idx && !(idx % 5))
            fprintf(form, "\n");
        if(idx)
            fprintf(form, " + ");
        
        fprintf(form, "[D%zd.p%zd.%zd]",  
                diagr->ngons, OP(diagr->order), idx + 1);
    }
    
    fprintf(form, ";");
}

