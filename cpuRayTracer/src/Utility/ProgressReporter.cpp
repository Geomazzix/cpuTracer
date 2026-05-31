/*
	pbrt source code is Copyright(c) 1998-2016
						Matt Pharr, Greg Humphreys, and Wenzel Jakob.

	This file is part of pbrt.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

	- Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.

	- Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
	TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
	PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/* Note that the original code has been modified as to adjust itself to modern cpp. */

#include "utility/progressReporter.hpp"
#include "jobsystem.hpp"
#include <format>
#include <iostream>

#if defined(_WIN64) || defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <cerrno>
#endif

namespace crt
{
	using namespace std::chrono_literals;

	ProgressReporter::ProgressReporter(JobSystem& jobSystem, int64_t m_totalWorkDone, std::string_view m_title) :
		m_jobSystem(jobSystem), 
		m_totalWorkDone{ std::max(static_cast<int64_t>(1), m_totalWorkDone) },
		m_title(m_title), 
		m_startTime(std::chrono::system_clock::now()) 
	{
		m_workDone = 0;
		m_jobSystem.Execute([this, st = m_stopSource.get_token()]()
		{
			PrintBar(st);
		});
	}

	ProgressReporter::~ProgressReporter() 
	{
		m_workDone.store(m_totalWorkDone);
		m_stopSource.request_stop();
		std::cout << std::endl;
	}

	void ProgressReporter::Update(int64_t num)
	{
		if (num == 0)
		{
			return;
		}
		m_workDone.fetch_add(num);
	}

	void ProgressReporter::Done()
	{
		m_workDone.store(m_totalWorkDone);
		m_stopSource.request_stop();
	}

	void ProgressReporter::PrintBar(std::stop_token stopToken)
	{
		const int barLength = TerminalWidth() - 28;
		const int totalPlusses = std::max(2, barLength - static_cast<int>(m_title.size()));
		int plussesPrinted = 0;

		auto RenderBar = [&totalPlusses, &plussesPrinted](const std::string_view title)
		{
			const int filled = std::min(totalPlusses, plussesPrinted);
			return std::format("\r{}: [{}{}] ", title, std::string(filled, '+'), std::string(totalPlusses - filled, ' '));
		};
		std::cout << RenderBar(m_title) << std::flush;

		auto sleepDuration = 250ms;
		int iterCount = 0;

		while (!stopToken.stop_requested())
		{
			std::this_thread::sleep_for(sleepDuration);

			/* Periodically increase sleepDuration to reduce overhead of updates. */
			switch (++iterCount) 
			{
			case 10:
				// Up to 0.5s after ~2.5s elapsed
				sleepDuration *= 2;
				break;
			case 70:
				// Up to 1s after an additional ~30s have elapsed.
				sleepDuration *= 2;
				break;
			case 520:
				// After 15m, jump up to 5s intervals
				sleepDuration *= 5;
				break;
			default:
				break;
			}

			/* Update elapsed time and estimated time to completion */
			const float percentDone = static_cast<float>(m_workDone.load()) / static_cast<float>(m_totalWorkDone);
			plussesPrinted = static_cast<int>(std::round(totalPlusses * percentDone));
			const float seconds = ElapsedMs() / 1000.f;
			const float estRemaining = seconds / percentDone - seconds;

			std::string suffix;
			if (percentDone == 1.f)
			{
				suffix = std::format("({:.1f}s)       ", seconds);
			}
			else if (!std::isinf(estRemaining))
			{
				suffix = std::format("({:.1f}s|{:.1f}s)  ", seconds, std::max(0.f, estRemaining));
			}
			else
			{
				suffix = std::format("({:.1f}s|?s)  ", seconds);
			}
			
			std::cout << RenderBar(m_title) << suffix << std::flush;
		}
	}

	[[nodiscard]] float ProgressReporter::ElapsedMs() const
	{
		const auto now = std::chrono::system_clock::now();
		const uint64_t elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
		return static_cast<float>(elapsedMS);
	}

	[[nodiscard]] int ProgressReporter::TerminalWidth()
	{
#if defined(_WIN64) || defined(_WIN32)
		const HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		if (h == INVALID_HANDLE_VALUE || h == nullptr) 
		{
			std::fputs("GetStdHandle() call failed\n", stderr);
			return 80;
		}
		
		CONSOLE_SCREEN_BUFFER_INFO info{};
		if (::GetConsoleScreenBufferInfo(h, &info) == 0)
		{
			return 80;
		}
		return info.dwSize.X;
#else
		winsize w{};
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0)
		{
			/* ENOTTY is fine and expected, e.g. if output is being piped to a file. */
			if (errno != ENOTTY)
			{
				static std::atomic<bool> warned{ false };
				if (!warned.exchange(true))
				{
					std::fprintf(stderr, "Error in ioctl() in TerminalWidth(): %d\n", errno);
				}
			}
			return 80;
		}
		return w.ws_col;
#endif
	}
}