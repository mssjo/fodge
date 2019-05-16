/* 
 * File:   fodge_util.h
 * Author: Mattias
 *
 * Created on 11 February 2019, 14:26
 * 
 * Some useful definitions and methods utilised by fodge in various ways.
 */

#ifndef FODGE_UTIL_H
#define	FODGE_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
    
    /* Boolean convenience */
#define TRUE 1
#define FALSE 0
    
/** Sets the ith element in an s-element array a to value v of type t.
 *  If i >= s, the array is extended and s is adjusted correspondingly.
 *  The expression v is only evaluated once. */
#define ARRSET(a,i,v,s,t)                                           \
    if(i >= s){ s *= 2; a = srealloc(a, s*sizeof(t)); }             \
    a[i] = v

        
    /* Retrieves the index in a table of different-size diagrams:
     * 4 -> 0, 6 -> 1, 8 -> 2, ... */
#define TABIDX(n) ((n-4)/2)
    /* Goes from order label to actual momentum order: 
     * 0 -> 2, 1 -> 4, 2 -> 6, ... */
#define OP(n) (2*((n)+1))
    /* The inverse of OP(n) */
#define INV_OP(n) ((n)/2 - 1)
    
    /* Swaps two values */
#define SWAP(x,y,t) (t) = (x); (x) = (y); (y) = (t);
    /* Maximum function, picks first argument in case of equality */
#define MAX(a,b) (a)>=(b)?(a):(b)
    /* Minimum function, picks first argument in case of equality */
#define MIN(a,b) (a)<=(b)?(a):(b)
    
    /** Shorthand for unsigned integers  */
    typedef unsigned int uint;
    /** The type used for polygon labels */
    typedef int gon_t;
    
    /* Gives the sign of the difference between two values. */
    /* Just subtraction can give errors for unsigned types. */
#define COMPARE(a,b) ((a)<(b) ? -1 : (a)>(b) ? +1 : 0)
    
    /* These are given here to allow circular inclusion between cycreps and
     * polygon diagrams. */
    typedef struct diagram_struct diagram;
    typedef struct polygon_struct polygon;
    typedef struct compound_cycrep_struct comprep;
    
    size_t integer_pow(size_t base, size_t exp);
    int integer_comp(const void* a, const void* b);
    
    int integer_width(int i, int radix);
    int decimal_width(int d);
    
#define DIAGRAM_ID (_diagram_id++)
    size_t _diagram_id;
    void init_diagram_id();
    
#ifndef INDENT
#define INDENT ". "
#endif
    
#define MORE_INDENT (_indent_level += 1)
#define LESS_INDENT (_indent_level -= (_indent_level > 0) ? 1 : 0)
    
    uint _indent_level;
    
    void init_indent();
    void indent_line();

    void idprintf(const char* fmt, ...);
    
    
    void* salloc(size_t n);
    void* scalloc(size_t nitems, size_t size);
    void* srealloc(void* ptr, size_t n);
    void* srecalloc(void* ptr, size_t old_n, size_t new_n, size_t size);

#ifdef	__cplusplus
}
#endif

#endif	/* FODGE_UTIL_H */

