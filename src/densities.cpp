#include "densities.h"

// ============================================================
// Normal (Gaussian) density
// ============================================================

// Constructor and Destructor ---------------------------------
Normal::Normal(int* observations, int T, double mean, double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->obs = observations;
	this->T = T;
	this->mean = mean;
	this->variance = variance;
	this->sd = sqrt(variance);
}

Normal::~Normal()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
}

// Methods ----------------------------------------------------
void Normal::calc_densities(double* density)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	for (int t=0; t<this->T; t++)
	{
		density[t] = dnorm(this->obs[t], this->mean, this->sd, 0);
	}
}

void Normal::calc_logdensities(double* logdensity)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	for (int t=0; t<this->T; t++)
	{
		logdensity[t] = dnorm(this->obs[t], this->mean, this->sd, 1);
	}
}

void Normal::update(double* weights)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
// TODO
}

// Getter and Setter ------------------------------------------
DensityName Normal::get_name()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(NORMAL);
}

double Normal::get_mean()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->mean);
}

void Normal::set_mean(double mean)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->mean = mean;
}

double Normal::get_variance()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->variance);
}

void Normal::set_variance(double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->variance = variance;
	this->sd = sqrt(variance);
}

double Normal::get_stdev()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->sd);
}

void Normal::set_stdev(double stdev)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->sd = stdev;
	this->variance = stdev*stdev;
}


// ============================================================
// Poisson density
// ============================================================

// Constructor and Destructor ---------------------------------
Poisson::Poisson(int* observations, int T, double lambda)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->obs = observations;
	this->T = T;
	this->lambda = lambda;
	this->lxfactorials = NULL;
	// Precompute the lxfactorials that are used in computing the densities
	if (this->obs != NULL)
	{
		this->max_obs = intMax(observations, T);
		this->lxfactorials = (double*) calloc(max_obs+1, sizeof(double));
		this->lxfactorials[0] = 0.0;	// Not necessary, already 0 because of calloc
		this->lxfactorials[1] = 0.0;
		for (int j=2; j<=max_obs; j++)
		{
			this->lxfactorials[j] = this->lxfactorials[j-1] + log(j);
		}
	}
}

Poisson::~Poisson()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	if (this->lxfactorials != NULL)
	{
		free(this->lxfactorials);
	}
}

// Methods ----------------------------------------------------
void Poisson::calc_logdensities(double* logdens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double logl = log(this->lambda);
	double l = this->lambda;
	double lxfactorial;
	// Select strategy for computing densities
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing densities in " << __func__ << " for every obs[t], because max(O)<=T";
		double logdens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			logdens_per_read[j] = j*logl - l - this->lxfactorials[j];
		}
		for (int t=0; t<this->T; t++)
		{
			logdens[t] = logdens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing densities in " << __func__ << " for every t, because max(O)>T";
		for (int t=0; t<this->T; t++)
		{
			lxfactorial = this->lxfactorials[(int) this->obs[t]];
			logdens[t] = this->obs[t]*logl - l - lxfactorial;
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
} 

void Poisson::calc_densities(double* dens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double logl = log(this->lambda);
	double l = this->lambda;
	double lxfactorial;
	// Select strategy for computing densities
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing densities in " << __func__ << " for every obs[t], because max(O)<=T";
		double dens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			dens_per_read[j] = exp( j*logl - l - this->lxfactorials[j] );
		}
		for (int t=0; t<this->T; t++)
		{
			dens[t] = dens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing densities in " << __func__ << " for every t, because max(O)>T";
		for (int t=0; t<this->T; t++)
		{
			lxfactorial = this->lxfactorials[(int) this->obs[t]];
			dens[t] = exp( this->obs[t]*logl - l - lxfactorial );
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
} 

void Poisson::update(double* weights)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double numerator, denominator;
	// Update lambda
	numerator=denominator=0.0;
// 	clock_t time, dtime;
// 	time = clock();
	for (int t=0; t<this->T; t++)
	{
		numerator += weights[t] * this->obs[t];
		denominator += weights[t];
	}
	this->lambda = numerator/denominator; // Update of size is now done with updated lambda
// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateL(): "<<dtime<< " clicks";
	FILE_LOG(logDEBUG1) << "l = " << this->lambda;

}

void Poisson::update_constrained(double** weights, int fromState, int toState)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	FILE_LOG(logDEBUG1) << "l = "<<this->lambda;
	double numerator, denominator;
	// Update lambda
	numerator=denominator=0.0;
// 	clock_t time, dtime;
// 	time = clock();
	for (int i=0; i<toState-fromState; i++)
	{
		for (int t=0; t<this->T; t++)
		{
			numerator += weights[i+fromState][t] * this->obs[t];
			denominator += weights[i+fromState][t] * (i+1);
		}
	}
	this->lambda = numerator/denominator; // Update of size is now done with old lambda
// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateL(): "<<dtime<< " clicks";
	FILE_LOG(logDEBUG1) << "l = "<<this->lambda;

}

