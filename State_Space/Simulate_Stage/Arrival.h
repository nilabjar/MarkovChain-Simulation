
#ifndef ARRIVALH
#define ARRIVALH

#include "Activity.h"

class Arrival
{
public:
	Arrival (Task* task);

	// creates an array of arrival times 
	void gen_arr_times (int nums);

	// creates an array of arrival times 
	void gen_arr_stg_times (int nums);

	// creates a cumulutive array of active events 
	void gen_active_events ();

	Event& get_next ();

	Event& get_next_stage ();

	void occurs ();

	void stage_occurs ();

	void save_task_times ();

private:

	// the task details ....
	Task* task_;

	// array of all events according to distribution
	vector<Event> all_events_;

	// array of only active events ...
	// these are cumulative,
	vector<Event> active_events_;

	// the index of the current event
	long long cur_idx_;

	// start time of the arrival 
	long long start_time_;

	long long stg_start_time_;
};


#endif