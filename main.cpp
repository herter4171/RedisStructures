#include <iostream>

#include <boost/geometry/geometry.hpp>

#ifndef NUM_DIMS
#define NUM_DIMS 3
#endif

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int main()
{
	typedef boost::geometry::model::point<double, NUM_DIMS, boost::geometry::cs::cartesian> point_bg;

	point_bg pt(0.1, 1.5, 2.6);
	int size = sizeof(pt);
	std::cout << "Point Size" << sizeof(pt) << std::endl;
    return 0;
}