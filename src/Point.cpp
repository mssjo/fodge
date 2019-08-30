/* 
 * File:   Point.cpp
 * Author: Mattias
 * 
 * Created on 19 June 2019, 19:28
 */

#include "Point.hpp"

Point::Point(double x, double y, const Point& origin) 
    : _x(x + origin._x), _y(y + origin._y) 
{
    if(std::isnan(x) || std::isnan(y)){
        std::cerr << "NaN!";
    }
}

Point Point::polar(double radius, double angle, const Point& origin){
    return Point(radius * cos(angle), radius * sin(angle), origin);
}

std::vector<Point> Point::circle(double radius, int n_points, 
        const Point& origin){
    auto circle = std::vector<Point>();
    
    double incr = 2 * PI / n_points;
    if(n_points < 0){
        incr *= -1;
        n_points *= -1;
    }
    
    double angle = 0;
    for(int i = 0; i < n_points; i++, angle += incr)
        circle.push_back(polar(radius, angle, origin));
        
    return circle;
}

double Point::x() const {   return _x;   }
double Point::y() const {   return _y;   }

double Point::magnitude() const{
    return hypot(_x, _y);
}

double Point::distance(const Point& a, const Point& b){
    return hypot(a._x - b._x, a._y - b._y);
}

double Point::angle(const Point& a, const Point& b, const Point& c){
    return normalise_angle(angle(a, b) - angle(c, b));
}

double Point::angle(const Point& a, const Point& b){
    return normalise_angle(atan2(a._y - b._y, a._x - b._x));
}

bool Point::collinear(const Point& a, const Point& b, const Point& c, 
            double ang_tol){
    
    double ang = angle_in_range(angle(a, b, c), 0, PI, PI);
    
    return ang <= ang_tol || PI - ang <= ang_tol;
}

double Point::deg_to_rad(double angle){
    return angle * PI/180;
}
double Point::rad_to_deg(double angle){
    return angle * 180/PI;
}
double Point::normalise_angle(double angle){
    assert(!std::isnan(angle));
    
    return angle_in_range(angle, 0, 2*PI);
}
double Point::angle_in_range(double angle, double min, double max, double incr){
    while(min >= max)
        max += 2*PI;
    
    while(angle >= max)
        angle -= incr;
    while(angle < min)
        angle += incr;
    
    return angle;//(angle >= min && angle < max) ? angle : std::nan("");
}

Point Point::to(const Point& target, double sep) const{
    double dist = distance(*this, target);
    return towards(target, dist != 0 ? (dist - sep)/dist : 1);
}

Point Point::towards(const Point& target, double ratio) const{
    return Point((1-ratio)*_x + ratio*target._x, (1-ratio)*_y + ratio*target._y);
}

Point operator+(const Point& p1, const Point& p2){
    return Point(p1._x + p2._x, p1._y + p2._y);
}

Point& operator+=(Point& p1, const Point& p2){
    p1._x += p2._x;
    p1._y += p2._y;
    return p1;
}

Point operator*(const Point& p, double scale){
    return Point(p._x * scale, p._y * scale);
}

Point& operator*=(Point& p, double scale){
    p._x *= scale;
    p._y *= scale;
    return p;
}

bool operator==(const Point& p1, const Point& p2){
    return p1._x == p2._x && p1._y == p2._y;
}

bool operator!=(const Point& p1, const Point& p2){
    return p1._x != p2._x || p1._y != p2._y;
}

std::ostream& operator<<(std::ostream& out, const Point& p){
    out << "(" << p._x << ", " << p._y << ")";
    return out;
}
