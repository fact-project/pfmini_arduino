#include "RunningStatistics.h"

RunningStatistics::RunningStatistics()
{
    samples = 0;
    sum = 0;
    sq_sum = 0;
}

void RunningStatistics::append(uint16_t value)
{
    sum += value;
    sq_sum += (uint64_t)value * (uint64_t)value;
    samples++;
}

uint32_t RunningStatistics::get_mean(void)
{
    if (samples == 0){
        return 0;
    }
    return (uint32_t) (sum / samples);
}

uint32_t RunningStatistics::get_var(void)
{
    uint64_t N = samples;
    return (uint32_t) ((N * sq_sum - sum * sum) / (N * N));
}
