
#include "Diagram.h"
#include "DiagramNode.h"

#define BASE_RADIUS .4
#define RADIUS_INCR .05
#define BEND_RADIUS "4pt" 
#define VERT_OFFS .12

#define SINGLET_APPROACH_RATIO .6
#define INTSCT_MARGIN .5

#define GAP_BONUS PI
#define COLL_TOL PI/180
#define ROUND_SEP .3

#define LINE "thick"
#define SINGLET "thick,dashed"

void Diagram::TikZ(std::ostream& tikz, double radius, int index) const{
    if(radius <= 0)
        radius = BASE_RADIUS + n_legs * RADIUS_INCR;
    
    tikz << std::setprecision(3) << std::fixed << std::uppercase;
    
    if(index >= 0)
        tikz    << "%%% [" << index 
                << "] O(p^" << order << ") " 
                << n_legs << "-point diagram" << std::endl;
    
    tikz << "\\begin{tikzpicture}[>=stealth]" << std::endl;
    tikz << "\t\\draw[black!50] (0,0) circle[radius=" << radius << "];" << std::endl;
    
    auto points = std::unordered_map<mmask, Point>();
    int idx = 0;
    while(!root.def_TikZ(Point::circle(radius, n_legs), &idx, points));
    //assert(idx == n_legs);
    
    root.adjust_TikZ(/*tikz,*/ points, radius);
//    
//    for(auto key_val : points){
//        tikz    << "\t\t\\draw[red] " << key_val.second << " circle[radius=.05]"
//                << " node[anchor=south] {\\tiny ";
//        if(bitwise::bitcount(key_val.first) == 1)
//            tikz << bitwise::unshift(key_val.first);
//        else
//            bitwise::print_bits(key_val.first, 0, tikz);
//        tikz << "};\n";
//    }
        
    root.draw_TikZ(tikz, points);
    
    tikz << "\\end{tikzpicture}" << std::endl;
}

bool DiagramNode::def_TikZ( 
        const std::vector<Point>& perimeter, int* idx,
        std::unordered_map<mmask, Point>& points, mmask parent_key) const
{
    
    if(is_leaf){
        Point pt = perimeter[(*idx)++];
        points.insert(std::make_pair(momenta, pt));
        
        return true;
    }
    
    Point pt = Point();
    int count = 0;
    bool subtree_done = true;
    bool self_done = (points.find(momenta) != points.end());
    for(const FlavourTrace& tr : traces){
        for(const DiagramNode& leg : tr.legs){
           
            if(!self_done){
                auto it = points.find(leg.momenta);
                
                if(leg.is_leaf){
                    if(it == points.end()){
                        leg.def_TikZ(perimeter, idx, points, momenta);
                        pt += points.at(leg.momenta);
                    }
                    else
                        pt += (*it).second;
                    
                    count++;
                }            
                else if(it != points.end()){
                    pt += (*it).second;
                    count++;
                }
            }
                
            if(!leg.is_leaf)
                subtree_done = leg.def_TikZ(perimeter, idx, points, momenta)
                        && subtree_done;
        }
    }  
    
    if(!is_root){
        auto it = points.find(parent_key);
        if(it != points.end()){
            pt += (*it).second;
            count++;
        }
    }
    
    if(count > 0 && !self_done){
        pt *= 1./(n_legs + (is_root ? 0 : 1) + 1);
        points.insert(std::make_pair(momenta, pt));
        return subtree_done;
    }
        
    return self_done && subtree_done;
}

void DiagramNode::adjust_TikZ(/*std::ostream& tikz,*/ 
        std::unordered_map<mmask,Point>& points, 
        double radius, mmask parent_key) const
{
    if(is_leaf)
        return;
       
    Point pt = points.at(momenta);
    double ang_i, ang_f, ang_diff, ang_avg;
    
    if(traces.size() > 1){
        int idx = 0;
        for(const FlavourTrace& tr : traces){
            bool incl_parent = (idx == 0 && !is_root);
            Point& i_pt = points.at(
                    incl_parent ? parent_key : tr.legs.front().momenta);
            
            ang_i = Point::angle(i_pt, pt);
            ang_f = Point::angle(points.at(tr.legs.back().momenta), pt);
            
            ang_diff = Point::normalise_angle(ang_f - ang_i);
            ang_avg  = Point::angle_in_range((ang_f + ang_i)/2, ang_i, ang_f, PI);

            if(ang_diff > PI){
//                tikz << "\t\\draw[thin,red] " 
//                        << i_pt
//                        << " -- " << pt
//                        << " -- " << points.at(tr.legs.back().momenta)
//                        << ";" << std::endl;
//                tikz << "\t\\draw[thin,red,->] "
//                        << pt << " -- " 
//                        << Point::polar(radius, ang_avg, pt)
//                        << ";" << std::endl;

                compress_points(points, pt, momenta, tr.momenta, incl_parent, 
                        ang_avg, ang_diff/PI, radius);
            }
            
            idx++;
        }
    }
    
    for(const FlavourTrace& tr : traces){
        for(const DiagramNode& leg : tr.legs){
            leg.adjust_TikZ(/*tikz,*/ points, radius, momenta);
        }
    }
}

