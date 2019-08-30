
#include "fodge.hpp"
#include "point.hpp"
#include "fodge_TikZ.hpp"

#include <time.h>


int _get_vertex_shade(size_t order){
    switch((order-1)%3){
        case 0:
            return 100;
        case 1:
            return 50;
        case 2:
        default:
            return 0;
    }
}

void _TikZ_diagram_poly(FILE* tex, diagram* diagr, point** gon_pts){
    
    for(size_t i = 0; i < diagr->npolys; i++)
        TikZ_polygon(tex, diagr, i, gon_pts, FALSE, TRUE);
    for(size_t i = 0; i < diagr->npolys; i++)
        TikZ_polygon(tex, diagr, i, gon_pts, TRUE, FALSE);
    
}

#define VERTEX_RADIUS       .05
#define LEG_MARGIN          2*VERTEX_RADIUS
#define PI		    3.14159265358979323846
void _TikZ_diagram_feyn(FILE* tex, diagram* diagr, 
        point** gon_pts, point** edge_pts){
    
    point* pt;
    point* vert_pts = scalloc(diagr->npolys, sizeof(point));
    polygon* poly;
    
    for(size_t i = 0; i < diagr->npolys; i++){
        poly = diagr->polys[i];
        for(size_t j = 0; j < poly->ngons; j++){
            shift_by_point(
                    vert_pts + i, gon_pts[diagr->gon_idx[poly->gons[j]]]);
        }
        scale_rot_point(vert_pts + i, 1.0/poly->ngons, 0);
        
        for(size_t j = 0; j < poly->ngons; j++){
            switch(poly->edges[j].type){
                case EXT_LEG:
                    pt = edge_pts[diagr->gon_idx[poly->gons[j]]];
                    
                    fprintf(tex, "\t\t\\draw[thick]"
                            " (%f,%f)--(%f,%f);\n",
                            vert_pts[i].x, vert_pts[i].y, pt->x, pt->y);
                    break;
                case PROPGTR:
                    if(poly->edges[j].idx < i){
                        pt = vert_pts + poly->edges[j].idx;
                        fprintf(tex, "\t\t\\draw[thick]"
                                " (%f,%f)--(%f,%f);\n",
                                vert_pts[i].x, vert_pts[i].y, pt->x, pt->y);
                    }
                    break;
                case SINGLET:
                    if(poly->edges[j].idx < i){
                        pt = vert_pts + poly->edges[j].idx;
                        fprintf(tex, "\t\t\\draw[thick,dashed]"
                                " (%f,%f)--(%f,%f);\n",
                                vert_pts[i].x, vert_pts[i].y, pt->x, pt->y);
                    }
                    break;
                case FLSPLIT:
/*
                    pt = gon_pts[poly->gons[(j+1) % poly->ngons]];
                    fprintf(tex, "\t\t\\draw[densely dotted]"
                            " (%f,%f)--(%f,%f);\n",
                            gon_pts[poly->gons[j]]->x, 
                            gon_pts[poly->gons[j]]->y,
                            pt->x, pt->y);
*/
                    break;
            }
        }
    }
    
    for(size_t i = 0; i < diagr->npolys; i++){
        if(diagr->polys[i]->order == 0)
            continue;
        
        fprintf(tex, "\t\t\\draw[black,fill=black!%d]"
                " (%f,%f) circle[radius=%f];\n",
                _get_vertex_shade(diagr->polys[i]->order),
                vert_pts[i].x, vert_pts[i].y, VERTEX_RADIUS);
    }
    
    free(vert_pts);
}

