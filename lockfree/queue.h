#pragma once

#include <iostream>
#include <utility>

#include "hazard_ptr.h"
#include "nop.h"

namespace lockfree {

template <typename T>
struct Queue {
  Queue() : _head(nullptr), _tail(nullptr) {
  }

  ~Queue() {
    auto* node = _head;

    while (node != nullptr) {
      auto* next = node->next;
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

    while (true) {

      // if q is empty
      // we need to set _heap and _tail the node
      // else
      // set _tail->next as node

      auto* next = _head.load(std::memory_order_relaxed);
      node->next = next;

      if (_head.compare_exchange_weak(    //
              node->next,                 // expected
              node,                       // desired
              std::memory_order_release,  //
              std::memory_order_relaxed)) {
        break;
      }
      back_off(&n);
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
    auto* node = _head;

    while (node != nullptr) {
      auto* next = node->next;
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
    explicit Node(T&& val) : val(std::forward<T>(val)), next(nullptr), prev(nullptr) {
    }
    ~Node() {
      std::string str = "deleted ";
      str += std::to_string(val);
      std::cout << str << std::endl;
    }
    Node* prev;
    Node* next;
    T val;
  };

  std::atomic<Node*> _head;
  std::atomic<Node*> _tail;
};

}  // namespace lockfree
