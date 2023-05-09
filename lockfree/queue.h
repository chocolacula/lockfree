#pragma once

#include <atomic>
#include <iostream>
#include <utility>

#include "hazard_ptr.h"
#include "nop.h"
#include "spin.h"

namespace lockfree {

template <typename T>
struct Queue {
  Queue() : _head(nullptr), _tail(nullptr) {
  }

  ~Queue() {
    auto* node = _head.load(std::memory_order_relaxed);

    while (node != nullptr) {
      auto* next = node->next.load(std::memory_order_relaxed);
      delete node;
      node = next;
    }
  }

  // from the beginning of the queue
  T& peek() {
    if (_head == nullptr) {
      throw std::runtime_error("The stack is empty");
    }
    return _head->val;
  }

  // to the end
  void push(T&& val) {
    auto* node = new Node(std::forward<T>(val));
    size_t n = 1;
    Node* np = nullptr;

    if (empty()) {
      while (true) {
        if (!_head.compare_exchange_weak(   //
                np,                         // expected
                node,                       // desired
                std::memory_order_release,  //
                std::memory_order_relaxed)) {
          back_off(&n);
          continue;
        }
        if (!_tail.compare_exchange_weak(   //
                np,                         // expected
                node,                       // desired
                std::memory_order_release,  //
                std::memory_order_relaxed)) {
          back_off(&n);
          continue;
        }
        break;
      }
      return;
    }

    while (true) {
      auto* tail = _tail.load(std::memory_order_relaxed);
      auto* hp = Hazard::get(0);
      // we have to protect tail because we set tail->next in following CAS
      *hp = tail;

      if (!tail->next.compare_exchange_weak(  //
              np,                             // expected
              node,                           // desired
              std::memory_order_release,      //
              std::memory_order_relaxed)) {
        back_off(&n);
        continue;
      }
      if (!_tail.compare_exchange_weak(   //
              tail,                       // expected
              node,                       // desired
              std::memory_order_release,  //
              std::memory_order_relaxed)) {
        back_off(&n);
        continue;
      }
      Hazard::retire(tail);
      break;
    }
  }

  // from the beginning
  void pop() {
    if (empty()) {
      return;
    }
    size_t n = 1;

    while (true) {
      // get _head->next as node
      // we have to protect node because we set node->next in following CAS
      // set _head as _head->next

      auto* hp = Hazard::get(0);
      auto* node = _head.load(std::memory_order_relaxed);
      *hp = node;
      if (_head.compare_exchange_weak(    //
              node,                       // expected
              node->next,                 // desired
              std::memory_order_release,  //
              std::memory_order_relaxed)) {
        Hazard::retire(node);
        break;
      }
      back_off(&n);
    }
  }

  bool empty() {
    return _head == nullptr;
  }

  void print() {
    std::cout << '[';
    auto* node = _head.load(std::memory_order_relaxed);

    while (node != nullptr) {
      auto* next = node->next.load(std::memory_order_relaxed);
      std::cout << node->val;

      node = next;
      if (node != nullptr) {
        std::cout << ", ";
      }
    }
    std::cout << ']' << std::endl;
  }

 private:
  struct Node {
    explicit Node(T&& val) : val(std::forward<T>(val)), next(nullptr) {
    }
    ~Node() {
      std::string str = "deleted ";
      str += std::to_string(val);
      std::cout << str << std::endl;
    }
    std::atomic<Node*> next;
    T val;
  };

  std::atomic<Node*> _head;
  std::atomic<Node*> _tail;
};

}  // namespace lockfree
