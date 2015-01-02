#ifndef FRACTALS_HPP
#define FRACTALS_HPP

#include "util/fractal_helpers.hpp"

#include <thread>
#include <memory>

//@backend: fractal_data make_fractal(fractal_params&& fractalgen_parameters)

//@frontend: std::shared_ptr<FractalBufferType> get_fractalgenevt_buffer()
//           void display_fractal (fractal_data&&)

template <typename fractalgen_type, typename visualize_type>
class Fractals
{
public:
  Fractals(fractalgen_type* fractalgen, visualize_type* fractalvis)
    : fractal_backend(fractalgen), fractal_frontend(fractalvis), fractal_evtflag(false)
  {
    fractal_genbuffer = fractal_frontend->get_fractalgenevt_buffer();
    fractal_displaybuffer = fractal_frontend->get_fractaldispevt_buffer();
    
    fractal_thread = nullptr;
  }

  ~Fractals()
  {
/*
    int leftovers = fractal_genbuffer->read_available();
    if(leftovers > 0)
    {
      fractal_genbuffer->pop();
    }
*/    
    if(fractal_thread)
    {
      stop_fractals();
    }
  }

  inline void start_fractals()
  {
    //spawn a thread to check for new events (and take action if an event is present)
    fractal_evtflag.store(true);
    fractal_thread = std::unique_ptr<std::thread>(new std::thread(&Fractals<fractalgen_type, visualize_type>::fractal_evtloop, this));
  }

  inline void stop_fractals()
  {
    fractal_evtflag.store(false);
    fractal_thread->join();
    fractal_thread.reset(nullptr);
  }

private:

  void fractal_evtloop()
  {
    while(fractal_evtflag.load())
    {
      //TODO: use a condition variable to sleep when the queue is empty
      fractal_genevent fgen_evt;
			while(fractal_genbuffer->pop(fgen_evt))
      {
        //TODO: see what else could be needed here.... (e.g. pre-process anything, get the parameters into a different form, etc)
        auto fractalgen_parameters = fgen_evt.params;
        auto generated_fractal = fractal_backend->make_fractal(std::move(fractalgen_parameters));
        fractal_displaybuffer->push(generated_fractal);
      }
    }
  }

  //gen events for making new fractals
  typedef typename visualize_type::FractalBufferType FractalBufferType;
  std::shared_ptr<FractalBufferType> fractal_genbuffer;
  //events for displaying generated fractals
  typedef typename visualize_type::FractalDisplayBufferType FractalDisplayBufferType;
  std::shared_ptr<FractalDisplayBufferType> fractal_displaybuffer;

  std::unique_ptr<fractalgen_type> fractal_backend;
  std::unique_ptr<visualize_type> fractal_frontend;

  std::unique_ptr<std::thread> fractal_thread;  
  std::atomic<bool> fractal_evtflag;
};


#endif
