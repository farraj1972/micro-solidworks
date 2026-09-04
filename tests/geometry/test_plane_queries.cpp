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

TEST(PlaneQueries, ContainsToleranceAndDomain) {
 const auto a=Plane{Point3{},Vector3{1,0,0}};
 EXPECT_TRUE(a.contains(Point3{},0));

 EXPECT_TRUE(a.contains(Point3{0,4,3},0));
 EXPECT_TRUE(a.contains(Point3{5e-10,0,0}));
 EXPECT_FALSE(a.contains(Point3{2e-9,0,0}));
 EXPECT_FALSE(a.contains(Point3{5e-10,0,0},0));
 EXPECT_TRUE(a.contains(Point3{0.25,0,0},0.25));
 EXPECT_FALSE(a.contains(Point3{0.25,0,0},0.24));
 EXPECT_EQ(a.contains(Point3{5e-10,0,0}),(Plane{Point3{},Vector3{-1,0,0}}).contains(Point3{5e-10,0,0}));

 EXPECT_FALSE(microsw::math::isNearlyZero(5e-10));
 for(Scalar t:{-1.0,std::numeric_limits<Scalar>::quiet_NaN(),std::numeric_limits<Scalar>::infinity(),-std::numeric_limits<Scalar>::infinity()})
  EXPECT_THROW((void)a.contains(Point3{},t),std::invalid_argument);
}
TEST(PlaneQueries, SymmetricRelationsAndTolerance) {
 const auto a=Plane{Point3{},Vector3{1,0,0}}, opposite=Plane{Point3{},Vector3{-1,0,0}}, ortho=Plane{Point3{},Vector3{0,1,0}};
 EXPECT_TRUE(isParallel(a,a,0));
 EXPECT_TRUE(isParallel(a,opposite));
 EXPECT_TRUE(isParallel(opposite,a));
 EXPECT_FALSE(isParallel(a,ortho));
 EXPECT_TRUE(isPerpendicular(a,ortho,0));
 EXPECT_TRUE(isPerpendicular(ortho,opposite));
 EXPECT_FALSE(isPerpendicular(a,a));
 const auto nearP=Plane{Point3{},Vector3{1,5e-10,0}}, farP=Plane{Point3{},Vector3{1,2e-9,0}};
 const auto nearO=Plane{Point3{},Vector3{5e-10,1,0}}, farO=Plane{Point3{},Vector3{2e-9,1,0}};
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
TEST(PlaneQueries, ExtremeOffsetPreservesSmallResidual) {
 const auto m=std::numeric_limits<Scalar>::max(), tiny=std::numeric_limits<Scalar>::denorm_min();
 const Plane a{Point3{-m,0,0},Vector3{0,1,0}};
 EXPECT_TRUE(a.contains(Point3{m,0,0},0));
 EXPECT_FALSE(a.contains(Point3{m,tiny,0},0));
 EXPECT_TRUE(a.contains(Point3{m,5e-10,0}));
 EXPECT_FALSE(a.contains(Point3{m,2e-9,0}));
}
}
