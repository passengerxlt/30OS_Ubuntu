#include "bootpack.h"

#define EFLAGS_AC_BIT        0x00040000
#define CR0_CACHE_DISABLE    0x60000000

unsigned int memtest_sub(unsigned int start, unsigned int end);

unsigned int memtest(unsigned int start, unsigned int end)
{
  char flg486 = 0;
  unsigned int eflg, cr0, i;
  
  /* cpu 386? 486 or later?*/
  eflg = io_load_eflags();
  eflg |= EFLAGS_AC_BIT; /* AC-bit = 1*/
  io_store_eflags(eflg);
  eflg = io_load_eflags();
  if (0 != (eflg & EFLAGS_AC_BIT)) {
    flg486 = 1; /* if 386, set AC=1, AC still 0*/
  }
  eflg &= ~EFLAGS_AC_BIT;
  io_store_eflags(eflg); /* AC-bit = 0 */

  if (0 != flg486) {
    cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE; /* disable cache */
    store_cr0(cr0);
  }

  i = memtest_sub(start, end);

  if (0 != flg486) {
    cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;
    store_cr0(cr0);
  }

  return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
  unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
  for (i = start; i <= end; i += 0x1000) {
    p = (unsigned int *)(i + 0xffc);
    old = *p;
    *p = pat0;
    *p ^= 0xffffffff; /* */
    if (pat1 != *p) {
    not_memory:
      *p = old;
      break;
    }
    *p ^= 0xffffffff;
    if (pat0 != *p) {
      goto not_memory;
    }
    *p = old;
  }
  return i;
}
