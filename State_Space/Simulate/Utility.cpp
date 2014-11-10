
#include "Utility.h"
#include <math.h>
#include <stdlib.h>

Utility* Utility::util_ptr_ = 0;

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