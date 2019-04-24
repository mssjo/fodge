
#include "point.h"
#include <math.h>

/**
 * Creates an instance of the origin.
 * @return a point with x = y = 0.
 */
point* origin_point(){
    return cartesian_point(0, 0);
}

/**
 * Creates a point from its cartesian coordinates.
 * @param x the x coordinate
 * @param y the y coordinate
 * @return  a newly allocated point with the given coordinates.
 */
point* cartesian_point(double x, double y){
    point* pt = salloc(sizeof(point));
    pt->x = x;
    pt->y = y;
    return pt;
}

/**
 * Creates a point from its plane polar coordinates. 
 * @param radius    the radial coordinate
 * @param angle     the angular coordinate, in radians
 * @return  a newly allocated point with the given coordinates converted to
 *          cartesian form.
 */
point* polar_point(double radius, double angle){
    return cartesian_point(radius * cos(angle), radius * sin(angle));
}

/**
 * Makes an exact independent copy of a point.
 * @param orig  the original point
 * @return  a newly allocated identical point.
 */
point* copy_point(const point* orig){
    return cartesian_point(orig->x, orig->y);
}

/**
 * Computes the magnitude of a point, i.e. its distance to the origin.
 * @param pt    the point
 * @return  the value R such that polar_point(R, point_angle(pt)) is identical
 *          to pt (up to floating-point rounding error). 
 */
double point_magnitude(const point* pt){
    return hypot(pt->y, pt->x);
}

/**
 * Computes the angle between a point an the positive x axis. It is given in
 * radians in the range [0, 2*M_PI)
 * @param pt    the point
 * @return  the value A such that polar_point(point_magnitude(pt), A) is 
 *          identical to pt (up to floating-point rounding error). 
 */
double point_angle(const point* pt){
    return atan2(pt->y, pt->x);
}

/**
 * Computes the distance between two points.
 * @param pt_1  a point
 * @param pt_2  another point
 * @return  sqrt(dx^2 + dy^2), where dx and dy are the differences in x and y
 *          coordinates between the points.
 */
double point_distance(const point* pt_1, const point* pt_2){
    return hypot(pt_1->x - pt_2->x, pt_1->y - pt_2->y);
}

/**
 * Shifts a point in the plane.
 * @param pt    the point - will be modified by the operation
 * @param dx    the amount of x shift
 * @param dy    the amount of y shift
 * @return  the shifted point.
 */
point* shift_point(point* pt, double dx, double dy){
    pt->x += dx;
    pt->y += dy;
    return pt;
}

/**
 * Shifts a point by an amount given by the coordinates of another point. This
 * is equivalent to vector addition or complex number addition.
 * @param pt    the point - will be modified by the operation
 * @param shift the other point - is left unchanged
 * @return  the shifted point.
 */
point* shift_by_point(point* pt, const point* shift){
    return shift_point(pt, shift->x, shift->y);
}

/**
 * Shifts a point in the radial direction without changing its angle. Unlike
 * scale_rot_point, the amount of shift is independent of the point's 
 * coordinates.
 * @param pt    the point - which will be modified by the operation
 * @param dr    the amount of radial shift
 * @return  the shifted point, which is now equal within rounding error to
 *          polar_point(point_magnitude(pt) + dr, point_angle(pt))
 *          (with pt being taken before the shift)
 */
point* radial_shift_point(point* pt, double dr){
    double angle = point_angle(pt);
    return shift_point(pt, dr * cos(angle), dr * sin(angle));
}

/**
 * Rescales the radial coordinate of a point and rotates it around the origin.
 * @param pt    the point - will be modified by the operation
 * @param scale the factor to rescale by
 * @param rot   the angle to rotate by, in radians
 * @return  the scaled and rotated point.
 */
point* scale_rot_point(point* pt, double scale, double rot){
    pt->x = scale * (pt->x * cos(rot) - pt->y * sin(rot));
    pt->y = scale * (pt->y * cos(rot) + pt->x * sin(rot));
    return pt;
}

/**
 * Applies scale_rot_point using the magnitude and angle of another point. This
 * is equivalent to complex number multiplication.
 * @param pt        the point - will be modified by the operation
 * @param scale_rot the other point - is left unchanged
 * @return  the scaled and rotated point.
 */
point* scale_rot_by_point(point* pt, const point* scale_rot){
    return scale_rot_point(
            pt, point_magnitude(scale_rot), point_angle(scale_rot));
}

point* midpoint(const point* pt_1, const point* pt_2){
    return cartesian_point((pt_1->x + pt_2->x)/2, (pt_1->y + pt_2->y)/2);
}

/**
 * Deletes a point and frees up its memory.
 * @param pt    the point; may be NULL
 */
void delete_point(point* pt){
    free(pt);
}

/**
 * Deletes an array of points, freeing up their memory.
 * @param pts   the array; may be NULL
 * @param n     the number of points in the array
 */
void delete_points(point** pts, size_t n){
    if(!pts)
        return;
    
    for(size_t i = 0; i < n; i++)
        delete_point(pts[i]);
    
    free(pts);
}