// Getter and Setter ------------------------------------------
double Poisson::get_mean()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->lambda );
}

void Poisson::set_mean(double mean)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->lambda = mean;
}

double Poisson::get_variance()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->lambda );
}

void Poisson::set_variance(double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->lambda = variance;
}

DensityName Poisson::get_name()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(POISSON);
}

double Poisson::get_lambda()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->lambda);
}


// ============================================================
// Negative Binomial density
// ============================================================

// Constructor and Destructor ---------------------------------
NegativeBinomial::NegativeBinomial(int* observations, int T, double size, double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->obs = observations;
	this->T = T;
	this->size = size;
	this->prob = prob;
	this->lxfactorials = NULL;
	// Precompute the lxfactorials that are used in computing the densities
	if (this->obs != NULL)
	{
		this->max_obs = intMax(observations, T);
		this->lxfactorials = (double*) calloc(max_obs+1, sizeof(double));
		this->lxfactorials[0] = 0.0;	// Not necessary, already 0 because of calloc
		this->lxfactorials[1] = 0.0;
		for (int j=2; j<=max_obs; j++)
		{
			this->lxfactorials[j] = this->lxfactorials[j-1] + log(j);
		}
	}
}

NegativeBinomial::~NegativeBinomial()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	if (this->lxfactorials != NULL)
	{
		free(this->lxfactorials);
	}
}

