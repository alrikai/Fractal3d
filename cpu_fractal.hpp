
#include <opencv2/opencv.hpp>

#include <tuple>

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


template <typename PixelType, size_t NUM_ITER>
std::tuple<bool, size_t> mandel_point(const PixelPoint<PixelType> px_idx, const int order) 
{
    PixelPoint<PixelType> coords (0, 0, 0);

    size_t iter_num = 0;
    bool is_valid = true;
    for (; iter_num < NUM_ITER; ++iter_num)
    {
        //get polar coordinates
        PixelType r = coords.get_magnitude();
        PixelType theta = order * std::atan2(std::sqrt(coords.row*coords.row + coords.col*coords.col), coords.depth);
        PixelType phi = order * std::atan2(coords.row, coords.col);

        assert(!std::isnan(theta));
        assert(!std::isnan(phi));

        PixelType r_factor = std::pow(r, order);
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


}
