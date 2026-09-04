#include "core/geometry/Segment2.h"
#include "core/geometry/Segment3.h"
#include "core/geometry/Line2.h"
#include "core/geometry/Line3.h"
#include "core/geometry/Ray2.h"
#include "core/geometry/Ray3.h"
#include "core/geometry/Plane.h"
#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace {
using namespace microsw::geometry;
using microsw::math::Vector2;
using microsw::math::Vector3;
using microsw::math::Scalar;

TEST(Segment3Queries, ContainsToleranceAndDomain) {
 const auto a=Segment3{Point3{},Point3{1,0,0}};
 EXPECT_TRUE(a.contains(Point3{},0));

 EXPECT_TRUE(a.contains(Point3{0.5,0,0},0));
 EXPECT_TRUE(a.contains(Point3{1,0,0},0));
 EXPECT_TRUE(a.contains(Point3{0.5,5e-10,0}));
 EXPECT_FALSE(a.contains(Point3{0.5,2e-9,0}));
 EXPECT_FALSE(a.contains(Point3{0.5,5e-10,0},0));
 EXPECT_TRUE(a.contains(Point3{0.5,0.25,0},0.25));
 EXPECT_FALSE(a.contains(Point3{0.5,0.25,0},0.24));
 EXPECT_FALSE(a.contains(Point3{-1,0,0}));
 EXPECT_FALSE(a.contains(Point3{10,0,0}));
 EXPECT_TRUE(a.contains(Point3{-5e-10,0,0}));
 EXPECT_FALSE(a.contains(Point3{-5e-10,0,0},0));
 EXPECT_TRUE(a.contains(Point3{1+5e-10,0,0}));

 EXPECT_FALSE(microsw::math::isNearlyZero(5e-10));
 for(Scalar t:{-1.0,std::numeric_limits<Scalar>::quiet_NaN(),std::numeric_limits<Scalar>::infinity(),-std::numeric_limits<Scalar>::infinity()})
  EXPECT_THROW((void)a.contains(Point3{},t),std::invalid_argument);
}
TEST(Segment3Queries, SymmetricRelationsAndTolerance) {
 const auto a=Segment3{Point3{},Point3{1,0,0}}, opposite=Segment3{Point3{},Point3{-1,0,0}}, ortho=Segment3{Point3{},Point3{0,1,0}};
 EXPECT_TRUE(isParallel(a,a,0));
 EXPECT_TRUE(isParallel(a,opposite));
 EXPECT_TRUE(isParallel(opposite,a));
 EXPECT_FALSE(isParallel(a,ortho));
 EXPECT_TRUE(isPerpendicular(a,ortho,0));
 EXPECT_TRUE(isPerpendicular(ortho,opposite));
 EXPECT_FALSE(isPerpendicular(a,a));
 const auto nearP=Segment3{Point3{},Point3{1,5e-10,0}}, farP=Segment3{Point3{},Point3{1,2e-9,0}};
 const auto nearO=Segment3{Point3{},Point3{5e-10,1,0}}, farO=Segment3{Point3{},Point3{2e-9,1,0}};
 EXPECT_TRUE(isParallel(a,nearP));
 EXPECT_TRUE(isParallel(nearP,a));
 EXPECT_FALSE(isParallel(a,farP));
 EXPECT_FALSE(isParallel(a,nearP,0));
 EXPECT_TRUE(isParallel(a,farP,3e-9));
 EXPECT_TRUE(isPerpendicular(a,nearO));
 EXPECT_TRUE(isPerpendicular(nearO,a));
 EXPECT_FALSE(isPerpendicular(a,farO));
 EXPECT_FALSE(isPerpendicular(a,nearO,0));
 EXPECT_TRUE(isPerpendicular(a,farO,3e-9));
 EXPECT_FALSE(microsw::math::isNearlyZero(5e-10));
 for(Scalar t:{-1.0,std::numeric_limits<Scalar>::quiet_NaN(),std::numeric_limits<Scalar>::infinity(),-std::numeric_limits<Scalar>::infinity()}) {
  EXPECT_THROW((void)isParallel(a,a,t),std::invalid_argument);
  EXPECT_THROW((void)isPerpendicular(a,ortho,t),std::invalid_argument);
 }
}
TEST(Segment3Queries, ExtremeOffsetPreservesSmallResidual) {
 const auto m=std::numeric_limits<Scalar>::max(), tiny=std::numeric_limits<Scalar>::denorm_min();
 const Segment3 a{Point3{-m,0,0},Point3{m,0,0}};
 EXPECT_TRUE(a.contains(Point3{m,0,0},0));
 EXPECT_FALSE(a.contains(Point3{m,tiny,0},0));
 EXPECT_TRUE(a.contains(Point3{m,5e-10,0}));
 EXPECT_FALSE(a.contains(Point3{m,2e-9,0}));
}

