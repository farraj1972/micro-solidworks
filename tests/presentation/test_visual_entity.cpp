#include "presentation/VisualEntity.h"
#include "presentation/WorldGeometry.h"
#include "core/math/Transformations.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
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
using microsw::math::Transform3;
using microsw::math::almostEqual;
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
static_assert(std::is_same_v<decltype(std::declval<VisualEntity&>().transform()), const Transform3&>);
static_assert(noexcept(std::declval<const VisualEntity&>().transform()));

void expectTransform(const Transform3& actual, const Transform3& expected)
{
    EXPECT_TRUE(almostEqual(actual.translation(), expected.translation()));
    EXPECT_TRUE(almostEqual(actual.rotation(), expected.rotation()));
    EXPECT_TRUE(almostEqual(actual.scale(), expected.scale()));
}

TEST(VisualEntity, DefaultsToIdentityTransformForEveryGeometryType)
{
    const PresentedGeometry geometries[]{Point3{1, 2, 3},
        Segment3{Point3{1, 2, 3}, Point3{4, 5, 6}},
        Line3{Point3{3, 2, 1}, Vector3{1, 2, 3}}};
    for (const auto& geometry : geometries)
    {
        const VisualEntity entity{VisualEntityId{1}, geometry};
        expectTransform(entity.transform(), Transform3{});
        EXPECT_TRUE(almostEqual(entity.transform().matrix(), microsw::math::Matrix4::identity()));
    }
}

TEST(VisualEntity, OwnsTransformValueAndSupportsReplacement)
{
    VisualEntity entity{VisualEntityId{1}, Point3{1, 2, 3}};
    Transform3 supplied{Vector3{4, 5, 6}, Vector3{0.1, 0.2, 0.3}, Vector3{2, 3, 4}};
    const Transform3 expected = supplied;
    entity.setTransform(supplied);
    supplied.setTranslation(Vector3{9, 8, 7});
    supplied.setRotation(Vector3{0.9, 0.8, 0.7});
    supplied.setScale(Vector3{5, 6, 7});
    expectTransform(entity.transform(), expected);

    entity.setTransform(supplied);
    expectTransform(entity.transform(), supplied);
    entity.setTransform(Transform3{});
    expectTransform(entity.transform(), Transform3{});
}

TEST(VisualEntity, TransformChangesPreserveIdentityAndAllLocalGeometryValues)
{
    const PresentedGeometry geometries[]{Point3{1, 2, 3},
        Segment3{Point3{1, 2, 3}, Point3{4, 5, 6}},
        Line3{Point3{3, 2, 1}, Vector3{1, 2, 3}}};
    const Transform3 transform{Vector3{4, 5, 6}, Vector3{0.1, 0.2, 0.3}, Vector3{2, 3, 4}};
    for (const auto& geometry : geometries)
    {
        VisualEntity entity{VisualEntityId{7}, geometry};
        entity.setTransform(transform);
        EXPECT_EQ(entity.id(), VisualEntityId{7});
        ASSERT_EQ(entity.geometry().index(), geometry.index());
        std::visit([&entity](const auto& original)
        {
            using Geometry = std::decay_t<decltype(original)>;
            const auto& actual = std::get<Geometry>(entity.geometry());
            if constexpr (std::is_same_v<Geometry, Point3>)
                EXPECT_TRUE(areCoincident(actual, original, 0));
            else if constexpr (std::is_same_v<Geometry, Segment3>)
            {
                EXPECT_TRUE(areCoincident(actual.a(), original.a(), 0));
                EXPECT_TRUE(areCoincident(actual.b(), original.b(), 0));
            }
            else
            {
                EXPECT_TRUE(areCoincident(actual.origin(), original.origin(), 0));
                EXPECT_TRUE(almostEqual(actual.direction(), original.direction(), 0, 0));
            }
        }, geometry);
    }
}

