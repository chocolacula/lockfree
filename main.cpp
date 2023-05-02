#include <iostream>
#include <thread>

#include "list.h"
#include "spinlock.h"
#include "stack.h"

int main(int argc, char** argv) {
  /*
  auto l = lockfree::List<int>();

  l.push_front(4);
  l.push_front(3);
  l.push_back(5);
  l.push_back(6);

  l.print();
  */

  auto s = lockfree::Stack<int>();
  std::thread t1([&]() {
    s.push(2);
    s.push(4);
    s.push(6);
    s.push(8);
    s.push(10);
  });

  std::thread t2([&]() {
    s.push(1);
    s.push(3);
    s.push(5);
    s.push(7);
    s.push(9);
  });

  t1.join();
  t2.join();

  s.print();

  std::thread t3([&]() {
    s.pop();
    s.pop();
    s.pop();
    s.pop();
    s.pop();
  });

  std::thread t4([&]() {
    s.pop();
    s.pop();
    s.pop();
    s.pop();
    s.pop();
  });

  t3.join();
  t4.join();

  s.print();

  return 0;
}
