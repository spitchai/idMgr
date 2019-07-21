#include "./index.h"

unsigned int
getRandom()
{
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
    return((unsigned int) rand());
}

int
main()
{

  vector<struct id> ut_map;
  struct id in{
    .start = 1,
    .size = 1024 * 1024,
  };

  index idx(in);

  /* allocate 4K index */
  for(int i = 0; i < 4096; ++i) {
    int sz = getRandom() % 128;
    struct id ent = idx.indexAlloc(sz);
    //printf("add-ent %d %d %d\n", ent.start, ent.size, ent.tId);
    ut_map.push_back(ent);
  }

  /* delete 1K entries */
  for(int i = 0; i < 1024; ++i) {
    auto x = getRandom() % 2048;
    auto ent = ut_map[x];
    //printf("del-ent %d %d %d\n", ent.start, ent.size, ent.tId);
    idx.indexDeAlloc(ent);
  }

  /* allocate 4K index */
  for(int i = 0; i < 4096; ++i) {
    int sz = getRandom() % 128;
    struct id ent = idx.indexAlloc(sz);
    //printf("add-ent %d %d %d\n", ent.start, ent.size, ent.tId);
    ut_map.push_back(ent);
  }

  idx.indexShow();
}
