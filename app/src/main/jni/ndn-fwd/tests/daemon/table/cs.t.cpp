/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "table/cs.hpp"
#include <ndn-cxx/lp/tags.hpp>
#include <ndn-cxx/util/crypto.hpp>

#include "tests/test-common.hpp"

#define CHECK_CS_FIND(expected) find([&] (uint32_t found) { BOOST_CHECK_EQUAL(expected, found); });

namespace nfd {
namespace cs {
namespace tests {

using namespace nfd::tests;

BOOST_AUTO_TEST_SUITE(Table)
BOOST_FIXTURE_TEST_SUITE(TestCs, BaseFixture)

class FindFixture : public UnitTestTimeFixture
{
protected:
  Name
  insert(uint32_t id, const Name& name, const std::function<void(Data&)>& modifyData = nullptr)
  {
    shared_ptr<Data> data = makeData(name);
    data->setFreshnessPeriod(time::milliseconds(99999));
    data->setContent(reinterpret_cast<const uint8_t*>(&id), sizeof(id));

    if (modifyData != nullptr) {
      modifyData(*data);
    }

    data->wireEncode();
    m_cs.insert(*data);

    return data->getFullName();
  }

  Interest&
  startInterest(const Name& name)
  {
    m_interest = make_shared<Interest>(name);
    return *m_interest;
  }

