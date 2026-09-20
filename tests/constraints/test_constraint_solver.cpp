#include "constraints/ConstraintSolver.h"
#include "core/geometry/GeometricTolerance.h"
#include "presentation/SketchPresentation.h"
#include <gtest/gtest.h>
#include <cmath>
namespace {
using namespace microsw;
using sketch::SketchElementRef; using sketch::SubElementKind;
SketchElementRef start(sketch::SketchEntityId id){return{id,SubElementKind::LineStart};}
SketchElementRef end(sketch::SketchEntityId id){return{id,SubElementKind::LineEnd};}
SketchElementRef body(sketch::SketchEntityId id){return{id,SubElementKind::LineBody};}
const geometry::Segment2& line(const sketch::Sketch&s,sketch::SketchEntityId id){return std::get<sketch::SketchLine>(s.find(id).geometry()).geometry;}

struct Rectangle { sketch::Sketch sketch; std::array<sketch::SketchEntityId,4> lines; sketch::SketchConstraintId width,height; };
Rectangle rectangle(){Rectangle r; r.lines=r.sketch.addRectangle({0,0},{10,5});auto&a=r.sketch;
 a.addConstraint(sketch::Coincident{end(r.lines[0]),start(r.lines[1])});a.addConstraint(sketch::Coincident{end(r.lines[1]),start(r.lines[2])});a.addConstraint(sketch::Coincident{end(r.lines[2]),start(r.lines[3])});a.addConstraint(sketch::Coincident{end(r.lines[3]),start(r.lines[0])});
 a.addConstraint(sketch::Horizontal{body(r.lines[0])});a.addConstraint(sketch::Horizontal{body(r.lines[2])});a.addConstraint(sketch::Vertical{body(r.lines[1])});a.addConstraint(sketch::Vertical{body(r.lines[3])});
 r.width=a.addConstraint(sketch::HorizontalDistance{start(r.lines[0]),end(r.lines[0]),10});r.height=a.addConstraint(sketch::VerticalDistance{start(r.lines[1]),end(r.lines[1]),5});return r;}

TEST(ConstraintSolver, SolvesRectangleAndDimensionEditsPreservingIdentity){auto r=rectangle();constraints::ConstraintSolver solver;EXPECT_EQ(solver.solve(r.sketch).status,constraints::SolveStatus::Solved);r.sketch.setDrivingValue(r.width,14);r.sketch.setDrivingValue(r.height,8);auto result=solver.solve(r.sketch);EXPECT_EQ(result.status,constraints::SolveStatus::Solved);EXPECT_NEAR(line(r.sketch,r.lines[0]).length(),14,1e-6);EXPECT_NEAR(line(r.sketch,r.lines[1]).length(),8,1e-6);for(auto id:r.lines)EXPECT_EQ(r.sketch.find(id).id(),id);EXPECT_TRUE(r.sketch.containsConstraint(r.width));EXPECT_TRUE(r.sketch.containsConstraint(r.height));}

TEST(ConstraintSolver, CircleRadiusSolveKeepsCenterStable){sketch::Sketch s;auto id=s.addCircle({{3,4},2});auto c=s.addConstraint(sketch::CircleRadius{{id,SubElementKind::CircleRadius},7});constraints::ConstraintSolver solver;auto result=solver.solve(s);ASSERT_EQ(result.status,constraints::SolveStatus::Solved);auto&circle=std::get<sketch::SketchCircle>(s.find(id).geometry()).geometry;EXPECT_NEAR(circle.radius(),7,1e-7);EXPECT_NEAR(circle.center().x(),3,1e-9);EXPECT_NEAR(circle.center().y(),4,1e-9);EXPECT_EQ(s.findConstraint(c).id(),c);}

TEST(ConstraintSolver, SolvesRectangleWhenEarlierEntityIdsBelongToAnotherComponent){sketch::Sketch s;auto circle=s.addCircle({{20,20},2});s.addConstraint(sketch::CircleRadius{{circle,SubElementKind::CircleRadius},3});auto lines=s.addRectangle({0,0},{10,5});s.addConstraint(sketch::Coincident{end(lines[0]),start(lines[1])});s.addConstraint(sketch::Coincident{end(lines[1]),start(lines[2])});s.addConstraint(sketch::Coincident{end(lines[2]),start(lines[3])});s.addConstraint(sketch::Coincident{end(lines[3]),start(lines[0])});s.addConstraint(sketch::Horizontal{body(lines[0])});s.addConstraint(sketch::Horizontal{body(lines[2])});s.addConstraint(sketch::Vertical{body(lines[1])});s.addConstraint(sketch::Vertical{body(lines[3])});s.addConstraint(sketch::HorizontalDistance{start(lines[0]),end(lines[0]),14});s.addConstraint(sketch::VerticalDistance{start(lines[1]),end(lines[1]),8});constraints::ConstraintSolver solver;auto result=solver.solve(s);ASSERT_EQ(result.status,constraints::SolveStatus::Solved);EXPECT_NEAR(line(s,lines[0]).length(),14,1e-6);EXPECT_NEAR(line(s,lines[1]).length(),8,1e-6);EXPECT_NEAR(std::get<sketch::SketchCircle>(s.find(circle).geometry()).geometry.radius(),3,1e-6);}

TEST(ConstraintSolver, SupportsParallelPerpendicularAndLineLength){sketch::Sketch s;auto a=s.addLine({{0,0},{3,0}}),b=s.addLine({{0,1},{2,1}}),c=s.addLine({{0,0},{0,2}});s.addConstraint(sketch::Parallel{body(a),body(b)});s.addConstraint(sketch::Perpendicular{body(a),body(c)});s.addConstraint(sketch::LineLength{body(a),5});constraints::ConstraintSolver solver;auto result=solver.solve(s);EXPECT_EQ(result.status,constraints::SolveStatus::Solved);EXPECT_NEAR(line(s,a).length(),5,1e-6);EXPECT_TRUE(geometry::isParallel(line(s,a),line(s,b),1e-6));EXPECT_TRUE(geometry::isPerpendicular(line(s,a),line(s,c),1e-6));}

TEST(ConstraintSolver, UnsatisfiedSystemDoesNotMutateSketch){sketch::Sketch s;auto id=s.addLine({{0,0},{5,0}});s.addConstraint(sketch::HorizontalDistance{start(id),end(id),5});s.addConstraint(sketch::HorizontalDistance{start(id),end(id),10});auto before=line(s,id);constraints::ConstraintSolver solver;auto result=solver.solve(s);EXPECT_EQ(result.status,constraints::SolveStatus::Unsatisfied);EXPECT_TRUE(geometry::areCoincident(line(s,id).a(),before.a(),0));EXPECT_TRUE(geometry::areCoincident(line(s,id).b(),before.b(),0));}

TEST(ConstraintSolver, DegenerateDirectionalInputIsRejectedWithoutMutation){sketch::Sketch s;auto id=s.addLine({{1,1},{1,1}});s.addConstraint(sketch::Horizontal{body(id)});constraints::ConstraintSolver solver;auto result=solver.solve(s);EXPECT_EQ(result.status,constraints::SolveStatus::InvalidInput);EXPECT_TRUE(line(s,id).isDegenerate());}

TEST(ConstraintSolver, IterationExhaustionIsUnsatisfiedAndTransactional){sketch::Sketch s;auto id=s.addLine({{0,0},{2,1}});s.addConstraint(sketch::Horizontal{body(id)});auto before=line(s,id);constraints::SolverPolicy p;p.maxIterations=1;p.residualSatisfactionTolerance=1e-16;constraints::ConstraintSolver solver{p};auto result=solver.solve(s);EXPECT_NE(result.status,constraints::SolveStatus::Solved);EXPECT_TRUE(geometry::areCoincident(line(s,id).a(),before.a(),0));EXPECT_TRUE(geometry::areCoincident(line(s,id).b(),before.b(),0));}

TEST(ConstraintSolver, PresentationRegeneratesAfterCommitWithStableMapping){auto r=rectangle();presentation::SketchPresentation p;p.regenerate(r.sketch);auto visual=p.visualId(r.lines[0]);r.sketch.setDrivingValue(r.width,12);constraints::ConstraintSolver solver;ASSERT_EQ(solver.solve(r.sketch).status,constraints::SolveStatus::Solved);p.regenerate(r.sketch);EXPECT_EQ(p.visualId(r.lines[0]),visual);EXPECT_EQ(p.sketchId(*visual),r.lines[0]);}
}
