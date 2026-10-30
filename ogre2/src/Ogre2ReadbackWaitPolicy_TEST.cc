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

#include <gtest/gtest.h>

#include "Ogre2ReadbackWaitPolicy.hh"

using namespace gz;
using namespace rendering;

/// \brief A synthetic render thread: each readback is a wait, then work on
/// the CPU, then time off the CPU outside the wait (e.g. waiting for the
/// next render request).
class SyntheticThread
{
  /// \brief Run one readback.
  /// \param[in] _waitUs Wall time of the wait.
  /// \param[in] _waitIdleUs Part of the wait spent off the CPU.
  /// \param[in] _workUs CPU time until the next readback.
  /// \param[in] _idleUs Time off the CPU until the next readback.
  /// \return Whether the policy chose to sleep for this wait.
  public: bool Readback(double _waitUs, double _waitIdleUs, double _workUs,
      double _idleUs)
  {
    const bool sleep = this->policy.Sleep(this->wallUs, this->cpuUs);
    this->wallUs += _waitUs;
    this->cpuUs += _waitUs - _waitIdleUs;
    this->policy.WaitIdle(_waitIdleUs);
    this->wallUs += _workUs + _idleUs;
    this->cpuUs += _workUs;
    return sleep;
  }

  /// \brief Run \p _n readbacks; the waits sleep (all idle) when the policy
  /// says so and spin (all CPU) otherwise.
  /// \return Whether the last one slept.
  public: bool Run(int _n, double _waitUs, double _workUs, double _idleUs)
  {
    bool sleep = this->sleeping;
    for (int i = 0; i < _n; ++i)
    {
      sleep = this->Readback(_waitUs, sleep ? _waitUs : 0.0, _workUs,
          _idleUs);
    }
    this->sleeping = sleep;
    return sleep;
  }

  /// \brief The policy under test.
  public: Ogre2ReadbackWaitPolicy policy;

  /// \brief Wall clock, us.
  public: double wallUs{1e6};

  /// \brief Thread CPU clock, us.
  public: double cpuUs{5e5};

  /// \brief Mode of the last wait.
  public: bool sleeping{false};
};

/////////////////////////////////////////////////
TEST(Ogre2ReadbackWaitPolicyTest, StartsSpinningUntilMeasured)
{
  SyntheticThread t;
  // Lots of slack, but too few intervals measured yet.
  EXPECT_FALSE(t.Run(Ogre2ReadbackWaitPolicy::kMinSamples, 500, 1000, 5000));
  // The next readback has enough samples.
  EXPECT_TRUE(t.Run(1, 500, 1000, 5000));
}

/////////////////////////////////////////////////
TEST(Ogre2ReadbackWaitPolicyTest, BusyThreadSpinsIdleThreadSleeps)
{
  // gz-sim at RTF 0, render-bound: the render thread never waits for work.
  SyntheticThread busy;
  EXPECT_FALSE(busy.Run(2000, 800, 1000, 10));
  EXPECT_LT(busy.policy.Slack(), Ogre2ReadbackWaitPolicy::kSpinBelow);

  // Physics-bound: the render thread is idle 15% of the time.
  SyntheticThread idle;
  EXPECT_TRUE(idle.Run(2000, 130, 300, 80));
  EXPECT_NEAR(80.0 / 510.0, idle.policy.Slack(), 1e-3);
}

/////////////////////////////////////////////////
TEST(Ogre2ReadbackWaitPolicyTest, OwnWaitIdleIsNotSlack)
{
  // Sleeping waits make the thread idle 45% of its wall time, but that is
  // the wait's own idle time: the busy thread still has no slack.
  SyntheticThread t;
  for (int i = 0; i < 1000; ++i)
    EXPECT_FALSE(t.Readback(800, 800, 1000, 0));
  EXPECT_NEAR(0.0, t.policy.Slack(), 1e-9);

  // The same idle time outside the waits is slack.
  SyntheticThread outside;
  for (int i = 0; i < 1000; ++i)
    outside.Readback(800, 0, 200, 800);
  EXPECT_NEAR(800.0 / 1800.0, outside.policy.Slack(), 1e-3);
}