// Methods ----------------------------------------------------
void NegativeBinomial::calc_logdensities(double* logdens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double logp = log(this->prob);
	double log1minusp = log(1-this->prob);
	double lGammaR,lGammaRplusX,lxfactorial;
	lGammaR=lgamma(this->size);
	// Select strategy for computing gammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing gammas in " << __func__ << " for every obs[t], because max(O)<=T";
		double logdens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			logdens_per_read[j] = lgamma(this->size + j) - lGammaR - lxfactorials[j] + this->size * logp + j * log1minusp;
		}
		for (int t=0; t<this->T; t++)
		{
			logdens[t] = logdens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing gammas in " << __func__ << " for every t, because max(O)>T";
		for (int t=0; t<this->T; t++)
		{
			lGammaRplusX = lgamma(this->size + this->obs[t]);
			lxfactorial = this->lxfactorials[(int) this->obs[t]];
			logdens[t] = lGammaRplusX - lGammaR - lxfactorial + this->size * logp + this->obs[t] * log1minusp;
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
} 

void NegativeBinomial::calc_densities(double* dens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double logp = log(this->prob);
	double log1minusp = log(1-this->prob);
	double lGammaR,lGammaRplusX,lxfactorial;
	lGammaR=lgamma(this->size);
	// Select strategy for computing gammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing gammas in " << __func__ << " for every obs[t], because max(O)<=T";
		double dens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			dens_per_read[j] = exp( lgamma(this->size + j) - lGammaR - lxfactorials[j] + this->size * logp + j * log1minusp );
		}
		for (int t=0; t<this->T; t++)
		{
			dens[t] = dens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing gammas in " << __func__ << " for every t, because max(O)>T";
		for (int t=0; t<this->T; t++)
		{
			lGammaRplusX = lgamma(this->size + this->obs[t]);
			lxfactorial = this->lxfactorials[(int) this->obs[t]];
			dens[t] = exp( lGammaRplusX - lGammaR - lxfactorial + this->size * logp + this->obs[t] * log1minusp );
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
} 

void NegativeBinomial::update(double* weights)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double eps = 1e-4, kmax;
	double numerator, denominator, size0, dSize, F, dFdSize, DigammaSize, DigammaSizePlusDSize;
	// Update prob (p)
	numerator=denominator=0.0;
// 	clock_t time, dtime;
// 	time = clock();
	for (int t=0; t<this->T; t++)
	{
		numerator+=weights[t]*this->size;
		denominator+=weights[t]*(this->size+this->obs[t]);
	}
	this->prob = numerator/denominator; // Update of size is now done with updated prob
	double logp = log(this->prob);
// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateP(): "<<dtime<< " clicks";
	// Update of size with Newton Method
	size0 = this->size;
	dSize = 0.00001;
	kmax = 20;
// 	time = clock();
	// Select strategy for computing digammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing digammas in " << __func__ << " for every obs[t], because max(O)<=T";
		double DigammaSizePlusX[this->max_obs+1], DigammaSizePlusDSizePlusX[this->max_obs+1];
		for (int k=1; k<kmax; k++)
		{
			F=dFdSize=0.0;
			DigammaSize = digamma(size0); // boost::math::digamma<>(size0);
			DigammaSizePlusDSize = digamma(size0 + dSize); // boost::math::digamma<>(size0+dSize);
			// Precompute the digammas by iterating over all possible values of the observation vector
			for (int j=0; j<=this->max_obs; j++)
			{
				DigammaSizePlusX[j] = digamma(size0+j);
				DigammaSizePlusDSizePlusX[j] = digamma(size0+dSize+j);
			}
			for(int t=0; t<this->T; t++)
			{
				if(this->obs[t]==0)
				{
					F += weights[t] * logp;
					//dFdSize+=0;
				}
				if(this->obs[t]!=0)
				{
					F += weights[t] * (logp - DigammaSize + DigammaSizePlusX[(int)obs[t]]);
					dFdSize += weights[t]/dSize * (DigammaSize - DigammaSizePlusDSize + DigammaSizePlusDSizePlusX[(int)obs[t]] - DigammaSizePlusX[(int)obs[t]]);
				}
			}
			if(fabs(F)<eps)
{
				break;
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing digammas in " << __func__ << " for every t, because max(O)>T";
		double DigammaSizePlusX, DigammaSizePlusDSizePlusX;
		for (int k=1; k<kmax; k++)
		{
			F = dFdSize = 0.0;
			DigammaSize = digamma(size0); // boost::math::digamma<>(size0);
			DigammaSizePlusDSize = digamma(size0 + dSize); // boost::math::digamma<>(size0+dSize);
			for(int t=0; t<this->T; t++)
			{
				DigammaSizePlusX = digamma(size0+this->obs[t]); //boost::math::digamma<>(size0+this->obs[ti]);
				DigammaSizePlusDSizePlusX = digamma(size0+dSize+this->obs[t]); // boost::math::digamma<>(size0+dSize+this->obs[ti]);
				if(this->obs[t]==0)
				{
					F+=weights[t]*logp;
					//dFdSize+=0;
				}
				if(this->obs[t]!=0)
				{
					F += weights[t] * (logp - DigammaSize + DigammaSizePlusX);
					dFdSize += weights[t]/dSize * (DigammaSize - DigammaSizePlusDSize + DigammaSizePlusDSizePlusX - DigammaSizePlusX);
				}
			}
			if(fabs(F)<eps)
			{
				break;
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	this->size = size0;
	FILE_LOG(logDEBUG1) << "size = "<<this->size << ", prob = "<<this->prob;

// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateR(): "<<dtime<< " clicks";

}

void NegativeBinomial::update_constrained(double** weights, int fromState, int toState)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	FILE_LOG(logDEBUG1) << "size = "<<this->size << ", prob = "<<this->prob;
	double eps = 1e-4, kmax;
// 	double numerator, denominator, size0, dSize, F, dFdSize, DigammaSize, DigammaSizePlusDSize;
	double numerator, denominator, size0, dSize, F, dFdSize, DigammaSize, TrigammaSize;
	double logp = log(this->prob);
	// Update prob (p)
	numerator=denominator=0.0;
// 	clock_t time, dtime;
// 	time = clock();
	for (int i=0; i<toState-fromState; i++)
	{
		for (int t=0; t<this->T; t++)
		{
			numerator += weights[i+fromState][t] * this->size*(i+1);
			denominator += weights[i+fromState][t] * (this->size*(i+1) + this->obs[t]);
		}
	}
	this->prob = numerator/denominator; // Update of size is now done with old prob
// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateP(): "<<dtime<< " clicks";
	// Update of size with Newton Method
	size0 = this->size;
	dSize = 0.00001;
	kmax = 20;
// 	time = clock();
	// Select strategy for computing digammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing digammas in " << __func__ << " for every obs[t], because max(O)<=T";
// 		double DigammaSizePlusX[this->max_obs+1], DigammaSizePlusDSizePlusX[this->max_obs+1];
		double DigammaSizePlusX[this->max_obs+1], TrigammaSizePlusX[this->max_obs+1];
		for (int k=1; k<kmax; k++)
		{
			F=dFdSize=0.0;
			for (int i=0; i<toState-fromState; i++)
			{
				DigammaSize = digamma((i+1)*size0); // boost::math::digamma<>(size0);
				TrigammaSize = trigamma((i+1)*size0); // boost::math::digamma<>(size0);
// 				DigammaSizePlusDSize = digamma((i+1)*(size0 + dSize)); // boost::math::digamma<>(size0+dSize);
				// Precompute the digammas by iterating over all possible values of the observation vector
				for (int j=0; j<=this->max_obs; j++)
				{
					DigammaSizePlusX[j] = digamma((i+1)*size0+j);
// 					DigammaSizePlusDSizePlusX[j] = digamma((i+1)*(size0+dSize)+j);
					TrigammaSizePlusX[j] = trigamma((i+1)*size0+j);
				}
				for(int t=0; t<this->T; t++)
				{
					if(this->obs[t]==0)
					{
						F += weights[i+fromState][t] * (i+1) * logp;
						//dFdSize+=0;
					}
					if(this->obs[t]!=0)
					{
						F += weights[i+fromState][t] * (i+1) * (logp - DigammaSize + DigammaSizePlusX[(int)obs[t]]);
// 						dFdSize += weights[i+fromState][t] / dSize * (i+1) * (DigammaSize - DigammaSizePlusDSize + DigammaSizePlusDSizePlusX[(int)obs[t]] - DigammaSizePlusX[(int)obs[t]]);
						dFdSize += weights[i+fromState][t] * pow((i+1),2) * (-TrigammaSize + TrigammaSizePlusX[(int)obs[t]]);
					}
				}
				if(fabs(F)<eps)
				{
					break;
				}
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing digammas in " << __func__ << " for every t, because max(O)>T";
// 		double DigammaSizePlusX, DigammaSizePlusDSizePlusX;
		double DigammaSizePlusX, TrigammaSizePlusX;
		for (int k=1; k<kmax; k++)
		{
			F = dFdSize = 0.0;
			for (int i=0; i<toState-fromState; i++)
			{
				DigammaSize = digamma((i+1)*size0); // boost::math::digamma<>(size0);
				TrigammaSize = trigamma((i+1)*size0); // boost::math::digamma<>(size0);
// 				DigammaSizePlusDSize = digamma((i+1)*(size0 + dSize)); // boost::math::digamma<>(size0+dSize);
				for(int t=0; t<this->T; t++)
				{
					DigammaSizePlusX = digamma((i+1)*size0+this->obs[t]); //boost::math::digamma<>(size0+this->obs[ti]);
// 					DigammaSizePlusDSizePlusX = digamma((i+1)*(size0+dSize)+this->obs[t]); // boost::math::digamma<>(size0+dSize+this->obs[ti]);
					TrigammaSizePlusX = trigamma((i+1)*size0+this->obs[t]);
					if(this->obs[t]==0)
					{
						F += weights[i+fromState][t] * (i+1) * logp;
						//dFdSize+=0;
					}
					if(this->obs[t]!=0)
					{
						F += weights[i+fromState][t] * (i+1) * (logp - DigammaSize + DigammaSizePlusX);
// 						dFdSize += weights[i+fromState][t] / dSize * (i+1) * (DigammaSize - DigammaSizePlusDSize + DigammaSizePlusDSizePlusX - DigammaSizePlusX);
						dFdSize += weights[i+fromState][t] * pow((i+1),2) * (-TrigammaSize + TrigammaSizePlusX);
					}
				}
			}
			if(fabs(F)<eps)
			{
				break;
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	this->size = size0;
	FILE_LOG(logDEBUG1) << "size = "<<this->size << ", prob = "<<this->prob;
	this->mean = this->fmean(this->size, this->prob);
	this->variance = this->fvariance(this->size, this->prob);

// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateR(): "<<dtime<< " clicks";

}

double NegativeBinomial::fsize(double mean, double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( mean*mean / (variance - mean) );
}

double NegativeBinomial::fprob(double mean, double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( mean / variance );
}

double NegativeBinomial::fmean(double size, double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( size / prob - size );
}

double NegativeBinomial::fvariance(double size, double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( (size - prob*size) / (prob*prob) );
}

// Getter and Setter ------------------------------------------
double NegativeBinomial::get_mean()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->fmean( this->size, this->prob ) );
}

void NegativeBinomial::set_mean(double mean)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double variance = this->get_variance();
	this->size = this->fsize( mean, variance );
	this->prob = this->fprob( mean, variance );
}

double NegativeBinomial::get_variance()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->fvariance( this->size, this->prob ) );
}

void NegativeBinomial::set_variance(double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double mean = this->get_mean();
	this->size = this->fsize( mean, variance );
	this->prob = this->fprob( mean, variance );
}

DensityName NegativeBinomial::get_name()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(NEGATIVE_BINOMIAL);
}

double NegativeBinomial::get_size()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->size);
}

