#include "State.h"

bool State::operator== (const State& rht) const
{
  return (this->task_1 == rht.task_1) && 
    (this->task_2 == rht.task_2);
}

State::~State ()
{
  this->out_edges.clear ();
  this->in_edges.clear ();
}