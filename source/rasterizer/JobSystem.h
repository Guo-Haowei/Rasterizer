#pragma once
#include <atomic>
#include <functional>

namespace jobsystem {

struct JobArgs {
    uint32_t jobIndex;
    uint32_t groupID;
    uint32_t groupIndex;
};

class Context {
   public:
    void AddTaskCount(int num) {
        mTaskCount.fetch_add(num);
    }

    bool IsBusy() const {
        return mTaskCount.load() > 0;
    }

    void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>& task);

    void Wait();

   private:
    std::atomic_int mTaskCount = 0;
};

bool initialize();
void finalize();

}  // namespace jobsystem
