/*
 * Copyright (C) 2026 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef GZ_RENDERING_OGRE2_OGRE2READBACKWAITPOLICY_HH_
#define GZ_RENDERING_OGRE2_OGRE2READBACKWAITPOLICY_HH_

#include <algorithm>
#include <cmath>

#include "gz/rendering/config.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Chooses how a thread waits for its GPU readbacks: sleep, which
    /// saves CPU, or spin in map(), which is faster when nothing else runs.
    ///
    /// Sleeping makes the readback itself slower when the rest of the
    /// process waits for the GPU too (an idle CPU slows the transfer down).
    /// That costs nothing while the thread has idle time anyway, and
    /// everything when the thread is the bottleneck. So the policy measures
    /// the thread's slack, the share of its wall time it was off the CPU
    /// outside the readback waits themselves (e.g. waiting for the next
    /// render request), and spins while there is (almost) none.
    ///
    /// - Starts spinning, as before the policy, and keeps spinning for the
    ///   first kMinSamples intervals between readbacks.
    /// - A spinning thread switches to sleeping when the slack exceeds
    ///   kSleepAbove; a sleeping one back to spinning when it drops below
    ///   kSpinBelow. In between, the mode stays (hysteresis).
    /// - The slack is averaged over about kWindowUs of wall time (each
    ///   interval weighs by its length, older ones decay exponentially), not
    ///   per readback, so a frame of many cameras followed by one idle gap
    ///   gives a steady value instead of one swinging with every gap.
    ///
    /// The waits' own idle time is never counted as slack, in either mode.
    /// A wait still counts in the wall time, so a slower (sleeping) wait
    /// lowers the slack a little, which only pushes towards spinning.
    ///
    /// One policy per thread, shared by all the readbacks of that thread
    /// (all cameras and lidars): slack is a property of the thread, and each
    /// interval of it is counted exactly once.
    /// Not thread-safe; use one instance per thread.
    class Ogre2ReadbackWaitPolicy
    {
      /// \brief Slack below which a sleeping thread switches to spinning.
      public: static constexpr double kSpinBelow = 0.03;

      /// \brief Slack above which a spinning thread switches to sleeping.
      public: static constexpr double kSleepAbove = 0.06;

      /// \brief Time constant of the slack average, in us of wall time.
      public: static constexpr double kWindowUs = 1e6;

      /// \brief Intervals measured before the first switch to sleeping.
      public: static constexpr int kMinSamples = 16;

      /// \brief Longest interval counted, in us: a longer one (e.g. after a
      /// sensor was off) counts as this long, with the same idle share, so a
      /// pause does not keep the thread sleeping long after it.
      public: static constexpr double kMaxIntervalUs = 100000;

      /// \brief Record the start of a readback wait and choose its mode.
      /// \param[in] _wallUs Monotonic wall time now, in us.
      /// \param[in] _cpuUs The thread's CPU time now, in us.
      /// \return True to sleep, false to spin (wait in map()).
      public: bool Sleep(double _wallUs, double _cpuUs)
      {
        if (this->lastWallUs >= 0 && _wallUs > this->lastWallUs)
        {
          double interval = _wallUs - this->lastWallUs;
          // Off the CPU outside our own waits.
          double idle = interval - (_cpuUs - this->lastCpuUs) -
              this->waitIdleUs;
          idle = std::clamp(idle, 0.0, interval);
          if (interval > kMaxIntervalUs)
          {
            idle *= kMaxIntervalUs / interval;
            interval = kMaxIntervalUs;
          }
          const double keep = std::exp(-interval / kWindowUs);
          this->idleUs = this->idleUs * keep + idle;
          this->intervalUs = this->intervalUs * keep + interval;
          if (this->samples < kMinSamples)
            ++this->samples;
        }
        this->lastWallUs = _wallUs;
        this->lastCpuUs = _cpuUs;
        this->waitIdleUs = 0;

        if (this->samples >= kMinSamples)
        {
          const double slack = this->Slack();
          if (this->sleep && slack < kSpinBelow)
            this->sleep = false;
          else if (!this->sleep && slack > kSleepAbove)
            this->sleep = true;
        }
        return this->sleep;
      }

      /// \brief Record the idle time of the wait that Sleep() started:
      /// its wall time minus the CPU time the thread spent in it.
      /// \param[in] _us Idle time in us.
      public: void WaitIdle(double _us)
      {
        this->waitIdleUs += std::max(_us, 0.0);
      }

      /// \brief Drop the interval in progress, e.g. when the end of a wait
      /// could not be measured; the averages and the mode stay.
      public: void Restart()
      {
        this->lastWallUs = -1;
        this->waitIdleUs = 0;
      }

      /// \brief Measured slack: idle share of the thread's wall time,
      /// outside the readback waits, averaged over about kWindowUs.
      /// \return Slack in [0, 1].
      public: double Slack() const
      {
        return this->intervalUs > 0 ? this->idleUs / this->intervalUs : 0.0;
      }

      /// \brief Wall time at the last Sleep() call, -1 before the first.
      private: double lastWallUs{-1};

      /// \brief Thread CPU time at the last Sleep() call.
      private: double lastCpuUs{0};

      /// \brief Idle time of the waits since the last Sleep() call.
      private: double waitIdleUs{0};

      /// \brief Decaying sum of the idle time outside the waits.
      private: double idleUs{0};

      /// \brief Decaying sum of the interval lengths.
      private: double intervalUs{0};

      /// \brief Number of intervals measured, up to kMinSamples.
      private: int samples{0};

      /// \brief Current mode; starts spinning, as before the policy.
      private: bool sleep{false};
    };
    }
  }
}
#endif
