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
   enum Color { RED, BLACK };

   struct Node {
       value_type data;
       Node *left, *right, *parent;
       Color color;
       size_t subtreeSize;

       Node(const value_type &val, Node *p = nullptr, Color c = RED)
           : data(val), left(nullptr), right(nullptr), parent(p), color(c), subtreeSize(1) {}
   };

   Node *root;
   Node *nil;
   size_t mapSize;
   Compare comp;

   void init() {
       nil = (Node*)operator new(sizeof(Node));
       nil->left = nil->right = nil->parent = nil;
       nil->color = BLACK;
       nil->subtreeSize = 0;
       root = nil;
       mapSize = 0;
   }

   void destroy(Node *node) {
       if (node != nil) {
           destroy(node->left);
           destroy(node->right);
           delete node;
       }
   }

   Node* copyNode(Node *other, Node *parent) {
       if (other == nil) return nil;
       Node *node = new Node(other->data, parent, other->color);
       node->left = copyNode(other->left, node);
       node->right = copyNode(other->right, node);
       node->subtreeSize = other->subtreeSize;
       return node;
   }

   void updateSubtreeSize(Node *node) {
       if (node != nil) {
           node->subtreeSize = node->left->subtreeSize + node->right->subtreeSize + 1;
       }
   }

   void leftRotate(Node *x) {
       Node *y = x->right;
       x->right = y->left;
       if (y->left != nil) {
           y->left->parent = x;
       }
       y->parent = x->parent;
       if (x->parent == nil) {
           root = y;
       } else if (x == x->parent->left) {
           x->parent->left = y;
       } else {
           x->parent->right = y;
       }
       y->left = x;
       x->parent = y;
       updateSubtreeSize(x);
       updateSubtreeSize(y);
   }

   void rightRotate(Node *y) {
       Node *x = y->left;
       y->left = x->right;
       if (x->right != nil) {
           x->right->parent = y;
       }
       x->parent = y->parent;
       if (y->parent == nil) {
           root = x;
       } else if (y == y->parent->right) {
           y->parent->right = x;
       } else {
           y->parent->left = x;
       }
       x->right = y;
       y->parent = x;
       updateSubtreeSize(y);
       updateSubtreeSize(x);
   }

   void insertFixup(Node *z) {
       while (z->parent->color == RED) {
           if (z->parent == z->parent->parent->left) {
               Node *y = z->parent->parent->right;
               if (y->color == RED) {
                   z->parent->color = BLACK;
                   y->color = BLACK;
                   z->parent->parent->color = RED;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       leftRotate(z);
                   }
                   z->parent->color = BLACK;
                   z->parent->parent->color = RED;
                   rightRotate(z->parent->parent);
               }
           } else {
               Node *y = z->parent->parent->left;
               if (y->color == RED) {
                   z->parent->color = BLACK;
                   y->color = BLACK;
                   z->parent->parent->color = RED;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       rightRotate(z);
                   }
                   z->parent->color = BLACK;
                   z->parent->parent->color = RED;
                   leftRotate(z->parent->parent);
               }
           }
       }
       root->color = BLACK;
   }

   void transplant(Node *u, Node *v) {
       if (u->parent == nil) {
           root = v;
       } else if (u == u->parent->left) {
           u->parent->left = v;
       } else {
           u->parent->right = v;
       }
       v->parent = u->parent;
   }

   Node* treeMinimum(Node *node) {
       while (node->left != nil) {
           node = node->left;
       }
       return node;
   }

   void eraseFixup(Node *x) {
       while (x != root && x->color == BLACK) {
           if (x == x->parent->left) {
               Node *w = x->parent->right;
               if (w->color == RED) {
                   w->color = BLACK;
                   x->parent->color = RED;
                   leftRotate(x->parent);
                   w = x->parent->right;
               }
               if (w->left->color == BLACK && w->right->color == BLACK) {
                   w->color = RED;
                   x = x->parent;
               } else {
                   if (w->right->color == BLACK) {
                       w->left->color = BLACK;
                       w->color = RED;
                       rightRotate(w);
                       w = x->parent->right;
                   }
                   w->color = x->parent->color;
                   x->parent->color = BLACK;
                   w->right->color = BLACK;
                   leftRotate(x->parent);
                   x = root;
               }
           } else {
               Node *w = x->parent->left;
               if (w->color == RED) {
                   w->color = BLACK;
                   x->parent->color = RED;
                   rightRotate(x->parent);
                   w = x->parent->left;
               }
               if (w->right->color == BLACK && w->left->color == BLACK) {
                   w->color = RED;
                   x = x->parent;
               } else {
                   if (w->left->color == BLACK) {
                       w->right->color = BLACK;
                       w->color = RED;
                       leftRotate(w);
                       w = x->parent->left;
                   }
                   w->color = x->parent->color;
                   x->parent->color = BLACK;
                   w->left->color = BLACK;
                   rightRotate(x->parent);
                   x = root;
               }
           }
       }
       x->color = BLACK;
   }

   Node* findNode(const Key &key) const {
       Node *current = root;
       while (current != nil) {
           if (comp(key, current->data.first)) {
               current = current->left;
           } else if (comp(current->data.first, key)) {
               current = current->right;
           } else {
               return current;
           }
       }
       return nil;
   }

  public:
   class const_iterator;
   class iterator {
      private:
       map *container;
       Node *node;

      public:
       iterator() : container(nullptr), node(nullptr) {}

       iterator(map *c, Node *n) : container(c), node(n) {}

       iterator(const iterator &other) : container(other.container), node(other.node) {}

       iterator operator++(int) {
           if (node == container->nil || node == container->nil->right) {
               throw invalid_iterator();
           }
           iterator tmp = *this;
           if (node->right != container->nil) {
               node = node->right;
               while (node->left != container->nil) {
                   node = node->left;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return tmp;
       }

       iterator &operator++() {
           if (node == container->nil || node == container->nil->right) {
               throw invalid_iterator();
           }
           if (node->right != container->nil) {
               node = node->right;
               while (node->left != container->nil) {
                   node = node->left;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       iterator operator--(int) {
           if (node == container->nil) {
               Node *max = container->root;
               while (max->right != container->nil) {
                   max = max->right;
               }
               node = max;
               return *this;
           }

           iterator tmp = *this;
           if (node->left != container->nil) {
               node = node->left;
               while (node->right != container->nil) {
                   node = node->right;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           if (node == container->nil) {
               throw invalid_iterator();
           }
           return tmp;
       }

       iterator &operator--() {
           if (node == container->nil) {
               Node *max = container->root;
               while (max->right != container->nil) {
                   max = max->right;
               }
               node = max;
               return *this;
           }

           if (node->left != container->nil) {
               node = node->left;
               while (node->right != container->nil) {
                   node = node->right;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           if (node == container->nil) {
               throw invalid_iterator();
           }
           return *this;
       }

       value_type &operator*() const {
           if (node == container->nil) {
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

      public:
       const_iterator() : container(nullptr), node(nullptr) {}

       const_iterator(const map *c, Node *n) : container(c), node(n) {}

       const_iterator(const const_iterator &other) : container(other.container), node(other.node) {}

       const_iterator(const iterator &other) : container(other.container), node(other.node) {}

       const_iterator operator++(int) {
           if (node == container->nil || node == container->nil->right) {
               throw invalid_iterator();
           }
           const_iterator tmp = *this;
           if (node->right != container->nil) {
               node = node->right;
               while (node->left != container->nil) {
                   node = node->left;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return tmp;
       }

       const_iterator &operator++() {
           if (node == container->nil || node == container->nil->right) {
               throw invalid_iterator();
           }
           if (node->right != container->nil) {
               node = node->right;
               while (node->left != container->nil) {
                   node = node->left;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       const_iterator operator--(int) {
           if (node == container->nil) {
               Node *max = container->root;
               while (max->right != container->nil) {
                   max = max->right;
               }
               node = max;
               return *this;
           }

           const_iterator tmp = *this;
           if (node->left != container->nil) {
               node = node->left;
               while (node->right != container->nil) {
                   node = node->right;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           if (node == container->nil) {
               throw invalid_iterator();
           }
           return tmp;
       }

       const_iterator &operator--() {
           if (node == container->nil) {
               Node *max = container->root;
               while (max->right != container->nil) {
                   max = max->right;
               }
               node = max;
               return *this;
           }

           if (node->left != container->nil) {
               node = node->left;
               while (node->right != container->nil) {
                   node = node->right;
               }
           } else {
               Node *parent = node->parent;
               while (parent != container->nil && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           if (node == container->nil) {
               throw invalid_iterator();
           }
           return *this;
       }

       const value_type &operator*() const {
           if (node == container->nil) {
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

   map() {
       init();
   }

   map(const map &other) {
       init();
       root = copyNode(other.root, nil);
       mapSize = other.mapSize;
   }

   map &operator=(const map &other) {
       if (this != &other) {
           clear();
           root = copyNode(other.root, nil);
           mapSize = other.mapSize;
       }
       return *this;
   }

   ~map() {
       destroy(root);
       operator delete(nil);
   }

   T &at(const Key &key) {
       Node *node = findNode(key);
       if (node == nil) {
           throw index_out_of_bound();
       }
       return node->data.second;
   }

   const T &at(const Key &key) const {
       Node *node = findNode(key);
       if (node == nil) {
           throw index_out_of_bound();
       }
       return node->data.second;
   }

   T &operator[](const Key &key) {
       Node *node = findNode(key);
       if (node == nil) {
           value_type tempValue(key, T());
           auto result = insert(tempValue);
           return result.first->second;
       }
       return node->data.second;
   }

   const T &operator[](const Key &key) const {
       return at(key);
   }

   iterator begin() {
       if (root == nil) {
           return iterator(this, nil);
       }
       Node *node = root;
       while (node->left != nil) {
           node = node->left;
       }
       return iterator(this, node);
   }

   const_iterator cbegin() const {
       if (root == nil) {
           return const_iterator(this, nil);
       }
       Node *node = root;
       while (node->left != nil) {
           node = node->left;
       }
       return const_iterator(this, node);
   }

   iterator end() {
       return iterator(this, nil);
   }

   const_iterator cend() const {
       return const_iterator(this, nil);
   }

   bool empty() const {
       return mapSize == 0;
   }

   size_t size() const {
       return mapSize;
   }

   void clear() {
       destroy(root);
       root = nil;
       mapSize = 0;
   }

   pair<iterator, bool> insert(const value_type &value) {
       Node *current = root;
       Node *parent = nil;

       while (current != nil) {
           parent = current;
           if (comp(value.first, current->data.first)) {
               current = current->left;
           } else if (comp(current->data.first, value.first)) {
               current = current->right;
           } else {
               return pair<iterator, bool>(iterator(this, current), false);
           }
       }

       Node *newNode = new Node(value, parent, RED);
       newNode->left = newNode->right = nil;

       if (parent == nil) {
           root = newNode;
       } else if (comp(value.first, parent->data.first)) {
           parent->left = newNode;
       } else {
           parent->right = newNode;
       }

       mapSize++;

       Node *temp = newNode;
       while (temp != nil) {
           updateSubtreeSize(temp);
           temp = temp->parent;
       }

       insertFixup(newNode);

       return pair<iterator, bool>(iterator(this, newNode), true);
   }

   void erase(iterator pos) {
       if (pos.node == nil || pos.container != this) {
           throw invalid_iterator();
       }

       Node *z = pos.node;
       Node *y = z;
       Node *x;
       Color yOriginalColor = y->color;

       if (z->left == nil) {
           x = z->right;
           transplant(z, z->right);
       } else if (z->right == nil) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = treeMinimum(z->right);
           yOriginalColor = y->color;
           x = y->right;
           if (y->parent == z) {
               x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->color = z->color;
       }

       Node *temp = x->parent;
       while (temp != nil) {
           updateSubtreeSize(temp);
           temp = temp->parent;
       }

       delete z;
       mapSize--;

       if (yOriginalColor == BLACK) {
           eraseFixup(x);
       }
   }

   size_t count(const Key &key) const {
       return findNode(key) != nil ? 1 : 0;
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