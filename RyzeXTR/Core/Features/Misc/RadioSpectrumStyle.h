#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace radio_visualizer {
    constexpr int MaxSpectrumBands = 2048;
    constexpr float SpectrumWidth = 1920.f, SpectrumHeight = 1080.f;
    constexpr float SpectrumPi = 3.14159265358979323846f;
    struct SpectrumStyle {
        float start[2]{0.f, 540.f}, end[2]{960.f, 540.f};
        bool polarPath = false;
        float startHz = 20.f, endHz = 140.f;
        int bands = 1000;
        float maximumHeight = 800.f, durationMs = 90.f, offsetMs = 0.f;
        float thickness = 4.f, softness = 0.f;
        float inside[4]{1.f, 0.f, 1.f, 1.f};
        float outside[4]{1.f, 50.f/255.f, 150.f/255.f, 1.f};
        bool blendOverlap = false;
        float hueDegrees = 0.f;
        bool dynamicHue = true, colorSymmetry = true;
        int display = 0, side = 1; // Digital / Analog lines / Analog dots; A / B / A & B.
        bool durationAverage = false, compositeOriginal = false;
        float polarAmount = 100.f;
        int conversion = 0; // Rect to polar / Polar to rect.
        bool mirror = true;
        float reflectionCenter[2]{960.f, 540.f}, reflectionAngle = 0.f;
    };
    inline float SpectrumClamp(float value, float lo, float hi, float fallback) {
        return std::isfinite(value) ? std::clamp(value,lo,hi) : fallback;
    }
    inline void SanitizeSpectrum(SpectrumStyle& s) {
        const SpectrumStyle d;
        for(int i=0;i<2;++i) {
            const float extent=i==0 ? SpectrumWidth : SpectrumHeight;
            s.start[i]=SpectrumClamp(s.start[i],0,extent,d.start[i]);
            s.end[i]=SpectrumClamp(s.end[i],0,extent,d.end[i]);
            s.reflectionCenter[i]=SpectrumClamp(s.reflectionCenter[i],0,extent,d.reflectionCenter[i]);
        }
        s.startHz=SpectrumClamp(s.startHz,1,21999,d.startHz);
        s.endHz=SpectrumClamp(s.endHz,s.startHz+1,22000,(std::max)(d.endHz,s.startHz+1));
        s.bands=std::clamp(s.bands,2,MaxSpectrumBands);
        s.maximumHeight=SpectrumClamp(s.maximumHeight,0,2000,d.maximumHeight);
        s.durationMs=SpectrumClamp(s.durationMs,5,1000,d.durationMs);
        // A live stream cannot supply future audio. Negative offsets select history.
        s.offsetMs=SpectrumClamp(s.offsetMs,-1000,0,d.offsetMs);
        s.thickness=SpectrumClamp(s.thickness,.1f,32,d.thickness);
        s.softness=SpectrumClamp(s.softness,0,100,d.softness);
        s.hueDegrees=SpectrumClamp(s.hueDegrees,-3600,3600,d.hueDegrees);
        s.display=std::clamp(s.display,0,2); s.side=std::clamp(s.side,0,2);
        s.polarAmount=SpectrumClamp(s.polarAmount,0,100,d.polarAmount);
        s.conversion=std::clamp(s.conversion,0,1);
        s.reflectionAngle=SpectrumClamp(s.reflectionAngle,-180,180,d.reflectionAngle);
        for(int c=0;c<4;++c) {
            s.inside[c]=SpectrumClamp(s.inside[c],0,1,d.inside[c]);
            s.outside[c]=SpectrumClamp(s.outside[c],0,1,d.outside[c]);
        }
    }
    struct SpectrumPoint {
        float x{},y{};
        SpectrumPoint operator+(SpectrumPoint b) const { return {x+b.x,y+b.y}; }
        SpectrumPoint operator-(SpectrumPoint b) const { return {x-b.x,y-b.y}; }
        SpectrumPoint operator*(float k) const { return {x*k,y*k}; }
    };
    inline SpectrumPoint SpectrumTransform(SpectrumPoint p,const SpectrumStyle& s) {
        const SpectrumPoint center{SpectrumWidth*.5f,SpectrumHeight*.5f};
        SpectrumPoint converted;
        if(s.conversion==0) {
            const float angle=SpectrumPi*.5f+p.x/SpectrumWidth*2.f*SpectrumPi;
            converted=center+SpectrumPoint{std::cos(angle),std::sin(angle)}*(p.y*.5f);
        } else {
            const auto d=p-center;
            float angle=std::atan2(d.y,d.x)-SpectrumPi*.5f;
            if(angle<0) angle+=2.f*SpectrumPi;
            converted={angle/(2.f*SpectrumPi)*SpectrumWidth,2.f*std::hypot(d.x,d.y)};
        }
        return p+(converted-p)*(s.polarAmount*.01f);
    }
    inline SpectrumPoint SpectrumSource(const SpectrumStyle& s,float t,float amplitude,int side,float across=0.f) {
        const SpectrumPoint begin{s.start[0],s.start[1]},finish{s.end[0],s.end[1]};
        if(s.polarPath) {
            const float angle=t*2.f*SpectrumPi-SpectrumPi*.5f;
            const SpectrumPoint dir{std::cos(angle),std::sin(angle)},tangent{-dir.y,dir.x};
            return begin+dir*(SpectrumHeight*.25f+side*amplitude)+tangent*across;
        }
        const auto delta=finish-begin; const float length=std::hypot(delta.x,delta.y);
        const SpectrumPoint tangent=length>.001f ? delta*(1.f/length) : SpectrumPoint{1,0};
        const SpectrumPoint normal{-tangent.y,tangent.x};
        return begin+delta*t+normal*(side*amplitude)+tangent*across;
    }
    inline SpectrumPoint ReflectionNormal(const SpectrumStyle& s) {
        const float angle=s.reflectionAngle*SpectrumPi/180.f;
        return {std::cos(angle),std::sin(angle)};
    }
    inline float ReflectionDistance(SpectrumPoint p,const SpectrumStyle& s) {
        const auto n=ReflectionNormal(s);
        return (p.x-s.reflectionCenter[0])*n.x+(p.y-s.reflectionCenter[1])*n.y;
    }
    inline SpectrumPoint ReflectSpectrum(SpectrumPoint p,const SpectrumStyle& s) {
        return p-ReflectionNormal(s)*(2.f*ReflectionDistance(p,s));
    }
    inline std::array<float,4> SpectrumColor(const SpectrumStyle& s,float frequency,float height,float dominant) {
        std::array<float,4> color;
        for(int c=0;c<4;++c) color[c]=s.inside[c]+(s.outside[c]-s.inside[c])*height;
        if(s.hueDegrees==0.f) return color;
        const float hi=(std::max)({color[0],color[1],color[2]}),lo=(std::min)({color[0],color[1],color[2]}),chroma=hi-lo;
        float hue=0;
        if(chroma>.00001f) {
            if(hi==color[0]) hue=(color[1]-color[2])/chroma;
            else if(hi==color[1]) hue=2.f+(color[2]-color[0])/chroma;
            else hue=4.f+(color[0]-color[1])/chroma;
            hue/=6.f;
        }
        const auto coordinate=[&](float t){return s.colorSymmetry ? 1.f-std::fabs(2.f*t-1.f) : t;};
        hue+=(coordinate(frequency)-(s.dynamicHue ? coordinate(dominant) : 0.f))*s.hueDegrees/360.f;
        hue-=std::floor(hue);
        const float sector=hue*6.f,x=chroma*(1.f-std::fabs(std::fmod(sector,2.f)-1.f));
        const float m=hi-chroma;
        const std::array<std::array<float,3>,6> rgb{{{chroma,x,0},{x,chroma,0},{0,chroma,x},{0,x,chroma},{x,0,chroma},{chroma,0,x}}};
        for(int c=0;c<3;++c) color[c]=rgb[(std::min)(5,int(sector))][c]+m;
        return color;
    }
}
