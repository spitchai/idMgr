#include "./index.h"

int
main()
{

  map<unsigned int, struct id>ut_map;
  struct id in{
    .start = 1,
    .size = 1024 * 1024,
  };

  index idx(in);

  for(int i = 0; i < 4096; ++i) {
    srand((unsigned)time(NULL));
    int sz = rand() % 512;
    auto ent = idx.indexAlloc(sz);
    ut_map.insert(make_pair(ent.tId, ent));
  }

#if 0
  for(int i = 0; i < 2048; ++i) {
    srand((unsigned)time(NULL));
    int x = rand() % 2048;
    auto ent = ut_map.begin() + x;
    idx.indexDeAlloc(ent->second);
  }
#endif

  idx.indexShow();
}
