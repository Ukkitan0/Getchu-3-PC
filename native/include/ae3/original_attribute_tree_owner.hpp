#pragma once
#include "ae3/original_attribute_lookup.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ae3 {
// Source 0x541CA0 inserts a 0x18-byte map node (the source's value
// starts at +0x10). Source 0x49C26C rebalances the red/black tree:
// +0x00 color (0 red, 1 black), +0x04 parent, +0x08 left,
// +0x0C right, +0x10 original string key object.
// The host uses native pointers and owned keys, not 32-bit guest VAs.
class OriginalAttributeTreeOwner final {
public:
    OriginalAttributeTreeOwner()=default;
    OriginalAttributeTreeOwner(const OriginalAttributeTreeOwner&)=delete;
    OriginalAttributeTreeOwner& operator=(const OriginalAttributeTreeOwner&)=delete;
    OriginalAttributeTreeOwner(OriginalAttributeTreeOwner&&)=delete;
    OriginalAttributeTreeOwner& operator=(OriginalAttributeTreeOwner&&)=delete;

    // 0x541DAC unique insertion: an existing key is not overwritten.
    const OriginalAttributeKeyNode* originalInsertOrFind_541dac(
        std::string_view originalKey, std::uint32_t sourceValueOffset) {
        Node* parent=nullptr;
        Node* node=root_;
        while(node) {
            parent=node;
            const int c=originalKey.compare(node->key);
            if(c==0)return &node->decoded;
            node=(c<0)?node->left:node->right;
        }
        auto owned=std::make_unique<Node>(originalKey,sourceValueOffset);
        Node* inserted=owned.get();
        inserted->parent=parent;
        nodes_.push_back(std::move(owned));
        if(!parent){
            root_=inserted;
            leftmost_=rightmost_=inserted;
        } else if(inserted->key.compare(parent->key)<0) {
            parent->left=inserted;
            sync(parent);
            if(parent==leftmost_)leftmost_=inserted;
        } else {
            parent->right=inserted;
            sync(parent);
            if(parent==rightmost_)rightmost_=inserted;
        }
        originalRebalance_49c26c(inserted);
        return &inserted->decoded;
    }
    const OriginalAttributeKeyNode* originalFind_5419ec(std::string_view key) const noexcept {
        return originalAttributeFind_5419ec(decodedRoot(),key);
    }
    const OriginalAttributeKeyNode* decodedRoot() const noexcept {
        return root_?&root_->decoded:nullptr;
    }
    const OriginalAttributeKeyNode* decodedLeftmost() const noexcept {
        return leftmost_?&leftmost_->decoded:nullptr;
    }
    const OriginalAttributeKeyNode* decodedRightmost() const noexcept {
        return rightmost_?&rightmost_->decoded:nullptr;
    }
    std::size_t size() const noexcept {return nodes_.size();}
    void clear() noexcept {root_=leftmost_=rightmost_=nullptr;nodes_.clear();}

    // Verifies the *source* RBT invariants after native rotations. This
    // is diagnostic only, not code executed by the original runtime.
    bool originalInvariantsHold() const noexcept {
        if(!root_)return nodes_.empty()&&!leftmost_&&!rightmost_;
        if(!root_->black||root_->parent!=nullptr)return false;
        int expectedDepth=-1;
        std::size_t count=0;
        if(!validate(root_,nullptr,nullptr,nullptr,0,expectedDepth,count))return false;
        if(count!=nodes_.size())return false;
        const Node* left=root_;while(left->left)left=left->left;
        const Node* right=root_;while(right->right)right=right->right;
        return left==leftmost_ && right==rightmost_;
    }
private:
    struct Node {
        // decoded must be updated after each source tree rotation;
        // the existing 0x5419EC lookup consumes this exact view.
        OriginalAttributeKeyNode decoded{};
        std::string key;
        Node* parent=nullptr;
        Node* left=nullptr;
        Node* right=nullptr;
        bool black=false;
        Node(std::string_view name,std::uint32_t off):key(name){
            decoded.originalKey=key;
            decoded.sourceValueOffset=off;
        }
    };
    static bool isBlack(const Node* n) noexcept{return !n||n->black;}
    static void sync(Node* n) noexcept {
        if(!n)return;
        n->decoded.left=n->left?&n->left->decoded:nullptr;
        n->decoded.right=n->right?&n->right->decoded:nullptr;
    }
    void originalRotateLeft_49c3fc(Node* x) noexcept {
        Node* y=x->right;
        x->right=y->left;
        if(y->left)y->left->parent=x;
        y->parent=x->parent;
        if(!x->parent)root_=y;
        else if(x==x->parent->left)x->parent->left=y;
        else x->parent->right=y;
        sync(x->parent);
        y->left=x;
        x->parent=y;
        sync(x);sync(y);
    }
    void originalRotateRight_49c3b0(Node* x) noexcept {
        Node* y=x->left;
        x->left=y->right;
        if(y->right)y->right->parent=x;
        y->parent=x->parent;
        if(!x->parent)root_=y;
        else if(x==x->parent->right)x->parent->right=y;
        else x->parent->left=y;
        sync(x->parent);
        y->right=x;
        x->parent=y;
        sync(x);sync(y);
    }
    void originalRebalance_49c26c(Node* n) noexcept {
        while(n!=root_ && n->parent && !n->parent->black) {
            Node* parent=n->parent;
            Node* grand=parent->parent;
            if(parent==grand->left) {
                Node* uncle=grand->right;
                if(!isBlack(uncle)){
                    parent->black=true;
                    uncle->black=true;
                    grand->black=false;
                    n=grand;
                } else {
                    if(n==parent->right){n=parent;originalRotateLeft_49c3fc(n);}
                    n->parent->black=true;
                    n->parent->parent->black=false;
                    originalRotateRight_49c3b0(n->parent->parent);
                }
            } else {
                Node* uncle=grand->left;
                if(!isBlack(uncle)){
                    parent->black=true;
                    uncle->black=true;
                    grand->black=false;
                    n=grand;
                } else {
                    if(n==parent->left){n=parent;originalRotateRight_49c3b0(n);}
                    n->parent->black=true;
                    n->parent->parent->black=false;
                    originalRotateLeft_49c3fc(n->parent->parent);
                }
            }
        }
        root_->black=true;
    }
    static bool validate(const Node* n,const Node* parent,
      const std::string* lo,const std::string* hi,int blacks,
      int& expected,std::size_t& count) noexcept {
        if(!n){
            if(expected<0)expected=blacks;
            return expected==blacks;
        }
        if(n->parent!=parent||
           (lo && n->key<=*lo)||(hi && n->key>=*hi)||
           n->decoded.originalKey!=n->key||
           n->decoded.left!=(n->left?&n->left->decoded:nullptr)||
           n->decoded.right!=(n->right?&n->right->decoded:nullptr))return false;
        if(!n->black && ((!isBlack(n->left))||(!isBlack(n->right))))return false;
        ++count;
        if(n->black)++blacks;
        return validate(n->left,n,lo,&(n->key),blacks,expected,count) &&
            validate(n->right,n,&(n->key),hi,blacks,expected,count);
    }
    std::vector<std::unique_ptr<Node>> nodes_{};
    Node* root_=nullptr;
    Node* leftmost_=nullptr;
    Node* rightmost_=nullptr;
};
} // namespace ae3
