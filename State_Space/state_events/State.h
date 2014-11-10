#ifndef STATEH
#define STATEH

#include "Edge.h"
#include <string>

class State
{
public:
  // normally there will be an array of tasks ... 
  // but for this example there are only two 
  //tasks ...

  int id;

  Task task_1;
  Task task_2;

  Edge_List out_edges;

  Edge_List in_edges;

	// contains the id of the task currently 
	// holding the cpu
	int cpu_holder_;

  bool operator== (const State& rht) const;

  long config;

  string state_name;

  ~State ();

	bool operator < (const State& rhs)
  {
		return id < rhs.id;
  }

	// the actual probability value ...
	float prob_;

	float p_val_;

};

typedef list <State> State_List;
#endif