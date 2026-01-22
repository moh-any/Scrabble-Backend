#pragma once
#include "trie.h"
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
using std::ifstream;
using std::string;

class Dict{
    private:
        trie tr;
    public:
        bool loadDictionary(string filename);
        bool isValidWord(string word) const;
        bool isPrefix(string partialWord) const;
};

bool Dict::isPrefix(string partialWord) const {
    return tr.isPrefix(partialWord);
}

bool Dict::loadDictionary(string filename){
        ifstream rf(filename);
        if(!rf){
            std::cout << "Failed to open file: " << filename << std::endl;
            return false;
        }
        string txt;
        int cnt=0;
        while(rf>>txt){
            for(auto&c:txt) c=tolower(c);
            tr.insert(txt);
            cnt++;
        }
        std::cout << "Loaded " << cnt << " words and created " << tr.numNodes << " nodes.\n";
        rf.close();
        return true;
}

bool Dict::isValidWord(string word) const{
    if(word.empty()) return false;
    return tr.search(word);
}
