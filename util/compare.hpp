#ifndef FRACTALS_UTILS_COMPARE_HPP
#define FRACTALS_UTILS_COMPARE_HPP

#include <assert.h>

template <typename pixel_t>
void fractal2d_compare(const std::vector<data_t>& cpu_image_stack, const std::vector<data_t>& ocl_image_stack, int imheight, int imwidth)
{
  assert(cpu_image_stack.size() == ocl_image_stack.size());

	for (int slice = 0; slice < ocl_image_stack.size(); ++slice)
	{
		const pixel_t* cpu_slice = &cpu_image_stack[slice];
	  const pixel_t* ocl_slice = &ocl_image_stack[slice];
		int slice_diffs = 0;
    for (int row = 0; row < imheight; ++row)
		{
      for (int col = 0; col < imwidth; ++col)
			{
        if(cpu_slice[row*imwidth+col] != ocl_slice[row*imwidth+col])
          slice_diffs++;
			}
		}

		std::cout << "@ Slice " << slice << " -- " << slice_diffs << " #diffs" << std::endl;
	}
}

#endif
