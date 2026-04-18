
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

#if defined(_MSC_VER)
#define NOMINMAX
#endif

#pragma once
#include <atomic>
#include <chrono>
#include <thread>
#include "jobsystem.hpp"

namespace CRT 
{
    /**
     * @brief The progress reporter, prints a progressionbar in the console, while running on a seperate thread.
     * This implementation is basically a ported and tweaked version of the progression bar of pbrt. It therefor falls under the license of.
     */
    class ProgressReporter 
    {
    public:
        ProgressReporter(std::shared_ptr<JobSystem> jobSystem, int64_t m_totalWorkDone, const std::string& m_title);
        ~ProgressReporter();

        void Update(int64_t num = 1);
        float ElapsedMS() const;
        void Done();

    private:
        void PrintBar();

        const int64_t m_totalWorkDone;
        const std::string m_title;
        const std::chrono::system_clock::time_point m_startTime;
        std::atomic<int64_t> m_workDone;
        std::atomic<bool> m_exitThread;
        std::shared_ptr<JobSystem> m_jobSystem;
    };
}