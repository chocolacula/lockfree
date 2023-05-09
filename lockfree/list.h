#pragma once

#include <iostream>
#include <utility>

#include "spin.h"

namespace lockfree {

template <typename T>
struct DList {
  DList() : _head(nullptr), _tail(nullptr) {
  }

  ~DList() {
    auto* node = _head;

    while (node != nullptr) {
      auto* next = node->next;
      delete node;
      node = next;
    }
  }

  T& front() {
    if (_head == nullptr) {
      throw std::runtime_error("The list is empty");
    }
    return _head->val;
  }

  T& back() {
    if (_head == nullptr) {
      throw std::runtime_error("The list is empty");
    }
    return _tail->val;
  }

  void push_front(T&& val) {
    auto* node = new Node(std::forward<T>(val));

    SpinGuard sg;
    if (_head == nullptr) {
      _head = node;
      _tail = node;
    } else {
      node->next = _head;
      _head->prev = node;
      _head = node;
    }
  }

  void push_back(T&& val) {
    auto* node = new Node(std::forward<T>(val));

    SpinGuard sg;
    if (_tail == nullptr) {
      _tail = node;
      _head = node;
    } else {
      node->prev = _tail;
      _tail->next = node;
      _tail = node;
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
      std::cout << "deleted " << val << std::endl;
    }
    Node* prev;
    Node* next;
    T val;
  };

  Node* _head;
  Node* _tail;
};

}  // namespace lockfree
