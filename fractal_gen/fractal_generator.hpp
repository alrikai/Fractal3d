/* fractal_generator.hpp -- part of the fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */



#ifndef FRACTAL_GEN_FRACTALGENERATOR_HPP
#define FRACTAL_GEN_FRACTALGENERATOR_HPP

#include "util/fractal_helpers.hpp"

//used for comparison/ground truth purposes
#include "cpu_fractals/fractalgen3d.hpp"
#include <chrono>

template <template <class, class> class ptcloud_t, typename pt_t, typename pixel_t, int debug_run=0>
void make_pointcloud(const std::vector<pixel_t>& h_image_stack, const fractal_params& params, ptcloud_t<pt_t, pixel_t>& pt_cloud)
{
    auto start = std::chrono::high_resolution_clock::now();

	std::cout << "Making pointcloud..." <<std::endl;

	using cpu_data_t = float;
    cpu_fractals::FractalLimits<cpu_data_t> limits(cpu_fractals::PixelPoint<cpu_data_t>(params.imheight, params.imwidth, params.imdepth));

    for (int k = 0; k < params.imdepth; ++k)
    {
        auto z_point = limits.offset_Z(k);

        cv::Mat_<pixel_t> slice_diff = cv::Mat_<pixel_t>::zeros(params.imheight, params.imwidth);
        cv::Mat_<pixel_t> ocl_slice = cv::Mat_<pixel_t>::zeros(params.imheight, params.imwidth);
        cv::Mat_<pixel_t> cpu_slice = cv::Mat_<pixel_t>::zeros(params.imheight, params.imwidth);
      
        int h_image_stack_offset = params.imheight * params.imwidth * k;
        const pixel_t* h_image_slice = &h_image_stack[h_image_stack_offset];
        for (int i = 0; i < params.imheight; ++i)
        {
            auto y_point = limits.offset_Y(i);
            for (int j = 0; j < params.imwidth; ++j)
            {
                auto fractal_itval = h_image_slice[i*params.imwidth+j];
                if(fractal_itval == params.MAX_ITER-1) {
                    pt_cloud.emplace_back(j,i,k,fractal_itval);
				}

                if(debug_run)
                {
                    auto x_point = limits.offset_X(j);
                    bool is_valid;
                    size_t iter_num;
                    std::tie(is_valid, iter_num) = cpu_fractals::mandel_point<pixel_t, cpu_data_t>
                      (cpu_fractals::PixelPoint<cpu_data_t>(y_point,x_point,z_point), params.ORDER,params.MAX_ITER);  
                 
                    slice_diff(i,j) = iter_num - fractal_itval;  
                    cpu_slice(i,j) = iter_num;
                }

                if(fractal_itval == params.MAX_ITER-1) {
                    ocl_slice(i,j) = 255;
				}
            }
        }

     
        if(debug_run)
        {
            auto px_sum = cv::sum(cv::sum(ocl_slice)) / 255;
            std::cout << "Image " << k << " Generated... has " << ((px_sum[0] > 0) ? std::to_string(px_sum[0]):"NO") << " non-zero elements" << std::endl;

			auto diff_extrema = std::minmax_element(slice_diff.begin(), slice_diff.end());
            auto min_diff = *diff_extrema.first;
            auto max_diff = *diff_extrema.second;
            if(max_diff > 1 || min_diff < -1) {
                std::cout << "Slice " << k << " differed" << std::endl;
			}

            std::string slice_diff_name = "slice_diff_" + std::to_string(k);
            cv::imwrite((slice_diff_name + ".png"), cv::abs(slice_diff));
            cv::FileStorage slice_storage((slice_diff_name + ".yml"), cv::FileStorage::WRITE);
            slice_storage << slice_diff_name << slice_diff;
            slice_storage << "ocl slice" << ocl_slice;
            slice_storage << "cpu slice" << cpu_slice;
            slice_storage.release();  
            std::string cpu_slice_name = "cpu_slice_" + std::to_string(k) + ".png";
            cv::imwrite(cpu_slice_name, cpu_slice);

            std::string ocl_slice_name = params.fractal_name + "_ocl_slice_" + std::to_string(k) + ".png";
            cv::imwrite(ocl_slice_name, ocl_slice);
        }        
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end - start);
    std::cout << "Fractal Comparison Time: " << duration.count() << " ms" << std::endl;   
}


template <template <class, class> class generator_t, typename point_t, typename pixel_t>
class fractal_generator
{
public:
    fractal_generator()
      : fgenerator()
    {}

    inline fractal_data<point_t, pixel_t> make_fractal(fractal_params&& fractalgen_params)
    {
        std::vector<pixel_t> h_image_stack (fractalgen_params.imheight * fractalgen_params.imwidth * fractalgen_params.imdepth);
        std::fill(h_image_stack.begin(), h_image_stack.end(), 0);

        fgenerator.make_fractal(h_image_stack, fractalgen_params);
  
        fractal_data<point_t, pixel_t> fdata;
        fdata.params = fractalgen_params;
//-----------------------------------------------------------------------------------------------------------------------    
        make_pointcloud<fractal_types::pointcloud, point_t, pixel_t> (h_image_stack, fractalgen_params, fdata.point_cloud);

        long int ptsum = 0;
        std::for_each(fdata.point_cloud.cloud.begin(), fdata.point_cloud.cloud.end(), 
            [&ptsum](const fractal_types::fractal_point<point_t, pixel_t>& fpt)
            {
                ptsum += fpt.value;
            });
		std::cout << "NOTE: stack sum @fractal_generator is " << ptsum << std::endl;
        return fdata; 
    }

private:
    generator_t<point_t, pixel_t> fgenerator;
};

#endif
