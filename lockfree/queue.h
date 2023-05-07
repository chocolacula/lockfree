#include <iostream>
#include <utility>

#include "hazard_ptr.h"

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

  T& front() {
    if (_head == nullptr) {
      throw std::runtime_error("The queue is empty");
    }
    return _head->val;
  }

  T& back() {
    if (_head == nullptr) {
      throw std::runtime_error("The queue is empty");
    }
    return _tail->val;
  }

  void push_front(T&& val) {
    auto* node = new Node(std::forward<T>(val));

    if (_head == nullptr) {
      _head = node;  // ???
      _tail = node;  // ???
    } else {
      do {
        auto* head = _head.load(std::memory_order_relaxed);
        node->next = head;
        Hazard::watch(head);

        _head->prev = node;  // ???

      } while (!_head.compare_exchange_weak(  //
          node,                               // expected
          node->next,                         // desired
          std::memory_order_release,          //
          std::memory_order_relaxed));
    }
  }

  void push_back(T&& val) {
    auto* node = new Node(std::forward<T>(val));

    if (_tail == nullptr) {
      _tail = node;  // ???
      _head = node;  // ???
    } else {
      do {
        auto* tail = _tail.load(std::memory_order_relaxed);
        node->prev = tail;
        Hazard::watch(tail);

        _tail->next = node;  // ???

      } while (!_tail.compare_exchange_weak(  //
          node->prev,                         // expected
          node,                               // desired
          std::memory_order_release,          //
          std::memory_order_relaxed));
      Hazard::retire_all();
    }
  }

  void remove(T&& val) {
    auto* node = _head;

    while (node != nullptr) {
      auto* next = node->next;

      if (node->val == val) {
        {
          SpinGuard sg;
          if (node == _head) {
            _head = node->next;
            if (_head != nullptr) {
              _head->prev = nullptr;
            } else {
              // last element was deleted
              _tail = _head;
            }
          } else if (node == _tail) {
            _tail = node->prev;
            if (_tail != nullptr) {
              _tail->next = nullptr;
            } else {
              // last element was deleted
              _head = _tail;
            }
          } else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
          }
        }
        delete node;
        break;
      }

      node = next;
    }
  }

  size_t size() {
    size_t s = 0;
    auto* node = _head;

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
