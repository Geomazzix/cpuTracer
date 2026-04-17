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
#include "Utility/ProgressReporter.h"

#if defined(_WIN64) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace CRT
{
	static int TerminalWidth();

	// ProgressReporter Method Definitions
	ProgressReporter::ProgressReporter(std::shared_ptr<JobSystem> jobSystem, int64_t m_totalWorkDone, const std::string& m_title)
		: m_jobSystem(jobSystem), m_totalWorkDone(std::max((int64_t)1, m_totalWorkDone)), m_title(m_title), m_startTime(std::chrono::system_clock::now()) 
	{
		m_workDone = 0;
		m_exitThread = false;
		m_jobSystem->Execute([this]() {
			PrintBar(); 
		});	
	}

	ProgressReporter::~ProgressReporter() 
	{
		m_workDone = m_totalWorkDone;
		m_exitThread = true;
		printf("\n");
	}

	void ProgressReporter::Update(int64_t num /*= 1*/)
	{
		if (num == 0) return;
		m_workDone += num;
	}

	void ProgressReporter::PrintBar()
	{
		int barLength = TerminalWidth() - 28;
		int totalPlusses = std::max(2, barLength - (int)m_title.size());
		int plussesPrinted = 0;

		// Initialize progress string
		const int bufLen = static_cast<int>(m_title.size()) + totalPlusses + 64;
		std::unique_ptr<char[]> buf(new char[bufLen]);
		snprintf(buf.get(), bufLen, "\r%s: [", m_title.c_str());
		char* curSpace = buf.get() + strlen(buf.get());
		char* s = curSpace;
		for (int i = 0; i < totalPlusses; ++i) *s++ = ' ';
		*s++ = ']';
		*s++ = ' ';
		*s++ = '\0';
		fputs(buf.get(), stdout);
		fflush(stdout);

		std::chrono::milliseconds sleepDuration(250);
		int iterCount = 0;
		while (!m_exitThread) 
		{
			std::this_thread::sleep_for(sleepDuration);

			// Periodically increase sleepDuration to reduce overhead of
			// updates.
			++iterCount;
			if (iterCount == 10)
				// Up to 0.5s after ~2.5s elapsed
				sleepDuration *= 2;
			else if (iterCount == 70)
				// Up to 1s after an additional ~30s have elapsed.
				sleepDuration *= 2;
			else if (iterCount == 520)
				// After 15m, jump up to 5s intervals
				sleepDuration *= 5;

			float percentDone = static_cast<float>(m_workDone) / static_cast<float>(m_totalWorkDone);
			int plussesNeeded = std::round(static_cast<double>(totalPlusses * percentDone));
			while (plussesPrinted < plussesNeeded) {
				*curSpace++ = '+';
				++plussesPrinted;
			}
			fputs(buf.get(), stdout);

			// Update elapsed time and estimated time to completion
			float seconds = ElapsedMS() / 1000.f;
			float estRemaining = seconds / percentDone - seconds;
			if (percentDone == 1.f)
			{
				printf(" (%.1fs)       ", seconds);
			}
			else if (!std::isinf(estRemaining))
			{
				printf(" (%.1fs|%.1fs)  ", seconds, std::max(0.0f, estRemaining));
			}
			else
			{
				printf(" (%.1fs|?s)  ", seconds);
			}
			
			fflush(stdout);
		}
	}

	float ProgressReporter::ElapsedMS() const
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		int64_t elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count();
		return static_cast<float>(elapsedMS);
	}

	void ProgressReporter::Done()
	{
		m_workDone = m_totalWorkDone;
		m_exitThread = true;
	}

	static int TerminalWidth() {
#if defined(_WIN64) || defined(_WIN32)
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		if (h == INVALID_HANDLE_VALUE || !h) {
			fprintf(stderr, "GetStdHandle() call failed");
			return 80;
		}
		CONSOLE_SCREEN_BUFFER_INFO bufferInfo = { 0 };
		GetConsoleScreenBufferInfo(h, &bufferInfo);
		return bufferInfo.dwSize.X;
#else
		struct winsize w;
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0) {
			// ENOTTY is fine and expected, e.g. if output is being piped to a file.
			if (errno != ENOTTY) {
				static bool warned = false;
				if (!warned) {
					warned = true;
					fprintf(stderr, "Error in ioctl() in TerminalWidth(): %d\n",
						errno);
				}
			}
			return 80;
		}
		return w.ws_col;
#endif
	}
}