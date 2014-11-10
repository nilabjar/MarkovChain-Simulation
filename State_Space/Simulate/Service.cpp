
#include "Service.h"
#include "Utility.h"
#include "Simulator.h"

Service::Service (Task* task)
	: task_(task),
		cur_idx_ (0),
		state_(States::arriving)
{
	
}

void Service::gen_svc_times(int nums)
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
}

void Service::preempt ()
{
	this->state_ = States::preempted;

	all_events_[cur_idx_].time_ -= 
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
	this->cur_idx_++;

	this->start_time_ = 
		Simulator::instance ()->current_sim_time ();
}

void Service::job_finished ()
{
	this->state_ = States::arriving;
}