double NegativeBinomial::get_prob()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->prob);
}


// ============================================================
//  Binomial density
// ============================================================

// Constructor and Destructor ---------------------------------
Binomial::Binomial(int* observations, int T, double size, double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->obs = observations;
	this->T = T;
	this->size = size;
	this->prob = prob;
}

Binomial::~Binomial()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
}

// Methods ----------------------------------------------------
void Binomial::calc_logdensities(double* logdens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double logp = log(this->prob);
	double log1minusp = log(1-this->prob);
	// Select strategy for computing gammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing densities in " << __func__ << " for every obs[t], because max(O)<=T";
		double logdens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			logdens_per_read[j] = lchoose(this->size, j) + j * logp + (this->size-j) * log1minusp;
		}
		for (int t=0; t<this->T; t++)
		{
			logdens[t] = logdens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing densities in " << __func__ << " for every t, because max(O)>T";
		int j;
		for (int t=0; t<this->T; t++)
		{
			j = (int) this->obs[t];
			logdens[t] = lchoose(this->size, j) + j * logp + (this->size-j) * log1minusp;
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
} 

void Binomial::calc_densities(double* dens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double logp = log(this->prob);
	double log1minusp = log(1-this->prob);
	// Select strategy for computing gammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing densities in " << __func__ << " for every obs[t], because max(O)<=T";
		double dens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			dens_per_read[j] = exp( lchoose(this->size, j) + j * logp + (this->size-j) * log1minusp );
		}
		for (int t=0; t<this->T; t++)
		{
			dens[t] = dens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing densities in " << __func__ << " for every t, because max(O)>T";
		int j;
		for (int t=0; t<this->T; t++)
		{
			j = (int) this->obs[t];
			dens[t] = exp( lchoose(this->size, j) + j * logp + (this->size-j) * log1minusp );
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
} 

void Binomial::update(double* weights)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double eps = 1e-4, kmax;
	double numerator, denominator, size0, dSize, F, dFdSize, DigammaSizePlus1, DigammaSizePlusDSizePlus1;
	// Update prob (p)
	numerator=denominator=0.0;
// 	clock_t time, dtime;
// 	time = clock();
	for (int t=0; t<this->T; t++)
	{
		numerator += weights[t] * this->obs[t];
		denominator += weights[t] * this->size;
	}
	this->prob = numerator/denominator; // Update of size is now done with updated prob
	double log1minusp = log(1-this->prob);
// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateP(): "<<dtime<< " clicks";
	// Update of size with Newton Method
	size0 = this->size;
	dSize = 0.00001;
	kmax = 20;
// 	time = clock();
	// Select strategy for computing digammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing digammas in " << __func__ << " for every obs[t], because max(O)<=T";
		double DigammaSizeMinusXPlus1[this->max_obs+1], DigammaSizePlusDSizeMinusXPlus1[this->max_obs+1];
		for (int k=1; k<kmax; k++)
		{
			F=dFdSize=0.0;
			DigammaSizePlus1 = digamma(size0+1);
			DigammaSizePlusDSizePlus1 = digamma((size0+dSize)+1);
			// Precompute the digammas by iterating over all possible values of the observation vector
			for (int j=0; j<=this->max_obs; j++)
			{
				DigammaSizeMinusXPlus1[j] = digamma(size0-j+1);
				DigammaSizePlusDSizeMinusXPlus1[j] = digamma((size0+dSize)-j+1);
			}
			for(int t=0; t<this->T; t++)
			{
				if(this->obs[t]==0)
				{
					F += weights[t] * log1minusp;
					//dFdSize+=0;
				}
				if(this->obs[t]!=0)
				{
					F += weights[t] * (DigammaSizePlus1 - DigammaSizeMinusXPlus1[(int)this->obs[t]] + log1minusp);
					dFdSize += weights[t]/dSize * (DigammaSizePlusDSizePlus1-DigammaSizePlus1 - DigammaSizePlusDSizeMinusXPlus1[(int)this->obs[t]]+DigammaSizeMinusXPlus1[(int)this->obs[t]]);
				}
			}
			if(fabs(F)<eps)
{
				break;
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing digammas in " << __func__ << " for every t, because max(O)>T";
		double DigammaSizePlusX, DigammaSizePlusDSizePlusX,	DigammaSizeMinusXPlus1, DigammaSizePlusDSizeMinusXPlus1;
		for (int k=1; k<kmax; k++)
		{
			F = dFdSize = 0.0;
			DigammaSizePlus1 = digamma(size0+1);
			DigammaSizePlusDSizePlus1 = digamma((size0+dSize)+1);
			for(int t=0; t<this->T; t++)
			{
				DigammaSizeMinusXPlus1 = digamma(size0-(int)this->obs[t]+1);
				DigammaSizePlusDSizeMinusXPlus1 = digamma((size0+dSize)-(int)this->obs[t]+1);
				if(this->obs[t]==0)
				{
					F += weights[t] * log1minusp;
					//dFdSize+=0;
				}
				if(this->obs[t]!=0)
				{
					F += weights[t] * (DigammaSizePlus1 - DigammaSizeMinusXPlus1 + log1minusp);
					dFdSize += weights[t]/dSize * (DigammaSizePlusDSizePlus1-DigammaSizePlus1 - DigammaSizePlusDSizeMinusXPlus1+DigammaSizeMinusXPlus1);
				}
			}
			if(fabs(F)<eps)
			{
				break;
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	this->size = size0;
	FILE_LOG(logDEBUG1) << "r = "<<this->size << ", p = "<<this->prob;

// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateR(): "<<dtime<< " clicks";

}

void Binomial::update_constrained(double** weights, int fromState, int toState)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double eps = 1e-4, kmax;
	double numerator, denominator, size0, dSize, F, dFdSize, DigammaSizePlus1, DigammaSizePlusDSizePlus1;
	// Update prob (p)
	numerator=denominator=0.0;
// 	clock_t time, dtime;
// 	time = clock();
	for (int i=0; i<toState-fromState; i++)
	{
		for (int t=0; t<this->T; t++)
		{
			numerator += weights[i+fromState][t] * this->obs[t];
			denominator += weights[i+fromState][t] * (i+1)*this->size;
		}
	}
	this->prob = numerator/denominator; // Update of size is now done with updated prob
	double log1minusp = log(1-this->prob);
// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateP(): "<<dtime<< " clicks";
	// Update of size with Newton Method
	size0 = this->size;
	dSize = 0.00001;
	kmax = 20;
// 	time = clock();
	// Select strategy for computing digammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing digammas in " << __func__ << " for every obs[t], because max(O)<=T";
		double DigammaSizeMinusXPlus1[this->max_obs+1], DigammaSizePlusDSizeMinusXPlus1[this->max_obs+1];
		for (int k=1; k<kmax; k++)
		{
			F=dFdSize=0.0;
			for (int i=0; i<toState-fromState; i++)
			{
				DigammaSizePlus1 = digamma(size0*(i+1) + 1);
				DigammaSizePlusDSizePlus1 = digamma((size0+dSize)*(i+1) + 1);
				// Precompute the digammas by iterating over all possible values of the observation vector
				for (int j=0; j<=this->max_obs; j++)
				{
					DigammaSizeMinusXPlus1[j] = digamma((i+1)*size0-j+1);
					DigammaSizePlusDSizeMinusXPlus1[j] = digamma((i+1)*(size0+dSize)-j+1);
				}
				for(int t=0; t<this->T; t++)
				{
					if(this->obs[t]==0)
					{
						F += weights[i+fromState][t] * (i+1) * log1minusp;
						//dFdSize+=0;
					}
					if(this->obs[t]!=0)
					{
						F += weights[i+fromState][t] * (i+1) * (DigammaSizePlus1 - DigammaSizeMinusXPlus1[(int)this->obs[t]] + log1minusp);
						dFdSize += weights[i+fromState][t]/dSize * (i+1) * (DigammaSizePlusDSizePlus1-DigammaSizePlus1 - DigammaSizePlusDSizeMinusXPlus1[(int)this->obs[t]]+DigammaSizeMinusXPlus1[(int)this->obs[t]]);
					}
				}
				if(fabs(F)<eps)
	{
					break;
				}
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing digammas in " << __func__ << " for every t, because max(O)>T";
		double DigammaSizePlusX, DigammaSizePlusDSizePlusX, DigammaSizeMinusXPlus1, DigammaSizePlusDSizeMinusXPlus1;
		for (int k=1; k<kmax; k++)
		{
			F = dFdSize = 0.0;
			for (int i=0; i<toState-fromState; i++)
			{
				DigammaSizePlus1 = digamma((i+1)*size0+1);
				DigammaSizePlusDSizePlus1 = digamma((i+1)*(size0+dSize)+1);
				for(int t=0; t<this->T; t++)
				{
					DigammaSizeMinusXPlus1 = digamma((i+1)*size0-(int)this->obs[t]+1);
					DigammaSizePlusDSizeMinusXPlus1 = digamma((i+1)*(size0+dSize)-(int)this->obs[t]+1);
					if(this->obs[t]==0)
					{
						F += weights[i+fromState][t] * (i+1) * log1minusp;
						//dFdSize+=0;
					}
					if(this->obs[t]!=0)
					{
						F += weights[i+fromState][t] * (i+1) * (DigammaSizePlus1 - DigammaSizeMinusXPlus1 + log1minusp);
						dFdSize += weights[i+fromState][t]/dSize * (i+1) * (DigammaSizePlusDSizePlus1-DigammaSizePlus1 - DigammaSizePlusDSizeMinusXPlus1+DigammaSizeMinusXPlus1);
					}
				}
				if(fabs(F)<eps)
				{
					break;
				}
			}
			if(F/dFdSize<size0) size0=size0-F/dFdSize;
			if(F/dFdSize>size0) size0=size0/2.0;
		}
	}
	this->size = size0;
	FILE_LOG(logDEBUG1) << "size = "<<this->size << ", prob = "<<this->prob;

// 	dtime = clock() - time;
// 	FILE_LOG(logDEBUG1) << "updateR(): "<<dtime<< " clicks";
}

double Binomial::fsize(double mean, double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( mean*mean / (mean - variance) );
}

double Binomial::fprob(double mean, double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( (mean-variance)/mean );
}

double Binomial::fmean(double size, double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( size*prob );
}

double Binomial::fvariance(double size, double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( size*prob * (1-prob) );
}

// Getter and Setter ------------------------------------------
double Binomial::get_mean()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->fmean( this->size, this->prob ) );
}

void Binomial::set_mean(double mean)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double variance = this->get_variance();
	this->size = this->fsize( mean, variance );
	this->prob = this->fprob( mean, variance );
}

double Binomial::get_variance()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->fvariance( this->size, this->prob ) );
}

