#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>
#include <queue>
#include <time.h>
#include <random>
#include <limits.h>
#include <map>

using namespace std;

struct id {
  unsigned int start;
  unsigned int size;
  unsigned int tId; /* transaction id */
};

class index {
  unsigned int start;
  unsigned int cap;
  unsigned int current;

  map<unsigned int, struct id>tMap;
  map<unsigned int, struct id*, greater<unsigned int>>idMap;
  multimap<unsigned int, struct id*, greater<unsigned int>>sizeMap;

  void incCurrent(int size) {
    this->current += size;
  }

  unsigned int getTransId() {
    srand((unsigned)time(NULL));
    return (rand() % UINT_MAX);
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
      assert(it->first >= size);

      struct id *ep = it->second;
      this->sizeMap.erase(it);
      this->idMap.erase(ep->start);

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
  printf("index reuseAlloc    : %d\n", (int)this->idMap.size());
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
    return false;
  } else {
    tMap.erase(index.tId);

    /* lookup the id map and find the closet smaller one */
    auto it = this->idMap.lower_bound(index.start);
    if(it != this->idMap.end()) {
      /* check - if re-fragment is a option */
      auto p = it->second;
      if(p->start + p->size == index.start) {
        /* reassemble the fragment @ id map */
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
      }
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
