#pragma once
#include "dictionary.h"
#include <bitset>
#include <tuple>

enum Multiplier {NONE,DL,TL,DW,TW};
const int letterPoints[26]={1,3,3,2,1,4,2,4,1,8,5,1,3,1,1,3,10,1,1,1,1,4,4,8,4,10};

struct Move{
    int r,c;
    string word;
    bool isHorizontal;
    int score;
    vector<int> blanks;
};

class Board{
    private:
        static const int sz=15;
        char tiles[sz][sz];
        Multiplier mul[sz][sz];
        bool was_blank[sz][sz];
        std::bitset<26> cross_checks[sz][sz];
        int cross_scores[sz][sz];
        bool is_anchor[sz][sz];
        Dict* dict;
        void calculateCrossChecks();
        void calculateAnchors();
        void calculateCrossScores();
        void LeftPart(int r,int c,string word,vector<int> &rack,int blanks,int limit,vector<Move> &res,int score,int mult,int corss_sum,int tile_placed,vector<int> blanks_used);
        void ExtendRight(int r,int c,string word,vector<int> &rack,int blanks,vector<Move> &res,int score,int mult,int corss_sum,int tile_placed,vector<int> blanks_used);    
        void Solve(string rack,vector<Move> &Moves);
        void Transpose();
    public:
        Board(Dict* dictionary);
        void Precompute();
        std::pair<bool,string> IsValidMove(Move move);
        int GetMoveScore(Move move);
        void PlaceMove(Move move);
        bool IsAnchor(int r,int c);
        char GetTile(int r,int c);
        bool IsvalidVertical(int r,int c,char letter);
        vector<Move> GetAllMoves(string rack);
        Move GetBestMove(string rack);
        void print();
};

Board::Board(Dict* dictionary):dict(dictionary){
    // intialize board arrays
    for(int i=0; i<sz; ++i){
        for(int j=0; j<sz; ++j){
            tiles[i][j]=' ';
            is_anchor[i][j]=false;
            cross_checks[i][j].reset();
            mul[i][j]=NONE;
            was_blank[i][j]=false;
        }
    }
    // put the multipliers
    // put triple words
    mul[0][0]=TW; mul[0][7]=TW; mul[0][14]=TW;
    mul[7][0]=TW; mul[7][14]=TW;
    mul[14][0]=TW; mul[14][7]=TW; mul[14][14]=TW;
    // put triple letters
    mul[1][5]=TL; mul[1][9]=TL;
    mul[5][1]=TL; mul[5][5]=TL; mul[5][9]=TL; mul[5][13]=TL;
    mul[9][1]=TL; mul[9][5]=TL; mul[9][9]=TL; mul[9][13]=TL;
    mul[13][5]=TL; mul[13][9]=TL;
    // put double words
    mul[1][1]=DW; mul[2][2]=DW; mul[3][3]=DW; mul[4][4]=DW;
    mul[7][7]=DW;
    mul[10][10]=DW; mul[11][11]=DW; mul[12][12]=DW; mul[13][13]=DW;
    mul[1][13]=DW; mul[2][12]=DW; mul[3][11]=DW; mul[4][10]=DW;
    mul[10][4]=DW; mul[11][3]=DW; mul[12][2]=DW; mul[13][1]=DW;
    // put double letters
    mul[0][3]=DL; mul[0][11]=DL;
    mul[2][6]=DL; mul[2][8]=DL;
    mul[3][0]=DL; mul[3][7]=DL; mul[3][14]=DL;
    mul[6][2]=DL; mul[6][6]=DL; mul[6][8]=DL; mul[6][12]=DL;
    mul[7][3]=DL; mul[7][11]=DL;
    mul[8][2]=DL; mul[8][6]=DL; mul[8][8]=DL; mul[8][12]=DL;
    mul[11][0]=DL; mul[11][7]=DL; mul[11][14]=DL;
    mul[12][6]=DL; mul[12][8]=DL;
    mul[14][3]=DL; mul[14][11]=DL;
    
    // calculate anchors and cross checks
    calculateAnchors();
    calculateCrossChecks();
    calculateCrossScores();
}

char Board::GetTile(int r,int c){
    return tiles[r][c];
}

bool Board::IsAnchor(int r,int c){
    return is_anchor[r][c];
}

