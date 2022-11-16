//
// Created by Tarasus on 12.10.2020.
//

#include "include/lStringCollection.h"
#include "include/lvstring.h"

static const char * str_hash_magic="STRS";

static int (str16_comparator)(const void * n1, const void * n2)
{
    lstring16_chunk_t ** s1 = (lstring16_chunk_t **)n1;
    lstring16_chunk_t ** s2 = (lstring16_chunk_t **)n2;
    return lStr_cmp( (*s1)->data16(), (*s2)->data16() );
}

static int(*custom_lstr16_comparator_ptr)(lString16 & s1, lString16 & s2);

static int (str16_custom_comparator)(const void * n1, const void * n2)
{
    lString16 s1(*((lstring16_chunk_t **)n1));
    lString16 s2(*((lstring16_chunk_t **)n2));
    return custom_lstr16_comparator_ptr(s1, s2);
}

lString16Collection::~lString16Collection()
{
    clear();
}

void lString16Collection::reserve(int space)
{
    if (count + space > size)
    {
        size = count + space + 64;
        chunks = (lstring16_chunk_t**) realloc(chunks, sizeof(lstring16_chunk_t*) * size);
    }
}

void lString16Collection::sort(int(comparator)(lString16 & s1, lString16 & s2))
{
    custom_lstr16_comparator_ptr = comparator;
    qsort(chunks,count,sizeof(lstring16_chunk_t*), str16_custom_comparator);
}

void lString16Collection::sort()
{
    qsort(chunks,count,sizeof(lstring16_chunk_t*), str16_comparator);
}

int lString16Collection::add( const lString16 & str )
{
    reserve(1);
    chunks[count] = str.pchunk;
    str.addref();
    return count++;
}

void lString16Collection::clear()
{
    for (int i=0; i<count; i++)
    {
        ((lString16 *)chunks)[i].release();
    }
    if (chunks)
        free(chunks);
    chunks = NULL;
    count = 0;
    size = 0;
}

void lString16Collection::erase(int offset, int cnt)
{
    if (count<=0)
        return;
    if (offset < 0 || offset + cnt >= count)
        return;
    int i;
    for (i = offset; i < offset + cnt; i++)
    {
        ((lString16 *)chunks)[i].release();
    }
    for (i = offset + cnt; i < count; i++)
    {
        chunks[i-cnt] = chunks[i];
    }
    count -= cnt;
    if (!count)
        clear();
}

void lString16Collection::split( const lString16 & str, const lString16 & delimiter )
{
    if (str.empty())
        return;
    for (int startpos = 0; startpos < str.length(); ) {
        int pos = str.pos(delimiter, startpos);
        if (pos < 0)
            pos = str.length();
        add(str.substr(startpos, pos - startpos));
        startpos = pos + delimiter.length();
    }
}

void lString16Collection::parse( lString16 string, lChar16 delimiter, bool flgTrim )
{
    int wstart=0;
    for ( int i=0; i<=string.length(); i++ ) {
        if ( i==string.length() || string[i]==delimiter ) {
            lString16 s( string.substr( wstart, i-wstart) );
            if ( flgTrim )
                s.trimDoubleSpaces(false, false, false);
            if ( !flgTrim || !s.empty() )
                add( s );
            wstart = i+1;
        }
    }
}

void lString16Collection::parse( lString16 string, lString16 delimiter, bool flgTrim )
{
    if ( delimiter.empty() || string.pos(delimiter)<0 ) {
        lString16 s( string );
        if ( flgTrim )
            s.trimDoubleSpaces(false, false, false);
        add(s);
        return;
    }
    int wstart=0;
    for ( int i=0; i<=string.length(); i++ ) {
        bool matched = true;
        for ( int j=0; j<delimiter.length() && i+j<string.length(); j++ ) {
            if ( string[i+j]!=delimiter[j] ) {
                matched = false;
                break;
            }
        }
        if ( matched ) {
            lString16 s( string.substr( wstart, i-wstart) );
            if ( flgTrim )
                s.trimDoubleSpaces(false, false, false);
            if ( !flgTrim || !s.empty() )
                add( s );
            wstart = i+delimiter.length();
            i+= delimiter.length()-1;
        }
    }
}

