#pragma once
#include <string>
#include <vector>
using std::string;
using std::vector;

struct node{
    node *children[26]={nullptr};
    bool isEndofWord=false;
};

struct nodeEq{
    bool operator()(const node* a, const node* b) const{
        if(a->isEndofWord!=b->isEndofWord) return false;
        for(int i=0; i<26; ++i){
            if(a->children[i]!=b->children[i]) return false;
        }
        return true;
    }
};

class trie{
    private:
    node* root;
    void insert(node* root,const string &s);
    bool search(node* root,const string &s) const;
    bool isPrefix(node* root, const string &s) const;
    void clear(node* root);
    public:
    trie();
    ~trie();
    void insert(const string &s);
    bool search(const string &s) const;
    bool isPrefix(const string &s) const;

    int numNodes=1;
};

void trie::insert(const string &s){
    insert(root,s);
}

bool trie::search(const string &s) const{
    return search(root,s);
}

bool trie::isPrefix(const string &s) const{
    return isPrefix(root,s);
}

trie::trie(){
    root=new node;
}

trie::~trie(){
    clear(root);
    root=nullptr;
}


void trie::clear(node* root){
    if(!root) return;
    for(int i=0; i<26; ++i){
        clear(root->children[i]);
        root->children[i]=nullptr;
    }
    delete root;
}

void trie::insert(node* root,const string &s){
    for(int i=0; i<s.length(); ++i){
        int ind=s[i]-'a';
        if(!root->children[ind]){
            root->children[ind]=new node;
            numNodes++;
        }
        root=root->children[ind];
    }
    root->isEndofWord=true;
}

bool trie::search(node* root,const string &s) const{
    if(!root) return false;
    for(int i=0; i<s.length(); ++i){
        int ind=s[i]-'a';
        if(!root->children[ind]) return false;
        root=root->children[ind];
    }
    return root->isEndofWord;
}

bool trie::isPrefix(node* root,const string &s) const{
    for(int i=0; i<s.length(); ++i){
        int ind=s[i]-'a';
        if(!root->children[ind]) return false;
        root=root->children[ind];
    }
    return true;
}
