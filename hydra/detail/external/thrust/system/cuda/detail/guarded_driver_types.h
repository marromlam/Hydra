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

// the purpose of this header is to #include <driver_types.h> without causing
// warnings from redefinitions of __hydra_host__ and __hydra_device__.
// carefully save their definitions and restore them
// can't tell exactly when push_macro & pop_macro were introduced to gcc; assume 4.5.0


#if !defined(__GNUC__) || ((10000 * __GNUC__ + 100 * __GNUC_MINOR__ + __GNUC_PATCHLEVEL__) >= 40500)
#  ifdef __hydra_host__
#    pragma push_macro("__hydra_host__")
#    undef __hydra_host__
#    define HYDRA_THRUST_HOST_NEEDS_RESTORATION
#  endif
#  ifdef __hydra_device__
#    pragma push_macro("__hydra_device__")
#    undef __hydra_device__
#    define HYDRA_THRUST_DEVICE_NEEDS_RESTORATION
#  endif
#else // GNUC pre 4.5.0
#  if !defined(__DRIVER_TYPES_H__)
#    ifdef __hydra_host__
#      undef __hydra_host__
#    endif
#    ifdef __hydra_device__
#      undef __hydra_device__
#    endif
#  endif // __DRIVER_TYPES_H__
#endif // __GNUC__


#include <driver_types.h>


#if !defined(__GNUC__) || ((10000 * __GNUC__ + 100 * __GNUC_MINOR__ + __GNUC_PATCHLEVEL__) >= 40500)
#  ifdef HYDRA_THRUST_HOST_NEEDS_RESTORATION
#    pragma pop_macro("__hydra_host__")
#    undef HYDRA_THRUST_HOST_NEEDS_RESTORATION
#  endif
#  ifdef HYDRA_THRUST_DEVICE_NEEDS_RESTORATION
#    pragma pop_macro("__hydra_device__")
#    undef HYDRA_THRUST_DEVICE_NEEDS_RESTORATION
#  endif
#endif // __GNUC__