int lString16Collection::add(const lChar16 *str) { return add(lString16(str)); }

int lString16Collection::add(const lChar8 *str) { return add(lString16(str)); }

bool lString16Collection::contains(lString16 value)
{
    for (int i = 0; i < count; i++)
    {
        if (value.compare(at(i)) == 0)
        { return true; }
    }
    return false;
}

void lString16Collection::addAll(const lString16Collection &v)
{
    for (int i=0; i<v.length(); i++)
    {add( v[i] );}
}

const lString16 &lString16Collection::at(int index)
{
    return ((lString16 *)chunks)[index];
}

const lString16 &lString16Collection::operator[](int index) const
{
    return ((lString16 *)chunks)[index];
}

lString16 &lString16Collection::operator[](int index)
{
    return ((lString16 *)chunks)[index];
}


lString8Collection::lString8Collection(const lString8 &str, const lString8 &delimiter) : chunks(NULL), count(0), size(0)
{
    split(str, delimiter);
}

lString8Collection::lString8Collection(lString8Collection &src) : chunks(NULL), count(0), size(0)
{
    addAll(src);
}

lString8Collection::~lString8Collection()
{
    clear();
}

void lString8Collection::split( const lString8 & str, const lString8 & delimiter )
{
    if (str.empty())
        return;
    for (int startpos = 0; startpos < str.length(); ) {
        int pos = str.pos(delimiter, startpos);
        if (pos < 0)
            pos = str.length();
        add(str.substr(startpos, pos - startpos));
        startpos = pos + delimiter.length();
    }
}

void lString8Collection::erase(int offset, int cnt)
{
    if (count <= 0)
        return;
    if (offset < 0 || offset + cnt > count)
        return;
    int i;
    for (i = offset; i < offset + cnt; i++)
    {
        ((lString8 *)chunks)[i].release();
    }
    for (i = offset + cnt; i < count; i++)
    {
        chunks[i-cnt] = chunks[i];
    }
    count -= cnt;
    if (!count)
        clear();
}

void lString8Collection::reserve(int space)
{
    if ( count + space > size )
    {
        size = count + space + 64;
        chunks = (lstring8_chunk_t * *)realloc( chunks, sizeof(lstring8_chunk_t *) * size );
    }
}

int lString8Collection::add( const lString8 & str )
{
    reserve( 1 );
    chunks[count] = str.pchunk;
    str.addref();
    return count++;
}

int lString8Collection::add(const char *str) { return add(lString8(str)); }

void lString8Collection::clear()
{
    for (int i=0; i<count; i++)
    {
        ((lString8 *)chunks)[i].release();
    }
    if (chunks)
        free(chunks);
    chunks = NULL;
    count = 0;
    size = 0;
}

const lString8 &lString8Collection::at(int index)
{
    return ((lString8 *)chunks)[index];
}

const lString8 &lString8Collection::operator[](int index) const
{
    return ((lString8 *)chunks)[index];
}

lString8 &lString8Collection::operator[](int index)
{
    return ((lString8 *)chunks)[index];
}

void lString8Collection::addAll(const lString8Collection &src)
{
    for (int i = 0; i < src.length(); i++)
        add(src[i]);
}


/// serialize to byte array (pointer will be incremented by number of bytes written)
void lString16HashedCollection::serialize( SerialBuf & buf )
{
    if ( buf.error() )
        return;
    int start = buf.pos();
    buf.putMagic( str_hash_magic );
    lUInt32 count = length();
    buf << count;
    for ( int i=0; i<length(); i++ )
    {
        buf << at(i);
    }
    buf.putCRC( buf.pos() - start );
}

