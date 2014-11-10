
#ifndef SERVICEH
#define SERVICEH

#include "task.h"
#include "Activity.h"

class Service 
{

public:
	typedef enum {arriving, executing, preempted, released} States;

public:
	Service (Task* task);

	// creates an array of arrival times 
	void gen_svc_times (int nums);

	// creates a cumulutive array of active events 
	void gen_active_events ();

	Event& get_next ();

//	void occurs ();

	// the job is pre empted 
	void preempt ();

	// the job is activated ...
	void start ();

	// the job is released 
	void release ();

	// stage occurs 
	void stage_occurs ();

	void job_finished ();

	//return state 
	States state (){return state_;}

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

	// the state of the task
	States state_;

	// start time 
	long long start_time_;
};


#endif