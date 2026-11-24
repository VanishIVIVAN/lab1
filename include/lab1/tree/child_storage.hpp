#pragma once

#include <cstddef>
#include <iterator>
#include <list>
#include <memory>
#include <string>
#include <utility>

namespace lab1 {

/**
 * @brief Child storage policy #1: own singly linked list.
 *
 * Together with StdChildList it forms a compile-time "concept":
 * size(), append(), at(), removeAt(), forEach(), storageName().
 */
template <typename Node>
class LinkedChildList {
 public:
  LinkedChildList() = default;
  LinkedChildList(const LinkedChildList&) = delete;
  LinkedChildList& operator=(const LinkedChildList&) = delete;

  /// Iterative destruction avoids deep recursion on long lists.
  ~LinkedChildList() {
    while (head_) head_ = std::move(head_->next);
  }

  std::size_t size() const { return size_; }

  /// Appends a child to the end and returns a reference to it.
  Node& append(std::unique_ptr<Node> node) {
    auto link = std::make_unique<Link>();
    link->node = std::move(node);
    Link* raw = link.get();
    if (tail_ != nullptr) {
      tail_->next = std::move(link);
    } else {
      head_ = std::move(link);
    }
    tail_ = raw;
    ++size_;
    return *raw->node;
  }

  /// Child with the given index or nullptr.
  Node* at(std::size_t index) const {
    if (index >= size_) return nullptr;
    const Link* link = head_.get();
    for (std::size_t i = 0; i < index; ++i) link = link->next.get();
    return link->node.get();
  }

  /// Detaches and returns the child with the given index (nullptr if out of range).
  std::unique_ptr<Node> removeAt(std::size_t index) {
    if (index >= size_) return nullptr;
    std::unique_ptr<Link>* slot = &head_;
    Link* previous = nullptr;
    for (std::size_t i = 0; i < index; ++i) {
      previous = slot->get();
      slot = &previous->next;
    }
    std::unique_ptr<Link> removed = std::move(*slot);
    *slot = std::move(removed->next);
    if (removed.get() == tail_) tail_ = previous;
    --size_;
    return std::move(removed->node);
  }

  template <typename Function>
  void forEach(Function&& function) const {
    for (Link* link = head_.get(); link != nullptr; link = link->next.get()) function(*link->node);
  }

  static std::string storageName() { return "linked list"; }

 private:
  struct Link {
    std::unique_ptr<Node> node;
    std::unique_ptr<Link> next;
  };

  std::unique_ptr<Link> head_;
  Link* tail_ = nullptr;
  std::size_t size_ = 0;
};

/**
 * @brief Child storage policy #2: library std::list.
 */
template <typename Node>
class StdChildList {
 public:
  std::size_t size() const { return items_.size(); }

  Node& append(std::unique_ptr<Node> node) {
    items_.push_back(std::move(node));
    return *items_.back();
  }

  Node* at(std::size_t index) const {
    if (index >= items_.size()) return nullptr;
    auto it = items_.begin();
    std::advance(it, static_cast<std::ptrdiff_t>(index));
    return it->get();
  }

  std::unique_ptr<Node> removeAt(std::size_t index) {
    if (index >= items_.size()) return nullptr;
    auto it = items_.begin();
    std::advance(it, static_cast<std::ptrdiff_t>(index));
    std::unique_ptr<Node> removed = std::move(*it);
    items_.erase(it);
    return removed;
  }

  template <typename Function>
  void forEach(Function&& function) const {
    for (const auto& item : items_) function(*item);
  }

  static std::string storageName() { return "std::list"; }

 private:
  std::list<std::unique_ptr<Node>> items_;
};

}  // namespace lab1
