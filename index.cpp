#include "./index.h"

int
main()
{
  struct id in{
    .start = 1,
    .size = 1024 * 1024,
  };

  index i(in);

  in = i.indexAlloc(20);
  i.indexAlloc(91);
  i.indexAlloc(1);
  i.indexAlloc(2);
  i.indexShow();
}
