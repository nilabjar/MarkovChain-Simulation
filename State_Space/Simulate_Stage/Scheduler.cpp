
#include "Scheduler.h"
#include "Simulator.h"
#include <fstream>
#include "Utility.h"

Scheduler* Scheduler::sched_ptr_;

Scheduler* Scheduler::instance ()
{
	if (sched_ptr_ == 0)
	{
		sched_ptr_ = new Scheduler ();
	}
	
	return sched_ptr_;
}


void Scheduler::update_state ()
{
	this->current_state_ = 
		Simulator::instance ()->get_current_state ();
}

float Scheduler::get_current_task ()
{
	int state = 
		Simulator::instance ()->get_current_state ();

	//check if all tasks are active ...

	int prio = this->state_p_vec_[state];

	int task_id = -1;
	
	if (prio == 0)
		task_id = 1;
	else if (prio == 1)
		task_id = 0;

	return task_id;
}

Scheduler::Scheduler ()
{
  ifstream in;

	string file_name = Utility::instance ()->state_file ();
	in.open (Utility::instance ()->state_file ().c_str ());
//  in.open ("p_states.txt");

  float val;
  char buf[20];
  while (!in.eof ())
  {
    memset (buf, 0 , 20);
    in >> buf;
    in >> val;

    int state = atoi(buf);
//    p_vector.push_back(val);
    this->state_p_vec_[state] = val;
  }

  in.close ();
}