# Fractal3d

This project generated point clouds of 3D (pseudo) fractals and displays them. 
I wrote it in C++, and it uses OpenCL and CUDA acceleration (selectable at compile time) for the point cloud generation. 
I use Ogre3D for the display, although it's possible to use VTK as well (or at least, it used to be, haven't tried using VTK in a while).

The problem with the current implementation is that I am using 'triplex' numbers as my 3D coordinates (1 real number, 2 complex numbers); these don't have a well-defined algebra however, so a move to using quaternions instead is in the works. 

Here's a brief screengrab of the output, captured with ffmpeg and converted to a gif. This was generated using the mandelbrot equation in 3D. The point cloud vertices are coloured such that inner points are darker and outer points are lighter. 

![f_v8_4.gif](https://bitbucket.org/repo/GypoKq/images/3916095333-f_v8_4.gif)
