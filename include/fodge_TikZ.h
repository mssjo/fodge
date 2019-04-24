/* 
 * File:   fodge_TikZ.h
 * Author: Mattias
 *
 * Created on 13 February 2019, 13:46
 */

#ifndef FODGE_TIKZ_H
#define	FODGE_TIKZ_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "fodge.h"
#include "point.h"
    
    enum draw_mode{
        DRAW_POLYGON = 0b01, DRAW_FEYNMAN = 0b10
    };
    
    void TikZ_polygon(FILE* tex, const diagram* diagr, size_t p_idx, 
            point** points, int draw_edges, int fill_face);
    void TikZ_diagrams(FILE* tex, diagram* diagr, enum draw_mode mode,
            double radius);    
    void TikZ_table(FILE* tex, diagram_table* tab, enum draw_mode mode,
            double base_radius, double incr_radius);


#ifdef	__cplusplus
}
#endif

#endif	/* FODGE_TIKZ_H */

