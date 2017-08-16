/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 Antonio Augusto Alves Junior
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
 * extended_logLL_fit.inl
 *
 *  Created on: Aug 16, 2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef EXTENDED_LOGLL_FIT_INL_
#define EXTENDED_LOGLL_FIT_INL_




#include <iostream>
#include <assert.h>
#include <time.h>
#include <chrono>

//command line
#include <tclap/CmdLine.h>

//this lib
#include <hydra/device/System.h>
#include <hydra/host/System.h>
#include <hydra/Function.h>
#include <hydra/FunctionWrapper.h>
#include <hydra/Random.h>
#include <hydra/LogLikelihoodFCN2.h>
#include <hydra/Parameter.h>
#include <hydra/UserParameters.h>
#include <hydra/Pdf.h>
#include <hydra/AddPdf.h>
#include <hydra/Copy.h>
#include <hydra/Filter.h>
#include <hydra/GaussKronrodQuadrature.h>

//Minuit2
#include "Minuit2/FunctionMinimum.h"
#include "Minuit2/MnUserParameterState.h"
#include "Minuit2/MnPrint.h"
#include "Minuit2/MnMigrad.h"
#include "Minuit2/MnMinimize.h"
#include "Minuit2/MnMinos.h"
#include "Minuit2/MnContours.h"
#include "Minuit2/CombinedMinimizer.h"
#include "Minuit2/MnPlot.h"
#include "Minuit2/MinosError.h"
#include "Minuit2/ContoursError.h"
#include "Minuit2/VariableMetricMinimizer.h"

/*-------------------------------------
 * Include classes from ROOT to fill
 * and draw histograms and plots.
 *-------------------------------------
 */
#ifdef _ROOT_AVAILABLE_

#include <TROOT.h>
#include <TH1D.h>
#include <TApplication.h>
#include <TCanvas.h>

#endif //_ROOT_AVAILABLE_


using namespace ROOT::Minuit2;

