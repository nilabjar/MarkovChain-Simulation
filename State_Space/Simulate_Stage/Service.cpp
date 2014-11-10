
#include "Service.h"
#include "Utility.h"
#include "Simulator.h"
#include <sstream>
#include <fstream>

Service::Service (Task* task)
	: task_(task),
		cur_idx_ (0),
		state_(States::arriving),
		stg_mean_(task->svc_mean_/task->svc_stgs_)
{
	
}

void Service::gen_svc_times(int nums)
{

	if (Utility::instance ()->use_file ())
	{
		ostringstream str;
		str << "Task_" << this->task_->id_ << "_Svc";
	  
		ifstream in (str.str ().c_str ());

		float time;

		while (!in.eof ())
		{
			in >> time;
	//    cout << "svc value is "<< val << endl;
			Event ev;
			ev.task_ = this->task_;
			ev.tp_ = Type::arrival;
			ev.time_ = time*1000; // convert to millisecond ...

			this->all_events_.push_back (ev);
		}

		in.close ();

		return;
	}


	for (unsigned int i = 0;
			 i < nums;
			 i++)
	{
		float time = 
			Utility::instance ()->gen_erlang_time (task_->svc_mean_, task_->svc_stgs_);

		Event ev;
		ev.task_ = this->task_;
//		ev.tp_ = Type::arrival;
				ev.tp_ = Type::service;
		ev.time_ = time*1000; // convert to millisecond ...

		this->all_events_.push_back (ev);
	}
}

// creates an array of service prediction times 
void Service::gen_svc_stg_times (int nums)
{
	for (unsigned int i = 0;
			 i < nums;
			 i++)
	{
		float mean_stage = task_->svc_mean_/task_->svc_stgs_;
		float time = 
			Utility::instance ()->gen_exp_time (mean_stage);

		Event ev;
		ev.task_ = this->task_;
//		ev.tp_ = Type::arrival;
				ev.tp_ = Type::service;
		ev.time_ = time*1000; // convert to millisecond ...

		this->all_events_.push_back (ev);
	}
}

void Service::gen_active_events ()
{
	int last_time = 0;
	for each (const Event& ev in this->all_events_)
	{
		Event act_event = ev;

		last_time += ev.time_;

		act_event.time_ = last_time;

		this->active_events_.push_back (act_event);
	}
}

Event& Service::get_next ()
{
//	return active_events_[cur_idx_];
	Event ev = all_events_[cur_idx_];

	//adjust ev by the actual occurence time 
	ev.time_ += this->start_time_;

	return ev;
}

void Service::start ()
{
	this->state_ = States::executing;	

	this->start_time_ = 
		Simulator::instance ()->current_sim_time ();

	this->stg_start_time_ = 
		Simulator::instance ()->current_sim_time ();
}

void Service::preempt ()
{
	this->state_ = States::preempted;

	all_events_[cur_idx_].time_ -= 
		Simulator::instance ()->current_sim_time () 
		- this->start_time_;

	this->stg_mean_ -= 
		Simulator::instance ()->current_sim_time () 
		- this->start_time_;
}

void Service::release ()
{
	// update the cur_idx, if it was already executing
	if (this->state_ != States::arriving)
		this->cur_idx_++;
	
	this->state_ = States::released;
}

void Service::stage_occurs ()
{
	this->stg_mean_ = 
		this->task_->svc_mean_/this->task_->svc_stgs_;

	this->stg_start_time_ = 
		Simulator::instance ()->current_sim_time ();
}

//void Service::service_ends ()
//{
//	this->cur_idx_++;
//
//	this->start_time_ = 
//		Simulator::instance ()->current_sim_time ();
//}

void Service::job_finished ()
{
	this->state_ = States::arriving;

	this->cur_idx_++;

	//this->start_time_ = 
	//	Simulator::instance ()->current_sim_time ();
}


Event& Service::get_next_stage ()
{
	Event ev;

	ev.time_ = (this->task_->svc_mean_*1000)/task_->svc_stgs_ + this->stg_start_time_;
	ev.tp_ = Type::service;
	ev.task_ = this->task_;

	return ev;
}

void Service::save_task_times ()
{
	char buf [50];
	memset(buf , 0 , 50);
	sprintf (buf, "Task_%d_Svc", task_->id_);


	ofstream out (buf);

	for (int i = 0; i < all_events_.size ();i++)
		out << all_events_[i].time_/1000 << endl;

	out.close ();
}