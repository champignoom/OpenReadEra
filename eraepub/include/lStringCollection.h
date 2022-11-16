//
// Created by Tarasus on 12.10.2020.
//

#ifndef _LSTRINGCOLLECTION_H
#define _LSTRINGCOLLECTION_H

class lString8;
class lString16;
struct lstring8_chunk_t;
struct lstring16_chunk_t;

#include "lvtypes.h"
#include "serialBuf.h"

/// collection of strings
class lString8Collection
{
private:
    lstring8_chunk_t * * chunks;
    int count;
    int size;
public:
    lString8Collection(): chunks(NULL), count(0), size(0) { }
    lString8Collection(lString8Collection & src);
    lString8Collection(const lString8 & str, const lString8 & delimiter);
    void reserve(int space);
    int add(const lString8 & str);
    int add(const char * str);
    void addAll(const lString8Collection & src);
    /// split string by delimiters, and add all substrings to collection
    void split(const lString8 & str, const lString8 & delimiter);
    void erase(int offset, int count);
    const lString8 & at(int index);
    const lString8 & operator [] (int index) const;
    lString8 & operator [] (int index);
    int length() const { return count; }
    void clear();
    ~lString8Collection();
};


/// collection of wide strings
class lString16Collection
{
private:
    lstring16_chunk_t * * chunks;
    int count;
    int size;
public:
    lString16Collection() : chunks(NULL), count(0), size(0) { }
    /// parse delimiter-separated string
    void parse( lString16 string, lChar16 delimiter, bool flgTrim );
    /// parse delimiter-separated string
    void parse( lString16 string, lString16 delimiter, bool flgTrim );
    void reserve(int space);
    int add( const lString16 & str );
    int add(const lChar16 * str);
    int add(const lChar8 * str);
    void addAll( const lString16Collection & v );
    void erase(int offset, int count);
    /// split into several lines by delimiter
    void split(const lString16 & str, const lString16 & delimiter);
    const lString16 & at(int index);
    const lString16 & operator [] (int index) const;
    lString16 & operator [] (int index);
    int length() const { return count; }
    void clear();
    bool contains( lString16 value );
    void sort();
    void sort(int(comparator)(lString16 & s1, lString16 & s2));
    ~lString16Collection();
};

/// hashed wide string collection
class lString16HashedCollection : public lString16Collection
{
private:
    int hashSize;
    struct HashPair {
        int index;
        HashPair * next;
        void clear() { index=-1; next=NULL; }
    };
    HashPair * hash;
    void addHashItem( int hashIndex, int storageIndex );
    void clearHash();
    void reHash( int newSize );
public:

    /// serialize to byte array (pointer will be incremented by number of bytes written)
    void serialize( SerialBuf & buf );
    /// deserialize from byte array (pointer will be incremented by number of bytes read)
    bool deserialize( SerialBuf & buf );

    lString16HashedCollection( lString16HashedCollection & v );
    lString16HashedCollection( lUInt32 hashSize );
    ~lString16HashedCollection();
    int add( const lChar16 * s );
    int find( const lChar16 * s );
};

#endif //_LSTRINGCOLLECTION_H