void Board::calculateCrossChecks(){
    // calculate which letters can be placed in each empty square to form valid vertical words
    for(int r=0; r<sz; ++r){
        for(int c=0; c<sz; ++c){
            if(tiles[r][c]!=' ') continue;

            cross_checks[r][c].set();
            bool up=(r>0 && tiles[r-1][c]!=' ');
            bool down=(r<sz-1 && tiles[r+1][c]!=' ');
            if(up || down){
                cross_checks[r][c].reset();
                string prefix="",suffix="";
                int i=r-1;
                while(i>=0 && tiles[i][c]!=' '){
                    prefix=tiles[i][c]+prefix;
                    i--;
                }

                i=r+1;
                while(i<sz && tiles[i][c]!=' '){
                    suffix+=tiles[i][c];
                    i++;
                }
                
                for(char ch='a'; ch<='z'; ++ch){
                    string candidate=prefix+ch+suffix;
                    if(dict->isValidWord(candidate)){
                        cross_checks[r][c].set(ch-'a');
                    }
                }
            }
        }
    }
}

bool Board::IsvalidVertical(int r,int c,char letter){
    if(tiles[r][c]!=' ') return false;
    return cross_checks[r][c].test(letter-'a');
}

void Board::calculateAnchors(){
    // calculate anchor squares
    for(int r=0; r<sz; ++r){
        for(int c=0; c<sz; ++c){
            is_anchor[r][c]=false;
            if(tiles[r][c]!=' ') continue;
            if((r>0 && tiles[r-1][c]!=' ') 
            || (r<sz-1 && tiles[r+1][c]!=' ')
            || (c>0 && tiles[r][c-1]!=' ')
            || (c<sz-1 && tiles[r][c+1]!=' ')){
                is_anchor[r][c]=true;
            }
        }
    }

    // if no move is played yet then the center is an anchor
    if(tiles[7][7]==' ') is_anchor[7][7]=true;
}

void Board::calculateCrossScores(){
    // calculate cross scores for each empty square
    for(int r=0; r<sz; ++r) for(int c=0; c<sz; ++c) cross_scores[r][c]=0;
    for(int r=0; r<sz; ++r){
        for(int c=0; c<sz; ++c){
            if(tiles[r][c]!=' ') continue;
            bool up=(r>0 && tiles[r-1][c]!=' ');
            bool down=(r<sz-1 && tiles[r+1][c]!=' ');

            if(up || down){
                int current_sum=0;
                int i=r-1;
                while(i>=0 && tiles[i][c]!=' '){
                    if(!was_blank[i][c]) current_sum+=letterPoints[tiles[i][c]-'a'];
                    i--;
                }

                i=r+1;
                while(i<sz && tiles[i][c]!=' '){
                    if(!was_blank[i][c]) current_sum+=letterPoints[tiles[i][c]-'a'];
                    i++;
                }
                cross_scores[r][c]=current_sum;
            }
        }
    }
}

void Board::print(){
    for(int i=0; i<sz; ++i){
        for(int j=0; j<sz; ++j){
            std::cout << (char)(!was_blank[i][j]?tiles[i][j]-'a'+'A':tiles[i][j]) << ' ';
        }
        std::cout << std::endl;
    }
}

