#ifndef UTIL_FRACTAL_HELPERS_HPP
#define UTIL_FRACTAL_HELPERS_HPP

#include <string>
#include <vector>

namespace fractal_types
{
struct point_type
{
  point_type()
    : x(0), y(0), z(0)
  {}

  point_type(int x_coord, int y_coord, int z_coord)
    : x(x_coord), y(y_coord), z(z_coord)
  {}

  int x, y, z;
};

template <typename point_t, typename data_t>
struct fractal_point : public point_t
{
  fractal_point()
    : point_t(), value(0)
  {}

  fractal_point(int x_coord, int y_coord, int z_coord, data_t val)
    : point_type(x_coord, y_coord, z_coord), value(val)
  {}
  
  data_t value;
};

template <typename point_t, typename data_t = int32_t>
struct pointcloud
{
	typedef fractal_point<point_t, data_t> cloud_point_t;
  void push_back(fractal_point<point_t, data_t> pt)
  {
    cloud.push_back(pt);
  }

	template <typename ... Args>
	void emplace_back(Args&& ... args)
	{
    cloud.emplace_back(std::forward<Args>(args)...);
	}

  std::vector<fractal_point<point_t, data_t>> cloud; 
};
} //namespace fractal_types

struct fractal_params
{
  int imheight;
  int imwidth;
  int imdepth;

  static constexpr int MAX_ITER = 80;
  static constexpr int ORDER = 8;

  float MIN_LIMIT;
  float MAX_LIMIT;
  float BOUNDARY_VAL;

  std::string fractal_name;
};

//holds the user input for fractal generation
struct fractal_genevent
{
  fractal_genevent()
  {}

  fractal_genevent(fractal_params fparams)
    : params(fparams)
  {}

  fractal_params params;
};

//holds the generated fractal data
template <typename point_t, typename data_t = int32_t>
struct fractal_data
{
  fractal_types::pointcloud<point_t, data_t> point_cloud;
	fractal_params params;
};

#endif
