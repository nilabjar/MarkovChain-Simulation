
#include "task.h"

Task::Task ()
	: arr_state_(1),
		svc_state_(0),
		deadline_miss_ (0),
		total_arrival_ (0)
{
}