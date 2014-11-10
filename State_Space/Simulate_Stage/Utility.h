

#ifndef UTILITYH
#define UTILITYH

#include <string>

using namespace std;

class Utility
{
public:
	static Utility* instance ();
	float gen_exp_time (float mean);

	float gen_erlang_time (float mean, int stages);

	long long total_runs_;

	void reset_seed ();

	void use_file (bool file){use_file_ = true;}

	bool use_file (){return use_file_;}

	void schedule_events (bool file){schedule_events_ = true;}

	bool schedule_events (){return schedule_events_;}

	void state_file (string file){state_file_ = file;}

	string state_file (){return state_file_;}

private:
	Utility ();

	static Utility* util_ptr_;

	bool use_file_;

	string state_file_;

	bool schedule_events_;
};

#endif