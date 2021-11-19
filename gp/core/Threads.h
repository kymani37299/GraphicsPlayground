#pragma once

#include <thread>
#include <mutex>
#include <vector>

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
}