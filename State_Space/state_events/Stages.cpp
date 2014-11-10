
#include "Stages.h"

Stages::Stages (int size, int rate)
: size_ (size),
  rate_ (rate)
{
  cur_ = 0;
}

Arrival::Arrival (int size, int rate)
: Stages (size, rate)
{
  cur_ = 1;
}

int Arrival::next ()
{
  cur_++;

  if (cur_ > size_)
    cur_ = 1;

  return cur_;
}

Service::Service (int size, int rate)
: Stages (size, rate)
{
}

int Service::next ()
{
  cur_++;

  if (cur_ > size_)
    cur_ = 0;

  return cur_;
}