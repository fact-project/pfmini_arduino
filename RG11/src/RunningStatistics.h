#ifndef RUNNINGSTATISTICS__H
#define RUNNINGSTATISTICS__H

#include <stdint.h>

class RunningStatistics
{
	uint16_t samples = 0;
	uint64_t sum = 0;
	uint64_t sq_sum = 0;

public:
	RunningStatistics();
	void append(uint16_t value);
	uint32_t get_mean(void);
	uint32_t get_var(void);
};

#endif // RUNNINGSTATISTICS__H