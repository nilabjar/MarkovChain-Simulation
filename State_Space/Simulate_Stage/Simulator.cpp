
#include "Simulator.h"
#include "task.h"
#include "Utility.h"
#include "Arrival.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Scheduler.h"


Simulator* Simulator::simulator_ = 0;

void Simulator::initialize ()
{
	// read in the task details from the files

	read_task_details ();

	// create the task objects , arrival and service

	for each (Task* t in this->tasks_)
	{
		t->arrival_ = new Arrival (t);

		//generate the arrival event arrays 	
		t->arrival_->gen_arr_times (Utility::instance ()->total_runs_);

		Utility::instance ()->reset_seed ();

		t->arrival_->gen_active_events ();

		//t->arr_ests_ = new Arrival (t);

		//t->arr_ests_->gen_arr_stg_times (Utility::instance ()->total_runs_*t->arr_stgs_);

		//Utility::instance ()->reset_seed ();

		t->service_ = new Service (t);

		t->service_->gen_svc_times (Utility::instance ()->total_runs_);

		//Utility::instance ()->reset_seed ();

		//t->svc_ests_ = new Service (t);

		//t->svc_ests_->gen_svc_stg_times (Utility::instance ()->total_runs_*t->svc_stgs_);

		Utility::instance ()->reset_seed ();
	}

	// setting the initial value of the current task ...
	this->active_= 0;

	// set the simulator time to 0
	this->time_ = 0;
}

void Simulator::start_simulation()
{
	// active set = all arrivals 

	// get the earliest event

	initialize ();

	this->run_simulation ();

	if (!Utility::instance ()->use_file ())
		save_task_times ();
}


void Simulator::read_task_details ()
{
  //ifstream task ("tasks_mean.txt");
  ifstream task ("tasks.txt");

  if (!task)
		cout << "Not able to open tasks.txt " << endl;

	float rate;

  while (!task.eof ())
  {
		Task * task_p = new Task ();
   
		task >> task_p->id_;
		task >> task_p->arr_stgs_;
		task >> rate;
		task_p->arr_mean_ = 60.0/rate;
		task >> task_p->svc_stgs_;
		task >> rate;

		task_p->svc_mean_ = 60.0/rate;

		this->tasks_.push_back (task_p);
  }
}

void Simulator::run_simulation ()
{
	// find the next eligible event ...
	while (this->time_ < this->sim_total_run_)
	{
		get_next_event ();

		//cout << "Time now is " 
		//		 << this->time_ 
		//		 << "System state is "
		//		 << this->get_current_state ()
		//		 << endl;
	}

	if (!Utility::instance ()->use_file ())
		this->save_task_times ();
}

void Simulator::get_next_event ()
{
	// get the next arrival stage ...
	Event arr_ev = get_next_arrival_stage ();

	//arr_ev.time_ += this->time_;


	// and look at the active task's service 
	Event svc_ev;

	svc_ev.time_ = this->time_ + 100000.0;

	if (this->active_)
	{
		svc_ev = this->active_->service_->get_next ();
		//svc_ev.time_ += this->time_;
	}

	Event stg_ev = get_next_est_stage ();

	bool stage_occurs = false;

	if (arr_ev.time_ <= svc_ev.time_)
	{
		// arrival stage fires ...

		// handle the deadine miss ..........

		if (arr_ev.time_ <= stg_ev.time_)
			this->arrival (arr_ev);
		else
			stage_occurs = true;
	}
	else 
	{
		if (svc_ev.time_ <= stg_ev.time_)
			this->service (svc_ev);
		else
			stage_occurs = true;
	}

	if (stage_occurs)
	{
		if (stg_ev.tp_ == Type::arrival)
			this->arrival_stage (stg_ev);
		else
			this->service_stage (stg_ev);
	}

	int tsk_id;

	// change scheduling .... according to state ....

	if ((tsk_id = Scheduler::instance ()->get_current_task ()) == -1)
		return;

	if (!(Utility::instance ()->schedule_events () && stage_occurs))
	{
		if (this->active_)
		{
		// if the task is a different one ......
			if (this->active_->id_ != tsk_id + 1)
			{
				this->active_->service_->preempt ();
				this->active_ = tasks_[tsk_id];
				this->active_->service_->start ();
			}
		}
		else
		{
			this->active_ = tasks_[tsk_id];
			this->active_->service_->start ();
		}
	} // stage occurs ..... 

}

