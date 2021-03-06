/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


/*! \file for_each.inl
 *  \brief Inline file for for_each.h.
 */

#include <hydra/detail/external/thrust/detail/config.h>
#include <hydra/detail/external/thrust/for_each.h>
#include <hydra/detail/external/thrust/distance.h>
#include <hydra/detail/external/thrust/system/cuda/detail/bulk.h>
#include <hydra/detail/external/thrust/detail/function.h>
#include <hydra/detail/external/thrust/detail/seq.h>
#include <hydra/detail/external/thrust/system/cuda/detail/execute_on_stream.h>


HYDRA_EXTERNAL_NAMESPACE_BEGIN  namespace thrust
{
namespace system
{
namespace cuda
{
namespace detail
{
namespace for_each_n_detail
{


struct for_each_kernel
{
  template<typename Iterator, typename Function, typename Size>
  __hydra_host__ __hydra_device__
  void operator()(bulk_::parallel_group<bulk_::concurrent_group<> > &grid, Iterator first, Function f, Size n)
  {
    Size grid_size = grid.size() * grid.this_exec.size();

    Size i = grid.this_exec.index() * grid.this_exec.size() + grid.this_exec.this_exec.index();

    first += i;

    while(i < n)
    {
      f(*first);
      i += grid_size;
      first += grid_size;
    }
  }
};


template<typename Size>
__hydra_host__ __hydra_device__
bool use_wide_counter(Size n, unsigned int narrow_grid_size)
{
  // use the wide counter when n will not fit within an unsigned int
  // or if incrementing an unsigned int by narrow_grid_size would overflow
  // the counter
  Size threshold = static_cast<Size>(UINT_MAX);

  bool result = (sizeof(Size) > sizeof(unsigned int)) && (n > threshold);

  if(!result)
  {
    // check if we'd overflow the little closure's counter
    unsigned int narrow_n = static_cast<unsigned int>(n);

    if((narrow_n - 1u) + narrow_grid_size < narrow_n)
    {
      result = true;
    }
  }

  return result;
}


} // end for_each_n_detail


template<typename DerivedPolicy,
         typename RandomAccessIterator,
         typename Size,
         typename UnaryFunction>
__hydra_host__ __hydra_device__
RandomAccessIterator for_each_n(execution_policy<DerivedPolicy> &exec,
                                RandomAccessIterator first,
                                Size n,
                                UnaryFunction f)
{
  // we're attempting to launch a kernel, assert we're compiling with nvcc
  // ========================================================================
  // X Note to the user: If you've found this line due to a compiler error, X
  // X you need to compile your code using nvcc, rather than g++ or cl.exe  X
  // ========================================================================
  HYDRA_THRUST_STATIC_ASSERT( (thrust::detail::depend_on_instantiation<RandomAccessIterator, HYDRA_THRUST_DEVICE_COMPILER == HYDRA_THRUST_DEVICE_COMPILER_NVCC>::value) );

  struct workaround
  {
    __hydra_host__ __hydra_device__
    static RandomAccessIterator parallel_path(execution_policy<DerivedPolicy> &exec, RandomAccessIterator first, Size n, UnaryFunction f)
    {
      thrust::detail::wrapped_function<UnaryFunction,void> wrapped_f(f);

      // opportunistically narrow the type of n

      unsigned int narrow_n = static_cast<unsigned int>(n);
      unsigned int narrow_num_groups = 0;
      unsigned int narrow_group_size = 0;

      // automatically choose a number of groups and a group size
      thrust::tie(narrow_num_groups, narrow_group_size) = bulk_::choose_sizes(bulk_::grid(), for_each_n_detail::for_each_kernel(), bulk_::root, first, wrapped_f, narrow_n);

      // do we need to use the wider type?
      if(for_each_n_detail::use_wide_counter(n, narrow_num_groups * narrow_group_size))
      {
        Size num_groups = 0;
        Size group_size = 0;
        thrust::tie(num_groups, group_size) = bulk_::choose_sizes(bulk_::grid(), for_each_n_detail::for_each_kernel(), bulk_::root, first, wrapped_f, n);

        num_groups = thrust::min<Size>(num_groups, thrust::detail::util::divide_ri(n, group_size));

        bulk_::async(bulk_::grid(num_groups,group_size,0,stream(thrust::detail::derived_cast(exec))), for_each_n_detail::for_each_kernel(), bulk_::root, first, wrapped_f, n);
      }
      else
      {
        // we can use the narrower type for n
        narrow_num_groups = thrust::min<unsigned int>(narrow_num_groups, thrust::detail::util::divide_ri(narrow_n, narrow_group_size));

        bulk_::async(bulk_::grid(narrow_num_groups,narrow_group_size,0,stream(thrust::detail::derived_cast(exec))), for_each_n_detail::for_each_kernel(), bulk_::root, first, wrapped_f, narrow_n);
      }

      return first + n;
    }

    __hydra_host__ __hydra_device__
    static RandomAccessIterator sequential_path(execution_policy<DerivedPolicy> &, RandomAccessIterator first, Size n, UnaryFunction f)
    {
      return thrust::for_each_n(thrust::seq, first, n, f);
    }
  };

#if __BULK_HAS_CUDART__
  return workaround::parallel_path(exec, first, n, f);
#else
  return workaround::sequential_path(exec, first, n, f);
#endif
} 


template<typename DerivedPolicy,
         typename InputIterator,
         typename UnaryFunction>
__hydra_host__ __hydra_device__
InputIterator for_each(execution_policy<DerivedPolicy> &exec,
                       InputIterator first,
                       InputIterator last,
                       UnaryFunction f)
{
  return cuda::detail::for_each_n(exec, first, thrust::distance(first,last), f);
} // end for_each()


} // end namespace detail
} // end namespace cuda
} // end namespace system
} // end HYDRA_EXTERNAL_NAMESPACE_BEGIN  namespace thrust

HYDRA_EXTERNAL_NAMESPACE_END