/// deserialize from byte array (pointer will be incremented by number of bytes read)
bool lString16HashedCollection::deserialize( SerialBuf & buf )
{
    if ( buf.error() )
        return false;
    clear();
    int start = buf.pos();
    buf.putMagic( str_hash_magic );
    lInt32 count = 0;
    buf >> count;
    for ( int i=0; i<count; i++ ) {
        lString16 s;
        buf >> s;
        if ( buf.error() )
            break;
        add( s.c_str() );
    }
    buf.checkCRC( buf.pos() - start );
    return !buf.error();
}

lString16HashedCollection::lString16HashedCollection( lString16HashedCollection & v )
        : lString16Collection( v )
        , hashSize( v.hashSize )
        , hash( NULL )
{
    hash = (HashPair *)malloc( sizeof(HashPair) * hashSize );
    for ( int i=0; i<hashSize; i++ ) {
        hash[i].clear();
        hash[i].index = v.hash[i].index;
        HashPair * next = v.hash[i].next;
        while ( next ) {
            addHashItem( i, next->index );
            next = next->next;
        }
    }
}

lString16HashedCollection::lString16HashedCollection( lUInt32 hash_size ) : hashSize(hash_size), hash(NULL)
{
    hash = (HashPair *)malloc( sizeof(HashPair) * hashSize );
    for ( int i=0; i<hashSize; i++ )
        hash[i].clear();
}

lString16HashedCollection::~lString16HashedCollection()
{
    clearHash();
}

void lString16HashedCollection::addHashItem( int hashIndex, int storageIndex )
{
    if ( hash[ hashIndex ].index == -1 ) {
        hash[hashIndex].index = storageIndex;
    } else {
        HashPair * np = (HashPair *)malloc(sizeof(HashPair));
        np->index = storageIndex;
        np->next = hash[hashIndex].next;
        hash[hashIndex].next = np;
    }
}

void lString16HashedCollection::clearHash()
{
    if ( hash ) {
        for ( int i=0; i<hashSize; i++) {
            HashPair * p = hash[i].next;
            while ( p ) {
                HashPair * tmp = p->next;
                free( p );
                p = tmp;
            }
        }
        free( hash );
    }
    hash = NULL;
}

int lString16HashedCollection::find( const lChar16 * s )
{
    if ( !hash || !length() )
        return -1;
    lUInt32 h = calcStringHash( s );
    lUInt32 n = h % hashSize;
    if ( hash[n].index!=-1 )
    {
        const lString16 & str = at( hash[n].index );
        if ( str == s )
            return hash[n].index;
        HashPair * p = hash[n].next;
        for ( ;p ;p = p->next ) {
            const lString16 & str = at( p->index );
            if ( str==s )
                return p->index;
        }
    }
    return -1;
}

void lString16HashedCollection::reHash( int newSize )
{
    if (hashSize == newSize)
        return;
    clearHash();
    hashSize = newSize;
    if (hashSize > 0) {
        hash = (HashPair *)malloc( sizeof(HashPair) * hashSize );
        for ( int i=0; i<hashSize; i++ )
            hash[i].clear();
    }
    for ( int i=0; i<length(); i++ ) {
        lUInt32 h = calcStringHash( at(i).c_str() );
        lUInt32 n = h % hashSize;
        addHashItem( n, i );
    }
}

int lString16HashedCollection::add( const lChar16 * s )
{
    if ( !hash || hashSize < length()*2 ) {
        int sz = 16;
        while ( sz<length() )
            sz <<= 1;
        sz <<= 1;
        reHash( sz );
    }
    lUInt32 h = calcStringHash( s );
    lUInt32 n = h % hashSize;
    if ( hash[n].index!=-1 )
    {
        const lString16 & str = at( hash[n].index );
        if ( str == s )
            return hash[n].index;
        HashPair * p = hash[n].next;
        for ( ;p ;p = p->next ) {
            const lString16 & str = at( p->index );
            if ( str==s )
                return p->index;
        }
    }
    lUInt32 i = lString16Collection::add( lString16(s) );
    addHashItem( n, i );
    return i;
}
