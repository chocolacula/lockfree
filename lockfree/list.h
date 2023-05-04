#include <iostream>
#include <utility>

#include "spin.h"

namespace lockfree {

template <typename T>
struct List {
  List() : _head(new Node(T())), _tail(new Node(T())) {
    _head->next = _tail;
    _tail->prev = _head;
  }

  ~List() {
    auto* node = _head;

    while (node != nullptr) {
      auto* next = node->next;
      delete node;
      node = next;
    }
  }

  T& front() {
    return _head->next->val;
  }

  T& back() {
    return _tail->prev->val;
  }

  void push_front(T&& val) {
    auto* node = new Node(std::forward<T>(val));

    SpinGuard sg;
    ins_next(_head, node);
  }

  void push_back(T&& val) {
    auto* node = new Node(std::forward<T>(val));

    SpinGuard sg;
    ins_prev(_tail, node);
  }

  void remove(T&& val) {
    auto* node = _head->next;

    while (node != _tail) {
      auto* next = node->next;

      if (node->val == val) {
        {
          SpinGuard sg;
          auto* prev = node->prev;
          auto* next = node->next;

          prev->next = next;
          next->prev = prev;
        }
        delete node;
        break;
      }

      node = next;
    }
  }

  size_t size() {
    size_t s = 0;
    auto* node = _head->next;

    while (node != _tail) {
      s++;
      auto* next = node->next;
      node = next;
    }
    return s;
  }

  bool empty() {
    return _head->next == _tail->prev;
  }

  void print() {
    std::cout << '[';
    auto* node = _head->next;

    while (node != _tail) {
      auto* next = node->next;
      std::cout << node->val;

      node = next;
      if (node != _tail) {
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
      std::cout << "deleted " << val << std::endl;
    }
    Node* prev;
    Node* next;
    T val;
  };

  Node* _head;
  Node* _tail;

  void ins_next(Node* pivot, Node* node) {
    auto* prev = pivot;
    auto* next = pivot->next;

    ins(prev, node, next);
  }

  void ins_prev(Node* pivot, Node* node) {
    auto* prev = pivot->prev;
    auto* next = pivot;

    ins(prev, node, next);
  }

  static inline void ins(Node* prev, Node* node, Node* next) {
    // doesn't affect other threads
    node->prev = prev;
    node->next = next;

    prev->next = node;
    next->prev = node;
  }
};

}  // namespace lockfree