std::pair<bool,string> Board::IsValidMove(Move mv){
    std::vector<std::tuple<int,int,char>> move;
    for(int i=0; i<mv.word.length(); ++i){
        int r=mv.r;
        int c=mv.c;
        if(mv.isHorizontal){
            c+=i;
        }
        else{
            r+=i;
        }
        if(tiles[r][c]!=' ' && tiles[r][c]!=mv.word[i]) move.push_back({r,c,mv.word[i]});
    }
    // validates player moves
    if(move.size()==0) return {false,"Empty move"};
    // sort moves by row and column
    std::sort(move.begin(),move.end(),[](auto a, auto b){
        if(std::get<0>(a)==std::get<0>(b))
        return std::get<1>(a)<std::get<1>(b);
        return std::get<0>(a)<std::get<0>(b);
    });
    // all positions must be empty and have a common row or column
    bool sameRow=true,sameCol=true;
    int Row=std::get<0>(move[0]);
    int Col=std::get<1>(move[0]);
    for(auto&[r,c,l]:move){
        if(r<0 || r>=sz || c<0 || c>=sz) return {false,"Out of bounds move"};
        if(tiles[r][c]!=' ') return {false,"Position not empty"};
        if(r!=Row) sameRow=false;
        if(c!=Col) sameCol=false;
    }
    if(!sameRow && !sameCol) return {false,"Moves must be in the same row or column"};
    
    // check if the moves hook to at least one existing tile and all hooks are valid words
    // if first move
    if(tiles[7][7]==' '){
        bool coversCenter=false;
        for(auto&[r,c,l]:move){
            if(r==7 && c==7) coversCenter=true;
        }
        if(!coversCenter) return {false,"First move must cover the center square"};
    }
    else{
        bool hooks=false;
        for(auto&[r,c,l]:move){
            if((r>0 && tiles[r-1][c]!=' ')
            || (r<sz-1 && tiles[r+1][c]!=' ')
            || (c>0 && tiles[r][c-1]!=' ')
            || (c<sz-1 && tiles[r][c+1]!=' ')){
                hooks=true;
            }
            if(sameRow){
                string prefix="",suffix="";
                int i=r-1;
                while(i>=0 && tiles[i][c]!=' '){
                    prefix=tiles[i][c]+prefix;
                    i--;
                }
                
                i=r+1;
                while(i<sz && tiles[i][c]!=' '){
                    suffix+=tiles[i][c];
                    i++;
                }
                
                if(!prefix.empty() || !suffix.empty()){
                    string candidate=prefix+l+suffix;
                    if(!dict->isValidWord(candidate)){
                        return {false,"Invalid cross word: "+candidate+" at ("+std::to_string(r)+","+std::to_string(c)+") letter "+l};
                    }
                }
            }
            else{
                string prefix="",suffix="";
                int i=c-1;
                while(i>=0 && tiles[r][i]!=' '){
                    prefix=tiles[r][i]+prefix;
                    i--;
                }
                
                i=c+1;
                while(i<sz && tiles[r][i]!=' '){
                    suffix+=tiles[r][i];
                    i++;
                }
                
                if(prefix!="" || suffix!=""){
                    string candidate=prefix+l+suffix;
                    if(!dict->isValidWord(candidate)){
                        return {false,"Invalid cross word: "+candidate+" at ("+std::to_string(r)+","+std::to_string(c)+") letter "+l};
                    }
                }
            }
        }
        if(!hooks) return {false,"Move must hook to existing tiles"};
    }
    
    string mainWord="";
    if(sameRow){
        int r=std::get<0>(move[0]);
        int startC=std::get<1>(move[0]);
        int endC=std::get<1>(move.back());

        int c=startC-1;
        while(c>=0 && tiles[r][c]!=' '){
            mainWord=tiles[r][c]+mainWord;
            c--;
        }

        for(int col=startC; col<=endC; ++col){
            bool found=false;
            for(auto&[mr,mc,ml]:move){
                if(mr==r && mc==col){
                    mainWord+=ml;
                    found=true;
                    break;
                }
            }
            if(!found){
                if(tiles[r][col]!=' '){
                    mainWord+=tiles[r][col];
                }
                else{
                    return {false,"Gap in the main word at ("+std::to_string(r)+","+std::to_string(col)+")"};
                }
            }
        }

        c=endC+1;
        while(c<sz && tiles[r][c]!=' '){
            mainWord+=tiles[r][c];
            c++;
        }
    }
    else{
        int c=std::get<1>(move[0]);
        int startR=std::get<0>(move[0]);
        int endR=std::get<0>(move.back());
        int r=startR-1;

        while(r>=0 && tiles[r][c]!=' '){
            mainWord=tiles[r][c]+mainWord;
            r--;
        }

        for(int row=startR; row<=endR; ++row){
            bool found=false;
            for(auto&[mr,mc,ml]:move){
                if(mr==row && mc==c){
                    mainWord+=ml;
                    found=true;
                    break;
                }
            }
            if(!found){
                if(tiles[row][c]!=' '){
                    mainWord+=tiles[row][c];
                }
                else{
                    return {false,"Gap at ("+std::to_string(row)+","+std::to_string(c)+")"};
                }
            }
        }

        r=endR+1;
        while(r<sz && tiles[r][c]!=' '){
            mainWord+=tiles[r][c];
            r++;
        }
    }
    if(mainWord.length()>1 && !dict->isValidWord(mainWord)){
        return {false,"Invalid main word: "+mainWord};
    }
    return {true,mainWord};
}

void Board::PlaceMove(Move move){
    for(int i=0; i<move.word.length(); ++i){
        int r=move.r;
        int c=move.c;
        if(move.isHorizontal){
            c+=i;
        }
        else{
            r+=i;
        }
        if(tiles[r][c]==' '){
            tiles[r][c]=move.word[i];
            was_blank[r][c]=(std::find(move.blanks.begin(),move.blanks.end(),i)!=move.blanks.end());
        }
    }
    calculateAnchors();
    calculateCrossChecks();
    calculateCrossScores();
}

