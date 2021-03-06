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


#pragma once

#include <hydra/detail/external/thrust/detail/config.h>
#include <hydra/detail/external/thrust/detail/static_assert.h>
#include <hydra/detail/external/thrust/system/detail/generic/scan.h>
#include <hydra/detail/external/thrust/iterator/iterator_traits.h>
#include <hydra/detail/external/thrust/scan.h>
#include <hydra/detail/external/thrust/detail/type_traits.h>
#include <hydra/detail/external/thrust/detail/type_traits/iterator/is_output_iterator.h>
#include <hydra/detail/external/thrust/functional.h>

HYDRA_EXTERNAL_NAMESPACE_BEGIN  namespace thrust
{
namespace system
{
namespace detail
{
namespace generic
{


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator>
__hydra_host__ __hydra_device__
  OutputIterator inclusive_scan(thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result)
{
  // the pseudocode for deducing the type of the temporary used below:
  // 
  // if OutputIterator is a "pure" output iterator
  //   TemporaryType = InputIterator::value_type
  // else
  //   TemporaryType = OutputIterator::value_type

  typedef typename thrust::detail::eval_if<
      thrust::detail::is_output_iterator<OutputIterator>::value,
      thrust::iterator_value<InputIterator>,
      thrust::iterator_value<OutputIterator>
  >::type ValueType;

  // assume plus as the associative operator
  return thrust::inclusive_scan(exec, first, last, result, thrust::plus<ValueType>());
} // end inclusive_scan()


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator>
__hydra_host__ __hydra_device__
  OutputIterator exclusive_scan(thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result)
{
  // the pseudocode for deducing the type of the temporary used below:
  // 
  // if OutputIterator is a "pure" output iterator
  //   TemporaryType = InputIterator::value_type
  // else
  //   TemporaryType = OutputIterator::value_type

  typedef typename thrust::detail::eval_if<
      thrust::detail::is_output_iterator<OutputIterator>::value,
      thrust::iterator_value<InputIterator>,
      thrust::iterator_value<OutputIterator>
  >::type ValueType;

  // assume 0 as the initialization value
  return thrust::exclusive_scan(exec, first, last, result, ValueType(0));
} // end exclusive_scan()


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename T>
__hydra_host__ __hydra_device__
  OutputIterator exclusive_scan(thrust::execution_policy<ExecutionPolicy> &exec,
                                InputIterator first,
                                InputIterator last,
                                OutputIterator result,
                                T init)
{
  // assume plus as the associative operator
  return thrust::exclusive_scan(exec, first, last, result, init, thrust::plus<T>());
} // end exclusive_scan()


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename BinaryFunction>
__hydra_host__ __hydra_device__
  OutputIterator inclusive_scan(thrust::execution_policy<ExecutionPolicy> &/*exec*/,
                                InputIterator /*first*/,
                                InputIterator /*last*/,
                                OutputIterator result,
                                BinaryFunction /*binary_op*/)
{
  // unimplemented primitive
  HYDRA_THRUST_STATIC_ASSERT( (thrust::detail::depend_on_instantiation<InputIterator, false>::value) );
  return result;
} // end inclusive_scan


template<typename ExecutionPolicy,
         typename InputIterator,
         typename OutputIterator,
         typename T,
         typename BinaryFunction>
__hydra_host__ __hydra_device__
  OutputIterator exclusive_scan(thrust::execution_policy<ExecutionPolicy> &/*exec*/,
                                InputIterator /*first*/,
                                InputIterator /*last*/,
                                OutputIterator result,
                                T /*init*/,
                                BinaryFunction /*binary_op*/)
{
  // unimplemented primitive
  HYDRA_THRUST_STATIC_ASSERT( (thrust::detail::depend_on_instantiation<InputIterator, false>::value) );
  return result;
} // end exclusive_scan()


} // end namespace generic
} // end namespace detail
} // end namespace system
} // end HYDRA_EXTERNAL_NAMESPACE_BEGIN  namespace thrust

HYDRA_EXTERNAL_NAMESPACE_END
