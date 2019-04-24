
#include "fsp_map.h"
#include "fodge.h"

/**
 * Recursive auxiliary to count.
 * 
 * @param map_ptr   a pointer to the map to be incremented. If the map is NULL,
 *                  it will be created at the pointed-to address. 
 *                  If the pointer is NULL, an error will occur.
 * @param fsp       the flavour split mapping to the count.
 * @param n_fsp     the number of elements in fsp.
 * @param idx       the index of the count.
 * @param depth     the current depth of recursion, starting at 0.
 * @return  the new, incremented value of the count.
 */
size_t _count(fsp_map** map_ptr, const size_t* fsp, 
        size_t n_fsp, size_t idx, size_t depth){
    
    if(!*map_ptr){
        //idprintf("Allocating new map\n");
        *map_ptr = salloc(sizeof(fsp_map));
        (*map_ptr)->n_child = 0;
        (*map_ptr)->n_count = 0;
        (*map_ptr)->tot_count = 0;
    }
    fsp_map* map = *map_ptr;
    size_t count;
        
    if(depth >= n_fsp){
        if(!map->n_count){
            map->counts = scalloc(idx + 1, sizeof(size_t));
            map->n_count = idx + 1;
        }
        else if(idx >= map->n_count){
            map->counts = srecalloc(map->counts, 
                    map->n_count, idx + 1, sizeof(size_t));
            map->n_count = idx + 1;
        }
        
        count = ++(map->counts[idx]);
        map->tot_count++;
        
        if(count == 0 || map->tot_count == 0){
            printf("ERROR: overflow occurred in fsp_map!\n");
            exit(EXIT_FAILURE);
        }
    }
    else{
        if(!map->n_child){
            map->children = scalloc(fsp[depth] + 1, sizeof(fsp_map*));
            map->n_child = fsp[depth] + 1;
        }
        else if(fsp[depth] >= map->n_child){
            map->children = srecalloc(map->children,
                    map->n_child, fsp[depth] + 1, sizeof(fsp_map*));
            map->n_child = fsp[depth] + 1;
        }
        
        MORE_INDENT;
        count = _count(map->children + fsp[depth], fsp, n_fsp, idx, depth + 1);
        LESS_INDENT;
    }

    return count;
}

/**
 * Increments a count in a fsp_map. The map will automatically be enlarged
 * (or created) to accommodate non-existing counts.
 * 
 * @param map_ptr   a pointer to the map to be incremented. If the map is NULL,
 *                  it will be created at the pointed-to address. 
 *                  If the pointer is NULL, an error will occur.
 * @param fsp       the flavour split mapping to the count.
 * @param n_fsp     the number of elements in fsp.
 * @param idx       the index of the count.
 * @return  the new, incremented value of the count.
 */
size_t count(fsp_map** map_ptr, const size_t* fsp, size_t n_fsp, size_t idx){
    return _count(map_ptr, fsp, n_fsp, idx, 0);
}

/**
 * Counts a diagram by incrementing the count corresponding to its flavour
 * splitting and symmetry factor.
 * 
 * @param map_ptr   a pointer to the map. If the map is NULL,
 *                  it will be created at the pointed-to address. 
 *                  If the pointer is NULL, an error will occur.
 * @param diagr     the diagram to be counted.
 * @return  the new, incremented count.
 */
size_t count_diagram(fsp_map** map_ptr, const diagram* diagr){
    size_t* fsp = salloc(diagr->rep->nreps * sizeof(size_t));
    size_t n_fsp = 0;
    for(; n_fsp < diagr->rep->nreps && diagr->rep->reps[n_fsp]->n_flavidx > 0; 
            n_fsp++){
        fsp[n_fsp] = diagr->rep->reps[n_fsp]->n_flavidx;
    }
    
    size_t count = _count(map_ptr, fsp, n_fsp, diagr->sym, 0);
    
    free(fsp);
    return count;
}

/**
 * Retrieves a count from a map. A nonexisting count (or an nonexisting map)
 * counts as zero.
 * 
 * @param map_ptr   the map. 
 * @param fsp       the flavour split mapping to the count.
 * @param n_fsp     the number of elements in fsp.
 * @param idx       the index of the count.
 * @return  the value of the count.
 */
size_t get_count(const fsp_map* map, 
        const size_t* fsp, size_t n_fsp, size_t idx){
    if(!map)
        return 0;
    
    for(size_t i = 0; i < n_fsp; i++){
        if(fsp[i] < map->n_child && map->children[fsp[i]])
            map = map->children[fsp[i]];
        else
            return 0;
    }
    
    if(idx < map->n_count){
        return map->counts[idx];
    }
    else
        return 0;
}

/**
 * Recursive auxiliary to print_fsp_map.
 */
void _print_fsp_map(const fsp_map* map, const char* idx_label, int full_detail,
        char* str, int len, size_t offs){
    
    if(map->n_child && offs > 1){
        str[offs++] = ',';
        str[offs++] = ' ';
    }
    
    int fsp_width;
    
    for(size_t fsp = 0; fsp < map->n_child; fsp++){
        if(!map->children[fsp])
            continue;
        
        fsp_width = decimal_width(fsp);
        
        sprintf(str + offs, "%zd", fsp);
        str[offs + fsp_width] = ' ';
        str[len - 1] = '}';
        
        _print_fsp_map(map->children[fsp], full_detail,
                str, len, offs + fsp_width);
        
        sprintf(str + offs, "%*s", fsp_width, " ");
        str[offs + fsp_width] = ' ';
        str[len - 1] = '}';
    }
    
    if(map->tot_count){
        printf("  %s   %s %7zd\n", 
                str, 
                full_detail ? "total:" : " ", 
                map->tot_count);
    }
    if(full_detail){
        for(size_t idx = 0; idx < map->n_count; idx++){
            if(!map->counts[idx])
                continue;

            printf("  %*s   %6s %zd: %7zd\n", 
                    len, " ",
		    idx_label,
                    idx, 
                    map->counts[idx]);
        }
    }
}

/**
 * Prints the contents of a fsp_map to standard output. Only non-zero counts
 * are printed.
 * 
 * @param map           The map.
 * @param idx_label	The label to print in front of each index in the map,
 *                      like "idx_label idx: count". Not used if full_detail
 *                      is FALSE.
 * @param full_detail   If TRUE, all entries are printed. If FALSE, only the
 *                      total of all counts mapped to each flavour split is
 *                      printed, rather than distributing them across their
 *                      indices.
 */
void print_fsp_map(const fsp_map* map, const char* idx_label, int full_detail){
    if(!map)
        return;
    
    int len = 3 * (map->n_child/2);
    char* str = malloc(len + 2);
    sprintf(str, "{%*s}", len - 2, " ");
       
    _print_fsp_map(map, idx_label, full_detail, str, len, 1);
    
    free(str);
    
}

/**
 * Frees the memory held by a fsp_map.
 * @param map   The map.
 */
void delete_fsp_map(fsp_map* map){
    if(!map)
        return;
    
    for(size_t fsp = 0; fsp < map->n_child; fsp++)
        delete_fsp_map(map->children[fsp]);
    
    if(map->n_child)
        free(map->children);
    if(map->n_count)
        free(map->counts);
    
    free(map);
}
