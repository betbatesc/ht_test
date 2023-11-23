#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>

struct KeyValuePair {
    int key;
    int value;
};

class HashTable {
private:
    int size;
    int count;
    std::vector<std::vector<KeyValuePair>> array;
    std::vector<int> histogram;
    std::vector<int> prefixSum;

    uint32_t rot(uint32_t x, int k);
    uint32_t hashFunction(int key);

public:
    HashTable(int initialSize);

    void insert(int key, int value);

    int search(int key);

    void remove(int key);

    std::vector<int>& getHistogram();

    std::vector<int>& getPrefixSum();

    void calculateHistogram();
    
    void calculatePrefixSum();

    int getSize() const;

    int getCount() const;
};

#endif // HASHTABLE_H