void Binomial::set_variance(double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double mean = this->get_mean();
	this->size = this->fsize( mean, variance );
	this->prob = this->fprob( mean, variance );
}

DensityName Binomial::get_name()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(BINOMIAL);
}

double Binomial::get_size()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->size);
}

double Binomial::get_prob()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->prob);
}


// ============================================================
// Zero Inflation density
// ============================================================

// Constructor and Destructor ---------------------------------
ZeroInflation::ZeroInflation(int* observations, int T)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->obs = observations;
	this->T = T;
}

ZeroInflation::~ZeroInflation()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
}

// Methods ----------------------------------------------------
void ZeroInflation::calc_logdensities(double* logdens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	for (int t=0; t<this->T; t++)
	{
		if(obs[t]==0)
		{
			logdens[t] = 0.0;
		};
		if(obs[t]>0)
		{
			logdens[t] = -INFINITY;
		}
		FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
	}
}

void ZeroInflation::calc_densities(double* dens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	for (int t=0; t<this->T; t++)
	{
		if(obs[t]==0)
		{
			dens[t] = 1.0;
		}
		if(obs[t]>0)
		{
			dens[t] = 0.0;
		}
		FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
	}
}

void ZeroInflation::update(double* weights)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
}

// Getter and Setter ------------------------------------------
DensityName ZeroInflation::get_name()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(ZERO_INFLATION);
}

