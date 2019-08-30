/* 
 * File:   fodge_FORM.h
 * Author: Mattias
 *
 * Created on 18 April 2019, 10:53
 */

#ifndef FODGE_FORM_H
#define	FODGE_FORM_H

#include "fodge.hpp"
#include "fsp_map.hpp"

#ifdef	__cplusplus
extern "C" {
#endif

    int fodge_FORM(const char* filename, const diagram* diagr);
    
    size_t FORM_diagrams(FILE* form, const diagram* diagr, fsp_map** vert_list);
    void FORM_vertices(FILE* form, const fsp_map* vert_list, size_t order);
    void FORM_amplitude(FILE* form, const diagram* diagr, size_t n_diagr);
    
    size_t FORM_poly(FILE* form, const diagram* diagr, 
            size_t p_idx, size_t f_idx,
            const size_t* start_idcs, size_t* momenta, int* visited, 
            size_t depth, 
            fsp_map** vert_list, fsp_map** main_vert_list);
    void FORM_cycl(FILE* form, const diagram* diagr, const size_t* start_idcs);
    void FORM_perm(FILE* form, size_t max_flavidx, size_t block_len,
            size_t start_L, size_t start_R);
    
    

#ifdef	__cplusplus
}
#endif

#endif	/* FODGE_FORM_H */

                
