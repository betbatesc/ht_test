#include <iostream>
#include "HashTable.hpp"

HashTable::HashTable(int initialSize) : size(initialSize), count(0), array(initialSize) {}

uint32_t HashTable::rot(uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

uint32_t HashTable::hashFunction(int key) {
    uint32_t a, b, c;
    a = b = c = 0xdeadbeef + static_cast<uint32_t>(key);

    c ^= b; c -= rot(b, 14); b += a;  
    a ^= c; a -= rot(c, 11); c += b;
    b ^= a; b -= rot(a, 25); a += c;
    c ^= a; c -= rot(a, 16); a += b;
    b ^= c; b -= rot(c, 4);  c += a;
    a ^= c; a -= rot(c, 14); c += b;
    b ^= a; b -= rot(a, 24); a += c;

    return c;
}

void HashTable::calculateHistogram() {
    histogram.clear();
    for (const auto& bucket : array) {
        histogram.push_back(bucket.size());
    }
}

void HashTable::calculatePrefixSum() {
    prefixSum.clear();
    prefixSum.resize(size);
    prefixSum[0] = array[0].size();

    for (size_t i = 1; i < size; ++i) {
        prefixSum[i] = prefixSum[i - 1] + array[i].size();
    }
}

void HashTable::insert(int key, int value) {
    int index = hashFunction(key) % size;
    array[index].push_back({key, value});
    count++;

    // Resize if load factor exceeds a threshold (e.g., 0.7)
    if (static_cast<double>(count) / size > 0.7) {
        int newSize = size * 2;
        array.resize(newSize);
        size = newSize;
    }
}

int HashTable::search(int key) {
    int index = hashFunction(key) % size;
    for (const auto& pair : array[index]) {
        if (pair.key == key) {
            return pair.value;
        }
    }
    return -1;  // Key not found
}

void HashTable::remove(int key) {
    int index = hashFunction(key) % size;
    auto& bucket = array[index];

    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
        if (it->key == key) {
            bucket.erase(it);
            count--;
            return;
        }
    }
}

std::vector<int>& HashTable::getHistogram() {
    return histogram;
}

std::vector<int>& HashTable::getPrefixSum() {
    return prefixSum;
}

int HashTable::getSize() const {
    return size;
}

int HashTable::getCount() const {
    return count;
}
