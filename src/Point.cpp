/* 
 * File:   Point.cpp
 * Author: Mattias
 * 
 * Created on 19 June 2019, 19:28
 */

#include "Point.hpp"

/**
 * @brief Simple constructor for a point.
 * 
 * @param x     the x coordinate.
 * @param y     the y coordinate.
 * @param origin the point relative to which the coordinates are measured; 
 *              defaults to <tt> (0,0) </tt>.
 */
Point::Point(double x, double y, const Point& origin) 
    : xcoord(x + origin.xcoord), ycoord(y + origin.ycoord) 
{}

/**
 * @brief Creates a point via polar coordinates.
 * 
 * @param radius the polar radius.
 * @param angle  the polar angle, in radians.
 * @param origin the point relative to which the coordinates are measured; 
 *               defaults to <tt> (0,0) </tt>.
 * @return the point specified by the coordinates.
 */
Point Point::polar(double radius, double angle, const Point& origin){
    return Point(radius * cos(angle), radius * sin(angle), origin);
}


/**
 * @brief Generates a number of points evenly spaced around a circle, 
 * thus forming the corners of a regular polygon.
 * 
 * @param radius    the radius of the circle.
 * @param n_points  the number of points. May be negative to generate points
 *                  going around the circle in the opposite direction.
 * @param origin    the center of the circle; defaults to the origin.
 * @param angle_offset  offsets the first point, effectively rotating the 
 *                  circle. Defaults to 0.
 * @return a vector containing the points in counterclockwise order, or 
 *          clockwise if @p n_points is negative.
 */
std::vector<Point> Point::circle(double radius, int n_points, 
        const Point& origin, double angle_offset){
    auto circle = std::vector<Point>();
    
    double incr = 2 * PI / n_points;
    if(n_points < 0){
        incr *= -1;
        n_points *= -1;
        angle_offset *= -1;
    }
    
    double angle = angle_offset;
    for(int i = 0; i < n_points; i++, angle += incr)
        circle.push_back(polar(radius, angle, origin));
    
    for(Point& pt : circle)
        assert(!std::isnan(pt.xcoord) && !std::isnan(pt.ycoord));
        
    return circle;
}


/**
 * @brief Retrieves the horizontal coordinate of a point.
 * @return a copy of the x coordinate.
 */
double Point::x() const {   return xcoord;   }
/**
 * @brief Retrieves the vertical coordinate of a point.
 * @return a copy of the y coordinate.
 */
double Point::y() const {   return ycoord;   }


/**
 * @brief Calculates the distance between a point and the origin, 
 *  i.e. its magnitude as a 2D vector.
 * @return the magnitude.
 */
double Point::magnitude() const{
    return hypot(xcoord, ycoord);
}

/**
 * @brief Calculates the distance between two points.
 * 
 * @param a one point.
 * @param b another point.
 * @return the Euclidean distance between them.
 */
double Point::distance(const Point& a, const Point& b){
    return hypot(a.xcoord - b.xcoord, a.ycoord - b.ycoord);
}

/**
 * @brief Calcultes the angle formed by three points.
 * 
 * @param a a point.
 * @param b another point, forming the center of the angle.
 * @param c yet another point.
 * @return the angle @c abc between the line @c ab and the line @c cb. 
 *      Alternatively, the difference in polar angle between @p a and @p c 
 *      relative to @p b. 
 */
double Point::angle(const Point& a, const Point& b, const Point& c){
    return normalise_angle(angle(a, b) - angle(c, b));
}

/**
 * @brief Calcultes the polar angle of one point relative to another.
 * 
 * @param a a point.
 * @param b another point, forming the center of the angle.
 * @return the angle between the line @c ab and the x axis
 *      Alternatively, the polar angle of @p a  
 *      relative to @p b. 
 */
double Point::angle(const Point& a, const Point& b){
    if(a == b)
        return std::nan("degenerate");
    
    double angle = atan2(a.ycoord - b.ycoord, a.xcoord - b.xcoord);    
    return normalise_angle(angle);
}

/**
 * @brief Tests if three points lie on a single line.
 * 
 * @param a     a point.
 * @param b     another point.
 * @param c     yet another point.
 * @param ang_tol the angle in radians within which the collinearity is allowed 
 *              to be inexact. A nonzero value is usually necessary due to 
 *              roundoff error.
 * @return @c true if the points are collinear within the tolerance, @c false
 *          if they are not.
 */
bool Point::collinear(const Point& a, const Point& b, const Point& c, 
            double ang_tol){
    
    if(a == b || b == c || c == a)
        return true;
    
    double ang = angle_in_range(angle(a, b, c), 0, PI, PI);
    
    return ang <= ang_tol || PI - ang <= ang_tol;
}


/**
 * @brief Converts an angle in degrees to radians.
 * 
 * @param angle the angle.
 * @return the angle multiplied by @f$ \pi/180 @f$.
 */
double Point::deg_to_rad(double angle){
    return angle * PI/180;
}
/**
 * @brief Converts an angle in radians to degrees.
 * 
 * @param angle the angle.
 * @return the angle multiplied by @f$ 180/\pi @f$.
 */
double Point::rad_to_deg(double angle){
    return angle * 180/PI;
}
/**
 * @brief Normalises an angle to be in the range @f$ [0,2\pi) @f$.
 * 
 * @param angle the angle.
 * @return the angle with a suitable integer multiple of @f$ 2\pi @f$ added.
 */
