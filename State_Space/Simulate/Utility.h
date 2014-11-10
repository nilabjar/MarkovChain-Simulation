

#ifndef UTILITYH
#define UTILITYH

class Utility
{
public:
	static Utility* instance ();
	float gen_exp_time (float rate);

	long long total_runs_;

	void reset_seed ();

private:
	static Utility* util_ptr_;
};

#endif