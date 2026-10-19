#include "ae3/i3m_animation.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace ae3 {
namespace {
std::uint16_t get16(const std::vector<std::uint8_t>& b,std::size_t p){
    if(p>b.size()||b.size()-p<2)throw std::runtime_error("I3M truncated u16");
    return std::uint16_t(b[p])|(std::uint16_t(b[p+1])<<8);
}
std::uint32_t get32(const std::vector<std::uint8_t>& b,std::size_t p){
    return get16(b,p)|(std::uint32_t(get16(b,p+2))<<16);
}
float getFloat(const std::vector<std::uint8_t>& b,std::size_t p){
    std::uint32_t bits=get32(b,p);
    float v=0;std::memcpy(&v,&bits,sizeof(v));
    return v;
}
float dot(const std::array<float,4>& a,const std::array<float,4>& b){
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
}
std::array<float,4> normalized(std::array<float,4> v){
    const float length=std::sqrt(dot(v,v));
    if(!std::isfinite(length)||length<0.05f||length>1.2f)
        throw std::runtime_error("I3M rotation quaternion malformed");
    for(auto& c:v)c/=length;
    return v;
}
std::string getName(const std::vector<std::uint8_t>& data,std::uint32_t p){
    if(p>=data.size())throw std::runtime_error("I3M track name offset exceeds file");
    std::string out;
    while(p<data.size() && data[p]!=0 && out.size()<128){
        if(data[p]<32||data[p]>126)
            throw std::runtime_error("invalid I3M track name");
        out.push_back(char(data[p]));
        ++p;
    }
    if(out.empty()||out.size()>=128||p>=data.size())
        throw std::runtime_error("unterminated I3M track name");
    return out;
}
}
OriginalAnimation parseI3m(const std::vector<std::uint8_t>& source){
    if(source.size()<36||source.size()>64*1024*1024 ||
       std::memcmp(source.data(),"I3D_I3M",7)!=0)
        throw std::runtime_error("not a valid original AE3 I3M animation");
    const auto total=get32(source,0x0c);
    const auto count=get16(source,0x14);
    const float duration=getFloat(source,0x18);
    const std::uint64_t table=get32(source,0x1c),pool=get32(source,0x20);
    if(total>source.size()||total<36||count>4096||
       !std::isfinite(duration)||duration<0||duration>3600||
       table>total||std::uint64_t(count)*12>total-table||
       pool>total)
        throw std::runtime_error("I3M header/track table malformed");
    OriginalAnimation animation;
    animation.duration=duration;
    animation.tracks.reserve(count);
    for(unsigned i=0;i<count;++i){
        const std::size_t record=static_cast<std::size_t>(table+i*12);
        const auto nameOffset=get32(source,record);
        const auto kind=get16(source,record+4);
        const auto keyCount=get16(source,record+6);
        const std::uint64_t keyOffset=get32(source,record+8);
        if(keyCount>10000 || keyOffset>total ||
           std::uint64_t(keyCount)*10>total-keyOffset)
            throw std::runtime_error("I3M keyframe table exceeds source");
        BoneRotationTrack track;
        track.name=getName(source,nameOffset);
        track.type=kind;
        track.keys.reserve(keyCount);
        std::uint16_t highestFrame=0;
        for(unsigned k=0;k<keyCount;++k){
            const std::size_t at=static_cast<std::size_t>(keyOffset+k*10);
            const auto frame=get16(source,at);
            const auto curve=get16(source,at+2);
            const auto paletteIndex=get16(source,at+6);
            if(std::uint64_t(paletteIndex)*8>total-pool ||
               8>total-pool-std::uint64_t(paletteIndex)*8)
                throw std::runtime_error("I3M shared quaternion pool index invalid");
            const auto pq=static_cast<std::size_t>(pool+std::uint64_t(paletteIndex)*8);
            std::array<float,4> q{};
            for(int j=0;j<4;++j){
                q[std::size_t(j)]=
                    static_cast<std::int16_t>(get16(source,pq+std::size_t(j)*2))/32768.f;
            }
            highestFrame=std::max(highestFrame,frame);
            track.keys.push_back({frame,curve,0,normalized(q)});
        }
        // Original files can end with out-of-order keys; keep all values,
        // sort by their actual frame index, then map to reported clip duration.
        std::stable_sort(track.keys.begin(),track.keys.end(),
                         [](const auto& a,const auto& b){return a.frame<b.frame;});
        for(auto& key:track.keys)
            key.seconds=highestFrame?
                duration*(float(key.frame)/float(highestFrame)):0.f;
        animation.tracks.push_back(std::move(track));
    }
    return animation;
}
OriginalAnimation parseI3m(const std::filesystem::path& file){
    std::error_code error;
    const auto size=std::filesystem::file_size(file,error);
    if(error||size<36||size>64*1024*1024)
        throw std::runtime_error("original I3M animation unavailable or oversized");
    std::ifstream stream(file,std::ios::binary);
    std::vector<std::uint8_t> b(static_cast<std::size_t>(size));
    if(!stream.read(reinterpret_cast<char*>(b.data()),
                    static_cast<std::streamsize>(b.size())))
        throw std::runtime_error("short host animation file read");
    return parseI3m(b);
}
std::array<float,4> sampleTrack(const BoneRotationTrack& track,float seconds){
    if(track.keys.empty())return {0,0,0,1};
    if(!std::isfinite(seconds))
        throw std::runtime_error("native animation time is not finite");
    const auto& keys=track.keys;
    if(seconds<=keys.front().seconds)return keys.front().xyzw;
    if(seconds>=keys.back().seconds)return keys.back().xyzw;
    const auto after=std::upper_bound(keys.begin(),keys.end(),seconds,
           [](float t,const RotationKey& key){return t<key.seconds;});
    if(after==keys.begin()||after==keys.end())return keys.back().xyzw;
    const auto& a=*(after-1);
    const auto& b=*after;
    const float span=b.seconds-a.seconds;
    const float alpha=span>0?std::clamp((seconds-a.seconds)/span,0.f,1.f):1.f;
    auto dest=b.xyzw;
    float similarity=dot(a.xyzw,dest);
    if(similarity<0){
        for(auto& c:dest)c=-c;
        similarity=-similarity;
    }
    similarity=std::clamp(similarity,-1.f,1.f);
    std::array<float,4> result{};
    if(similarity>0.9995f){
        for(int k=0;k<4;++k)
            result[std::size_t(k)]=(1-alpha)*a.xyzw[std::size_t(k)]
                                    +alpha*dest[std::size_t(k)];
    }else{
        const float angle=std::acos(similarity);
        const float sinAngle=std::sin(angle);
        const float left=std::sin((1-alpha)*angle)/sinAngle;
        const float right=std::sin(alpha*angle)/sinAngle;
        for(int k=0;k<4;++k)
            result[std::size_t(k)]=left*a.xyzw[std::size_t(k)]
                                  +right*dest[std::size_t(k)];
    }
    return normalized(result);
}
} // namespace ae3