int main(int argv, char** argc)
{
	size_t nentries = 0;

	try {

		TCLAP::CmdLine cmd("Command line arguments for ", '=');

		TCLAP::ValueArg<size_t> EArg("n", "number-of-events","Number of events", true, 10e6, "size_t");
		cmd.add(EArg);

		// Parse the argv array.
		cmd.parse(argv, argc);

		// Get the value parsed by each arg.
		nentries = EArg.getValue();

	}
	catch (TCLAP::ArgException &e)  {
		std::cerr << "error: " << e.error() << " for arg " << e.argId()
														<< std::endl;
	}


	//generator
	hydra::Random<thrust::random::default_random_engine>
	Generator( std::chrono::system_clock::now().time_since_epoch().count() );

	//----------------------
    //fit model

	//gaussian 1
	auto GAUSSIAN1 =  [=] __host__ __device__
			(unsigned int npar, const hydra::Parameter* params,unsigned int narg, double* x )
	{
		double m2 = (x[0] -  params[0])*(x[0] - params[0] );
		double s2 = params[1]*params[1];
		double g = exp(-m2/(2.0 * s2 ))/( sqrt(2.0*s2*PI));

		return g;
	};

	std::string  Mean1("Mean_1"); 	// mean of gaussian
	std::string Sigma1("Sigma_1"); // sigma of gaussian
	hydra::Parameter  mean1_p  = hydra::Parameter::Create().Name(Mean1).Value(0.5) .Error(0.0001).Limits(-1.0, 1.0);
	hydra::Parameter  sigma1_p = hydra::Parameter::Create().Name(Sigma1).Value(0.5).Error(0.0001).Limits(0.01, 1.5);

    auto gaussian1 = hydra::wrap_lambda(GAUSSIAN1, mean1_p, sigma1_p);

    //-------------------------------------------
    //gaussian 2
    auto GAUSSIAN2 =  [=] __host__ __device__
    		(unsigned int npar, const hydra::Parameter* params,unsigned int narg, double* x )
    {
    	double m2 = (x[0] -  params[0])*(x[0] - params[0] );
    	double s2 = params[1]*params[1];
    	double g = exp(-m2/(2.0 * s2 ))/( sqrt(2.0*s2*PI));

    	return g;
    };

    std::string  Mean2("Mean_2"); 	// mean of gaussian
    std::string Sigma2("Sigma_2"); // sigma of gaussian
    hydra::Parameter  mean2_p  = hydra::Parameter::Create().Name(Mean2).Value(1.5) .Error(0.0001).Limits(-1.0, 1.0);
    hydra::Parameter  sigma2_p = hydra::Parameter::Create().Name(Sigma2).Value(0.5).Error(0.0001).Limits(0.01, 1.5);

    auto gaussian2 = hydra::wrap_lambda(GAUSSIAN2, mean2_p, sigma2_p);

    //--------------------------------------------
    //exponential
    auto EXPONENTIAL =  [=] __host__ __device__
    		(unsigned int npar, const hydra::Parameter* params,unsigned int narg, double* x )
    {
    	return exp( -x[0]*params[0]);
    };

    std::string  Tau("Tau"); 	// tau of the exponential
    hydra::Parameter  tau_p  = hydra::Parameter::Create().Name(Tau).Value(0.5) .Error(0.0001).Limits(-1.0, 1.0);

    auto exponential = hydra::wrap_lambda(EXPONENTIAL, tau_p);


    //------------------
    //make model
	std::string na("NGauss_1");
	std::string nb("NGauss_2");
	std::string nc("NExp");

	hydra::Parameter NGauss_1_p(na ,nentries, sqrt(nentries), nentries-nentries/2 , nentries+nentries/2) ;
	hydra::Parameter NGauss_2_p(nb ,nentries, sqrt(nentries), nentries-nentries/2 , nentries+nentries/2) ;
	hydra::Parameter     NExp_p(nc ,nentries, sqrt(nentries), nentries-nentries/2 , nentries+nentries/2) ;

    std::array<Parameter*, 2>  yields{ &NGauss_1_p, &NGauss_2_p, &NExp_p };

    auto model = hydra::add_pdfs(yields, gaussian1, gaussian2, exponential);

    model.SetExtended(1);

    //-----------------
    // some definitions
    double min   = -5.0;
    double max   =  5.0;
    double mean  =  0.0;
    double sigma =  1.0;

	//device
	//------------------------
#ifdef _ROOT_AVAILABLE_

	TH1D hist_gaussian_d("gaussian_d", "Gaussian",    100, min, max);
	TH1D hist_fitted_gaussian_d("fitted_gaussian_d", "Gaussian",    100, min, max);

#endif //_ROOT_AVAILABLE_
	{
		//1D device buffer
		hydra::device::vector<double>  data_d(3*nentries);
		hydra::host::vector<double>    data_h(3*nentries);

		//-------------------------------------------------------
		// Generate data

		// gaussian1
		Generator.Gauss(mean1_p.Value(), sigma1_p.Value(), data_d.begin(), data_d.begin()+nentries);

		// gaussian1
		Generator.Gauss(mean2_p.Value(), sigma2_p.Value(), data_d.begin()+nentries, data_d.begin()+2*nentries);

		// exponential
		Generator.Exponential(tau_p.Value(), data_d.begin() + 2*nentries,  data_d.end());


		for(size_t i=0; i<10; i++)
			std::cout << "< Data > [" << i << "] :" << data_d[i] << std::endl;

		//numerical integral to normalize the pdf
		hydra::GaussKronrodQuadrature<61,100, hydra::device::sys_t> GKQ61_d(min,  max);

		//filtering
		auto FILTER = [=]__host__ __device__(unsigned int n, double* x){
			return (x[0] > min) && (x[0] < max );
		};

		auto filter = hydra::wrap_lambda(FILTER);
		auto range  = hydra::apply_filter(data_d,  filter);

		std::cout<< std::endl<< std::endl;
		for(size_t i=0; i<10; i++)
			std::cout << "< Random::Gauss > [" << i << "] :" << range.first[i] << std::endl;


		//make model and fcn
		auto model = hydra::make_pdf(gaussian, GKQ61_d );
		auto fcn   = hydra::make_loglikehood_fcn(range.first, range.second, model);

		//-------------------------------------------------------
		//fit
		ROOT::Minuit2::MnPrint::SetLevel(3);
		hydra::Print::SetLevel(hydra::WARNING);
		//minimization strategy
		MnStrategy strategy(2);

		// create Migrad minimizer
		MnMigrad migrad_d(fcn, fcn.GetParameters().GetMnState() ,  strategy);

		std::cout<<fcn.GetParameters().GetMnState()<<std::endl;

		// ... Minimize and profile the time

		auto start_d = std::chrono::high_resolution_clock::now();
		FunctionMinimum minimum_d =  FunctionMinimum(migrad_d(std::numeric_limits<unsigned int>::max(), 5));
		auto end_d = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_d = end_d - start_d;

		// output
		std::cout<<"minimum: "<<minimum_d<<std::endl;

		//time
		std::cout << "-----------------------------------------"<<std::endl;
		std::cout << "| GPU Time (ms) ="<< elapsed_d.count() <<std::endl;
		std::cout << "-----------------------------------------"<<std::endl;

		//bring data to device
		hydra::copy( data_d.begin() , data_d.end(), data_h.begin() );

		//draw fitted function


#ifdef _ROOT_AVAILABLE_
		for(auto value : data_h)
			hist_gaussian_d.Fill( value);

		//draw fitted function
		hist_fitted_gaussian_d.Sumw2();
		for (size_t i=0 ; i<=100 ; i++) {
			double x = hist_fitted_gaussian_d.GetBinCenter(i);
	        hist_fitted_gaussian_d.SetBinContent(i, fcn.GetPDF()(x) );
		}
		hist_fitted_gaussian_d.Scale(hist_gaussian_d.Integral()/hist_fitted_gaussian_d.Integral() );
#endif //_ROOT_AVAILABLE_

	}//device end



	//host
	//------------------------
#ifdef _ROOT_AVAILABLE_

	TH1D hist_gaussian_h("gaussian_h", "Gaussian",    100, min, max );
	TH1D hist_fitted_gaussian_h("fitted_gaussian_h", "Gaussian",    100, min, max);
#endif //_ROOT_AVAILABLE_

	{
		//1D device buffer
		hydra::host::vector<double>    data_h(nentries);

		//-------------------------------------------------------
		//gaussian
		Generator.Gauss(mean, sigma, data_h.begin(), data_h.end());

		for(size_t i=0; i<10; i++)
			std::cout << "< Random::Gauss > [" << i << "] :" << data_h[i] << std::endl;

		//numerical integral to normalize the pdf
		hydra::GaussKronrodQuadrature<61,100, hydra::host::sys_t> GKQ61_h(min,  max);

		//filtering
		auto FILTER = [=]__host__ __device__(unsigned int n, double* x){
			return (x[0] > min) && (x[0] < max );
		};

		auto filter = hydra::wrap_lambda(FILTER);
		auto range  = hydra::apply_filter(data_h,  filter);

		std::cout<< std::endl<< std::endl;
		for(size_t i=0; i<10; i++)
			std::cout << "< Random::Gauss > [" << i << "] :" << range.first[i] << std::endl;


		//make model and fcn
		auto model = hydra::make_pdf(gaussian, GKQ61_h );
		auto fcn   = hydra::make_loglikehood_fcn(range.first, range.second, model);

		//-------------------------------------------------------
		//fit
		ROOT::Minuit2::MnPrint::SetLevel(3);
		//minimization strategy
		MnStrategy strategy(2);

		// create Migrad minimizer
		MnMigrad migrad_h(fcn, fcn.GetParameters().GetMnState() ,  strategy);

		std::cout << fcn.GetParameters().GetMnState() << std::endl;

		// ... Minimize and profile the time
		auto start_h = std::chrono::high_resolution_clock::now();
		FunctionMinimum minimum_h =  FunctionMinimum(migrad_h(std::numeric_limits<unsigned int>::max(), 5));
		auto end_h = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> elapsed_h = end_h - start_h;

		// output
		std::cout<<"minimum: "<< minimum_h <<std::endl;

		//time
		std::cout << "-----------------------------------------"<<std::endl;
		std::cout << "| GPU Time (ms) ="<< elapsed_h.count() <<std::endl;
		std::cout << "-----------------------------------------"<<std::endl;


		//draw fitted function
#ifdef _ROOT_AVAILABLE_
		for(auto value : data_h)
			hist_gaussian_h.Fill( value);

		//draw fitted function
		hist_fitted_gaussian_h.Sumw2();
		for (size_t i=0 ; i<=100 ; i++) {
			double x = hist_fitted_gaussian_h.GetBinCenter(i);
			hist_fitted_gaussian_h.SetBinContent(i, fcn.GetPDF()(x) );
		}
		hist_fitted_gaussian_h.Scale(hist_gaussian_h.Integral()/hist_fitted_gaussian_h.Integral() );
#endif //_ROOT_AVAILABLE_

	}



#ifdef _ROOT_AVAILABLE_
	TApplication *myapp=new TApplication("myapp",0,0);

	//draw histograms
	TCanvas canvas_d("canvas_d" ,"Distributions - Device", 500, 500);
	hist_gaussian_d.Draw("hist");
	hist_fitted_gaussian_d.Draw("histsameC");
	hist_fitted_gaussian_d.SetLineColor(2);

	//draw histograms
	TCanvas canvas_h("canvas_h" ,"Distributions - Host", 500, 500);
	hist_gaussian_h.Draw("hist");
	hist_fitted_gaussian_h.Draw("histsameC");
	hist_fitted_gaussian_h.SetLineColor(2);

	myapp->Run();

#endif //_ROOT_AVAILABLE_

	return 0;



}


#endif /* EXTENDED_LOGLL_FIT_INL_ */