double ZeroInflation::get_mean()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(0);
}

void ZeroInflation::set_mean(double mean)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
}

double ZeroInflation::get_variance()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(0);
}

void ZeroInflation::set_variance(double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
}


// ============================================================
// Geometric density
// ============================================================

// Constructor and Destructor ---------------------------------
Geometric::Geometric(int* observations, int T, double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	this->obs = observations;
	this->T = T;
	this->prob = prob;
	if (this->obs != NULL)
	{
		this->max_obs = intMax(observations, T);
	}
}

Geometric::~Geometric()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
}

// Methods ----------------------------------------------------
void Geometric::calc_logdensities(double* logdens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double logp = log(this->prob);
	double log1minusp = log(1-this->prob);
	// Select strategy for computing logdensities
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing logdensities in " << __func__ << " for every obs[t], because max(O)<=T";
		double logdens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			logdens_per_read[j] = logp + j * log1minusp;
		}
		for (int t=0; t<this->T; t++)
		{
			logdens[t] = logdens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing logdensities in " << __func__ << " for every t, because max(O)>T";
		for (int t=0; t<this->T; t++)
		{
			logdens[t] = logp + this->obs[t] * log1minusp;
			FILE_LOG(logDEBUG4) << "logdens["<<t<<"] = " << logdens[t];
			if (isnan(logdens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "logdens["<<t<<"] = "<< logdens[t];
				throw nan_detected;
			}
		}
	}
} 

