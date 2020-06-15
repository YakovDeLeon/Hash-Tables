#ifndef _PROCESSOR_TIME_
#define _PROCESSOR_TIME_

inline unsigned long long rdtsc()
{
    unsigned aux;
    return __builtin_ia32_rdtscp(&aux);
}

#endif

