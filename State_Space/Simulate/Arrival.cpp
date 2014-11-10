
#include "Arrival.h"
#include "Utility.h"
#include "Simulator.h"

Arrival::Arrival (Task* task)
	: task_(task),
		cur_idx_ (0),
		start_time_ (0)
{
	
}

void Arrival::gen_arr_times(int nums)
{
	for (unsigned int i = 0;
			 i < nums;
			 i++)
	{
		float time = 
			Utility::instance ()->gen_exp_time (task_->arr_mean_/task_->arr_stgs_);

		Event ev;
		ev.task_ = this->task_;
		ev.tp_ = Type::arrival;
		ev.time_ = time*1000; // convert to millisecond ...

		this->all_events_.push_back (ev);
	}
}

void Arrival::gen_active_events ()
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

Event& Arrival::get_next ()
{
//	return active_events_[cur_idx_];
	Event ev = all_events_[cur_idx_];

	//adjust ev by the actual occurence time 
	ev.time_ += this->start_time_;

	return ev;
}

void Arrival::occurs ()
{
	cur_idx_++;

	this->start_time_ = 
		Simulator::instance ()->current_sim_time ();
}