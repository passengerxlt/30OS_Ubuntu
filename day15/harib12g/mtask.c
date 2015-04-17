#include "bootpack.h"

struct TIMER *mt_timer;
int mt_tr;

void mt_init(void)
{
	mt_timer = timer_alloc();
	/* it's not necesasry to timer_init */
	timer_settime(mt_timer, 2);
	mt_tr = 3 * 8;
	return;
}

void mt_taskswitch(void)
{
	if (3 * 8 == mt_tr) {
		mt_tr = 4 * 8;
	} else {
		mt_tr = 3 * 8;
	}

	timer_settime(mt_timer, 2);
	farjmp(0, mt_tr);
}
