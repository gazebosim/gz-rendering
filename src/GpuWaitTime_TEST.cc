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

#include <chrono>
#include <thread>

#include "gz/rendering/GpuWaitTime.hh"

using namespace gz;
using namespace rendering;
using namespace std::chrono_literals;

/////////////////////////////////////////////////
TEST(GpuWaitTimeTest, CounterAccumulates)
{
  const auto start = ThreadGpuWaitTime();
  AddThreadGpuWaitTime(5ms);
  AddThreadGpuWaitTime(7ms);
  EXPECT_EQ(start + 12ms, ThreadGpuWaitTime());
}

/////////////////////////////////////////////////
TEST(GpuWaitTimeTest, CounterIsPerThread)
{
  const auto start = ThreadGpuWaitTime();

  std::chrono::nanoseconds other{-1};
  std::thread thread([&other]()
  {
    AddThreadGpuWaitTime(50ms);
    other = ThreadGpuWaitTime();
  });
  thread.join();

  // The other thread starts from zero and only sees its own wait.
  EXPECT_EQ(50ms, other);
  EXPECT_EQ(start, ThreadGpuWaitTime());
}

/////////////////////////////////////////////////
TEST(GpuWaitTimeTest, ScopedWaitCountsItsLifetime)
{
  const auto start = ThreadGpuWaitTime();
  {
    ScopedGpuWait wait;
    std::this_thread::sleep_for(10ms);
  }
  EXPECT_GE(ThreadGpuWaitTime() - start, 10ms);
}
