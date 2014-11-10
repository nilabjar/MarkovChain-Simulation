#ifndef STAGESH
#define STAGESH

class Stages
{
public:
  Stages (int size, int rate);
  int size_;
  int cur_;
  int rate_;

  int next (){}
};

class Arrival : public Stages
{
public:
  Arrival (int size, int rate);

  int next ();
};

class Service : public Stages
{
public:
  Service (int size, int rate);

  int next ();
};

#endif