#ifndef SCHEDULERH
#define SCHEDULERH

#include "task.h"
#include <map>

using namespace std;

class Scheduler
{
public:

	static Scheduler* instance ();
	void update_state ();

	float get_current_task ();

private:

	Scheduler ();
	
	// the current state of the system ..
	int current_state_;

	static Scheduler* sched_ptr_;

	// the p vector values 
	map<int, float> state_p_vec_;
};

#endif