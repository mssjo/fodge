/* 
 * File:   Point.h
 * Author: Mattias
 *
 * Created on 19 June 2019, 19:28
 */

#ifndef POINT_H
#define	POINT_H

#include "mf_fodge.h"

class Point {
public:
    Point() = default;
    Point(double x, double y = 0, 
            const Point& origin = Point());
    Point(const Point& orig) = default;
    virtual ~Point() = default;
    
    static Point polar(double radius, double angle, 
            const Point& origin = Point());
    static std::vector<Point> circle(double radius, int n_points, 
        const Point& origin = Point());
    
    double x() const;
    double y() const;
    
    double magnitude() const;
    static double distance(const Point& a, const Point& b = Point());
    
    static double angle(const Point& a, const Point& b, const Point& c);
    static double angle(const Point& a, const Point& b = Point());
    static bool collinear(const Point& a, const Point& b, 
        const Point& c = Point(), double ang_tol = PI/180);
    
    static double deg_to_rad(double angle);
    static double rad_to_deg(double angle);
    static double normalise_angle(double angle);
    static double angle_in_range(double angle, double min, double max, double incr = 2*PI);
    
    Point to(const Point& target, double sep) const;
    Point towards(const Point& target, double ratio = .5) const;
    
    friend Point operator+(const Point& p1, const Point& p2);
    friend Point& operator+=(Point& p1, const Point& p2);
    
    friend Point operator*(const Point& p, double scale);
    friend Point& operator*=(Point& p, double scale);
    
    friend bool operator==(const Point& p1, const Point& p2);
    friend bool operator!=(const Point& p1, const Point& p2);
    
    friend std::ostream& operator<<(std::ostream& out, const Point& p);
    
private:
    double _x;
    double _y;
};

#endif	/* POINT_H */

