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
        vector<string> findAllWordsFromLetters(string rack);
        bool isValidWord(string word) const;
        bool isPrefix(string partialWord) const;
        vector<string> getWordsWithPrefix(string prefix);
        vector<string> getAllPrefixes(string rack, int maxLength) const;
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

vector<string> Dict::findAllWordsFromLetters(string letters){
    vector<string> out;
    sort(letters.begin(),letters.end());
    for(int i=0; i<letters.size(); ++i){
        if(tr.search(letters.substr(0,i+1))) out.push_back(letters.substr(0,i+1));
    }
    while(next_permutation(letters.begin(),letters.end())){
        for(int i=0; i<letters.size(); ++i){
            if(tr.search(letters.substr(0,i+1))) out.push_back(letters.substr(0,i+1));
        }
    }
    return out;
}

bool Dict::isValidWord(string word) const{
    return tr.search(word);
}

vector<string> Dict::getWordsWithPrefix(string prefix){
    vector<string> out=tr.getSuffixWithPrefix(prefix);
    for(int i=0; i<out.size(); ++i) out[i]=prefix+out[i];
    return out;
}

vector<string> Dict::getAllPrefixes(string rack, int maxLength) const{
    return tr.getAllPrefixes(rack,maxLength);
}