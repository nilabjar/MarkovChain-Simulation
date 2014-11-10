
#ifndef TASKH
#define TASKH

class Arrival;
class Service;

class Task
{
public:

	// Constructor ...
	Task ();

		// the number of stages
	int stages_;

	// The arrival rate 
	float arr_mean_;

	//The service rate
	float svc_mean_;

	//arrival stages 
	int arr_stgs_;

	//service stages 
	int svc_stgs_;

	// id of the task
	int id_;

	// state variable 
	int arr_state_;

	int svc_state_;

	long long deadline_miss_;

	long long total_arrival_;

	Arrival* arrival_;

	Service* service_;

	Arrival* arr_ests_;

	Service* svc_ests_;
};

#endif