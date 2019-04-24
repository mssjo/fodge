/* 
 * File:   cycle.h
 * Author: Mattias
 *
 * Created on 11 February 2019, 14:26
 */

#ifdef CYCLE_T

#ifndef CYCLE_H
#define CYCLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
    
#include "fodge_util.h"
        
#define COPY_TYPE(T)        TOKENPASTE(copy_, T)
#define DEF_COPY_TYPE(T)    T    COPY_TYPE(T)   (T orig)
    
#define COMPARE_TYPE(T)     TOKENPASTE(compare_, T)
#define DEF_COMPARE_TYPE(T) int  COMPARE_TYPE(T)(T left, T right)
    
#define DELETE_TYPE(T)      TOKENPASTE(delete_, T)
#define DEF_DELETE_TYPE(T)  void DELETE_TYPE(T) (T del)
     
    DEF_COPY_TYPE(CYCLE_T);
    DEF_COMPARE_TYPE(CYCLE_T);
    DEF_DELETE_TYPE(CYCLE_T);
    
    typedef struct {
        uint length;
        CYCLE_T* array;
        
        int modified;
        uint offset;
        uint period;
    } cycle;
    
    cycle* make_cycle(CYCLE_T* array, uint length);
    cycle* copy_cycle(const cycle* orig);
    
    uint get_length(cycle* cycl);
    uint get_offset(cycle* cycl);
    uint get_period(cycle* cycl);
    
    CYCLE_T get_element(cycle* cycl, idx_t index, int LLR);
    CYCLE_T set_element(cycle* cycl, CYCLE_T elem, idx_t index, int LLR);
    CYCLE_T set_periodic(cycle* cycl, CYCLE_T elem, idx_t index, int LLR);
    
    void normalise(cycle* cycl);
    
    int compare_cycle(cycle* cycl_1, cycle* cycl_2);
    int compare_offset(cycle* cycl, uint offset);
    
    void print_cycle(cycle* cycl, int LLR, const char* format);
    
    void delete_cycle(cycle* cycl, int deep);

#ifdef	__cplusplus
}
#endif

#endif  /* CYCLE_H */
#endif	/* CYCLE_T */

