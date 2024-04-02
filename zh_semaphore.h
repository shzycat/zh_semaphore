#pragma once

#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <stdint.h>


/*
* Written by shzy.
* Happy for using.
*/


namespace zh{


template <typename T,
	std::enable_if<std::is_same<T, uint64_t>::value || std::is_same<T, bool>::value> * = nullptr>
class basic_semaphore;



typedef basic_semaphore<bool>     binary_semaphore;
typedef binary_semaphore          semaphore;
typedef basic_semaphore<uint64_t> counting_semaphore;



template <typename T,
	std::enable_if<std::is_same<T, uint64_t>::value || std::is_same<T, bool>::value> *>
class basic_semaphore
{
public:
	basic_semaphore(T value = 0)
		:value_(value)
	{}

	~basic_semaphore() = default;

public:
	/* for binary_semaphore */
	void notify()           { notify_internal(); }
	void wait()             { wait_internal(); }
	bool wait_for(int ms)   { return wait_internal(ms); }

	/* for counting_semaphore */
	template <std::enable_if<std::is_same<T, uint64_t>::value> * = nullptr>
	inline
	void release(uint64_t count = 1) { release_internal(count); }

	template <std::enable_if<std::is_same<T, uint64_t>::value> * = nullptr>
	inline
	void acquire(uint64_t count = 1) { acquire_internal(count); }

	template <std::enable_if<std::is_same<T, uint64_t>::value> * = nullptr>
	inline
	bool acquire_for(uint64_t count = 1, int ms = -1) { return acquire_internal(count, ms); }


	//reset to zero
	void reset()
	{
		std::lock_guard<std::mutex> lock(mtx_);
		value_ = static_cast<T>(0);
	}


private:
	inline void notify_internal()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		value_ = true;
		lock.unlock();

		cv_lk_.notify_all();
	}

	inline bool wait_internal(int ms = -1)
	{
		std::unique_lock<std::mutex> lock(mtx_);

		if (ms <= 0)
		{
			while (!value_)
			{
				cv_lk_.wait(lock);
			}
		}
		else
		{
			auto predicate_inside = [this]() { return this->value_; };
			bool acq_suc = cv_lk_.wait_for(lock, std::chrono::milliseconds(ms), predicate_inside);
			if (!acq_suc)
			{
				return false;
			}
		}

		value_ = false;
		return true;
	}

	inline void release_internal(uint64_t count)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		value_ += count;
		lock.unlock();

		cv_lk_.notify_all();
	}

	inline bool acquire_internal(uint64_t count, int ms = -1)
	{
		std::unique_lock<std::mutex> lock(mtx_);

		if (ms <= 0)
		{
			while (value_ < count)
			{
				cv_lk_.wait(lock);
			}
		}
		else
		{
			auto predicate_inside = [this, &count]() { return (this->value_ > count); };
			bool acq_suc = cv_lk_.wait_for(lock, std::chrono::milliseconds(ms), predicate_inside);
			if (!acq_suc)
			{
				return false;
			}
		}

		value_ -= count;
		return true;
	}


private:
	std::condition_variable cv_lk_;
	std::mutex mtx_;

	T value_;

};














} /*namespace zh*/