void Board::Precompute(){
    calculateAnchors();
    calculateCrossChecks();
    calculateCrossScores();
}

int Board::GetMoveScore(Move mv){
    std::vector<std::tuple<int,int,char>> move;
    for(int i=0; i<mv.word.length(); ++i){
        int r=mv.r;
        int c=mv.c;
        if(mv.isHorizontal){
            c+=i;
        }
        else{
            r+=i;
        }
        if(tiles[r][c]==' '){
            move.push_back({r,c,mv.word[i]});
        }
    }
    vector<std::pair<int,int>> blanks;
    for(int i=0; i<mv.blanks.size(); ++i){
        int idx=mv.blanks[i];
        int r=mv.r;
        int c=mv.c;
        if(mv.isHorizontal){
            c+=idx;
        }
        else{
            r+=idx;
        }
        blanks.push_back({r,c});
    }
    sort(move.begin(),move.end(),[](auto a, auto b){
        if(std::get<0>(a)==std::get<0>(b))
        return std::get<1>(a)<std::get<1>(b);
        return std::get<0>(a)<std::get<0>(b);
    });
    int totalScore=0;
    bool sameRow=true;
    if(move.size()>1) sameRow=(std::get<0>(move[0])==std::get<0>(move[1]));
    if(sameRow){
        int r=std::get<0>(move[0]);
        int startC=std::get<1>(move[0]);
        int endC=std::get<1>(move.back());

        int c=startC-1;
        while(c>=0 && tiles[r][c]!=' '){
            c--;
        }
        startC=c+1;
        c=endC+1;
        while(c<sz && tiles[r][c]!=' '){
            c++;
        }
        endC=c-1;

        if(endC>startC){
            int wordMultiplier=1;
            int wordScore=0;
            for(int col=startC; col<=endC; ++col){
                bool isNewTile=false;
                char letter=' ';
                for(auto&[mr,mc,ml]:move){
                    if(mr==r && mc==col){
                        isNewTile=true;
                        letter=ml;
                        break;
                    }
                }
                if(isNewTile){
                    int letterScore=0;
                    if(std::find(blanks.begin(),blanks.end(),std::make_pair(r,col))!=blanks.end()) letterScore=0;
                    else letterScore=letterPoints[letter-'a'];
                    Multiplier m=mul[r][col];
                    if(m==DL) letterScore*=2;
                    else if(m==TL) letterScore*=3;
                    else if(m==DW) wordMultiplier*=2;
                    else if(m==TW) wordMultiplier*=3;
                    wordScore+=letterScore;
                }
                else{
                    if(!was_blank[r][col]) wordScore+=letterPoints[tiles[r][col]-'a'];
                }
            }
            totalScore+=wordScore*wordMultiplier;
        }
    }
    else{
        int c=std::get<1>(move[0]);
        int startR=std::get<0>(move[0]);
        int endR=std::get<0>(move.back());

        int r=startR-1;
        while(r>=0 && tiles[r][c]!=' '){
            r--;
        }
        startR=r+1;
        r=endR+1;
        while(r<sz && tiles[r][c]!=' '){
            r++;
        }
        endR=r-1;

        if(endR>startR){
            int wordMultiplier=1;
            int wordScore=0;
            for(int row=startR; row<=endR; ++row){
                bool isNewTile=false;
                char letter=' ';
                for(auto&[mr,mc,ml]:move){
                    if(mr==row && mc==c){
                        isNewTile=true;
                        letter=ml;
                        break;
                    }
                }
                if(isNewTile){
                    int letterScore=0;
                    if(std::find(blanks.begin(),blanks.end(),std::make_pair(row,c))!=blanks.end()) letterScore=0;
                    else letterScore=letterPoints[letter-'a'];
                    Multiplier m=mul[row][c];
                    if(m==DL) letterScore*=2;
                    else if(m==TL) letterScore*=3;
                    else if(m==DW) wordMultiplier*=2;
                    else if(m==TW) wordMultiplier*=3;
                    wordScore+=letterScore;
                }
                else{
                    if(!was_blank[row][c]) wordScore+=letterPoints[tiles[row][c]-'a'];
                }
            }
            totalScore+=wordScore*wordMultiplier;
        }
    }

    for(auto&[r,c,l]:move){
        if(sameRow){
            bool up=(r>0 && tiles[r-1][c]!=' ');
            bool down=(r<sz-1 && tiles[r+1][c]!=' ');
            if(up || down){
                int row=r-1;
                while(row>=0 && tiles[row][c]!=' '){
                    row--;
                }
                int startR=row+1;
                row=r+1;
                while(row<sz && tiles[row][c]!=' '){
                    row++;
                }
                int endR=row-1;
                int wordMultiplier=1;
                int wordScore=0;
                for(int row=startR; row<=endR; ++row){
                    if(row==r){
                        int letterScore=0;
                        if(std::find(blanks.begin(),blanks.end(),std::make_pair(r,c))!=blanks.end()) letterScore=0;
                        else letterScore=letterPoints[l-'a'];
                        Multiplier m=mul[r][c];
                        if(m==DL) letterScore*=2;
                        else if(m==TL) letterScore*=3;
                        else if(m==DW) wordMultiplier*=2;
                        else if(m==TW) wordMultiplier*=3;
                        wordScore+=letterScore;
                    }
                    else{
                        if(!was_blank[row][c]) wordScore+=letterPoints[tiles[row][c]-'a'];
                    }
                }
                totalScore+=wordScore*wordMultiplier;
            }
        }
        else{
            bool left=(c>0 && tiles[r][c-1]!=' ');
            bool right=(c<sz-1 && tiles[r][c+1]!=' ');
            if(left || right){
                int col=c-1;
                while(col>=0 && tiles[r][col]!=' '){
                    col--;
                }
                int startC=col+1;
                col=c+1;
                while(col<sz && tiles[r][col]!=' '){
                    col++;
                }
                int endC=col-1;
                int wordMultiplier=1;
                int wordScore=0;
                for(int col=startC; col<=endC; ++col){
                    if(col==c){
                        int letterScore=0;
                        if(std::find(blanks.begin(),blanks.end(),std::make_pair(r,c))!=blanks.end()) letterScore=0;
                        else letterScore=letterPoints[l-'a'];
                        Multiplier m=mul[r][c];
                        if(m==DL) letterScore*=2;
                        else if(m==TL) letterScore*=3;
                        else if(m==DW) wordMultiplier*=2;
                        else if(m==TW) wordMultiplier*=3;
                        wordScore+=letterScore;
                    }
                    else{
                        if(!was_blank[r][col]) wordScore+=letterPoints[tiles[r][col]-'a'];
                    }
                }
                totalScore+=wordScore*wordMultiplier;
            }
        }
    }
    if(move.size()==7) totalScore+=50;
    return totalScore;
}

