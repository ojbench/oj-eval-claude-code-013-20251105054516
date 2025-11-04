/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   typedef pair<const Key, T> value_type;

  private:
   struct Node {
       value_type data;
       Node *left, *right, *parent;
       int height;

       Node(const value_type &val)
           : data(val), left(nullptr), right(nullptr), parent(nullptr), height(1) {}
   };

   Node *root;
   size_t mapSize;
   Compare comp;

   int getHeight(Node *node) {
       return node ? node->height : 0;
   }

   int getBalance(Node *node) {
       return node ? getHeight(node->left) - getHeight(node->right) : 0;
   }

   void updateHeight(Node *node) {
       if (node) {
           node->height = 1 + (getHeight(node->left) > getHeight(node->right) ?
                              getHeight(node->left) : getHeight(node->right));
       }
   }

   Node* rightRotate(Node *y) {
       Node *x = y->left;
       Node *T2 = x->right;

       x->right = y;
       y->left = T2;

       if (T2) T2->parent = y;
       x->parent = y->parent;
       y->parent = x;

       updateHeight(y);
       updateHeight(x);

       return x;
   }

   Node* leftRotate(Node *x) {
       Node *y = x->right;
       Node *T2 = y->left;

       y->left = x;
       x->right = T2;

       if (T2) T2->parent = x;
       y->parent = x->parent;
       x->parent = y;

       updateHeight(x);
       updateHeight(y);

       return y;
   }

   Node* balanceNode(Node *node) {
       if (!node) return nullptr;

       updateHeight(node);

       int balance = getBalance(node);

       if (balance > 1) {
           if (getBalance(node->left) >= 0) {
               Node *newRoot = rightRotate(node);
               if (newRoot->parent) {
                   if (newRoot->parent->left == node) {
                       newRoot->parent->left = newRoot;
                   } else {
                       newRoot->parent->right = newRoot;
                   }
               }
               return newRoot;
           } else {
               node->left = leftRotate(node->left);
               node->left->parent = node;
               Node *newRoot = rightRotate(node);
               if (newRoot->parent) {
                   if (newRoot->parent->left == node) {
                       newRoot->parent->left = newRoot;
                   } else {
                       newRoot->parent->right = newRoot;
                   }
               }
               return newRoot;
           }
       }

       if (balance < -1) {
           if (getBalance(node->right) <= 0) {
               Node *newRoot = leftRotate(node);
               if (newRoot->parent) {
                   if (newRoot->parent->left == node) {
                       newRoot->parent->left = newRoot;
                   } else {
                       newRoot->parent->right = newRoot;
                   }
               }
               return newRoot;
           } else {
               node->right = rightRotate(node->right);
               node->right->parent = node;
               Node *newRoot = leftRotate(node);
               if (newRoot->parent) {
                   if (newRoot->parent->left == node) {
                       newRoot->parent->left = newRoot;
                   } else {
                       newRoot->parent->right = newRoot;
                   }
               }
               return newRoot;
           }
       }

       return node;
   }

   Node* insertNode(Node *node, const value_type &value) {
       if (!node) {
           mapSize++;
           return new Node(value);
       }

       if (comp(value.first, node->data.first)) {
           node->left = insertNode(node->left, value);
           node->left->parent = node;
       } else if (comp(node->data.first, value.first)) {
           node->right = insertNode(node->right, value);
           node->right->parent = node;
       } else {
           return node;
       }

       return balanceNode(node);
   }

   Node* findMin(Node *node) {
       while (node && node->left) {
           node = node->left;
       }
       return node;
   }

   Node* eraseNode(Node *node, const Key &key) {
       if (!node) return nullptr;

       if (comp(key, node->data.first)) {
           node->left = eraseNode(node->left, key);
           if (node->left) node->left->parent = node;
       } else if (comp(node->data.first, key)) {
           node->right = eraseNode(node->right, key);
           if (node->right) node->right->parent = node;
       } else {
           if (!node->left || !node->right) {
               Node *temp = node->left ? node->left : node->right;

               if (!temp) {
                   delete node;
                   mapSize--;
                   return nullptr;
               } else {
                   temp->parent = node->parent;
                   delete node;
                   mapSize--;
                   return temp;
               }
           } else {
               Node *temp = findMin(node->right);
               const_cast<Key&>(node->data.first) = temp->data.first;
                   node->data.second = temp->data.second;
               node->right = eraseNode(node->right, temp->data.first);
               if (node->right) node->right->parent = node;
           }
       }

       Node *result = balanceNode(node);
       if (result && result->left) result->left->parent = result;
       if (result && result->right) result->right->parent = result;
       return result;
   }

   void destroy(Node *node) {
       if (node) {
           destroy(node->left);
           destroy(node->right);
           delete node;
       }
   }

   Node* copyNode(Node *other) {
       if (!other) return nullptr;
       Node *node = new Node(other->data);
       node->left = copyNode(other->left);
       node->right = copyNode(other->right);
       node->height = other->height;
       if (node->left) node->left->parent = node;
       if (node->right) node->right->parent = node;
       return node;
   }

   Node* findNode(const Key &key) const {
       Node *current = root;
       while (current) {
           if (comp(key, current->data.first)) {
               current = current->left;
           } else if (comp(current->data.first, key)) {
               current = current->right;
           } else {
               return current;
           }
       }
       return nullptr;
   }

  public:
   class const_iterator;
   class iterator {
      private:
       map *container;
       Node *node;

       Node* findInorderSuccessor(Node *n) {
           if (!n) return nullptr;

           if (n->right) {
               Node *curr = n->right;
               while (curr->left) {
                   curr = curr->left;
               }
               return curr;
           }

           Node *p = n->parent;
           while (p && n == p->right) {
               n = p;
               p = p->parent;
           }
           return p;
       }

       Node* findInorderPredecessor(Node *n) {
           if (!n) return nullptr;

           if (n->left) {
               Node *curr = n->left;
               while (curr->right) {
                   curr = curr->right;
               }
               return curr;
           }

           Node *p = n->parent;
           while (p && n == p->left) {
               n = p;
               p = p->parent;
           }
           return p;
       }

      public:
       iterator() : container(nullptr), node(nullptr) {}

       iterator(map *c, Node *n) : container(c), node(n) {}

       iterator(const iterator &other) : container(other.container), node(other.node) {}

       iterator operator++(int) {
           if (!node || !container) {
               throw invalid_iterator();
           }
           iterator tmp = *this;
           node = findInorderSuccessor(node);
           return tmp;
       }

       iterator &operator++() {
           if (!node || !container) {
               throw invalid_iterator();
           }
           node = findInorderSuccessor(node);
           return *this;
       }

       iterator operator--(int) {
           if (!container) {
               throw invalid_iterator();
           }
           iterator tmp = *this;
           if (!node) {
               node = container->root;
               if (node) {
                   while (node->right) {
                       node = node->right;
                   }
               }
           } else {
               node = findInorderPredecessor(node);
           }
           if (!node) {
               throw invalid_iterator();
           }
           return tmp;
       }

       iterator &operator--() {
           if (!container) {
               throw invalid_iterator();
           }
           if (!node) {
               node = container->root;
               if (node) {
                   while (node->right) {
                       node = node->right;
                   }
               }
           } else {
               node = findInorderPredecessor(node);
           }
           if (!node) {
               throw invalid_iterator();
           }
           return *this;
       }

       value_type &operator*() const {
           if (!node) {
               throw invalid_iterator();
           }
           return node->data;
       }

       bool operator==(const iterator &rhs) const {
           return container == rhs.container && node == rhs.node;
       }

       bool operator==(const const_iterator &rhs) const {
           return container == rhs.container && node == rhs.node;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       value_type *operator->() const noexcept {
           return &(node->data);
       }

       friend class const_iterator;
       friend class map;
   };

   class const_iterator {
      private:
       const map *container;
       Node *node;

       Node* findInorderSuccessor(Node *n) {
           if (!n) return nullptr;

           if (n->right) {
               Node *curr = n->right;
               while (curr->left) {
                   curr = curr->left;
               }
               return curr;
           }

           Node *p = n->parent;
           while (p && n == p->right) {
               n = p;
               p = p->parent;
           }
           return p;
       }

       Node* findInorderPredecessor(Node *n) {
           if (!n) return nullptr;

           if (n->left) {
               Node *curr = n->left;
               while (curr->right) {
                   curr = curr->right;
               }
               return curr;
           }

           Node *p = n->parent;
           while (p && n == p->left) {
               n = p;
               p = p->parent;
           }
           return p;
       }

      public:
       const_iterator() : container(nullptr), node(nullptr) {}

       const_iterator(const map *c, Node *n) : container(c), node(n) {}

       const_iterator(const const_iterator &other) : container(other.container), node(other.node) {}

       const_iterator(const iterator &other) : container(other.container), node(other.node) {}

       const_iterator operator++(int) {
           if (!node || !container) {
               throw invalid_iterator();
           }
           const_iterator tmp = *this;
           node = findInorderSuccessor(node);
           return tmp;
       }

       const_iterator &operator++() {
           if (!node || !container) {
               throw invalid_iterator();
           }
           node = findInorderSuccessor(node);
           return *this;
       }

       const_iterator operator--(int) {
           if (!container) {
               throw invalid_iterator();
           }
           const_iterator tmp = *this;
           if (!node) {
               node = container->root;
               if (node) {
                   while (node->right) {
                       node = node->right;
                   }
               }
           } else {
               node = findInorderPredecessor(node);
           }
           if (!node) {
               throw invalid_iterator();
           }
           return tmp;
       }

       const_iterator &operator--() {
           if (!container) {
               throw invalid_iterator();
           }
           if (!node) {
               node = container->root;
               if (node) {
                   while (node->right) {
                       node = node->right;
                   }
               }
           } else {
               node = findInorderPredecessor(node);
           }
           if (!node) {
               throw invalid_iterator();
           }
           return *this;
       }

       const value_type &operator*() const {
           if (!node) {
               throw invalid_iterator();
           }
           return node->data;
       }

       bool operator==(const iterator &rhs) const {
           return container == rhs.container && node == rhs.node;
       }

       bool operator==(const const_iterator &rhs) const {
           return container == rhs.container && node == rhs.node;
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       const value_type *operator->() const noexcept {
           return &(node->data);
       }

       friend class map;
   };

   map() : root(nullptr), mapSize(0) {}

   map(const map &other) : mapSize(0) {
       root = copyNode(other.root);
       mapSize = other.mapSize;
   }

   map &operator=(const map &other) {
       if (this != &other) {
           clear();
           root = copyNode(other.root);
           mapSize = other.mapSize;
       }
       return *this;
   }

   ~map() {
       destroy(root);
   }

   T &at(const Key &key) {
       Node *node = findNode(key);
       if (!node) {
           throw index_out_of_bound();
       }
       return node->data.second;
   }

   const T &at(const Key &key) const {
       Node *node = findNode(key);
       if (!node) {
           throw index_out_of_bound();
       }
       return node->data.second;
   }

   T &operator[](const Key &key) {
       Node *node = findNode(key);
       if (!node) {
           value_type tempValue(key, T());
           root = insertNode(root, tempValue);
           node = findNode(key);
       }
       return node->data.second;
   }

   const T &operator[](const Key &key) const {
       return at(key);
   }

   iterator begin() {
       if (!root) {
           return iterator(this, nullptr);
       }
       Node *node = root;
       while (node->left) {
           node = node->left;
       }
       return iterator(this, node);
   }

   const_iterator cbegin() const {
       if (!root) {
           return const_iterator(this, nullptr);
       }
       Node *node = root;
       while (node->left) {
           node = node->left;
       }
       return const_iterator(this, node);
   }

   iterator end() {
       return iterator(this, nullptr);
   }

   const_iterator cend() const {
       return const_iterator(this, nullptr);
   }

   bool empty() const {
       return mapSize == 0;
   }

   size_t size() const {
       return mapSize;
   }

   void clear() {
       destroy(root);
       root = nullptr;
       mapSize = 0;
   }

   pair<iterator, bool> insert(const value_type &value) {
       Node *existingNode = findNode(value.first);
       if (existingNode) {
           return pair<iterator, bool>(iterator(this, existingNode), false);
       }

       root = insertNode(root, value);
       Node *newNode = findNode(value.first);
       return pair<iterator, bool>(iterator(this, newNode), true);
   }

   void erase(iterator pos) {
       if (!pos.node || pos.container != this) {
           throw invalid_iterator();
       }

       root = eraseNode(root, pos.node->data.first);
   }

   size_t count(const Key &key) const {
       return findNode(key) ? 1 : 0;
   }

   iterator find(const Key &key) {
       Node *node = findNode(key);
       return iterator(this, node);
   }

   const_iterator find(const Key &key) const {
       Node *node = findNode(key);
       return const_iterator(this, node);
   }
};

}

#endif