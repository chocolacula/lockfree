#pragma once

#include <atomic>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "hazard_ptr.h"
#include "nop.h"

namespace lockfree {

template <typename T>
struct Stack {
  Stack() : _head(nullptr) {
  }

  ~Stack() {
    auto* node = _head.load();

    while (node != nullptr) {
      auto* next = node->next;
      delete node;
      node = next;
    }
  }

  // from top of the stack
  T& peek() {
    if (_head == nullptr) {
      throw std::runtime_error("The stack is empty");
    }
    return _head->val;
  }

  // to the beginning
  void push(T&& val) {
    auto* node = new Node(std::forward<T>(val));
    size_t n = 1;

    while (true) {
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
      auto* hp = Hazard::get(0);
      auto* node = _head.load(std::memory_order_relaxed);
      // we have to protect node because we set node->next in following CAS
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
    auto* node = _head.load();

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
    explicit Node(T&& val) : val(std::forward<T>(val)), next(nullptr) {
    }
    ~Node() {
      std::string str = "deleted ";
      str += std::to_string(val);
      str += '\n';
      std::cout << str;
    }
    Node* next;
    T val;
  };

  std::atomic<Node*> _head;
};

}  // namespace lockfree
