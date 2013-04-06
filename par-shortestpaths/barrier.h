#include <xmmintrin.h>
#include <condition_variable>
#include <mutex>
#include <atomic>

// Simple spinning memory barrier.
// Turns out to be much faster than using anything that goes through the kernel.
class Barrier
{
public:
    Barrier(unsigned int count) : threshold(count), count(count), generation(0)
    {
    }

    bool wait()
    {
        unsigned int gen = generation;

        if (--count == 0)
        {
            count.store(threshold);
            generation++;
            return true;
        }
        while (gen == generation)
        {
        }
        return false;
    }

private:
    unsigned int threshold;
    atomic<unsigned int> count;
    atomic<unsigned int> generation;
};
