
#include "Arrival.h"
#include "Utility.h"
#include "Simulator.h"
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

Arrival::Arrival (Task* task)
	: task_(task),
		cur_idx_ (0),
		start_time_ (0),
		stg_start_time_ (0)
{
	
}

void Arrival::gen_arr_times(int nums)
{

	if (Utility::instance ()->use_file ())
	{
		ostringstream str;
		str << "Task_" << this->task_->id_ << "_Arr";
	  
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
			Utility::instance ()->gen_erlang_time (task_->arr_mean_,task_->arr_stgs_);

		Event ev;
		ev.task_ = this->task_;
		ev.tp_ = Type::arrival;
		ev.time_ = time*1000; // convert to millisecond ...

		this->all_events_.push_back (ev);
	}
}

void Arrival::gen_arr_stg_times (int nums)
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

void Arrival::stage_occurs ()
{
	this->stg_start_time_ = 
		Simulator::instance ()->current_sim_time ();
}

Event& Arrival::get_next_stage ()
{
	Event ev;

	ev.time_ = (this->task_->arr_mean_*1000)/this->task_->arr_stgs_ + this->stg_start_time_;
	ev.tp_ = Type::arrival;
	ev.task_ = this->task_;

	return ev;
}

void Arrival::save_task_times ()
{
	char buf [50];
	memset(buf , 0 , 50);
	sprintf (buf, "Task_%d_Arr", task_->id_);

	ofstream out (buf);

	for (int i = 0; i < all_events_.size ();i++)
		out << all_events_[i].time_ /1000<< endl;

	out.close ();
}