double Point::normalise_angle(double angle){
    assert(!std::isnan(angle));
    
    return angle_in_range(angle, 0, 2*PI);
}

/**
 * @brief Adjusts an angle so that it falls within a given range.
 * 
 * @param angle the angle.
 * @param min   the lower bound of the range.
 * @param max   the upper bound ot the range. If less than @p min, 
 *              multiples of @f$ 2\pi @f$ of are added until it isn't.
 * @param incr  the angle step to be used in the adjustments.
 * @return the adjusted angle, or @c nan if the adjustment is impossible.
 */
double Point::angle_in_range(double angle, double min, double max, double incr){
    while(min >= max)
        max += 2*PI;
    
    while(angle >= max)
        angle -= incr;
    while(angle < min)
        angle += incr;
        
    return (angle <= max) ? angle : std::nan("angle range");
}

/**
 * @brief Generates the endpoint of a line drawn from this point to within a 
 *      given separation of another point.
 * 
 * @param target    the other point.
 * @param sep       the separation from the other point.
 * @return the specified point.
 */
Point Point::to(const Point& target, double sep) const{
    double dist = distance(*this, target);
    return towards(target, dist != 0 ? (dist - sep)/dist : 1);
}

/**
 * @brief Generates a point partway between this point and another.
 * 
 * @param target    the other point.
 * @param ratio     the resulting point moves linearly along the line between 
 *                  the points as @p ratio goes from 0 to 1, with 0 bein this 
 *                  point and 1 being @p target. Values outside of this range 
 *                  are allowed to generate overshots.
 * @return the specified point.
 */
Point Point::towards(const Point& target, double ratio) const{
    return Point((1-ratio)*xcoord + ratio*target.xcoord, (1-ratio)*ycoord + ratio*target.ycoord);
}

/**
 * @brief Adds the coordinates of two points together.
 * 
 * @param p1 a point.
 * @param p2 another point.
 * @return the point whose coordinates are the sum of the points.
 *      Alternatively, the vector sum of the point.
 */
Point operator+(const Point& p1, const Point& p2){
    return Point(p1.xcoord + p2.xcoord, p1.ycoord + p2.ycoord);
}

/**
 * @brief Assigns <tt> p1 + p2 </tt> as the new value of @p p1.
 * 
 * @param p1 the point to be changed.
 * @param p2 the point to add to it.
 * @return a reference to the now-changed @p p1.
 */
Point& operator+=(Point& p1, const Point& p2){
    p1.xcoord += p2.xcoord;
    p1.ycoord += p2.ycoord;
    return p1;
}

/**
 * @brief Generates a point by rescaling the coordinates of another.
 * 
 * @param p     the point.
 * @param scale the scaling factor.
 * @return the rescaled point. Alternatively, the vector formed by scalar
 *          multiplication of @p p with @p scale.
 */
Point operator*(const Point& p, double scale){
    return Point(p.xcoord * scale, p.ycoord * scale);
}
/**
 * @brief Rescales the coordinates of a point.
 * 
 * @param scale the scaling factor.
 * @param p     the point.
 * @return the rescaled point. Alternatively, the vector formed by scalar
 *          multiplication of @p p with @p scale.
 */
Point operator*(double scale, const Point& p){
    return p * scale;
}

/**
 * @brief Assigns <tt> p * scale </tt> as the new value of @p p.
 * 
 * @param p     the point.
 * @param scale the scaling factor.
 * @return a reference to the now-changed @p p.
 */
Point& operator*=(Point& p, double scale){
    p.xcoord *= scale;
    p.ycoord *= scale;
    return p;
}

/**
 * @brief Rotates a point.
 * 
 * @param angle the angle of rotation.
 * @param ref the centre of rotation, defaults to the origin.
 * @return a reference to the rotated point.
 */
Point& Point::rotate(double angle, const Point& ref)
{
    double x = xcoord - ref.xcoord, y = ycoord - ref.ycoord;
    double s = std::sin(angle), c = std::cos(angle);
    
    xcoord = c*x - s*y + ref.xcoord;
    ycoord = s*x + c*y + ref.ycoord;
    
    return *this;
}


/**
 * @brief Makes a rotated copy of a point.
 * 
 * @param angle the angle of rotation.
 * @param ref the centre of rotation, defaults to the origin.
 * @return the rotated copy.
 */
Point Point::rotated(double angle, const Point& ref) const
{
    return Point(*this).rotate(angle, ref);
}


/**
 * @brief Compares two points for equality.
 * 
 * @param p1 a point.
 * @param p2 another point.
 * @return @c true if they are mathematically the same, @c false otherwise. 
 */
bool operator==(const Point& p1, const Point& p2){
    return p1.xcoord == p2.xcoord && p1.ycoord == p2.ycoord;
}
/**
 * @brief Compares two points for inequality.
 * 
 * @param p1 a point.
 * @param p2 another point.
 * @return @c true if they are mathematically different, @c false otherwise. 
 */
bool operator!=(const Point& p1, const Point& p2){
    return p1.xcoord != p2.xcoord || p1.ycoord != p2.ycoord;
}


/**
 * @brief Prints a point like <tt> (x, y) </tt>.
 * 
 * @param out   the stream to which the point should be printed.
 * @param p     the point.
 * @return the stream.
 */
std::ostream& operator<<(std::ostream& out, const Point& p){
    out << "(" << p.xcoord << ", " << p.ycoord << ")";
    return out;
}