Simulator* Simulator::instance ()
{
	if (simulator_ == 0)
		simulator_ = new Simulator ();

	return simulator_;
}

Event Simulator::get_next_arrival_stage ()
{
	float next_time = -1;
	Task* cur_tsk = 0;

	// getting the nearest next arrival time ....
	for each (Task* t in tasks_)
	{
		float tm = t->arrival_->get_next ().time_;

		if (next_time == -1)
		{
			cur_tsk = t;
			next_time = tm;
		}

		if (tm < next_time)
		{
			cur_tsk = t;
			next_time = tm;
		}
	}

	Event ev;

	ev.task_ = cur_tsk;
	ev.time_ = next_time;
	ev.tp_ = Type::arrival;

	return ev;
}

Event Simulator::arrival_stage (Event ev)
{
	Task* tsk = ev.task_;

	this->time_ = ev.time_;
	tsk->arrival_->stage_occurs ();


	if (tsk->arr_state_ != tsk->arr_stgs_)
	{
		ev.task_->arr_state_++;
	}

	return ev;
}

Event Simulator::service_stage (Event ev)
{
	Task* tsk = ev.task_;

	this->time_ = ev.time_;
	tsk->service_->stage_occurs ();
	
	if (tsk->svc_state_ != tsk->svc_stgs_)
	{
		tsk->svc_state_++;
	}

//	Scheduler::instance ()->update_state ();
	return ev;
}

int Simulator::get_current_state ()
{

	stringstream str;

	for each (Task* t in tasks_)
	{
		str << t->arr_state_ << t->svc_state_;
	}

	int state;
	str >> state;

	return state;
}

void Simulator::print_results ()
{
	ofstream out ("results.txt");

	out << "Total Time " << this->time_ << endl;
	
	float total_arrival = 0;
	float total_miss = 0;

	for each (Task* t in tasks_)
	{
		out << "Task " << t->id_ << endl;
		
		out << "Arrival=" << t->total_arrival_ << endl;
		total_arrival += t->total_arrival_;

		out << "Missed=" << t->deadline_miss_ << endl;
		total_miss += t->deadline_miss_;
	}

	out << "Total Arrive = " 
			<< total_arrival 
			<< endl
			<< "Total Miss = "
			<< total_miss
			<< endl
			<< "% miss = " 
			<< (total_miss/total_arrival)*100
			<< endl;

	out.close ();
}

Event Simulator::get_next_est_stage ()
{
	float next_time = -1;
	Task* cur_tsk;

	for each (Task* t in tasks_)
	{
		float tm = t->arrival_->get_next_stage ().time_;

		if (next_time == -1)
		{
			cur_tsk = t;
			next_time = tm;
		}

		if (tm < next_time)
		{
			cur_tsk = t;
			next_time = tm;
		}
	}

	Event ev;

	ev.task_ = cur_tsk;
	ev.time_ = next_time;
	ev.tp_ = Type::arrival;

	next_time = -1;

	// for this, we use only the active service ....
	if (this->active_)
	{
		Event svc_ev = active_->service_->get_next_stage ();

		if (ev.time_ >= svc_ev.time_)
			ev = svc_ev;


	}

	return ev;
}

Event Simulator::arrival (Event ev)
{
	Task* tsk = ev.task_;

	// update arrival count 
	tsk->total_arrival_++;

	// handle deadline miss .... 
	if (tsk->service_->state () != Service::States::arriving)
		tsk->deadline_miss_++;

	// A new task has arrived .....
	tsk->arr_state_ = 1; 
		
	// release the task ....
	tsk->service_->release ();

	// do something here when task arrives .....
	if (this->active_ && this->active_->id_ == tsk->id_)
		this->active_ = 0;

	tsk->svc_state_ = 1;
	
	// update the time for the simulator 
	this->time_ = ev.time_;


	tsk->arrival_->occurs ();

	return ev;
}

	// current job completes ...
Event Simulator::service (Event ev)
{
	Task* tsk = ev.task_;

	tsk->svc_state_ = 0; // service finished .....

	// do something here when task is finished 
	tsk->service_->job_finished ();

	// here active should be set to 0
	this->active_ = 0;

//	Scheduler::instance ()->update_state ();

	this->time_ = ev.time_;

	return ev;
}

void Simulator::save_task_times ()
{
	for each (Task* t in tasks_)
	{
		t->arrival_->save_task_times ();
		t->service_->save_task_times ();
	}
}