#include <iostream>
#include <cstring>
#include <fstream>
using namespace std;
#define FIFO 1
#define LRU 2
#define fully 10

class CacheBlock{
public:
    int dirty;
    int vaild;
    int tag;
    int order; // 優先拿0的
    CacheBlock(){
        dirty = 0;
        vaild = 0;
        tag = 0;
        order = 0;
    }
};

class Cache{
public:
    long long blockNum;
    long long blockOrderId;
    int replace;
    CacheBlock *cacheBlock;
    
    void setCache(long long blockNum, int replace){
        cacheBlock = new CacheBlock[blockNum];
        this->blockNum = blockNum;
        this->replace = replace;
        blockOrderId = 0;
    }

    int find(long long tag, int *cacheHit){
        for(int i=0; i<blockNum; i++){
            if(cacheBlock[i].tag == tag && cacheBlock[i].vaild == 1){
                (*cacheHit)++;
                reorderLRU(i);
                return i;
            }
        }
        return -1;
    }

    int load(long long tag, int *toMem){
        for(int i=0; i<blockNum; i++){
            if(cacheBlock[i].order == 0){
                if(cacheBlock[i].dirty == 1){ // 弄髒的
                    (*toMem)++; // write back
                }
                // new data
                cacheBlock[i].vaild = 1;
                cacheBlock[i].tag = tag;
                
                // reorder
                if(blockOrderId+1 < blockNum){ // 還有空位，直接放在最後
                    blockOrderId++;
                    cacheBlock[i].order = blockOrderId;
                }
                else{ // 沒空位，重新排序
                    if(replace == FIFO){ 
                        reorderFIFO(i);
                    }
                    else if(replace == LRU){
                        reorderLRU(i);
                    }
                }
                return i;

            }
        }
        cout << "load error" << endl;
        exit(1);
        
    }

    void reorderLRU(int i){
        if(replace == LRU){
            for(int j=0; j<blockNum; j++){
                if(cacheBlock[j].order > cacheBlock[i].order){
                    cacheBlock[j].order--;
                }
            }
            cacheBlock[i].order = blockOrderId;
        }
    }

    void reorderFIFO(int i){
        if(replace == FIFO){
            for(int j=0; j<blockNum; j++){
                cacheBlock[j].order--;
            }
            cacheBlock[i].order = blockOrderId;
        }
    }

    void read(long long tag, int *cacheHit, int *toMem){
        int i;
        if((find(tag,cacheHit)) == -1){ // not hit
            i = load(tag,toMem);
            cacheBlock[i].dirty = 0; // not dirty
            reorderLRU(i); // reorder
        }
        
    }

    void write(long long tag, int *cacheHit, int *toMem){
        int i;
        if((i=find(tag,cacheHit)) != -1){ // hit
            cacheBlock[i].dirty = 1; // dirty
        }
        else{
            i = load(tag,toMem);
            cacheBlock[i].dirty = 1; // dirty
            reorderLRU(i); // reorder
        }
    }

    void writeBackAll(int *toMem){
        for(int i=0; i<blockNum; i++){
            if(cacheBlock[i].dirty == 1){
                (*toMem)++; // write back
            }
        }
    }
};

long long HEXstring2DECnum(char *addr){
    int len = strlen(addr);
    long long num = 0;
    long long base = 1;
    for(int i=len-1; i>=0; i--){
        if(isdigit(addr[i])){
            num += (addr[i]-'0')*base;
        }
        else{
            num += (addr[i]-'a'+10)*base;
        }
        // cout << "debug" << num << endl;
        base *= 16;
    }
    return num;
}

int main(int argc,char *argv[]){
    int cacheSize,blockSize,associativity,replace; // 1: FIFO, 2:LRU
    int label;
    char word[100];
    long long addr,blockAddr,index,tag;
    long long indexNum,blockNum,totalBlockNum;

    int demandFetch=0;
    int readData=0; // label 0
    int writeData=0; // label 1
    int cacheHit=0;
    int cacheMiss=0;
    int toMem=0;

    // cache 8 32 1 FIFO gcc.din
    // 參數設定
    cacheSize = stoi(argv[1])*1024;
    blockSize = stoi(argv[2]);
    
    if(strcmp(argv[3],"f") == 0){
        associativity = fully;
    }
    else{
        associativity = stoi(argv[3]);
    }

    if((strcmp(argv[4],"FIFO")) == 0){
        replace = FIFO;
    }
    else if((strcmp(argv[4],"LRU")) == 0){
        replace = LRU;
    }
    else{
        cout << "replace error\n";
    }

    // 計算相關資訊
    totalBlockNum = cacheSize/blockSize;
    if(associativity == fully){
        indexNum = 1;
        blockNum = totalBlockNum;
    }
    else{
        indexNum = totalBlockNum/associativity;
        blockNum = associativity;
    }

    // create cache
    Cache *cache = new Cache[indexNum];
    for(int i=0; i<indexNum; i++){
        cache[i].setCache(blockNum,replace);
    }

    // read file
    ifstream file(argv[5]);
    while((file >> label >> word)){

        addr = HEXstring2DECnum(word);
        blockAddr = addr/blockSize;
        index = blockAddr%(indexNum);
        tag = blockAddr/indexNum;

        demandFetch++;
        if(label == 0){
            readData++;
            cache[index].read(tag,&cacheHit,&toMem);
        }
        else if(label == 1){
            writeData++;
            cache[index].write(tag,&cacheHit,&toMem);
            
        }
        else{
            cache[index].read(tag,&cacheHit,&toMem);
        }
    }
    file.close();

    // 全部寫回
    for(int i=0; i<indexNum; i++){
        cache[i].writeBackAll(&toMem);
    }

    // output
    cacheMiss = demandFetch - cacheHit;
    cout << "Input file = " << argv[5] << endl;
    cout << "Demand fetch = " << demandFetch << endl;
    cout << "Cache hit = " << cacheHit << endl;
    cout << "Cache miss = " << cacheMiss << endl;
    cout << "Miss rate = " << cacheMiss/double(demandFetch) << endl;
    cout << "Read data = " << readData << endl;
    cout << "Write data = " << writeData << endl;
    cout << "Bytes from memory = " << cacheMiss*blockSize << endl;
    cout << "Bytes to memory = " << toMem*blockSize << endl;
    return 0;
}

