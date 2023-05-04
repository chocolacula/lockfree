#include <atomic>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>

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

  T& peek() {
    if (_head == nullptr) {
      throw std::runtime_error("The stack is empty");
    }
    return _head->val;
  }

  void push(T&& val) {
    auto* node = new Node(std::forward<T>(val));

    node->next = _head.load(std::memory_order_relaxed);

    while (!_head.compare_exchange_weak(  //
        node->next,                       // expected
        node,                             // desired
        std::memory_order_release,        //
        std::memory_order_relaxed)) {
    }
  }

  void pop() {
    auto* node = _head.load(std::memory_order_relaxed);

    while (!_head.compare_exchange_weak(  //
        node,                             // expected
        node->next,                       // desired
        std::memory_order_release,        //
        std::memory_order_relaxed)) {
    }

    delete node;
  }

  size_t size() {
    size_t s = 0;
    auto* node = _head.load();

    while (node != nullptr) {
      auto* next = node->next;
      s++;
      node = next;
    }
    return s;
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
      std::cout << "deleted " << val << std::endl;
    }
    Node* next;
    T val;
  };

  std::atomic<Node*> _head;
};

}  // namespace lockfree
