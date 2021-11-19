#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <queue>

#define CURRENT_THREAD std::this_thread::get_id()

namespace GP
{
	using ThreadID = std::thread::id;
	inline bool IsThread(ThreadID id) { return id == CURRENT_THREAD; }

	template<typename T>
	class MutexVector
	{
	public:
		inline void Lock() { m_Mutex.lock(); }
		inline void Unlock() { m_Mutex.unlock(); }

		inline void Add(const T& element) 
		{ 
			Lock();
			m_Data.push_back(element);
			Unlock();
		}

		inline void Remove(size_t index) 
		{ 
			Lock();
			m_Data.erase(m_Data.begin() + index);
			Unlock();
		}

		inline void Clear()
		{
			Lock();
			m_Data.clear();
			Unlock();
		}

		template<typename F>
		void ForEach(F& f)
		{
			Lock();
			for (T& e : m_Data) f(e);
			Unlock();
		}

		// TODO: Add [] override
		// TODO: Add foreach override

	private:
		std::recursive_mutex m_Mutex;
		std::vector<T> m_Data;
	};

	template <typename T>
    class BlockingQueue
    {
    public:
        void Push(const T& value)
        {
            {
                std::unique_lock<std::mutex> lock(m_Mutex);
                m_Queue.push_front(value);
            }
            m_Condition.notify_one();
        }

        T Pop()
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Condition.wait(lock, [=] { return !m_Queue.empty(); });
            T rc(std::move(m_Queue.back()));
            m_Queue.pop_back();
            return rc;
        }

		void Clear()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Queue.clear();
		}

    private:
        std::mutex              m_Mutex;
        std::condition_variable m_Condition;
        std::deque<T>           m_Queue;
    };
}