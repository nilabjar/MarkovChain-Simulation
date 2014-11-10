
#include "Task.h"


Task::Task ()
: arr_ (1, 10),
  svc_ (0, 10)
{
}

Task::Task (int id, int arr, int arr_rate, int svc, int svc_rate)
: id_(id),
  arr_(arr, arr_rate),
  svc_(svc, svc_rate)
{
}

int Task::next_arrival_stage(Event& ev)
{
  ev.task_id = id_;
  ev.state = na;

  if (arr_.next () == 1) // task arrived
  {
    if (svc_.cur_ > 0)// check if deadline missed 
      ev.state = missed;

    svc_.cur_ = 1;
  }

  return arr_.rate_;
}

bool Task::operator== (const Task& rht) const
{
  return (this->arr_.cur_ == rht.arr_.cur_) &&
    (this->svc_.cur_ == rht.svc_.cur_);
}

int Task::next_service_stage (Event& ev)
{
  ev.task_id = id_;
  ev.state = na;

  svc_.next ();

  if (svc_.cur_ == 0) // deadline met ...
    ev.state = met;

  return svc_.rate_;
}

Task Task::operator = (const Task& tsk)
{
  id_ = tsk.id_;
  arr_ = tsk.arr_;
  svc_ = tsk.svc_;

  return (*this);
}