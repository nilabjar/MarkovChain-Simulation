
#include "Utility.h"
#include <math.h>
#include <stdlib.h>

Utility* Utility::util_ptr_ = 0;

Utility::Utility ()
	: use_file_ (false),
		schedule_events_ (false)
{
}

Utility* Utility::instance ()
{
	if (util_ptr_ == 0)
	{
		util_ptr_ = new Utility ();
	}
	
	return util_ptr_;
}

float Utility::gen_exp_time (float mean)
{
  float r;

  r = rand ();
  r = r/(RAND_MAX + 1);
  
  return -log (r)*mean;
}

void Utility::reset_seed ()
{
	float r = rand ();

	srand (r);
}

float Utility::gen_erlang_time (float mean, int stages)
{
	float result = 0;
	float er_mean = mean/stages;

	for (int i = 0;i < stages;i++)
	{
		result += gen_exp_time (er_mean);
	}

	return result;
}