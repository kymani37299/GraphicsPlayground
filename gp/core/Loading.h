#pragma once

#include <atomic>

#include "core/Threads.h"
#include "gfx/GfxDevice.h"

namespace GP
{
    class LoadingTask
    {
    public:
        virtual ~LoadingTask() {}
        virtual void Run(GfxContext* context) = 0;

        inline bool ShouldStop() const { return !m_Running; }
        inline bool IsRunning() const { return m_Running; }
        inline void SetRunning(bool running) { m_Running = running; }

    private:
        bool m_Running = false;
    };

    class PoisonPillTask : public LoadingTask
    {
        static PoisonPillTask* s_Instance;
    public:
        static PoisonPillTask* Get() { if (!s_Instance) s_Instance = new PoisonPillTask(); return s_Instance; }
        void Run(GfxContext*) override {}
    private:
        PoisonPillTask() {}
    };

    class LoadingThread
    {
    public:
        LoadingThread()
        {
            m_ThreadHandle = new std::thread(&LoadingThread::Run, this);
        }

        ~LoadingThread()
        {
            Stop();
            m_ThreadHandle->join();
            delete m_ThreadHandle;
        }

        void Submit(LoadingTask* task)
        {
            m_TaskQueue.Push(task);
        }

        void Run()
        {
            m_Running = true;
            GfxContext* context = new GfxContext();
            while (m_Running)
            {
                m_CurrentTask = m_TaskQueue.Pop();
                if (m_CurrentTask.load() == PoisonPillTask::Get()) break;
                m_CurrentTask.load()->SetRunning(true);
                m_CurrentTask.load()->Run(context);
                context->Submit();
                m_CurrentTask.load()->SetRunning(false);

                LoadingTask* lastTask = m_CurrentTask.exchange(nullptr);
                delete lastTask;
            }
            delete context;
        }

        void ResetAndWait()
        {
            // 1. Stop loading thread
            Stop();

            // 2. Wait it to finish
            m_ThreadHandle->join();

            // 3. Restart loading thread
            delete m_ThreadHandle;
            m_Running = true;
            m_TaskQueue.Clear();
            m_ThreadHandle = new std::thread(&LoadingThread::Run, this);
        }

        void Stop()
        {
            m_TaskQueue.Clear();
            m_TaskQueue.Push(PoisonPillTask::Get());
            LoadingTask* currentTask = m_CurrentTask;
            if (currentTask) currentTask->SetRunning(false);
            m_Running = false;
        }

    private:
        BlockingQueue<LoadingTask*> m_TaskQueue;
        std::thread* m_ThreadHandle = nullptr;
        std::atomic<LoadingTask*> m_CurrentTask = nullptr;
        bool m_Running = false;
    };

    extern LoadingThread* g_LoadingThread;
}