#ifndef UTIL_FRACTAL_HELPERS_HPP
#define UTIL_FRACTAL_HELPERS_HPP

#include <string>
#include <vector>

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
struct fractal_data
{
  std::vector<std::vector<float>> point_cloud;
	fractal_params params;
};

#endif
