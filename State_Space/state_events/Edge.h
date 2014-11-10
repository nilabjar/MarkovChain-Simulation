#ifndef EDGEH
#define EDGEH

#include "Task.h"

class Edge
{
public:
  float rate;
  int end;
  Event ev;
  State_Type  type;
  int variable_no;
};

typedef vector<Edge> Edge_List;

#endif