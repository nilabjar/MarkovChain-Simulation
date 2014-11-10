#ifndef TASKH
#define TASKH

#include <vector>
#include <list>
#include "Stages.h"

using namespace std;

typedef enum {met, missed, na} Deadline;

typedef enum {contend, non_contend} State_Type;

class Event
{
public:
  int task_id;
  Deadline state;
};

class Task
{
  
public:

  Task (int id, int arr, int arr_rate, int svc, int svc_rate);

  Task ();

  Task operator = (const Task& tsk);

  int id_;
  Arrival arr_;
  Service svc_;
  Deadline state;

  int next_arrival_stage (Event& ev);

  int next_service_stage (Event& ev);

  bool operator== (const Task& rht) const;
};


typedef list<Task> Task_List;

#endif