#include "fractal_gen/fractal_generator.hpp"
#include "visualize/ogre_vis/ogre_vis.hpp"
#include "fractals.hpp"

/* The main idea: the frontend will listen for user inputs, then tell the fractal class the 
 * user request. The fractal class will then dispatch to the backend to generate the user 
 * request, then send the result back to the frontend for visualization.
 */

int main()
{
  using fractal_backend_t = oclFractals;
  using fractal_frontend_t = FractalOgre; 
  using fractal_t = Fractals<fractal_backend_t, fractal_frontend_t>;

  auto fractal_generator = new fractal_backend_t ();

	const float rotate_magnitude = 0.10f;
	const float pan_magnitude = 1.0f;
	auto fractal_viewer = new fractal_frontend_t (rotate_magnitude, pan_magnitude);
  fractal_t fractal_maker (fractal_generator, fractal_viewer);

  //async call
  fractal_maker.start_fractals();
  //non-async call
  fractal_viewer->start_display();

  std::cout << "All Done" << std::endl;
  //kill the backend too
  fractal_maker.stop_fractals();
}
