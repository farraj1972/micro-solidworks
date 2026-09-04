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

TEST(Ray2Queries, ContainsToleranceAndDomain) {
 const auto a=Ray2{Point2{},Vector2{1,0}};
 EXPECT_TRUE(a.contains(Point2{},0));

 EXPECT_TRUE(a.contains(Point2{0.5,0},0));
 EXPECT_TRUE(a.contains(Point2{1,0},0));
 EXPECT_TRUE(a.contains(Point2{0.5,5e-10}));
 EXPECT_FALSE(a.contains(Point2{0.5,2e-9}));
 EXPECT_FALSE(a.contains(Point2{0.5,5e-10},0));
 EXPECT_TRUE(a.contains(Point2{0.5,0.25},0.25));
 EXPECT_FALSE(a.contains(Point2{0.5,0.25},0.24));
 EXPECT_FALSE(a.contains(Point2{-1,0}));
 EXPECT_TRUE(a.contains(Point2{10,0}));
 EXPECT_TRUE(a.contains(Point2{-5e-10,0}));
 EXPECT_FALSE(a.contains(Point2{-5e-10,0},0));


 EXPECT_FALSE(microsw::math::isNearlyZero(5e-10));
 for(Scalar t:{-1.0,std::numeric_limits<Scalar>::quiet_NaN(),std::numeric_limits<Scalar>::infinity(),-std::numeric_limits<Scalar>::infinity()})
  EXPECT_THROW((void)a.contains(Point2{},t),std::invalid_argument);
}
TEST(Ray2Queries, SymmetricRelationsAndTolerance) {
 const auto a=Ray2{Point2{},Vector2{1,0}}, opposite=Ray2{Point2{},Vector2{-1,0}}, ortho=Ray2{Point2{},Vector2{0,1}};
 EXPECT_TRUE(isParallel(a,a,0));
 EXPECT_TRUE(isParallel(a,opposite));
 EXPECT_TRUE(isParallel(opposite,a));
 EXPECT_FALSE(isParallel(a,ortho));
 EXPECT_TRUE(isPerpendicular(a,ortho,0));
 EXPECT_TRUE(isPerpendicular(ortho,opposite));
 EXPECT_FALSE(isPerpendicular(a,a));
 const auto nearP=Ray2{Point2{},Vector2{1,5e-10}}, farP=Ray2{Point2{},Vector2{1,2e-9}};
 const auto nearO=Ray2{Point2{},Vector2{5e-10,1}}, farO=Ray2{Point2{},Vector2{2e-9,1}};
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
TEST(Ray2Queries, ExtremeOffsetPreservesSmallResidual) {
 const auto m=std::numeric_limits<Scalar>::max(), tiny=std::numeric_limits<Scalar>::denorm_min();
 const Ray2 a{Point2{-m,0},Vector2{1,0}};
 EXPECT_TRUE(a.contains(Point2{m,0},0));
 EXPECT_FALSE(a.contains(Point2{m,tiny},0));
 EXPECT_TRUE(a.contains(Point2{m,5e-10}));
 EXPECT_FALSE(a.contains(Point2{m,2e-9}));
}

TEST(Ray2Queries, PointAtValuesAndValidation) {
 const auto a=Ray2{Point2{},Vector2{1,0}};
 EXPECT_TRUE(areCoincident(a.pointAt(0),Point2{},0));
 EXPECT_TRUE(areCoincident(a.pointAt(1),Point2{1,0},0));
 EXPECT_TRUE(areCoincident(a.pointAt(0.5),Point2{0.5,0},0));
 EXPECT_TRUE(areCoincident(a.pointAt(0.25),Point2{0.25,0},0));
 EXPECT_THROW((void)a.pointAt(-0.1),std::domain_error);

 for(Scalar t:{std::numeric_limits<Scalar>::quiet_NaN(),std::numeric_limits<Scalar>::infinity(),-std::numeric_limits<Scalar>::infinity()})
  EXPECT_THROW((void)a.pointAt(t),std::invalid_argument);
 const auto diagonal=Ray2{Point2{},Vector2{3,4}};
 EXPECT_TRUE(diagonal.contains(diagonal.pointAt(0.37)));
}
TEST(Ray2Queries, ExtremePointAt) {
 const auto m=std::numeric_limits<Scalar>::max();

 const Ray2 a{Point2{m,0},Vector2{1,0}};
 EXPECT_TRUE(areCoincident(a.pointAt(0),a.origin(),0));
 EXPECT_THROW((void)a.pointAt(m),std::overflow_error);

}
}