void TikZ_diagrams(FILE* tex, diagram* diagr, enum draw_mode mode,
        double radius){
    
    if(!mode)
        return;
    
    point** gon_pts = salloc(diagr->ngons * sizeof(point*));
    for(size_t i = 0; i < diagr->ngons; i++)
        gon_pts[i] = polar_point(radius, (2*PI*i)/diagr->ngons);
    
    point** edge_pts = NULL;
    if(mode & DRAW_FEYNMAN){
        edge_pts = salloc(diagr->ngons * sizeof(point*));
        double feyn_rad = radius * cos(PI/diagr->ngons) + LEG_MARGIN;
        for(size_t i = 0; i < diagr->ngons; i++)
            edge_pts[i] = polar_point(feyn_rad, 2*PI*(i+.5)/diagr->ngons);
    }
        
    fprintf(tex, "\\begin{center}\n");
    size_t ngons = diagr->ngons;
    for(size_t index = 0; diagr; diagr = diagr->next, index++){
        
        fprintf(tex, "\t%% O(p^%zd) %zd-point diagram (%zd)\n", 
              OP(diagr->order), diagr->ngons, index);
        fprintf(tex, "\t\\tikz{\n");
        
        if(mode & DRAW_POLYGON)
            _TikZ_diagram_poly(tex, diagr, gon_pts);
        if(mode & DRAW_FEYNMAN)
            _TikZ_diagram_feyn(tex, diagr, gon_pts, edge_pts);
        
        fprintf(tex, "\t\t\\draw (%f,-%f) node {\\tiny %zd};\n", 
            radius, radius, diagr->sym);
        fprintf(tex, "\t}\\:\n");
    }
    fprintf(tex, "\\end{center}\n\n");
    
    delete_points(gon_pts, ngons);
    delete_points(edge_pts, ngons);
}

int _get_poly_shade(size_t order){
    switch(order){
        case 0:
            return 0;
        case 1:
            return 25;
        case 2:
            return 50;
        case 3:
            return 65;
        default:
            return 100;
    }
}

void TikZ_polygon(FILE* tex, const diagram* diagr, size_t p_idx, point** points, 
        int draw_edges, int fill_face){
    polygon* poly = diagr->polys[p_idx];
    point* pt;
    
    if(fill_face){
        fprintf(tex, "\t\t\\filldraw[rounded corners=.01mm,black!%d]", 
                _get_poly_shade(poly->order));
        for(size_t i = 0; i < poly->ngons; i++){
            pt = points[diagr->gon_idx[poly->gons[i % poly->ngons]]];
            fprintf(tex, "(%f,%f)--", pt->x, pt->y);
        }
        fprintf(tex, "cycle;\n");
    }
    if(draw_edges){
        for(size_t i = 0; i < poly->ngons; i++){
            if(poly->edges[i].type != EXT_LEG && poly->edges[i].idx > p_idx)
                continue;

            fprintf(tex, "\t\t\\draw[black");
            if(poly->edges[i].type == SINGLET)
                fprintf(tex, ",dashed");
            else if(poly->edges[i].type == FLSPLIT)
                fprintf(tex, ",densely dotted");

            pt = points[diagr->gon_idx[poly->gons[ i    % poly->ngons]]];
            fprintf(tex, "](%f,%f)", pt->x, pt->y);

            pt = points[diagr->gon_idx[poly->gons[(i+1) % poly->ngons]]];
            fprintf(tex, "--(%f,%f);\n", pt->x, pt->y);
        }
    }
}

void _print_header(FILE* tex){
    time_t current_time = time(NULL);
    char* time_string = ctime(&current_time);    
    fprintf(tex, "%% Generated by fodge on %s\n", time_string);
    fprintf(tex, "%% Written by Mattias Sjoe, 2019-02-13\n");
    fprintf(tex, "%% To render, use TikZ under LaTeX, e.g. \n");
    fprintf(tex, "%% \n");
    fprintf(tex, "%% \\documentclass{article}\n");
    fprintf(tex, "%% \\usepackage{pgf,tikz}\n");
    fprintf(tex, "%% \\begin{document}\n");
    fprintf(tex, "%%    \\input{<this file>}\n");
    fprintf(tex, "%% \\end{document}\n\n\n");
}

void TikZ_table(FILE* tex, diagram_table* tab, enum draw_mode mode,
        double base_radius, double incr_radius){
        
    _print_header(tex);
    
    if(!tab){
        fprintf(tex, "%% [no table]\n");
        return;
    }
    
    for(size_t o = 0; o <= tab->max_order; o++){
        if(!tab->table[o])
            continue;
        
        for(size_t n = 0; n <= TABIDX(tab->max_ngons); n ++){
            if(!tab->table[o][n])
                continue;
            
            TikZ_diagrams(tex, tab->table[o][n], mode, 
                    base_radius + n*incr_radius);
        }
    }
    
    printf("[done]\n");
}
