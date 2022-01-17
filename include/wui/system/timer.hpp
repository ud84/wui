//
// Copyright (c) 2021-2022 Anton Golovkov (udattsk at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/ud84/wui
//

#pragma once

#ifndef _WIN32
#include <chrono>
#include <thread>
#else
#include <windows.h>
#endif

#include <atomic>
#include <functional>
#include <exception>
#include <cstdint>

namespace wui
{

#ifndef _WIN32
class timer
{
public:
	explicit timer(std::function<void(void)> callback_)
		: runned(false), thread(), callback(callback_), interval(1000)
	{
	}

	~timer()
	{
		stop();
	}

	void start(const uint32_t interval_ = 1000 /* in milliseconds */)
	{
		if (!runned)
		{
			interval = interval_;
			runned = true;

			thread = std::thread(&timer::run, this);
		}
	}

	void stop()
	{
		if (runned)
		{
			runned = false;
			if (thread.joinable()) thread.join();
		}
	}
	
	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;

private:
	std::atomic<bool> runned;
	std::thread thread;
	std::function<void(void)> callback;

	uint32_t interval;

	void run()
	{
	    auto begin = std::chrono::high_resolution_clock::now();

		while (runned)
		{
			auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count();
			if (elapsed > interval * 1000)
			{
			    callback();
			    begin = std::chrono::high_resolution_clock::now();
			}
			else
			{
			    std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
		}
	}
};
#else
class timer
{
public:
	explicit timer(std::function<void(void)> callback_)
		: callback(callback_), h_timer(nullptr), h_timer_queue(nullptr), runned(false)
	{
	}

	~timer()
	{
		stop();
	}

	void start(const uint32_t interval = 1000)
	{
		if (runned || h_timer_queue)
		{
			return;
		}
		
        h_timer_queue = CreateTimerQueue();
		if (!h_timer_queue)
		{
			throw std::exception("Error CreateTimerQueue()");
		}

		if (!CreateTimerQueueTimer(&h_timer, h_timer_queue,
			(WAITORTIMERCALLBACK)TimerRoutine, this, 0, interval, WT_EXECUTEDEFAULT))
		{
			throw std::exception("Error CreateTimerQueueTimer()");
		}
		runned = true;
	}

	void stop()
	{
		if (!runned)
		{
			return;
		}
				
		if (h_timer && h_timer_queue)
		{
			DeleteTimerQueueTimer(h_timer_queue, h_timer, INVALID_HANDLE_VALUE);
            h_timer = nullptr;
		}
		if (h_timer_queue)
		{
			DeleteTimerQueue(h_timer_queue);
            h_timer_queue = nullptr;
		}
		runned = false;
	}

	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;

private:
	std::function<void(void)> callback;

	HANDLE h_timer;
    HANDLE h_timer_queue;

	std::atomic<bool> runned;
	
	static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN)
	{
		static_cast<timer*>(lpParam)->callback();
	}
};
#endif

}