void DiagramNode::compress_points(
        std::unordered_map<mmask,Point>& points, const Point& ref,
        mmask key, mmask sub_key, bool incl_parent, 
        double mid_angle, double compression, double radius)
{
    std::cout << "Adjusting points..." << std::endl;
    
    for(auto& key_val : points){
        //Only adjusts children of the specific trace (i.e. points whose keys
        //are subsets of sub_key)
        //If incl_parent, also adjusts what would be children if the node's
        //parent were its child (i.e. points whose keys are not subsets of
        //this node's key)
        bool adjust =  SUBSET(key_val.first, sub_key) 
                    || (incl_parent && !SUBSET(key_val.first, key));
        //Also ignores the point itself
        if(!adjust || (key_val.first == key))
            continue;
        
        key_val.second = compress_point(ref, Point::angle(key_val.second, ref),
                mid_angle, compression, radius);
    }
}

Point DiagramNode::compress_point(const Point& ref,
        double angle, double mid_angle, double compression, double radius){
    
    double diff = Point::angle_in_range(angle - mid_angle, -PI, PI);
    angle = mid_angle + (diff / compression);
    
    //Constructs unit vector (c,s) = (cos('angle'), sin('angle')), and 
    //then rescales it so that it reaches from ref = (x,y) to a distance 
    //'radius' from origin
    // [ a (c,s) + (x,y) ]^2 = r^2 
    // -> a^2 + 2a[cx+sy] + [xx+yy] = r^2
    // -> a = -[cx+sy] + sqrt([cx+sy]^2 + r^2 - [xx+yy])
    // (choosing positive solution)
    double cxsy = cos(angle) * ref.x() + sin(angle) * ref.y();
    double magn = ref.magnitude();
    double scale = sqrt(cxsy*cxsy + radius*radius - magn*magn) - cxsy;
    
    if(std::isnan(scale + angle)){
        std::cerr << "radius: " << radius << ", magn: " << magn << ", cxsy: " << cxsy << std::endl;
        assert(0);
    }
    
    return Point::polar(scale, angle, ref);
}

#define ENCOMP_NAME(m) "p" << std::hex << m << std::dec
#define INTSCT_NAME(m,n) "p" << std::hex << m << "x" << n << std::dec

