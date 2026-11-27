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

#include <memory>
#include <string>
#include <vector>

#include "gz/rendering/base/BaseStorage.hh"

using namespace gz;
using namespace rendering;

/// \brief Minimal object type that can be held by a BaseStore
class TestObject
{
  public: TestObject(unsigned int _id, const std::string &_name)
      : id(_id), name(_name)
  {
  }

  public: unsigned int Id() const
  {
    return this->id;
  }

  public: std::string Name() const
  {
    return this->name;
  }

  public: void Destroy()
  {
    ++this->destroyCount;
  }

  public: unsigned int destroyCount = 0;

  private: unsigned int id;

  private: std::string name;
};

using TestObjectPtr = std::shared_ptr<TestObject>;
using TestStore = BaseStore<TestObject, TestObject>;

/// \brief Order in which object IDs are handed to the store
enum class IdOrder
{
  /// \brief Decreasing, as assigned by BaseScene::CreateObjectId
  DESCENDING,
  /// \brief Decreasing and wrapping past zero
  DESCENDING_WRAPPED,
  /// \brief Increasing, e.g. caller-chosen IDs
  ASCENDING,
  /// \brief No particular order
  UNORDERED
};

/////////////////////////////////////////////////
std::vector<TestObjectPtr> CreateObjects(IdOrder _order, unsigned int _count)
{
  std::vector<TestObjectPtr> objects;
  for (unsigned int i = 0; i < _count; ++i)
  {
    unsigned int id = 0u;
    switch (_order)
    {
      case IdOrder::DESCENDING:
        id = 65535u - i;
        break;
      case IdOrder::DESCENDING_WRAPPED:
        id = 20u - i;
        break;
      case IdOrder::ASCENDING:
        id = 100u + i;
        break;
      case IdOrder::UNORDERED:
      default:
        id = (i * 37u) % 101u + 1u;
        break;
    }
    objects.push_back(std::make_shared<TestObject>(id,
        "object_" + std::to_string(id)));
  }
  return objects;
}

/////////////////////////////////////////////////
/// \brief Expect the store to hold exactly _expected, in order, with every
/// object reachable by index, ID, name and pointer
void ExpectContents(const TestStore &_store,
    const std::vector<TestObjectPtr> &_expected)
{
  ASSERT_EQ(_expected.size(), _store.Size());
  for (unsigned int i = 0; i < _expected.size(); ++i)
  {
    const TestObjectPtr &object = _expected[i];
    EXPECT_EQ(object, _store.GetByIndex(i));
    EXPECT_EQ(object, _store.GetById(object->Id()));
    EXPECT_EQ(object, _store.GetByName(object->Name()));
    EXPECT_TRUE(_store.ContainsId(object->Id()));
    EXPECT_TRUE(_store.ContainsName(object->Name()));
    EXPECT_TRUE(_store.Contains(object));
  }
}

/////////////////////////////////////////////////
TEST(BaseStore, AddRejectsDuplicates)
{
  TestStore store;
  auto a = std::make_shared<TestObject>(10u, "a");
  auto b = std::make_shared<TestObject>(20u, "b");
  EXPECT_TRUE(store.Add(a));
  EXPECT_TRUE(store.Add(b));

  // Same ID, different name
  EXPECT_FALSE(store.Add(std::make_shared<TestObject>(10u, "c")));
  // Same name, different ID
  EXPECT_FALSE(store.Add(std::make_shared<TestObject>(30u, "b")));
  ExpectContents(store, {a, b});

  EXPECT_EQ(nullptr, store.GetById(30u));
  EXPECT_FALSE(store.ContainsId(30u));
  EXPECT_EQ(nullptr, store.GetByName("c"));
  EXPECT_FALSE(store.ContainsName("c"));
}

/////////////////////////////////////////////////
TEST(BaseStore, EmptyStore)
{
  TestStore store;
  EXPECT_EQ(0u, store.Size());
  EXPECT_EQ(nullptr, store.GetById(0u));
  EXPECT_FALSE(store.ContainsId(0u));
  EXPECT_EQ(nullptr, store.RemoveById(0u));
  store.DestroyAll();
  EXPECT_EQ(0u, store.Size());
}

/////////////////////////////////////////////////
TEST(BaseStore, LookupAndRemove)
{
  for (IdOrder order : {IdOrder::DESCENDING, IdOrder::DESCENDING_WRAPPED,
      IdOrder::ASCENDING, IdOrder::UNORDERED})
  {
    SCOPED_TRACE(static_cast<int>(order));

    std::vector<TestObjectPtr> expected = CreateObjects(order, 64u);
    TestStore store;
    for (const auto &object : expected)
      EXPECT_TRUE(store.Add(object));
    ExpectContents(store, expected);

    // IDs that are not in the store
    for (unsigned int id : {0u, 1u, 99u, 65534u - 64u, 65536u, 4294967295u})
    {
      bool present = false;
      for (const auto &object : expected)
        present = present || object->Id() == id;
      EXPECT_EQ(present, store.ContainsId(id)) << id;
      EXPECT_EQ(present, nullptr != store.GetById(id)) << id;
    }

    // Remove from the middle, the front and the back using every accessor,
    // checking all remaining objects are still reachable after each removal
    TestObjectPtr object = expected[30];
    EXPECT_EQ(object, store.RemoveById(object->Id()));
    expected.erase(expected.begin() + 30);
    ExpectContents(store, expected);
    EXPECT_EQ(nullptr, store.GetById(object->Id()));
    EXPECT_FALSE(store.ContainsName(object->Name()));

    object = expected[10];
    EXPECT_EQ(object, store.RemoveByName(object->Name()));
    expected.erase(expected.begin() + 10);
    ExpectContents(store, expected);

    object = expected.front();
    EXPECT_EQ(object, store.RemoveByIndex(0u));
    expected.erase(expected.begin());
    ExpectContents(store, expected);

    object = expected.back();
    EXPECT_EQ(object, store.Remove(object));
    expected.pop_back();
    ExpectContents(store, expected);

    // Re-adding a removed object appends it, out of ID order
    EXPECT_TRUE(store.Add(object));
    expected.push_back(object);
    ExpectContents(store, expected);

    object = expected[20];
    store.DestroyById(object->Id());
    EXPECT_EQ(1u, object->destroyCount);
    expected.erase(expected.begin() + 20);
    ExpectContents(store, expected);

    store.DestroyAll();
    EXPECT_EQ(0u, store.Size());
    for (const auto &destroyed : expected)
    {
      EXPECT_EQ(1u, destroyed->destroyCount);
      EXPECT_EQ(nullptr, store.GetById(destroyed->Id()));
      EXPECT_FALSE(store.ContainsName(destroyed->Name()));
    }
  }
}

/////////////////////////////////////////////////
TEST(BaseStore, RemoveAll)
{
  std::vector<TestObjectPtr> objects =
      CreateObjects(IdOrder::DESCENDING, 8u);
  TestStore store;
  for (const auto &object : objects)
    EXPECT_TRUE(store.Add(object));

  store.RemoveAll();
  EXPECT_EQ(0u, store.Size());
  for (const auto &object : objects)
  {
    EXPECT_EQ(0u, object->destroyCount);
    EXPECT_FALSE(store.ContainsId(object->Id()));
  }

  // The same IDs and names can be added again
  for (const auto &object : objects)
    EXPECT_TRUE(store.Add(object));
  ExpectContents(store, objects);
}
