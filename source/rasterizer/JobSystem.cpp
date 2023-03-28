#include "JobSystem.h"
#include <Windows.h>
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "Container/ring_buffer.h"

namespace jobsystem {

static constexpr uint32_t JS_MAIN_THREAD_ID = 0;
static constexpr uint32_t JS_INVALID_WORKER_ID = JS_MAIN_THREAD_ID;

struct Worker {
    uint32_t id = JS_INVALID_WORKER_ID;
    std::wstring name;
    std::shared_ptr<std::thread> thread;
};

struct Job {
    Context* ctx;
    std::function<void(JobArgs)> task;
    uint32_t groupID;
    uint32_t groupJobOffset;
    uint32_t groupJobEnd;
};

template <size_t N>
struct ThreadSafeRingBuffer {
    ring_buffer<Job, N> mRingBuffer;
    std::mutex mLock;

    bool PopFront(Job& out_value) {
        std::lock_guard<std::mutex> lock(mLock);
        if (mRingBuffer.empty()) {
            return false;
        }

        out_value = mRingBuffer.front();
        mRingBuffer.pop_front();
        return true;
    }

    bool PushBack(const Job& value) {
        std::lock_guard<std::mutex> lock(mLock);
        if (mRingBuffer.size() == mRingBuffer.capacity()) {
            return false;
        }

        mRingBuffer.push_back(value);
        return true;
    }
};

static struct
{
    uint32_t numWorker;
    fixed_stack<Worker, 32> workers;

    std::condition_variable wakeCondition;
    std::mutex wakeMutex;
    std::atomic_bool quit;
    ThreadSafeRingBuffer<128> jobQueue;
    bool initialized;
} sJSGlob;

thread_local uint32_t sThreadID;

static bool is_main_thread() {
    return sThreadID == JS_MAIN_THREAD_ID;
}

static void JS_AssertMainThreadOperation() {
    assert(is_main_thread());
    assert(sJSGlob.initialized);
}

static bool JS_Work() {
    Job job;
    if (!sJSGlob.jobQueue.PopFront(job)) {
        return false;
    }

    for (uint32_t i = job.groupJobOffset; i < job.groupJobEnd; ++i) {
        JobArgs args;
        args.groupID = job.groupID;
        args.jobIndex = i;
        args.groupIndex = i - job.groupJobOffset;
        job.task(args);
    }

    job.ctx->AddTaskCount(-1);
    return true;
}

bool initialize() {
    uint32_t numWorkers = std::thread::hardware_concurrency() - 1;
    assert(numWorkers > 0);
    sJSGlob.numWorker = numWorkers;

    sThreadID = JS_MAIN_THREAD_ID;

    for (uint32_t threadID = 1; threadID <= numWorkers; ++threadID) {
        Worker worker;
        worker.id = threadID;
        worker.name = L"JS worker" + std::to_wstring(threadID);
        worker.thread = std::make_shared<std::thread>([&] {
            sThreadID = threadID;

            while (true) {
                if (sJSGlob.quit.load()) {
                    break;
                }

                if (!JS_Work()) {
                    std::unique_lock<std::mutex> lock(sJSGlob.wakeMutex);
                    sJSGlob.wakeCondition.wait(lock);
                }
            }
        });

        HANDLE handle = (HANDLE)worker.thread->native_handle();

        DWORD_PTR affinityMask = 1ull << threadID;
        DWORD_PTR affinityResult = SetThreadAffinityMask(handle, affinityMask);
        assert(affinityResult > 0);

        HRESULT hr = SetThreadDescription(handle, worker.name.c_str());
        assert(!FAILED(hr));
        sJSGlob.workers.push_back(worker);
    }

    sJSGlob.initialized = true;
    return true;
}

void finalize() {
    sJSGlob.quit.store(true);
    sJSGlob.wakeCondition.notify_all();
    for (auto& worker : sJSGlob.workers) {
        worker.thread->join();
    }
    sJSGlob.workers.clear();
}

void Context::Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>& task) {
    JS_AssertMainThreadOperation();

    if (jobCount == 0 || groupSize == 0) {
        return;
    }

    const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;  // make sure round up
    mTaskCount.fetch_add(groupCount);

    for (uint32_t groupID = 0; groupID < groupCount; ++groupID) {
        Job job;
        job.ctx = this;
        job.task = task;
        job.groupID = groupID;
        job.groupJobOffset = groupID * groupSize;
        job.groupJobEnd = min(job.groupJobOffset + groupSize, jobCount);

        while (!sJSGlob.jobQueue.PushBack(job)) {
            // if job queue is full, notify all and let main thread do the work as well
            sJSGlob.wakeCondition.notify_all();
            JS_Work();
        }
    }

    sJSGlob.wakeCondition.notify_all();
}

void Context::Wait() {
    // Wake any threads that might be sleeping:
    sJSGlob.wakeCondition.notify_all();

    // Waiting will also put the current thread to good use by working on an other job if it can:
    while (IsBusy()) {
        JS_Work();
    }
}

}  // namespace jobsystem
