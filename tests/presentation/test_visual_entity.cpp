#include "presentation/VisualEntity.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace
{
using microsw::geometry::Line3;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::areCoincident;
using microsw::math::Vector3;
using microsw::presentation::PresentedGeometry;
using microsw::presentation::VisualEntity;
using microsw::presentation::VisualEntityId;
using microsw::presentation::VisualEntityIdGenerator;

static_assert(!std::is_default_constructible_v<VisualEntityId>);
static_assert(std::is_copy_constructible_v<VisualEntityId> && std::is_move_constructible_v<VisualEntityId>);
static_assert(std::is_copy_assignable_v<VisualEntityId> && std::is_move_assignable_v<VisualEntityId>);
static_assert(std::variant_size_v<PresentedGeometry> == 3);
static_assert(std::is_same_v<std::variant_alternative_t<0, PresentedGeometry>, Point3>);
static_assert(std::is_same_v<std::variant_alternative_t<1, PresentedGeometry>, Segment3>);
static_assert(std::is_same_v<std::variant_alternative_t<2, PresentedGeometry>, Line3>);
static_assert(std::is_copy_constructible_v<VisualEntity> && std::is_move_constructible_v<VisualEntity>);
static_assert(std::is_same_v<decltype(std::declval<const VisualEntity&>().geometry()), const PresentedGeometry&>);

TEST(VisualEntityId, ValidValuesCompareAndZeroIsRejected)
{
    const VisualEntityId first{1}, same{1}, other{2};
    EXPECT_EQ(first, same);
    EXPECT_NE(first, other);
    EXPECT_EQ(first.value(), std::uint64_t{1});
    EXPECT_THROW((void)VisualEntityId{0}, std::invalid_argument);
    auto copy = first;
    auto moved = std::move(copy);
    EXPECT_EQ(moved, first);
}

TEST(VisualEntityId, GeneratorProducesUniqueMonotonicProcessLocalValues)
{
    VisualEntityIdGenerator generator;
    const auto first = generator.generate();
    const auto second = generator.generate();
    const auto third = generator.generate();
    EXPECT_EQ(first.value(), 1U);
    EXPECT_EQ(second.value(), 2U);
    EXPECT_EQ(third.value(), 3U);
    EXPECT_NE(first, second);
    EXPECT_NE(second, third);
}

TEST(VisualEntity, OwnsPointValueAndPreservesIdentity)
{
    Point3 original{1, 2, 3};
    const VisualEntity entity{VisualEntityId{7}, PresentedGeometry{original}};
    original = Point3{9, 9, 9};
    EXPECT_EQ(entity.id(), VisualEntityId{7});
    ASSERT_TRUE(std::holds_alternative<Point3>(entity.geometry()));
    EXPECT_TRUE(areCoincident(std::get<Point3>(entity.geometry()), Point3{1, 2, 3}, 0));
}

TEST(VisualEntity, OwnsSegmentAndLineValuesAcrossCopyAndMove)
{
    const Segment3 segment{Point3{1, 2, 3}, Point3{4, 5, 6}};
    const VisualEntity segmentEntity{VisualEntityId{8}, PresentedGeometry{segment}};
    auto copied = segmentEntity;
    auto moved = std::move(copied);
    ASSERT_TRUE(std::holds_alternative<Segment3>(moved.geometry()));
    EXPECT_TRUE(areCoincident(std::get<Segment3>(moved.geometry()).a(), segment.a(), 0));
    EXPECT_TRUE(areCoincident(std::get<Segment3>(moved.geometry()).b(), segment.b(), 0));

    const Line3 line{Point3{3, 2, 1}, Vector3{2, 0, 0}};
    const VisualEntity lineEntity{VisualEntityId{9}, PresentedGeometry{line}};
    ASSERT_TRUE(std::holds_alternative<Line3>(lineEntity.geometry()));
    EXPECT_TRUE(areCoincident(std::get<Line3>(lineEntity.geometry()).origin(), line.origin(), 0));
    EXPECT_EQ(std::get<Line3>(lineEntity.geometry()).direction().x(), line.direction().x());
}
}
