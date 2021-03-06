
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

	// creates an array of service times 
	void gen_svc_times (int nums);

	// creates an array of service prediction times 
	void gen_svc_stg_times (int nums);


	// creates a cumulutive array of active events 
	void gen_active_events ();

	Event& get_next ();

	Event& get_next_stage ();

//	void occurs ();

	// the job is pre empted 
	void preempt ();

	// the job is activated ...
	void start ();

	// the job is released 
	void release ();

	// stage occurs 
	void stage_occurs ();

	//void Service::service_ends ()

	void job_finished ();

	//return state 
	States state (){return state_;}

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

	// the state of the task
	States state_;

	// start time 
	long long start_time_;

	//stage start time 
	long long stg_start_time_;

	// the mean time for each stage.. normally this is equal to the svc_mean
	long stg_mean_;
};


#endif