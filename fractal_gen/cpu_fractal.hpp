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
    explicit FractalLimits(const PixelPoint<size_t> dims, const T MIN = -1.2, const T MAX = 1.2)
        : DIMENSIONS(dims), MIN_LIMIT(MIN), MAX_LIMIT(MAX), LIMIT_DIFF(MAX-MIN)
    {}

    inline T offset_X(const T x_idx) { return MIN_LIMIT + x_idx * (LIMIT_DIFF / DIMENSIONS.col);   }
    inline T offset_Y(const T y_idx) { return MIN_LIMIT + y_idx * (LIMIT_DIFF / DIMENSIONS.row);   }
    inline T offset_Z(const T z_idx) { return MIN_LIMIT + z_idx * (LIMIT_DIFF / DIMENSIONS.depth); }

    const PixelPoint<size_t> DIMENSIONS;
    const T MIN_LIMIT;
    const T MAX_LIMIT;
    const T LIMIT_DIFF;
};


template <typename pixel_t>
std::tuple<bool, size_t> mandel_point(const PixelPoint<pixel_t> px_idx, const int order, const size_t num_iter) 
{
    PixelPoint<pixel_t> coords (0, 0, 0);

    size_t iter_num = 0;
    bool is_valid = true;
    for (; iter_num < num_iter; ++iter_num)
    {
        //get polar coordinates
        pixel_t r = coords.get_magnitude();
        pixel_t theta = order * std::atan2(std::sqrt(coords.row*coords.row + coords.col*coords.col), coords.depth);
        pixel_t phi = order * std::atan2(coords.row, coords.col);

        assert(!std::isnan(theta));
        assert(!std::isnan(phi));

        pixel_t r_factor = std::pow(r, order);
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

template <typename pixel_t>
void run_cpu_fractal(std::vector<pixel_t>& h_image_stack, const fractal_params& params)
{
		using fpixel_t = double;
    FractalLimits<fpixel_t> limits(PixelPoint<size_t>(params.imheight, params.imwidth, params.imdepth)); 

    cv::namedWindow("cpuslice", CV_WINDOW_AUTOSIZE);

    for (size_t z = 0; z < params.imdepth; ++z)
    {
        auto z_point = limits.offset_Z(z);
        const int slice_offset = params.imheight * params.imwidth * z;
        cv::Mat_<pixel_t> image = cv::Mat_<pixel_t>(params.imheight, params.imwidth, &h_image_stack[slice_offset]);
        image = cv::Mat_<pixel_t>::zeros(params.imheight, params.imwidth);
        for (size_t y = 0; y < params.imheight; ++y)
        {
            auto y_point = limits.offset_Y(y);
            for (size_t x = 0; x < params.imwidth; ++x)
            {
                auto x_point = limits.offset_X(x);

                bool is_valid;
                size_t iter_num;
                std::tie(is_valid, iter_num) = mandel_point<fpixel_t>
                    (PixelPoint<fpixel_t>(y_point,x_point,z_point), params.ORDER, params.MAX_ITER);   

                if(is_valid)
                {
                    image(y,x) = 255;
                    //cloud_indices.emplace_back(x, y, z);
                }
            }   
        }

        auto px_sum = cv::sum(cv::sum(image)) / 255;
        std::cout << "Image " << z << " Generated... has " << ((px_sum[0] > 0) ? std::to_string(px_sum[0]):"NO") << " non-zero elements" << std::endl;

        std::string cpuslice_fname {"cpuslice_" + std::to_string(z) + ".png"};
        cv::imwrite(cpuslice_fname, image);

        cv::Mat_<pixel_t> display_image = image;
        cv::imshow("cpuslice", display_image);
        cv::waitKey(10);
    }
}



}

#endif
