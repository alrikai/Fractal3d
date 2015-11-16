/* cpufractal_main.cpp -- part of the fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "fractal_gen/fractal_generator.hpp"
#include "fractal_gen/cpu_fractals/cpufractal_generator.hpp"
#include "visualize/ogre_vis/ogre_vis.hpp"
#include "fractals.hpp"

int main()
{
    using pixel_t = unsigned char;
    using fpoint_t = fractal_types::point_type;
    using fractal_backend_t = fractal_generator<cpuFractals, fpoint_t, pixel_t>;
    using fractal_frontend_t = FractalOgre<pixel_t>; 
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
