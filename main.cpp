#include <cassert>
#include <iostream>
#include <thread>

#include "lockfree/list.h"
#include "lockfree/stack.h"

void test_stack() {
  auto s = lockfree::Stack<int>();
  for (auto i = 0; i < 100; i++) {
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

    assert(s.empty());

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

    // assert(s.empty());
  }
}

void test_list() {
  auto l = lockfree::Queue<int>();

  std::thread t1([&]() {
    l.push_front(4);
    l.push_front(2);
    l.push_back(6);
    l.push_back(8);
    l.push_back(10);
  });

  std::thread t2([&]() {
    l.push_front(3);
    l.push_front(1);
    l.push_back(5);
    l.push_back(7);
    l.push_back(9);
  });

  t1.join();
  t2.join();

  l.print();

  std::thread t3([&]() {
    l.remove(4);
    l.remove(2);
    l.remove(6);
    l.remove(8);
    l.remove(10);
  });

  std::thread t4([&]() {
    l.remove(3);
    l.remove(1);
    l.remove(5);
    l.remove(7);
    l.remove(9);
  });

  t3.join();
  t4.join();

  l.print();
}

int main(int argc, char** argv) {
  test_stack();
  // test_list();

  return 0;
}
