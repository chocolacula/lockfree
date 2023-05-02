#include <iostream>
#include <utility>

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

  T& front() {
    return _head->next->val;
  }

  T& back() {
    return _tail->prev->val;
  }

  void push_front(T&& val) {
    auto* node = new Node(std::forward<T>(val));
    ins_next(_head, node);
  }

  void push_back(T&& val) {
    auto* node = new Node(std::forward<T>(val));
    ins_prev(_tail, node);
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