TEST(Segment3Queries, PointAtValuesAndValidation) {
 const auto a=Segment3{Point3{},Point3{4,0,0}};
 EXPECT_TRUE(areCoincident(a.pointAt(0),Point3{},0));
 EXPECT_TRUE(areCoincident(a.pointAt(1),Point3{4,0,0},0));
 EXPECT_TRUE(areCoincident(a.pointAt(0.5),Point3{2,0,0},0));
 EXPECT_TRUE(areCoincident(a.pointAt(0.25),Point3{1,0,0},0));
 EXPECT_THROW((void)a.pointAt(-0.1),std::domain_error);
 EXPECT_THROW((void)a.pointAt(1.01),std::domain_error);
 for(Scalar t:{std::numeric_limits<Scalar>::quiet_NaN(),std::numeric_limits<Scalar>::infinity(),-std::numeric_limits<Scalar>::infinity()})
  EXPECT_THROW((void)a.pointAt(t),std::invalid_argument);
 const auto diagonal=Segment3{Point3{},Point3{3,4,2}};
 EXPECT_TRUE(diagonal.contains(diagonal.pointAt(0.37)));
}
TEST(Segment3Queries, ExtremePointAt) {
 const auto m=std::numeric_limits<Scalar>::max();

 const Segment3 a{Point3{-m,0,0},Point3{m,0,0}};
 EXPECT_TRUE(areCoincident(a.pointAt(0),a.a(),0));
 EXPECT_TRUE(areCoincident(a.pointAt(1),a.b(),0));
 EXPECT_TRUE(areCoincident(a.pointAt(0.5),Point3{},0));
 EXPECT_TRUE(std::isfinite(a.pointAt(0.25).x()));
 const Segment3 zero{Point3{m,0,0},Point3{m,0,0}};
 EXPECT_TRUE(areCoincident(zero.pointAt(0.37),zero.a(),0));

}

TEST(Segment3Queries, DegenerateAndShortSegments) {
 const Segment3 zero{};
 const auto a=Segment3{Point3{},Point3{1,0,0}}, shortSegment=Segment3{Point3{},Point3{5e-10,0,0}};
 EXPECT_TRUE(zero.contains(Point3{}));
 EXPECT_TRUE(zero.contains(Point3{5e-10,0,0}));
 EXPECT_FALSE(zero.contains(Point3{2e-9,0,0}));
 EXPECT_THROW((void)isParallel(zero,a),std::domain_error);
 EXPECT_THROW((void)isParallel(a,zero),std::domain_error);
 EXPECT_THROW((void)isPerpendicular(zero,a),std::domain_error);
 EXPECT_THROW((void)isPerpendicular(a,zero),std::domain_error);
 EXPECT_THROW((void)isParallel(shortSegment,a,0),std::domain_error);
 EXPECT_TRUE(shortSegment.contains(Point3{2.5e-10,0,0},0));
 EXPECT_FALSE(shortSegment.contains(Point3{1e-9,0,0},0));
 EXPECT_FALSE(shortSegment.contains(Point3{2.5e-10,1e-20,0},0));
}
}
