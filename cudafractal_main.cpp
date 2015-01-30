#include "fractal_gen/fractal_generator.hpp"
#include "fractal_gen/cuda_fractals/cudafractal_generator.hpp"
#include "visualize/ogre_vis/ogre_vis.hpp"
#include "fractals.hpp"

/* The main idea: the frontend will listen for user inputs, then tell the fractal class the 
 * user request. The fractal class will then dispatch to the backend to generate the user 
 * request, then send the result back to the frontend for visualization.
 */

int main()
{
  using data_t = float;
  using fpoint_t = fractal_types::point_type;
  using fractal_backend_t = fractal_generator<cudaFractals, fpoint_t, data_t>;
  using fractal_frontend_t = FractalOgre; 
  using fractal_t = Fractals<fractal_backend_t, fractal_frontend_t>;

  auto fgenerator = new fractal_backend_t ();

	const float rotate_magnitude = 0.20f;
	const float pan_magnitude = 10.0f;
	auto fractal_viewer = new fractal_frontend_t (rotate_magnitude, pan_magnitude);
  fractal_t fractal_maker (fgenerator, fractal_viewer);

  //async call
  fractal_maker.start_fractals();
  //non-async call
  fractal_viewer->start_display();

  std::cout << "All Done" << std::endl;
  //kill the backend too
  fractal_maker.stop_fractals();
}
