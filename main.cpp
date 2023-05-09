#include <cassert>
#include <iostream>
#include <thread>

#include "lockfree/list.h"
#include "lockfree/queue.h"
#include "lockfree/stack.h"

static const size_t N = 50;

void test_stack() {
  auto s = lockfree::Stack<int>();
  for (auto i = 0; i < N; i++) {
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

    assert(!s.empty());

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

    assert(s.empty());
  }
  std::cout << "stack: ok" << std::endl;
}

void test_list() {
  for (auto i = 0; i < N; i++) {

    auto l = lockfree::DList<int>();

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
  std::cout << "stack: ok" << std::endl;
}

void test_queue() {
  for (auto i = 0; i < N; i++) {

    auto q = lockfree::Queue<int>();

    std::thread t1([&]() {
      q.push(4);
      q.push(2);
      q.push(6);
      q.push(8);
      q.push(10);
    });

    std::thread t2([&]() {
      q.push(3);
      q.push(1);
      q.push(5);
      q.push(7);
      q.push(9);
    });

    t1.join();
    t2.join();

    q.print();

    std::thread t3([&]() {
      q.pop();
      q.pop();
      q.pop();
      q.pop();
      q.pop();
    });

    std::thread t4([&]() {
      q.pop();
      q.pop();
      q.pop();
      q.pop();
      q.pop();
    });

    t3.join();
    t4.join();

    q.print();
  }
  std::cout << "stack: ok" << std::endl;
}

int main(int argc, char** argv) {
  test_stack();
  test_list();
  test_queue();

  return 0;
}