TEST(VisualEntity, CopyAndMovePreserveTransformWithoutSharingState)
{
    VisualEntity original{VisualEntityId{7}, Point3{1, 2, 3}};
    const Transform3 expected{Vector3{4, 5, 6}, Vector3{0.1, 0.2, 0.3}, Vector3{2, 3, 4}};
    original.setTransform(expected);
    auto copied = original;
    original.setTransform(Transform3{});
    expectTransform(copied.transform(), expected);
    auto moved = std::move(copied);
    expectTransform(moved.transform(), expected);
    EXPECT_EQ(moved.id(), VisualEntityId{7});

    VisualEntity assigned{VisualEntityId{8}, Point3{}};
    assigned = moved;
    moved.setTransform(Transform3{});
    expectTransform(assigned.transform(), expected);
    original = std::move(assigned);
    expectTransform(original.transform(), expected);
    EXPECT_EQ(original.id(), VisualEntityId{7});
}

TEST(VisualEntity, SeparateEntitiesMaintainIndependentTransforms)
{
    VisualEntity first{VisualEntityId{1}, Point3{1, 2, 3}};
    VisualEntity second{VisualEntityId{2}, Point3{1, 2, 3}};
    const Transform3 firstTransform{Vector3{4, 5, 6}, Vector3{0.1, 0.2, 0.3}, Vector3{2, 3, 4}};
    const Transform3 secondTransform{Vector3{-1, -2, -3}, Vector3{-0.1, -0.2, -0.3}, Vector3{4, 3, 2}};
    first.setTransform(firstTransform);
    expectTransform(second.transform(), Transform3{});
    second.setTransform(secondTransform);
    expectTransform(first.transform(), firstTransform);
    first.setTransform(Transform3{});
    expectTransform(second.transform(), secondTransform);
}

TEST(WorldGeometry, DerivesPointSegmentAndLineFromCompleteTrs)
{
    const Point3 origin{1, 2, 3};
    const Point3 end{4, 6, 8};
    const Transform3 transform{Vector3{4, -2, 1}, Vector3{0.3, -0.4, 0.5}, Vector3{2, 3, 4}};
    auto expectedPoint = [&](const Point3& point)
    {
        const auto value = microsw::math::transformPoint(transform.matrix(), {point.x(), point.y(), point.z()});
        return Point3{value.x(), value.y(), value.z()};
    };
    const PresentedGeometry values[]{origin, Segment3{origin, end}, Line3{origin, end - origin}};
    for (const auto& local : values)
    {
        VisualEntity entity{VisualEntityId{1}, local};
        entity.setTransform(transform);
        const auto world = microsw::presentation::worldGeometry(entity);
        ASSERT_EQ(world.index(), local.index());
        if (const auto* point = std::get_if<Point3>(&world))
            EXPECT_TRUE(areCoincident(*point, expectedPoint(origin)));
        else if (const auto* segment = std::get_if<Segment3>(&world))
        {
            EXPECT_TRUE(areCoincident(segment->a(), expectedPoint(origin)));
            EXPECT_TRUE(areCoincident(segment->b(), expectedPoint(end)));
        }
        else
        {
            const auto& line = std::get<Line3>(world);
            EXPECT_TRUE(areCoincident(line.origin(), expectedPoint(origin)));
            EXPECT_TRUE(line.contains(expectedPoint(end)));
            EXPECT_NEAR(line.direction().length(), 1.0, 1e-12);
        }
        EXPECT_EQ(entity.id(), VisualEntityId{1});
        expectTransform(entity.transform(), transform);
    }
}

TEST(WorldGeometry, NormalizesLineAcrossTinyAndLargePositiveScale)
{
    for (double scale : {1e-200, 1e200})
    {
        VisualEntity entity{VisualEntityId{1}, Line3{Point3{}, Vector3{1, 2, 3}}};
        entity.setTransform(Transform3{Vector3{}, Vector3{}, Vector3{scale, scale, scale}});
        const auto world = microsw::presentation::worldGeometry(entity);
        EXPECT_TRUE(almostEqual(std::get<Line3>(world).direction(),
            std::get<Line3>(entity.geometry()).direction()));
    }
}

TEST(WorldGeometry, NonRepresentableWorldValueThrowsWithoutChangingLocalGeometry)
{
    VisualEntity entity{VisualEntityId{1}, Point3{2, 0, 0}};
    entity.setTransform(Transform3{Vector3{}, Vector3{}, Vector3{std::numeric_limits<double>::max(), 1, 1}});
    EXPECT_THROW((void)microsw::presentation::worldGeometry(entity), std::overflow_error);
    EXPECT_TRUE(areCoincident(std::get<Point3>(entity.geometry()), Point3{2, 0, 0}, 0));
}

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
