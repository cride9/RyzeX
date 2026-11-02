#pragma once
#include "RadioVisualizerMath.h"
#include "../../../Dependecies/ImGui/imgui.h"

namespace radio_visualizer {
    namespace spectrum_draw {
        struct Vertex { SpectrumPoint p; std::array<float,4> color; };
        inline void Polygon(ImDrawList* draw,std::array<Vertex,16> points,int count,ImVec2 origin,float scale,
            const SpectrumStyle& s,SpectrumPoint normal,float opacity) {
            const auto distance=[&](SpectrumPoint p){return (p.x-s.reflectionCenter[0])*normal.x+(p.y-s.reflectionCenter[1])*normal.y;};
            if(s.mirror) {
                std::array<Vertex,16> clipped{}; int output=0;
                for(int i=0;i<count;++i) {
                    const auto& a=points[i]; const auto& b=points[(i+1)%count];
                    const float da=distance(a.p),db=distance(b.p);
                    const bool insideA=da<=0.f,insideB=db<=0.f;
                    if(insideA) clipped[output++]=a;
                    if(insideA!=insideB) {
                        const float t=da/(da-db); Vertex cut;
                        cut.p=a.p+(b.p-a.p)*t;
                        for(int c=0;c<4;++c) cut.color[c]=a.color[c]+(b.color[c]-a.color[c])*t;
                        clipped[output++]=cut;
                    }
                }
                points=clipped;count=output;
            }
            if(count<3) return;
            const ImVec2 uv=ImGui::GetFontTexUvWhitePixel();
            for(int reflection=0;reflection<(s.mirror?2:1);++reflection) {
                if(sizeof(ImDrawIdx)==2 && draw->_VtxCurrentIdx+count>=65536
                    && !(draw->Flags&ImDrawListFlags_AllowVtxOffset)) return;
                draw->PrimReserve((count-2)*3,count);
                const unsigned base=draw->_VtxCurrentIdx;
                for(int i=0;i<count;++i) {
                    const auto p=reflection ? points[i].p-normal*(2.f*distance(points[i].p)) : points[i].p;
                    const auto& c=points[i].color;
                    draw->PrimWriteVtx(ImVec2(origin.x+(p.x-SpectrumWidth*.5f)*scale,
                        origin.y+(p.y-SpectrumHeight*.5f)*scale),uv,
                        ImGui::ColorConvertFloat4ToU32(ImVec4(c[0],c[1],c[2],c[3]*opacity)));
                }
                for(int i=1;i<count-1;++i) {
                    draw->PrimWriteIdx(static_cast<ImDrawIdx>(base));
                    draw->PrimWriteIdx(static_cast<ImDrawIdx>(base+i));
                    draw->PrimWriteIdx(static_cast<ImDrawIdx>(base+i+1));
                }
            }
        }
        inline void Line(ImDrawList* draw,SpectrumPoint a,SpectrumPoint b,float width,
            std::array<float,4> ca,std::array<float,4> cb,ImVec2 origin,float scale,const SpectrumStyle& s,SpectrumPoint normal,float opacity) {
            const auto delta=b-a; const float length=std::hypot(delta.x,delta.y);
            if(length<.0001f) return;
            const SpectrumPoint n{-delta.y/length*width*.5f,delta.x/length*width*.5f};
            std::array<Vertex,16> quad{{{a-n,ca},{a+n,ca},{b+n,cb},{b-n,cb}}};
            Polygon(draw,quad,4,origin,scale,s,normal,opacity);
        }
    }
    inline void DrawAudioSpectrum(ImDrawList* draw,ImVec2 topLeft,float size,const SpectrumEnvelope& audio,const Settings& settings) {
        const auto& s=settings.spectrum;
        const ImVec2 origin(topLeft.x+size*.5f,topLeft.y+size*.5f);
        const float scale=size/SpectrumHeight;
        const int count=std::clamp(s.bands,2,MaxSpectrumBands);
        const auto mirrorNormal=ReflectionNormal(s);
        const bool circular=!s.polarPath && s.start[1]==s.end[1] && s.end[0]>s.start[0]
            && s.conversion==0 && s.polarAmount==100.f;
        // Overlay and preview share one render thread and the same settings.
        // Cache angular geometry instead of running thousands of trig calls per frame.
        struct Directions { int bands{};float start{},end{};std::array<SpectrumPoint,MaxSpectrumBands> value; };
        static Directions directions;
        if(circular && (directions.bands!=count || directions.start!=s.start[0] || directions.end!=s.end[0])) {
            directions.bands=count;directions.start=s.start[0];directions.end=s.end[0];
            for(int i=0;i<count;++i) {
                const float x=s.start[0]+(s.end[0]-s.start[0])*i/(count-1);
                const float angle=SpectrumPi*.5f+x/SpectrumWidth*2.f*SpectrumPi;
                directions.value[i]={std::cos(angle),std::sin(angle)};
            }
        }
        const SpectrumPoint canvasCenter{SpectrumWidth*.5f,SpectrumHeight*.5f};
        const auto rootColor=SpectrumColor(s,0,0,audio.dominant),tipColor=SpectrumColor(s,0,1,audio.dominant);
        const int feather=s.softness>0.f ? 1+settings.quality : 0;
        const float span=s.polarPath ? 2.f*SpectrumPi*SpectrumHeight*.25f : std::hypot(s.end[0]-s.start[0],s.end[1]-s.start[1]);
        for(int layer=feather;layer>=0;--layer) {
            const float spread=feather ? s.softness*.16f*layer/feather : 0.f;
            const float width=s.thickness+spread*2.f;
            const float coverage=s.display==0 ? (std::min)(1.f,span/((count-1)*width)) : 1.f;
            const float layerOpacity=settings.opacity*(layer==0 ? 1.f-s.softness*.006f : .12f/feather);
            // Keep the requested physical thickness. Compensate coverage when
            // overlap blending is off instead of narrowing the requested bands.
            const float opacity=s.blendOverlap ? layerOpacity : 1.f-std::pow(1.f-layerOpacity,coverage);
            const float halfAngle=width*.5f/SpectrumWidth*2.f*SpectrumPi;
            const float halfCos=std::cos(halfAngle),halfSin=std::sin(halfAngle);
            for(int i=0;i<count;++i) {
                const float t=float(i)/(count-1);
                const auto inner=s.hueDegrees==0.f ? rootColor : SpectrumColor(s,t,0,audio.dominant);
                const auto outer=s.hueDegrees==0.f ? tipColor : SpectrumColor(s,t,1,audio.dominant);
                const float length=audio.bands[i]*s.maximumHeight;
                for(int half=0;half<(s.side==2?2:1);++half) {
                    const int side=s.side==2 ? (half==0?-1:1) : s.side==0?-1:1;
                    if(s.display==0) {
                        if(length<.001f) continue;
                        std::array<spectrum_draw::Vertex,16> quad{};
                        if(circular) {
                            const auto d=directions.value[i];const SpectrumPoint tangent{-d.y,d.x};
                            const auto left=d*halfCos-tangent*halfSin,right=d*halfCos+tangent*halfSin;
                            const float base=(s.start[1]-side*spread)*.5f,tip=(s.start[1]+side*(length+spread))*.5f;
                            quad[0]={canvasCenter+left*base,inner};quad[1]={canvasCenter+right*base,inner};
                            quad[2]={canvasCenter+right*tip,outer};quad[3]={canvasCenter+left*tip,outer};
                        } else {
                            quad[0]={SpectrumTransform(SpectrumSource(s,t,-spread,side,-width*.5f),s),inner};
                            quad[1]={SpectrumTransform(SpectrumSource(s,t,-spread,side,width*.5f),s),inner};
                            quad[2]={SpectrumTransform(SpectrumSource(s,t,length+spread,side,width*.5f),s),outer};
                            quad[3]={SpectrumTransform(SpectrumSource(s,t,length+spread,side,-width*.5f),s),outer};
                        }
                        spectrum_draw::Polygon(draw,quad,4,origin,scale,s,mirrorNormal,opacity);
                    } else {
                        const auto end=circular ? canvasCenter+directions.value[i]*((s.start[1]+side*length)*.5f)
                            : SpectrumTransform(SpectrumSource(s,t,length,side),s);
                        if(s.display==1 && i>0) {
                            const float previous=float(i-1)/(count-1);
                            const float previousLength=audio.bands[i-1]*s.maximumHeight;
                            const auto start=circular ? canvasCenter+directions.value[i-1]*((s.start[1]+side*previousLength)*.5f)
                                : SpectrumTransform(SpectrumSource(s,previous,previousLength,side),s);
                            spectrum_draw::Line(draw,start,end,width,s.hueDegrees==0.f ? tipColor : SpectrumColor(s,previous,1,audio.dominant),outer,origin,scale,s,mirrorNormal,opacity);
                        } else if(s.display==2) {
                            std::array<spectrum_draw::Vertex,16> dot{};
                            constexpr SpectrumPoint unit[]{{1,0},{.70710678f,.70710678f},{0,1},{-.70710678f,.70710678f},
                                {-1,0},{-.70710678f,-.70710678f},{0,-1},{.70710678f,-.70710678f}};
                            for(int n=0;n<8;++n) dot[n]={end+unit[n]*(width*.5f),outer};
                            spectrum_draw::Polygon(draw,dot,8,origin,scale,s,mirrorNormal,opacity);
                        }
                    }
                }
            }
        }
    }
}