/////////////////////////////////////////////////
TEST(Ogre2ReadbackWaitPolicyTest, Hysteresis)
{
  // Slack 4.1%: between the two thresholds, the mode stays as it is.
  const double work = 955;
  const double idle = 45;

  SyntheticThread spinning;
  EXPECT_FALSE(spinning.Run(5000, 100, work, idle));

  SyntheticThread sleeping;
  EXPECT_TRUE(sleeping.Run(100, 100, 300, 300));
  EXPECT_TRUE(sleeping.Run(5000, 100, work, idle));
  EXPECT_NEAR(idle / (100 + work + idle), sleeping.policy.Slack(), 1e-3);

  // Below kSpinBelow it switches to spinning, above kSleepAbove back.
  EXPECT_FALSE(sleeping.Run(5000, 100, 990, 10));
  EXPECT_TRUE(sleeping.Run(5000, 100, 900, 100));
}

/////////////////////////////////////////////////
TEST(Ogre2ReadbackWaitPolicyTest, NoFlappingOnFrames)
{
  // Frames of 16 readbacks, 1 ms each, then one idle gap of 0.8 ms: 4.8%
  // slack, between the thresholds. Single intervals are far above
  // kSleepAbove (the gap) and below kSpinBelow (the rest): the mode must
  // not switch with them, starting either way.
  for (const bool startSleeping : {false, true})
  {
    SyntheticThread t;
    t.Run(100, 100, 300, startSleeping ? 300 : 0);
    ASSERT_EQ(startSleeping, t.sleeping);
    int switches = 0;
    bool last = t.sleeping;
    for (int frame = 0; frame < 300; ++frame)
    {
      for (int i = 0; i < 16; ++i)
      {
        const bool s =
            t.Readback(100, last ? 100 : 0, 900, i == 15 ? 800 : 0);
        switches += s != last;
        last = s;
      }
    }
    EXPECT_EQ(0, switches) << "starting asleep: " << startSleeping;
    EXPECT_NEAR(800.0 / (16 * 1000 + 800), t.policy.Slack(), 2e-3);
  }
}

/////////////////////////////////////////////////
TEST(Ogre2ReadbackWaitPolicyTest, LongIntervals)
{
  // A sensor that was off for 10 s: the pause is slack, but it counts as
  // 100 ms, so a busy thread spins again within about 1.5 s, not 10 s.
  SyntheticThread t;
  EXPECT_FALSE(t.Run(1000, 800, 1000, 10));
  t.wallUs += 1e7;
  EXPECT_TRUE(t.Run(2, 800, 1000, 10));
  EXPECT_FALSE(t.Run(1000, 800, 1000, 10));

  // Long intervals keep their idle share: 120 ms of work, then 200 ms idle
  // (62% slack) sleeps.
  SyntheticThread slow;
  EXPECT_TRUE(slow.Run(50, 800, 120000, 200000));
  EXPECT_NEAR(200000.0 / 320800.0, slow.policy.Slack(), 1e-3);
}

/////////////////////////////////////////////////
TEST(Ogre2ReadbackWaitPolicyTest, RestartDropsTheIntervalInProgress)
{
  SyntheticThread t;
  EXPECT_FALSE(t.Run(1000, 800, 1000, 10));
  // A wait whose end was not measured: its 90 ms of idle must not count
  // (it would be 10% slack).
  t.policy.Restart();
  t.wallUs += 90000;
  EXPECT_FALSE(t.Run(2, 800, 1000, 10));
  EXPECT_LT(t.policy.Slack(), Ogre2ReadbackWaitPolicy::kSpinBelow);
}
