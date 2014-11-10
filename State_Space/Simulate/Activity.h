
#ifndef ACTIVITYH
#define ACTIVITYH

#include "task.h"
#include <vector>

using namespace std;

typedef enum {arrival, service} Type;

class Event
{
public:
	Task* task_;
	long double time_;
	Type	tp_;
};

class Activity 
{
public:
	Activity (int stages, int mean);

	// creates an array of arrival times 
	void gen_arr_times (int nums);

private:

	// the task details ....
	Task* task_;

	// array of all events according to distribution
	vector<Event> all_events_;

	// array of only active events ...
	// these are cumulative,
	vector<Event> active_events_;
};
#endif