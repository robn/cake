/*
 * lowlevel.h
 *
 *  Created on: Sep 30, 2008
 *      Author: misc
 */

#ifndef LOWLEVEL_H_
#define LOWLEVEL_H_

#include <inttypes.h>
#include "timer_intern.h"

#define TIMEBASE_FREQUENCY 33000000

void EClockUpdate(struct TimerBase *TimerBase);
void EClockSet(struct TimerBase *TimerBase);
void TimerSetup(struct TimerBase *TimerBase, uint32_t waste);

static volatile uint32_t mftbl()
{
	uint32_t tb;

	asm volatile("mftb %0":"=r"(tb));

	return tb;
}

static inline uint64_t mftb()
{
	uint32_t lo,hi,tmp;

	do {
		asm volatile("mftbu %0; mftb %1; mftbu %2":"=r"(hi),"=r"(lo),"=r"(tmp));
	} while(tmp != hi);

	return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}

#endif /* LOWLEVEL_H_ */
