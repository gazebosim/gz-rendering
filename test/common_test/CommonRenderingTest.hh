/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#ifndef COMMON_TEST_HH_
#define COMMON_TEST_HH_

#include <gtest/gtest.h>

#include <string>
#include <unordered_set>

#include <gz/common/Console.hh>

#include <gz/rendering/RenderingIface.hh>
#include <gz/rendering/RenderEngine.hh>
#include <gz/utils/Environment.hh>

constexpr const char * kEngineToTestEnv = "GZ_ENGINE_TO_TEST";
constexpr const char * kEngineBackend = "GZ_ENGINE_BACKEND";
constexpr const char * kEngineHeadless = "GZ_ENGINE_HEADLESS";

class CommonRenderingTest: public testing::Test
{
  /// \brief Set up the test case
  public: void SetUp() override
  {
    gz::common::Console::SetVerbosity(4);
    gz::utils::env(kEngineToTestEnv, this->engineToTest);

    if (this->engineToTest.empty())
    {
      GTEST_SKIP() << kEngineToTestEnv << "environment not set";
    }

    std::map<std::string, std::string> engineParams;

    std::string backend;
    if (gz::utils::env(kEngineBackend, backend))
    {
      if(this->engineToTest != "ogre2")
      {
        GTEST_SKIP() << "Attempted to change BACKEND with engine '" 
            << this->engineToTest << " is unsupported";
      }

      if(backend == "vulkan")
      {
        gzdbg << "Using OGRE2-VULKAN backend to test" << std::endl;
        engineParams["vulkan"] = "1";
      }
      else if(backend == "metal")
      {
        gzdbg << "Using OGRE2-VULKAN backend to test" << std::endl;
        engineParams["metal"] = "1";
      }
    }

    std::string headless;
    if (gz::utils::env(kEngineHeadless, headless))
    {
      if(this->engineToTest != "ogre2")
      {
        GTEST_SKIP() << "Attempted to use HEADLESS with engine '" 
            << this->engineToTest << " is unsupported";
      }
      engineParams["headless"] = "1";
    }

    engine = gz::rendering::engine(this->engineToTest, engineParams);
    if (!engine)
    {
      GTEST_SKIP() << "Engine '" << this->engineToTest << "' could not be loaded" << std::endl;
    }
  }

  /// \brief Tear down the test case 
  public: void TearDown() override
  {
    ASSERT_TRUE(gz::rendering::unloadEngine(this->engineToTest));
  }

  /// \brief String name of the engine to test 
  public: std::string engineToTest;

  /// \brief Pointer to the rendering engine to test 
  public: gz::rendering::RenderEngine *engine = nullptr;
};


#define CHECK_SUPPORTED_ENGINE(...) \
if(std::unordered_set<std::string>({__VA_ARGS__}).count(this->engineToTest) == 0) \
  GTEST_SKIP() << "Engine '" << this->engineToTest << "' unsupported";

#define CHECK_UNSUPPORTED_ENGINE(...) \
if(std::unordered_set<std::string>({__VA_ARGS__}).count(this->engineToTest) != 0) \
  GTEST_SKIP() << "Engine '" << this->engineToTest << "' unsupported";

#define CHECK_RENDERPASS_SUPPORTED() \
{ \
  gz::rendering::RenderPassSystemPtr rpSystem = this->engine->RenderPassSystem(); \
  if (!rpSystem) \
  { \
    GTEST_SKIP() << "Engine '" << this->engineToTest << "' does not support RenderPassSystem"; \
  } \
} \


#endif  // COMMON_TEST_HH_
