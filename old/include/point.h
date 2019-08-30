/* 
 * File:   point.h
 * Author: Mattias Sjoe
 *
 * Created on 13 February 2019, 10:31
 * 
 * A point struct is a simple representation of a point in the plane. It is
 * equipped with methods for conveniently creating, comparing and transforming
 * points in various ways.
 */

#ifndef POINT_H
#define	POINT_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdlib.h>
#include <math.h>
    
#include "fodge_util.hpp"
    
#define TO_RAD(d) ((d)*M_PI/180)
#define TO_DEG(d) ((d)*180/M_PI)

    typedef struct {
        /** The point's x coordinate */
        double x;
        /** The point's y coordinate */
        double y;
    } point;
    
    point* origin_point();
    point* cartesian_point(double x, double y);
    point* polar_point(double radius, double angle);
    
    point* copy_point(const point* orig);
    
    double point_magnitude(const point* pt);
    double point_angle(const point* pt);
    double point_distance(const point* pt_1, const point* pt_2);
    
    point* shift_point(point* pt, double dx, double dy);
    point* shift_by_point(point* pt, const point* shift);
    point* scale_rot_point(point* pt, double scale, double rot);
    point* scale_rot_by_point(point* pt, const point* scale_rot);
    point* radial_shift_point(point* pt, double dr);
    
    point* midpoint(const point* pt_1, const point* pt_2);
    
    void delete_point(point* pt);
    void delete_points(point** pts, size_t n);

#ifdef	__cplusplus
}
#endif

#endif	/* POINT_H */

