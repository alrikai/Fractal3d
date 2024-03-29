/* fractalgen3d.hpp -- part of the CPU fractal3d implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#ifndef CPU_FRACTAL_HPP
#define CPU_FRACTAL_HPP
#include <opencv2/opencv.hpp>

#include <tuple>

#include "util/fractal_helpers.hpp"

namespace cpu_fractals
{

template <typename T>
struct PixelPoint
{
    PixelPoint(T y, T x, T z)
        : row(y), col(x), depth(z)
    {}

    inline T get_magnitude() { return std::sqrt(row*row + col*col + depth*depth); }
    inline void add_point(const PixelPoint& other)
    {
        row += other.row;
        col += other.col;
        depth += other.depth;
    }

    T row;
    T col;
    T depth;
};

template <typename T>
struct FractalLimits
{
    explicit FractalLimits(const PixelPoint<T> dims, const T MIN = -1.2, const T MAX = 1.2)
        : DIMENSIONS(dims), MIN_LIMIT(MIN), MAX_LIMIT(MAX), LIMIT_DIFF(MAX-MIN)
    {}

    inline T offset_X(const T x_idx) { return MIN_LIMIT + x_idx * (LIMIT_DIFF / DIMENSIONS.col);   }
    inline T offset_Y(const T y_idx) { return MIN_LIMIT + y_idx * (LIMIT_DIFF / DIMENSIONS.row);   }
    inline T offset_Z(const T z_idx) { return MIN_LIMIT + z_idx * (LIMIT_DIFF / DIMENSIONS.depth); }

    const PixelPoint<T> DIMENSIONS;
    const T MIN_LIMIT;
    const T MAX_LIMIT;
    const T LIMIT_DIFF;
};


template <typename pixel_t, typename data_t>
std::tuple<bool, pixel_t> mandel_point(const PixelPoint<data_t> px_idx, const int order, const size_t num_iter) 
{
    PixelPoint<data_t> coords (0, 0, 0);

    pixel_t iter_num = 0;
    bool is_valid = true;
    for (; iter_num < num_iter; ++iter_num)
    {
        //get polar coordinates
        data_t r = coords.get_magnitude();
        data_t theta = order * std::atan2(std::sqrt(coords.row*coords.row + coords.col*coords.col), coords.depth);
        data_t phi = order * std::atan2(coords.row, coords.col);

        assert(!std::isnan(theta));
        assert(!std::isnan(phi));

        data_t r_factor = std::pow(r, order);
        coords.col = r_factor * std::sin(theta) * std::cos(phi);
        coords.row = r_factor * std::sin(theta) * std::sin(phi);
        coords.depth = r_factor * std::cos(theta);

        coords.add_point(px_idx);
        if(coords.get_magnitude() > 2)
        {
            is_valid = false;
            break;
        }
    }
    return std::make_tuple(is_valid, iter_num);
}

template <typename pixel_t, typename data_t>
std::tuple<bool, pixel_t> mandel_quaternion_point(const PixelPoint<data_t> px_idx, const int order, const size_t num_iter) 
{
    
}

template <typename pixel_t>
void run_cpu_fractal(std::vector<pixel_t>& h_image_stack, const fractal_params& params)
{
    using fpixel_t = float;
    FractalLimits<fpixel_t> limits(PixelPoint<fpixel_t>(params.imheight, params.imwidth, params.imdepth)); 

    cv::namedWindow("cpuslice", CV_WINDOW_AUTOSIZE);

    for (size_t z = 0; z < params.imdepth; ++z)
    {
        auto z_point = limits.offset_Z(z);
        const int slice_offset = params.imheight * params.imwidth * z;
        cv::Mat_<pixel_t> image = cv::Mat_<pixel_t>(params.imheight, params.imwidth, &h_image_stack[slice_offset]);
        //image = cv::Mat_<pixel_t>::zeros(params.imheight, params.imwidth);
        //std::fill(image.begin(), image.end(), 0);
        for (size_t y = 0; y < params.imheight; ++y)
        {
            auto y_point = limits.offset_Y(y);
            for (size_t x = 0; x < params.imwidth; ++x)
            {
                auto x_point = limits.offset_X(x);

                bool is_valid;
                size_t iter_num;
                std::tie(is_valid, iter_num) = mandel_point<pixel_t, fpixel_t>
                    (PixelPoint<fpixel_t>(y_point,x_point,z_point), params.ORDER, params.MAX_ITER);   

                if(is_valid)
                {
                    image(y,x) = params.MAX_ITER-1; 
                    //cloud_indices.emplace_back(x, y, z);
                }
            }   
        }

        bool debug_mode = false;
        if(debug_mode)
        {
            auto px_sum = cv::sum(cv::sum(image)) / static_cast<float>(params.MAX_ITER-1);
            std::cout << "Image " << z << " Generated... has " << ((px_sum[0] > 0) ? std::to_string(px_sum[0]):"NO") << " non-zero elements" << std::endl;

            std::string cpuslice_fname {"cpuslice_" + std::to_string(z) + ".png"};
            cv::imwrite(cpuslice_fname, image);

            cv::Mat_<pixel_t> display_image = image;
            cv::imshow("cpuslice", display_image);
            cv::waitKey(10);
        }
    }
}


//EXPERIMENTAL: want to try generating 3D fractals using quaternion coordinates, as that's 
//a more well-behaved / complete algebra than these chimeric triplex numbers 
template <typename pixel_t>
void run_cpu_fractal_quaternion (std::vector<pixel_t>& h_image_stack, const fractal_params& params)
{



}


}

#endif