  void
  find(const std::function<void(uint32_t)>& check)
  {
    bool hasResult = false;
    m_cs.find(*m_interest,
              [&] (const Interest& interest, const Data& data) {
                hasResult = true;
                const Block& content = data.getContent();
                uint32_t found = *reinterpret_cast<const uint32_t*>(content.value());
                check(found);
              },
              bind([&] {
                hasResult = true;
                check(0);
              }));

    // current Cs::find implementation performs lookup synchronously
    BOOST_CHECK(hasResult);
  }

protected:
  Cs m_cs;
  shared_ptr<Interest> m_interest;
};

BOOST_FIXTURE_TEST_SUITE(Find, FindFixture)

BOOST_AUTO_TEST_CASE(EmptyDataName)
{
  insert(1, "/");

  startInterest("/");
  CHECK_CS_FIND(1);
}

BOOST_AUTO_TEST_CASE(EmptyInterestName)
{
  insert(1, "/A");

  startInterest("/");
  CHECK_CS_FIND(1);
}

BOOST_AUTO_TEST_CASE(ExactName)
{
  insert(1, "/");
  insert(2, "/A");
  insert(3, "/A/B");
  insert(4, "/A/C");
  insert(5, "/D");

  startInterest("/A");
  CHECK_CS_FIND(2);
}

BOOST_AUTO_TEST_CASE(FullName)
{
  Name n1 = insert(1, "/A");
  Name n2 = insert(2, "/A");

  startInterest(n1);
  CHECK_CS_FIND(1);

  startInterest(n2);
  CHECK_CS_FIND(2);
}

BOOST_AUTO_TEST_CASE(Leftmost)
{
  insert(1, "/A");
  insert(2, "/B/p/1");
  insert(3, "/B/p/2");
  insert(4, "/B/q/1");
  insert(5, "/B/q/2");
  insert(6, "/C");

  startInterest("/B");
  CHECK_CS_FIND(2);
}

BOOST_AUTO_TEST_CASE(Rightmost)
{
  insert(1, "/A");
  insert(2, "/B/p/1");
  insert(3, "/B/p/2");
  insert(4, "/B/q/1");
  insert(5, "/B/q/2");
  insert(6, "/C");

  startInterest("/B")
    .setChildSelector(1);
  CHECK_CS_FIND(4);
}

BOOST_AUTO_TEST_CASE(MinSuffixComponents)
{
  insert(1, "/");
  insert(2, "/A");
  insert(3, "/B/1");
  insert(4, "/C/1/2");
  insert(5, "/D/1/2/3");
  insert(6, "/E/1/2/3/4");

  startInterest("/")
    .setMinSuffixComponents(0);
  CHECK_CS_FIND(1);

  startInterest("/")
    .setMinSuffixComponents(1);
  CHECK_CS_FIND(1);

  startInterest("/")
    .setMinSuffixComponents(2);
  CHECK_CS_FIND(2);

  startInterest("/")
    .setMinSuffixComponents(3);
  CHECK_CS_FIND(3);

  startInterest("/")
    .setMinSuffixComponents(4);
  CHECK_CS_FIND(4);

  startInterest("/")
    .setMinSuffixComponents(5);
  CHECK_CS_FIND(5);

  startInterest("/")
    .setMinSuffixComponents(6);
  CHECK_CS_FIND(6);

  startInterest("/")
    .setMinSuffixComponents(7);
  CHECK_CS_FIND(0);
}

BOOST_AUTO_TEST_CASE(MaxSuffixComponents)
{
  insert(1, "/");
  insert(2, "/A");
  insert(3, "/B/2");
  insert(4, "/C/2/3");
  insert(5, "/D/2/3/4");
  insert(6, "/E/2/3/4/5");

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(0);
  CHECK_CS_FIND(0);

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(1);
  CHECK_CS_FIND(1);

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(2);
  CHECK_CS_FIND(2);

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(3);
  CHECK_CS_FIND(3);

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(4);
  CHECK_CS_FIND(4);

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(5);
  CHECK_CS_FIND(5);

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(6);
  CHECK_CS_FIND(6);

  startInterest("/")
    .setChildSelector(1)
    .setMaxSuffixComponents(7);
  CHECK_CS_FIND(6);
}

BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(MustBeFresh, 1)
BOOST_AUTO_TEST_CASE(MustBeFresh)
{
  ///\todo #3944 add /A/1 without FreshnessPeriod, which is same as FreshnessPeriod=0ms
  insert(2, "/A/2", [] (Data& data) { data.setFreshnessPeriod(time::seconds(0)); });
  insert(3, "/A/3", [] (Data& data) { data.setFreshnessPeriod(time::seconds(1)); });
  insert(4, "/A/4", [] (Data& data) { data.setFreshnessPeriod(time::seconds(3600)); });
  insert(5, "/A/5"); // omitted FreshnessPeriod means infinite
  ///\todo #3944 delete /A/5

  // lookup at exact same moment as insertion is not tested because this won't happen in reality

  this->advanceClocks(time::milliseconds(500)); // @500ms
  startInterest("/A")
    .setMustBeFresh(true);
  CHECK_CS_FIND(3);

  this->advanceClocks(time::milliseconds(1500)); // @2s
  startInterest("/A")
    .setMustBeFresh(true);
  CHECK_CS_FIND(4);

  this->advanceClocks(time::seconds(3500)); // @3502s
  startInterest("/A")
    .setMustBeFresh(true);
  CHECK_CS_FIND(4);

  this->advanceClocks(time::seconds(3500)); // @7002s
  startInterest("/A")
    .setMustBeFresh(true);
  CHECK_CS_FIND(5); // expected failure https://redmine.named-data.net/issues/3944#note-2
  ///\todo #3944 this should not find any Data
}

BOOST_AUTO_TEST_CASE(DigestOrder)
{
  Name n1 = insert(1, "/A");
  Name n2 = insert(2, "/A");

  uint32_t expectedLeftmost = 0, expectedRightmost = 0;
  if (n1 < n2) {
    expectedLeftmost = 1;
    expectedRightmost = 2;
  }
  else {
    BOOST_CHECK_MESSAGE(n1 != n2, "implicit digest collision detected");
    expectedLeftmost = 2;
    expectedRightmost = 1;
  }

  startInterest("/A")
    .setChildSelector(0);
  CHECK_CS_FIND(expectedLeftmost);
  startInterest("/A")
    .setChildSelector(1);
  CHECK_CS_FIND(expectedRightmost);
}

BOOST_AUTO_TEST_CASE(DigestExclude)
{
  insert(1, "/A");
  Name n2 = insert(2, "/A");
  insert(3, "/A/B");

  uint8_t digest00[ndn::crypto::SHA256_DIGEST_SIZE];
  std::fill_n(digest00, sizeof(digest00), 0x00);
  uint8_t digestFF[ndn::crypto::SHA256_DIGEST_SIZE];
  std::fill_n(digestFF, sizeof(digestFF), 0xFF);

  Exclude excludeDigest;
  excludeDigest.excludeRange(
    name::Component::fromImplicitSha256Digest(digest00, sizeof(digest00)),
    name::Component::fromImplicitSha256Digest(digestFF, sizeof(digestFF)));

  startInterest("/A")
    .setChildSelector(0)
    .setExclude(excludeDigest);
  CHECK_CS_FIND(3);

  startInterest("/A")
    .setChildSelector(1)
    .setExclude(excludeDigest);
  CHECK_CS_FIND(3);

  Exclude excludeGeneric;
  excludeGeneric.excludeAfter(name::Component(static_cast<uint8_t*>(nullptr), 0));

  startInterest("/A")
    .setChildSelector(0)
    .setExclude(excludeGeneric);
  find([] (uint32_t found) { BOOST_CHECK(found == 1 || found == 2); });

  startInterest("/A")
    .setChildSelector(1)
    .setExclude(excludeGeneric);
  find([] (uint32_t found) { BOOST_CHECK(found == 1 || found == 2); });

  Exclude exclude2 = excludeGeneric;
  exclude2.excludeOne(n2.get(-1));

  startInterest("/A")
    .setChildSelector(0)
    .setExclude(exclude2);
  CHECK_CS_FIND(1);

  startInterest("/A")
    .setChildSelector(1)
    .setExclude(exclude2);
  CHECK_CS_FIND(1);
}

BOOST_AUTO_TEST_SUITE_END() // Find

// When the capacity limit is set to zero, Data cannot be inserted;
// this test case covers this situation.
// The behavior of non-zero capacity limit depends on the eviction policy,
// and is tested in policy test suites.
BOOST_FIXTURE_TEST_CASE(ZeroCapacity, FindFixture)
{
  m_cs.setLimit(0);

  BOOST_CHECK_EQUAL(m_cs.getLimit(), 0);
  BOOST_CHECK_EQUAL(m_cs.size(), 0);
  BOOST_CHECK(m_cs.begin() == m_cs.end());

  insert(1, "/A");
  BOOST_CHECK_EQUAL(m_cs.size(), 0);

  startInterest("/A");
  CHECK_CS_FIND(0);
}

BOOST_FIXTURE_TEST_CASE(CachePolicyNoCache, FindFixture)
{
  insert(1, "/A", [] (Data& data) {
    data.setTag(make_shared<lp::CachePolicyTag>(
      lp::CachePolicy().setPolicy(lp::CachePolicyType::NO_CACHE)));
  });

  startInterest("/A");
  CHECK_CS_FIND(0);
}

BOOST_AUTO_TEST_CASE(Enumeration)
{
  Cs cs;

  Name nameA("/A");
  Name nameAB("/A/B");
  Name nameABC("/A/B/C");
  Name nameD("/D");

  BOOST_CHECK_EQUAL(cs.size(), 0);
  BOOST_CHECK(cs.begin() == cs.end());

  cs.insert(*makeData(nameABC));
  BOOST_CHECK_EQUAL(cs.size(), 1);
  BOOST_CHECK(cs.begin() != cs.end());
  BOOST_CHECK(cs.begin()->getName() == nameABC);
  BOOST_CHECK((*cs.begin()).getName() == nameABC);

  auto i = cs.begin();
  auto j = cs.begin();
  BOOST_CHECK(++i == cs.end());
  BOOST_CHECK(j++ == cs.begin());
  BOOST_CHECK(j == cs.end());

  cs.insert(*makeData(nameA));
  cs.insert(*makeData(nameAB));
  cs.insert(*makeData(nameD));

  std::set<Name> expected = {nameA, nameAB, nameABC, nameD};
  std::set<Name> actual;
  for (const auto& csEntry : cs) {
    actual.insert(csEntry.getName());
  }
  BOOST_CHECK_EQUAL_COLLECTIONS(actual.begin(), actual.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_SUITE_END() // TestCs
BOOST_AUTO_TEST_SUITE_END() // Table

} // namespace tests
} // namespace cs
} // namespace nfd