void Board::Solve(string rack,vector<Move> &Moves){
    vector<int> rackTiles(26,0);
    int blanks=0;
    for(char ch:rack){
        if(ch=='?') blanks++;
        else rackTiles[ch-'a']++;
    }
    for(int r=0; r<sz; ++r){
        for(int c=0; c<sz; ++c){
            if(!is_anchor[r][c]) continue;
            int limit=0;
            while(c-limit>0 && tiles[r][c-limit-1]==' ' && limit<rack.size()) limit++;
            
            for(int lim=0; lim<=limit; ++lim){
                LeftPart(r,c,"",rackTiles,blanks,lim,Moves,0,1,0,0,{});
            }
        }
    }
}

void Board::LeftPart(int r,int c,string word,vector<int> &rackTiles,int blanks,int limit,vector<Move> &res,int score,int mult,int cross_sum,int tile_placed,vector<int> blanks_used){
    if(limit==0){
        ExtendRight(r,c,word,rackTiles,blanks,res,score,mult,cross_sum,tile_placed,blanks_used);
        return;
    }

    for(int i=0; i<26; ++i){
        if(rackTiles[i]==0 && blanks==0) continue;

        if(!dict->isPrefix(word+(char)('a'+i))) continue;
        if(!cross_checks[r][c-limit].test(i)) continue;
        bool usedBlank=false;
        if(!rackTiles[i] && blanks) usedBlank=true;

        int letterScore=usedBlank?0:letterPoints[i];
        int crossScore=0;
        Multiplier m=mul[r][c-limit];
        if(m==DL) letterScore*=2;
        else if(m==TL) letterScore*=3;

        if(cross_scores[r][c-limit]){
            crossScore=(letterScore+cross_scores[r][c-limit]);
            if(m==DW) crossScore*=2;
            else if(m==TW) crossScore*=3;
        }

        int nextMul=mult;
        if(m==DW) nextMul*=2;
        else if(m==TW) nextMul*=3;

        if(!usedBlank){
            rackTiles[i]--;
            LeftPart(r,c,word+(char)('a'+i),rackTiles,blanks,limit-1,res,score+letterScore,nextMul,cross_sum+crossScore,tile_placed+1,blanks_used);
            rackTiles[i]++;
        }
        else{
            vector<int> nbu=blanks_used;
            nbu.push_back(word.length());
            LeftPart(r,c,word+(char)('a'+i),rackTiles,blanks-1,limit-1,res,score+letterScore,nextMul,cross_sum+crossScore,tile_placed+1,nbu);
        }
    }
}

