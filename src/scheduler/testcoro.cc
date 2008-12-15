#include <cassert>
#include <iostream>
#include <scheduler/coroutine.hh>

Coro* mc;
Coro* c1, *c2;

static void start2(void*)
{
  int x=1;
  std::cerr <<"c2 start " << std::endl;
  std::cerr <<"c2->c1 " << std::endl;
  x++;
  coroutine_switch_to(c2, c1);
  assert(x==2);
  x++;
  std::cerr <<"c2->main " << std::endl;
  assert(x==3);
  coroutine_switch_to(c2, mc);
  std::cerr <<"END!!" << std::endl;
}
static void start(void*)
{
  int x=0;
  std::cerr <<"c1 start" << std::endl;
  c2 = coroutine_new();
  std::cerr <<"c1->start c2" << std::endl;
  x++;
  coroutine_start(c1, c2, &start2, (void*)0);
  assert(x==1);x++;
  std::cerr <<"c1->main" << std::endl;
  coroutine_switch_to(c1, mc);
  assert(x==2);x++;
  std::cerr <<"c1->main" << std::endl;
  coroutine_switch_to(c1, mc);
  assert(x==3);x++;
  std::cerr <<"END!!" << std::endl;
}
int main()
{
  mc = coroutine_new();
  coroutine_initialize_main(mc);
  c1 = coroutine_new();
  std::cerr <<"Starting c1 " << std::endl;
  coroutine_start(mc, c1, &start, (void*)0);
  std::cerr <<"Main->c1" << std::endl;
  coroutine_switch_to(mc, c1);
  std::cerr <<"Main->c2" << std::endl;
  coroutine_switch_to(mc, c2);
  std::cerr <<"Main done" << std::endl;
}
