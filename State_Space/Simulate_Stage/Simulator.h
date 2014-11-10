#ifndef SIMULATORH
#define SIMULATORH

#include "task.h"
#include "Arrival.h"
#include "Service.h"
#include <vector>

using namespace std;

class Simulator
{
public:
	
	// read in task parameters and p_values from 
	// the files ....
	void initialize ();

	// starts the simulation
	void start_simulation ();

	static Simulator* instance ();

	int get_current_state ();

	// returns the current simlutar time 
	const int current_sim_time () {return time_;}

	void total_sim_run (long long run) {sim_total_run_ = run;}

	// prints the simulation results 
	void Simulator::print_results ();
private:

	// generate arrival timings 
	void gen_arr_times ();

	void gen_svc_times ();

	//start the arrival processes ...
	void start_arrival ();

	// read the task details from the files 
	void read_task_details ();

	//create the arrival jobs 
	void create_arrival_jobs ();

	//create the service
	void create_svc_jobs ();

	// runs the actual simulation 
	void run_simulation ();

	/// get the next event 
	void get_next_event ();

	// get the next arrival event ...
	Event get_next_arrival_stage ();

	// get the next stage prediction ...
	Event get_next_est_stage ();

	// arrival occurs ....
	Event arrival (Event ev);

	// current job completes ...
	Event service (Event ev);

	Event arrival_stage (Event ev);

	Event service_stage (Event ev);

	void save_task_times ();

private :
	// array of tasks in the system
	vector <Task*> tasks_;

	//	The active task
	Task*	active_;

	static Simulator* simulator_;

	// the current time in the simulator
	long long time_;

	// total time for which to run simulation 
	long long sim_total_run_;
};

#endif