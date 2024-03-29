#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstdlib>
#include <limits.h>
#include <unistd.h>
#include <time.h>


#include <vector>
#include <queue>
#include <random>
#include <map>
#include <algorithm>
#include <chrono>

using namespace std;

struct id {
  unsigned int start;
  unsigned int size;
  unsigned int tId; /* transaction id */
};

/* functor comparator */
struct id_comp {
  bool operator()(const unsigned int a, unsigned int b)
  {
    return (a < b);
  }
};

/* functor comparator */
struct size_comp {
  bool operator()(const unsigned int a, unsigned int b)
  {
    return (a < b);
  }
};

class index {
  unsigned int start;
  unsigned int cap;
  unsigned int current;

  map<unsigned int, struct id>tMap;
  map<unsigned int, struct id*, id_comp>idMap;
  multimap<unsigned int, struct id*, size_comp>sizeMap;
  //multimap<unsigned int, struct id*, greater<unsigned int>>sizeMap;

  void incCurrent(int size) {
    this->current += size;
  }

  unsigned int getTransId()
  {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
    auto ret = rand() % UINT_MAX;

    auto it = this->tMap.find(ret);
    while(it != this->tMap.end()) {
      ret = rand() % UINT_MAX;
      it = this->tMap.find(ret);
    }
    //printf("tID : %d \n", ret);
    return (ret);
  }

  /*
   * check if there is capacity to increment current;
   *  if(true) :
   *    get a transsaction id
   *    increment current
   *    update transaction entry
   *    retutn current
   *  else :
   *    return invalid id
   */
  struct id
  currentAlloc(int size)
  {
    struct id ret;

    if(this->current + size < this->cap + this->start) {
      ret.start = this->current;
      ret.size = size;
      ret.tId = getTransId();
      this->tMap[ret.tId] = ret;
      incCurrent(size);
    } else {
      /* went past the capacity. Return invalid id */
      ret.start = -1;
      ret.size = 0;
      ret.tId = -1;
    }
    return ret;
  }

  struct id
  reuseAlloc(unsigned int size)
  {
    struct id ret;

    auto it = this->sizeMap.upper_bound(size);
    if(it != this->sizeMap.end()) {
      struct id *ep = it->second;
      this->sizeMap.erase(it);
      this->idMap.erase(ep->start);

      //printf("reusing: size - %d reuse start - %d, reuse size - %d\n", size, ep->start, ep->size);
      ret.size = size;
      ret.start = ep->start;
      ret.tId = getTransId();

      if(ep->size == size) {
        free(ep);
      } else {
        ep->start += size;
        ep->size -= size;
        ep->tId = 0;
        this->idMap.insert(make_pair(ep->start, ep));
        this->sizeMap.insert(make_pair(ep->size, ep));
      }
    } else {
      /* cannot resue the de-allocated id */
      ret = currentAlloc(size);
    }
    return ret;
  }

  public:
    index(struct id);
    struct id indexAlloc(int size);
    bool indexDeAlloc(struct id);
    bool indexFragment();
    void indexShow();
};

index::index(struct id i)
{
  this->start = i.start;
  this->cap = i.size;
  this->current = i.start;
}

void
index::indexShow()
{
  printf("index start         : %d\n", this->start);
  printf("index capacity      : %d\n", this->cap);
  printf("index current       : %d\n", this->current);
  printf("index idMap         : %d\n", (int)this->idMap.size());
  printf("index szMap         : %d\n", (int)this->sizeMap.size());
  printf("index total alloc   : %d\n", (int)this->tMap.size());

}

struct id
index::indexAlloc(int size)
{
  struct id ret;

  if(this->idMap.empty()) {
    ret = currentAlloc(size);
  } else {
    ret = reuseAlloc(size);
  }
  return ret;
}

bool
index::indexDeAlloc(struct id index)
{
  auto it = this->tMap.find(index.tId);
  if(it == this->tMap.end()) {
    printf("Entry not present\n");
    return false;
  } else {
    tMap.erase(index.tId);

    /* lookup the id map and find the closet smaller one */
    auto it = this->idMap.lower_bound(index.start);
    /* check - if re-fragment is a option */
    if((it != this->idMap.end()) &&
        (it->second->start + it->second->size == index.start)) {
        /* reassemble the fragment @ id map */
        auto p = it->second;
        //printf("reassembling: index(st %d, sz %d), p(st %d, sz %d)\n",
        //        index.start, index.size, p->start, p->size);
        p->size += index.size;

        /* reevaluate the size map */
        auto sm = this->sizeMap.equal_range(index.size);
        for(auto xy = sm.first; xy != sm.second; ++xy) {
          if(xy->second->start == index.start) {
            this->sizeMap.erase(xy);
            break;
          }
        }
        this->sizeMap.insert(make_pair(p->size, p));
    } else {
      /* re-fragment is not an option */
      struct id *ep = new struct id;
      ep->start = index.start;
      ep->size = index.size;
      ep->tId = 0;
      /* add the freed ent into id and size map */
      this->idMap.insert(make_pair(ep->start, ep));
      this->sizeMap.insert(make_pair(ep->size, ep));
    }
    return true;
  }
}