Point DiagramNode::draw_TikZ(
        std::ostream& tikz,
        const std::unordered_map<mmask, Point> points,
        mmask parent_key) const
{
    if(is_leaf){
//        tikz << "\t\\draw[red] " << points.at(momenta) << " circle[radius=.05];\n";
        return points.at(momenta);
    }
    
    vertex_order_TikZ(tikz, points, parent_key);
    
    Point this_pt = points.at(momenta);
//    if(is_root)
//        tikz << "\t\\draw[red] " << this_pt << " circle[radius=.05];\n";
    
    //Non-split vertex: print lines towards all children
    if(traces.size() == 1){        
        for(const DiagramNode& leg : traces[0].legs){
            tikz    << "\t\\draw[" 
                    << (leg.is_singlet ? SINGLET : LINE)
                    << "] " << this_pt << " -- " 
                    << leg.draw_TikZ(tikz, points, momenta)
                    << ";\n";
        }
        
        return this_pt;
    }
    //Split vertex: iterate through all traces, and do the following:
    Point return_pt;
    for(FlavourTrace tr : traces){
        //First, define a line that will encompass the trace.
        //Beginning of line: parent if connected, otherwise first leg of trace
        Point begin = tr.connected ? points.at(parent_key) 
                : tr.legs.front().draw_TikZ(tikz, points, momenta);
        //End of line: last leg of trace
        Point end = tr.legs.back().draw_TikZ(tikz, points, momenta);
        
        //If beginning and end are collinear, there is no need to mess with
        //curved lines -- just draw all lines as if drawing a non-split vertex!
        //(Drawing line to parent is left to the parent.)
        if(Point::collinear(begin, this_pt, end, COLL_TOL)){ 
            for(int i = 0; i < tr.legs.size(); i++){
                Point target_pt;
                if(i == 0 && !tr.connected)
                    target_pt = begin;
                else if(i == tr.legs.size() - 1)
                    target_pt = end;
                else
                    target_pt = tr.legs[i].draw_TikZ(tikz, points, momenta);
                
                tikz    << "\t\\draw["
                        << (tr.legs[i].is_singlet ? SINGLET : LINE)
                        << "] " << this_pt << " -- " << target_pt
                        << ";\n";
            }

            if(tr.connected)
                return_pt = this_pt;
        }
        //Otherwise: draw an encompassing line from begin to end curving via
        //this point, and draw other lines intersecting it.
        else{
            //True if the entire encompassing line is a singlet
            bool fully_singlet = tr.legs.back().is_singlet 
                && ((tr.connected && is_singlet) 
                    || (!tr.connected && tr.legs.front().is_singlet));

            //Draws the curved segment. Names it using momenta as key in case
            //intersecting lines are needed.
            tikz    << "\t\\draw[name path=" << ENCOMP_NAME(tr.momenta)
                    << ", " << (fully_singlet ? SINGLET : LINE)
                    << "] "
                    << begin.to(this_pt, ROUND_SEP)
                    << " .. controls " << this_pt << " .. "
                    << end.to(this_pt, ROUND_SEP)
                    << ";\n";
                    
            //Draws other lines, using the intersections library
            for(int i = 0; i < tr.legs.size(); i++){
                Point source_pt;
                //Draws lines from begin or end specially: they attach to the
                //end of the curve, rather than intersecting it.
                //(Drawing line to parent is left to the parent.)
                if(i == tr.legs.size() - 1 || (i == 0 && !tr.connected)){
                    //Beware corner case when 0 == tr.legs.size() - 1 !!!
                    source_pt = (i == tr.legs.size() - 1) ? end : begin;
                    tikz    << "\t\\draw["
                            << (tr.legs[i].is_singlet ? SINGLET : LINE)
                            << "] "
                            << source_pt << " -- " 
                            << source_pt.to(this_pt, ROUND_SEP)
                            << ";\n";
                }
                else{
                    source_pt = tr.legs[i].draw_TikZ(tikz, points, momenta);
                    tikz    << "\t\\path[name path=" 
                            << INTSCT_NAME(tr.momenta, tr.legs[i].momenta)
                            << "] " << this_pt << " -- " << source_pt
                            << ";\n";
                    tikz    << "\t\\draw[name intersections={of="
                            << ENCOMP_NAME(tr.momenta)
                            << " and "
                            << INTSCT_NAME(tr.momenta, tr.legs[i].momenta)
                            << "}, "
                            << (tr.legs[i].is_singlet ? SINGLET : LINE)
                            << "] "
                            << source_pt << " -- (intersection-1)"
                            << ";\n";
                }
            }
            
            if(tr.connected)
                return_pt = begin.to(this_pt, ROUND_SEP);
        }
    }
    
    return return_pt;    
}

void DiagramNode::vertex_order_TikZ(
        std::ostream& tikz,
        const std::unordered_map<mmask, Point> points,
        mmask parent_key) const
{
    if(is_leaf || order == 2)
        return;
    
    Point pt = points.at(momenta);
    std::vector<std::pair<double, bool>> angles_gaps 
        = std::vector<std::pair<double, bool>>();
    
    if(!is_root)
        angles_gaps.push_back(std::make_pair(
                Point::angle(points.at(parent_key), pt),
                false 
        ));
    
    for(const FlavourTrace& tr : traces){
        for(const DiagramNode& leg : tr.legs){
            angles_gaps.push_back(std::make_pair(
                    Point::angle(points.at(leg.momenta), pt),
                    false
            ));
        }
        angles_gaps.back().second = (traces.size() > 1);
    }
        
    std::sort(angles_gaps.begin(), angles_gaps.end());
//    for(int i = 0; i < angles_gaps.size(); i++){
//        tikz    << "\t\\draw[thin,red,->] " << pt << " -- "
//                << Point::polar(.5, angles_gaps[i], pt)
//                << "node {\\tiny " << i << "};\n";
//    }
    double max_angle = Point::normalise_angle(
        angles_gaps.front().first - angles_gaps.back().first);
    if(angles_gaps.back().second)
        max_angle += GAP_BONUS;
    
    double angle;
    int max_idx = angles_gaps.size() - 1;
    for(int i = 0; i < angles_gaps.size() - 1; i++){
        angle = angles_gaps[i+1].first - angles_gaps[i].first;
        if(angles_gaps[i].second)
            angle += GAP_BONUS;
        
        if(angle > max_angle){
            max_angle = angle;
            max_idx = i;
        }
    }
    if(angles_gaps[max_idx].second)
        max_angle -= GAP_BONUS;
    
    double hmax = max_angle / 2;
    double offs = VERT_OFFS / sin(hmax);
    
    assert(!std::isnan(offs + hmax));
    
    tikz    << "\t\\draw"
           // << "[thin, red] " << pt << " -- " 
            << Point::polar(offs, angles_gaps[max_idx].first + hmax, pt)
           // << " circle [radius=" << VERT_OFFS << "]"
            << " node [anchor=center] {\\ordidx " << order << "}"
            << ";\n";
    
    for(const FlavourTrace& tr : traces){
        for(const DiagramNode& leg : tr.legs){
            leg.vertex_order_TikZ(tikz, points, momenta);
        }
    }
}


