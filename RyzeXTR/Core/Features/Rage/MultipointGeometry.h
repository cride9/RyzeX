#pragma once
#include "HitscanMath.h"

namespace multipoint {
using hitbox::Vec;
using hitbox::Shape;
using Points = hitscan::Points<Vec>;
enum class Region { Body, Head, Foot, Leg };
inline Vec Cross(Vec a, Vec b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
inline Vec Unit(Vec value, Vec fallback = {1,0,0}) {
    const double square=value.LengthSq();
    return value.Valid() && square>1e-16 ? value/std::sqrt(square) : fallback;
}
inline bool Valid(const Shape& shape) {
    if (!shape.valid || !shape.a.Valid() || !shape.Center().Valid()) return false;
    if (shape.capsule) return shape.b.Valid() && std::isfinite(shape.radius) && shape.radius>=0.;
    for (int i=0;i<3;++i) if (!shape.axes[i].Valid() || std::fabs(shape.axes[i].LengthSq()-1.)>.001
        || !std::isfinite(shape.extents[i]) || shape.extents[i]<0.) return false;
    return std::fabs(shape.axes[0].Dot(shape.axes[1]))<.001 && std::fabs(shape.axes[0].Dot(shape.axes[2]))<.001
        && std::fabs(shape.axes[1].Dot(shape.axes[2]))<.001;
}
inline void Add(Points& points, const Shape& shape, Vec point) {
    if (!point.Valid() || !shape.Contains(point)) return;
    for (const auto old:points) if ((point-old).LengthSq()<1e-6) return;
    points.push_back(point);
}
inline Vec Support(const Shape& shape, Vec direction, double scale) {
    const Vec center=shape.Center(), normal=Unit(direction);
    if (shape.capsule) {
        const double projection=(shape.b-shape.a).Dot(normal);
        const Vec end=projection>1e-8 ? shape.b : projection< -1e-8 ? shape.a : center;
        return center+(end-center+normal*shape.radius)*scale;
    }
    Vec offset{};
    for (int i=0;i<3;++i) {
        const double projection=shape.axes[i].Dot(normal);
        offset=offset+shape.axes[i]*(shape.extents[i]*(projection>1e-8 ? 1. : projection< -1e-8 ? -1. : 0.));
    }
    return center+offset*scale;
}
// RenderCapsule describes two end spheres joined along the transformed
// segment. Sample that analytic volume; its display tessellation is not a
// collision mesh. Shrink about the center to retain an interior margin.
inline Points Generate(const Shape& shape, Vec eye, Region region, double percent) {
    Points points;
    if (!Valid(shape) || !eye.Valid() || !std::isfinite(percent)) return points;
    const Vec center=shape.Center(); Add(points,shape,center);
    const double scale=std::clamp(percent*.01,0.,.98);
    if (scale<=0.) return points;
    const Vec toward=Unit(eye-center);
    const Vec axis=shape.capsule ? Unit(shape.b-shape.a,Unit(shape.axes[2],{0,0,1})) : Unit(shape.axes[2],{0,0,1});
    Vec side=Cross(axis,toward);
    if (side.LengthSq()<1e-12) side=Cross(axis,std::fabs(axis.z)<.8 ? Vec{0,0,1} : Vec{1,0,0});
    side=Unit(side); const Vec front=Unit(Cross(side,axis));
    if (region==Region::Head) Add(points,shape,Support(shape,{0,0,1},scale));
    Add(points,shape,Support(shape,side,scale)); Add(points,shape,Support(shape,side*-1.,scale));
    if (region==Region::Body) return points;
    Add(points,shape,Support(shape,toward,scale)); Add(points,shape,Support(shape,toward*-1.,scale));
    if (shape.capsule) {
        Add(points,shape,Support(shape,axis,scale)); Add(points,shape,Support(shape,axis*-1.,scale));
        Add(points,shape,center+(shape.a-center)*scale); Add(points,shape,center+(shape.b-center)*scale);
        constexpr double diagonal=.7071067811865475244;
        constexpr std::array<std::array<double,2>,8> ring{{{1,0},{-1,0},{0,1},{0,-1},
            {diagonal,diagonal},{-diagonal,diagonal},{diagonal,-diagonal},{-diagonal,-diagonal}}};
        for (double fraction:{.22,.78}) {
            const Vec anchor=center+(shape.a+(shape.b-shape.a)*fraction-center)*scale;
            const int count=region==Region::Leg ? 4 : 8;
            for (int i=0;i<count;++i) Add(points,shape,anchor+(side*ring[i][0]+front*ring[i][1])*(shape.radius*scale));
        }
    } else {
        int vertical=0;
        for (int i=1;i<3;++i) if (std::fabs(shape.axes[i].z)>std::fabs(shape.axes[vertical].z)) vertical=i;
        int length=(vertical+1)%3, width=(vertical+2)%3;
        if (shape.extents[width]>shape.extents[length]) std::swap(length,width);
        const double up=shape.axes[vertical].z<0. ? -1. : 1.;
        const auto boxPoint=[&](double x,double y,double z) {
            return center+(shape.axes[length]*(shape.extents[length]*x)+shape.axes[width]*(shape.extents[width]*y)
                +shape.axes[vertical]*(shape.extents[vertical]*z*up))*scale;
        };
        // Heel/toe and top edges precede the lower corners in the scan order.
        for (double x:{-1.,1.}) Add(points,shape,boxPoint(x,0,.65));
        for (double x:{-1.,1.}) for (double y:{-1.,1.}) Add(points,shape,boxPoint(x,y,1.));
        for (int i=0;i<3;++i) for (double sign:{-1.,1.})
            Add(points,shape,center+shape.axes[i]*(shape.extents[i]*sign*scale));
        for (double x:{-1.,1.}) Add(points,shape,boxPoint(x,0,1.));
        for (double y:{-1.,1.}) Add(points,shape,boxPoint(0,y,1.));
        for (double x:{-1.,1.}) for (double y:{-1.,1.}) Add(points,shape,boxPoint(x,y,-1.));
    }
    return points;
}
}