void Geometric::calc_densities(double* dens)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double p = this->prob;
	double oneminusp = 1-this->prob;
	// Select strategy for computing gammas
	if (this->max_obs <= this->T)
	{
		FILE_LOG(logDEBUG2) << "Precomputing densities in " << __func__ << " for every obs[t], because max(O)<=T";
		double dens_per_read [this->max_obs+1];
		for (int j=0; j<=this->max_obs; j++)
		{
			dens_per_read[j] = p * pow(oneminusp,j);
		}
		for (int t=0; t<this->T; t++)
		{
			dens[t] = dens_per_read[(int) this->obs[t]];
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
	else
	{
		FILE_LOG(logDEBUG2) << "Computing densities in " << __func__ << " for every t, because max(O)>T";
		for (int t=0; t<this->T; t++)
		{
			dens[t] = p * pow(oneminusp,this->obs[t]);
			FILE_LOG(logDEBUG4) << "dens["<<t<<"] = " << dens[t];
			if (isnan(dens[t]))
			{
				FILE_LOG(logERROR) << __PRETTY_FUNCTION__;
				FILE_LOG(logERROR) << "dens["<<t<<"] = "<< dens[t];
				throw nan_detected;
			}
		}
	}
} 

void Geometric::update(double* weights)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double numerator, denominator;
	// Update prob (p)
	numerator=denominator=0.0;
	for (int t=0; t<this->T; t++)
	{
		numerator+=weights[t];
		denominator+=weights[t]*(1+this->obs[t]);
	}
	this->prob = numerator/denominator;
	FILE_LOG(logDEBUG1) << "p = "<<this->prob;
}

double Geometric::fprob(double mean, double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( mean / variance );
}

double Geometric::fmean(double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( (1-prob) / prob );
}

double Geometric::fvariance(double prob)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( (1-prob) / (prob*prob) );
}

// Getter and Setter ------------------------------------------
double Geometric::get_mean()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->fmean( this->prob ) );
}

void Geometric::set_mean(double mean)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double variance = this->get_variance();
	this->prob = this->fprob( mean, variance );
}

double Geometric::get_variance()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return( this->fvariance( this->prob ) );
}

void Geometric::set_variance(double variance)
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	double mean = this->get_mean();
	this->prob = this->fprob( mean, variance );
}

DensityName Geometric::get_name()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(GEOMETRIC);
}

double Geometric::get_prob()
{
	FILE_LOG(logDEBUG2) << __PRETTY_FUNCTION__;
	return(this->prob);
}



