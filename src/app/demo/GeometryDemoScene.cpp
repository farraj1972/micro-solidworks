#include "app/demo/GeometryDemoScene.h"

namespace microsw::demo
{
presentation::GeometryPresentation createGeometryDemoScene()
{
    presentation::GeometryPresentation presentation;
    (void)presentation.add(geometry::Point3{0, 0, 0});
    (void)presentation.add(geometry::Point3{2, 2, 1});
    (void)presentation.add(geometry::Point3{-2, 1, 2});
    (void)presentation.add(geometry::Segment3{
        geometry::Point3{-3, -2, 0.5},
        geometry::Point3{3, -2, 0.5}});
    (void)presentation.add(geometry::Segment3{
        geometry::Point3{-3, 2, 0.5},
        geometry::Point3{-1, 4, 2.5}});
    (void)presentation.add(geometry::Segment3{
        geometry::Point3{3, 1, 0.5},
        geometry::Point3{3, 2, 3.5}});
    (void)presentation.add(geometry::Line3{
        geometry::Point3{-2, 3, 1},
        math::Vector3{1, 1, 0.25}});
    (void)presentation.add(geometry::Line3{
        geometry::Point3{2, -1, 2},
        math::Vector3{-0.5, 1, 1.5}});
    return presentation;
}
}
