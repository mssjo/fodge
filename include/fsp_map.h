/* 
 * File:   fsp_map.h
 * Author: Mattias
 *
 * Created on 17 April 2019, 10:07
 */

#ifndef FSP_MAP_H
#define	FSP_MAP_H

#include "fodge_util.h"

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct fsp_map_node fsp_map;    
    struct fsp_map_node {
        size_t tot_count;      
        
        size_t n_count;
        size_t* counts;
        
        size_t n_child;
        fsp_map** children;
    };
    
    size_t count(fsp_map** map_ptr, const size_t* fsp, size_t n_fsp, size_t idx);
    size_t count_diagram(fsp_map** map_ptr, const diagram* diagr);
    
    size_t get_count(const fsp_map* map, 
            const size_t* fsp, size_t n_fsp, size_t idx);
    
    void print_fsp_map(const fsp_map* map, const char* idx_label, int full_detail);
    void delete_fsp_map(fsp_map* map);

#ifdef	__cplusplus
}
#endif

#endif	/* FSP_MAP_H */

