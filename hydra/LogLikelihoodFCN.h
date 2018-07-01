/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 - 2018 Antonio Augusto Alves Junior
 *
 *   This file is part of Hydra Data Analysis Framework.
 *
 *   Hydra is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Hydra is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Hydra.  If not, see <http://www.gnu.org/licenses/>.
 *
 *---------------------------------------------------------------------------*/

/*
 * LogLikelihoodFCN2.h
 *
 *  Created on: 13/08/2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef LOGLIKELIHOODFCN_H_
#define LOGLIKELIHOODFCN_H_


#include <hydra/FCN.h>
#include <hydra/Pdf.h>
#include <hydra/PDFSumExtendable.h>
#include <hydra/PDFSumNonExtendable.h>
#include <hydra/detail/Iterable_traits.h>

namespace hydra {

template<typename PDF, typename Iterator, typename... Extensions>
class LogLikelihoodFCN;

/**
 * \ingroup fit
 * \brief LogLikehood object for not composed models represeted by hydra::Pdf objects
 * \tparam Functor
 * \tparam Integrator
 * \tparam IteratorD
 * \tparam IteratorW
 */
template<typename Functor, typename Integrator, typename IteratorD, typename ...IteratorW>
class LogLikelihoodFCN< Pdf<Functor,Integrator> , IteratorD, IteratorW...>;

/**
 * \class LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>
 * \ingroup fit
 */
template<typename ...Pdfs, typename IteratorD , typename ...IteratorW>
class LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, IteratorD, IteratorW...>;

/**
 * \ingroup fit
 */
template<typename ...Pdfs, typename IteratorD , typename ...IteratorW>
class LogLikelihoodFCN< PDFSumNonExtendable<Pdfs...>, IteratorD, IteratorW...>;

/**
 * \ingroup fit
 * \brief Conveniency function to build up loglikehood fcns
 * @param pdf hydra::Pdf object
 * @param first iteraror pointing to begin of data range
 * @param last iteraror pointing to end of data range
 * @param weights iteraror pointing to begin of weights range
 * @return
 */
template< typename Functor, typename Integrator,  typename Iterator, typename ...Iterators>
auto make_loglikehood_fcn(Pdf<Functor,Integrator> const& pdf, Iterator first, Iterator last,  Iterators... weights )
-> LogLikelihoodFCN< Pdf<Functor,Integrator>, Iterator , Iterators... >;

/**
 * \ingroup fit
 * \brief Conveniency function to build up loglikehood fcns
 * @param pdf hydra::Pdf object
 * @param first iteraror pointing to begin of data range
 * @param last iteraror pointing to end of data range
 * @param weights iteraror pointing to begin of weights range
 * @return
 */
/*
template<typename... Pdfs,  typename Iterator, typename ...Iterators>
auto make_loglikehood_fcn(PDFSumExtendable<Pdfs...> const& functor, Iterator first, Iterator last, Iterators... weights )
-> LogLikelihoodFCN< PDFSumExtendable<Pdfs...>, Iterator,Iterators... >;
*/
template<typename...Functors, typename ...Integrators,  typename Iterator, typename ...Iterators>
auto make_loglikehood_fcn(PDFSumExtendable<Pdf<Functors,Integrators>...> const& functor, Iterator first, Iterator last, Iterators... weights )
-> LogLikelihoodFCN< PDFSumExtendable<Pdf<Functors,Integrators>...>, Iterator,Iterators... >;

/**
 * \ingroup fit
 * \brief Conveniency function to build up loglikehood fcns
 * @param pdf hydra::Pdf object
 * @param first iteraror pointing to begin of data range
 * @param last iteraror pointing to end of data range
 * @param weights iteraror pointing to begin of weights range
 * @return
 */
/*
template<typename... Pdfs,  typename Iterator, typename ...Iterators>
auto make_loglikehood_fcn(PDFSumNonExtendable<Pdfs...>const& pdf, Iterator first, Iterator last, Iterators... weights)
-> LogLikelihoodFCN< PDFSumNonExtendable<Pdfs...>, Iterator,Iterators...  >;
*/
template<typename...Functors, typename ...Integrators,  typename Iterator, typename ...Iterators>
auto make_loglikehood_fcn(PDFSumNonExtendable<Pdf<Functors,Integrators>...> const& functor, Iterator first, Iterator last, Iterators... weights )
-> LogLikelihoodFCN< PDFSumNonExtendable<Pdf<Functors,Integrators>...>, Iterator,Iterators... >;
/**
 * \ingroup fit
 * \brief Conveniency function to build up loglikehood fcns
 * @param pdf hydra::Pdf object
 * @param points "iterable" storing the data
 * @param weights "iterables" storing the weights
 * @return
 */
template< typename Functor, typename Integrator, typename Iterable, typename ...Iterables,
typename U =typename std::conditional<sizeof...(Iterables)==0, std::true_type, detail::all_true< detail::is_iterable<Iterables>::value...> >::type >
inline typename std::enable_if< hydra::detail::is_iterable<Iterable>::value && U::value,
LogLikelihoodFCN< Pdf<Functor,Integrator>, decltype(std::declval<Iterable&>().begin()),
                  decltype(std::declval<Iterables&>().begin())... >>::type
make_loglikehood_fcn(Pdf<Functor,Integrator> const& pdf, Iterable& points, Iterables&... weights );

/**
 * \ingroup fit
 * \brief Conveniency function to build up loglikehood fcns
 * @param pdf hydra::Pdf object
 * @param points "iterable" storing the data
 * @param weights "iterables" storing the weights
 * @return
 */
template<typename...Functors, typename ...Integrators, typename Iterable, typename ...Iterables,
typename U = typename std::conditional<sizeof...(Iterables)==0, std::true_type, detail::all_true< detail::is_iterable<Iterables>::value...> >::type >
inline typename std::enable_if< hydra::detail::is_iterable<Iterable>::value && U::value,
LogLikelihoodFCN<  PDFSumExtendable<Pdf<Functors,Integrators>...>,
                     decltype(std::declval<Iterable>().begin()),
                     decltype(std::declval<Iterables>().begin())... > >::type
make_loglikehood_fcn(PDFSumExtendable<Pdf<Functors,Integrators>...> const& functor, Iterable& points, Iterables&... weights );


/**
 * \ingroup fit
 * \brief Conveniency function to build up loglikehood fcns
 * @param pdf hydra::Pdf object
 * @param first iteraror pointing to begin of data range
 * @param last iteraror pointing to end of data range
 * @param weights iteraror pointing to begin of weights range
 * @return
 */
template<typename...Functors, typename ...Integrators, typename Iterable, typename ...Iterables,
typename U = typename std::conditional<sizeof...(Iterables)==0, std::true_type, detail::all_true< detail::is_iterable<Iterables>::value...> >::type >
inline typename std::enable_if< hydra::detail::is_iterable<Iterable>::value && U::value,
LogLikelihoodFCN<  PDFSumNonExtendable<Pdf<Functors,Integrators>...>,
                     decltype(std::declval<Iterable>().begin()),
                     decltype(std::declval<Iterables>().begin())... > >::type
make_loglikehood_fcn(PDFSumNonExtendable<Pdf<Functors,Integrators>...> const& functor, Iterable& points, Iterables&... weights );

}  // namespace hydra

#include<hydra/detail/LogLikelihoodFCN1.inl>
#include<hydra/detail/LogLikelihoodFCN2.inl>
#include<hydra/detail/LogLikelihoodFCN3.inl>

#endif /* LOGLIKELIHOODFCN2_H_ */
