
#include "Activity.h"
#include "Utility.h"


Activity::Activity (int stages, int mean)
{
	
}

void Activity::gen_arr_times (int nums)
{
	// generate an array of events and keep them
	// in a vector

	for (unsigned int i = 0;
			 i < nums;
			 i++)
	{
		Utility::instance ()->gen_exp_time (this->task_->arr_mean_);
	}
}