void Board::ExtendRight(int r,int c,string word,vector<int> &rackTiles,int blanks,vector<Move> &res,int score,int mult,int cross_sum,int tile_placed,vector<int> blanks_used){
    if(!dict->isPrefix(word)) return;

    if(c>=sz){
        if(dict->isValidWord(word) && tile_placed>0){
            int totalScore=(score*mult)+cross_sum;
            if(tile_placed==7) totalScore+=50;
            res.push_back(Move{r,c-(int)word.length(),word,true,totalScore,blanks_used});
        }
        return;
    }

    if(tiles[r][c]!=' '){
        if(dict->isPrefix(word+tiles[r][c])){
            ExtendRight(r,c+1,word+tiles[r][c],rackTiles,blanks,res,score+(was_blank[r][c]?0:letterPoints[tiles[r][c]-'a']),mult,cross_sum,tile_placed,blanks_used);
        }
        return;
    }

    if(dict->isValidWord(word) && tile_placed>0){
        int totalScore=(score*mult)+cross_sum;
        if(tile_placed==7) totalScore+=50;
        res.push_back(Move{r,c-(int)word.length(),word,true,totalScore,blanks_used});
    }

    for(int i=0; i<26; ++i){
        if(rackTiles[i]==0 && blanks==0) continue;
        if(!dict->isPrefix(word+(char)('a'+i))) continue;
        if(!cross_checks[r][c].test(i)) continue;
        bool usedBlank=false;
        if(!rackTiles[i] && blanks) usedBlank=true;

        int letterScore=usedBlank?0:letterPoints[i];
        int crossScore=0;
        Multiplier m=mul[r][c];
        if(m==DL) letterScore*=2;
        else if(m==TL) letterScore*=3;

        if(cross_scores[r][c]){
            crossScore=(letterScore+cross_scores[r][c]);
            if(m==DW) crossScore*=2;
            else if(m==TW) crossScore*=3;
        }

        int nextMul=mult;
        if(m==DW) nextMul*=2;
        else if(m==TW) nextMul*=3;

        if(!usedBlank){
            rackTiles[i]--;
            ExtendRight(r,c+1,word+(char)('a'+i),rackTiles,blanks,res,score+letterScore,nextMul,cross_sum+crossScore,tile_placed+1,blanks_used);
            rackTiles[i]++;
        }
        else{
            vector<int> nbu=blanks_used;
            nbu.push_back(word.length());
            ExtendRight(r,c+1,word+(char)('a'+i),rackTiles,blanks-1,res,score+letterScore,nextMul,cross_sum+crossScore,tile_placed+1,nbu);
        }
    }
}

void Board::Transpose(){
    for(int i=0; i<sz; ++i){
        for(int j=i+1; j<sz; ++j){
            std::swap(tiles[i][j],tiles[j][i]);
            std::swap(mul[i][j],mul[j][i]);
            std::swap(was_blank[i][j],was_blank[j][i]);
        }
    }
}

vector<Move> Board::GetAllMoves(string rack){
    vector<Move> Moves;
    Precompute();
    Solve(rack,Moves);
    for(int i=0; i<Moves.size(); ++i){
        Moves[i].isHorizontal=true;
    }
    int h=Moves.size();
    Transpose();
    Precompute();
    Solve(rack,Moves);
    for(int i=h; i<Moves.size(); ++i){
        std::swap(Moves[i].r,Moves[i].c);
        Moves[i].isHorizontal=false;
    }
    Transpose();
    return Moves;
}

Move Board::GetBestMove(string rack){
    vector<Move> Moves=GetAllMoves(rack);
    if(Moves.size()==0) return Move{-1,-1,"",true,0};
    sort(Moves.begin(),Moves.end(),[](Move a, Move b){
        return a.score>b.score;
    });
    return Moves[0];
}