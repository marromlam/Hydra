/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a fill of the License at
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

// the purpose of this header is to #include the swap_ranges.h header
// of the sequential, host, and device systems. It should be #included in any
// code which uses adl to dispatch swap_ranges

#include <hydra/detail/external/thrust/system/detail/sequential/swap_ranges.h>

// SCons can't see through the #defines below to figure out what this header
// includes, so we fake it out by specifying all possible files we might end up
// including inside an #if 0.
#if 0
#include <hydra/detail/external/thrust/system/cpp/detail/swap_ranges.h>
#include <hydra/detail/external/thrust/system/cuda/detail/swap_ranges.h>
#include <hydra/detail/external/thrust/system/omp/detail/swap_ranges.h>
#include <hydra/detail/external/thrust/system/tbb/detail/swap_ranges.h>
#endif

#define __HYDRA_THRUST_HOST_SYSTEM_SWAP_RANGES_HEADER <__HYDRA_THRUST_HOST_SYSTEM_ROOT/detail/swap_ranges.h>
#include __HYDRA_THRUST_HOST_SYSTEM_SWAP_RANGES_HEADER
#undef __HYDRA_THRUST_HOST_SYSTEM_SWAP_RANGES_HEADER

#define __HYDRA_THRUST_DEVICE_SYSTEM_SWAP_RANGES_HEADER <__HYDRA_THRUST_DEVICE_SYSTEM_ROOT/detail/swap_ranges.h>
#include __HYDRA_THRUST_DEVICE_SYSTEM_SWAP_RANGES_HEADER
#undef __HYDRA_THRUST_DEVICE_SYSTEM_SWAP_RANGES_HEADER

