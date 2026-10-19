#include "ae3/tim2.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace ae3 {
namespace {
std::uint16_t read16(const std::vector<std::uint8_t>& b, std::size_t p) {
    if(p>b.size() || b.size()-p<2) throw std::runtime_error("TIM2 truncated u16");
    return std::uint16_t(b[p]) | (std::uint16_t(b[p+1])<<8);
}
std::uint32_t read32(const std::vector<std::uint8_t>& b, std::size_t p) {
    return std::uint32_t(read16(b,p)) | (std::uint32_t(read16(b,p+2))<<16);
}
bool within(std::size_t pos,std::size_t count,std::size_t total) {
    return pos<=total && count<=total-pos;
}
std::uint8_t gsAlpha(std::uint8_t a) {
    return a>=0x80?0xff:std::uint8_t((unsigned(a)*255+64)/128);
}
void color16(std::uint16_t v,std::uint8_t* dst){
    dst[0]=std::uint8_t(((v    )&31)*255/31);
    dst[1]=std::uint8_t(((v>>5 )&31)*255/31);
    dst[2]=std::uint8_t(((v>>10)&31)*255/31);
    dst[3]=(v&0x8000)?255:0;
}
unsigned csm1Index(unsigned i) {
    // Source-order index lookup documented in Ape Escape 3 TIM2 tooling.
    return (i&0xe7) | ((i&0x08)<<1) | ((i&0x10)>>1);
}
void decodePaletteEntry(const std::vector<std::uint8_t>& b,std::size_t at,
                        unsigned kind,std::uint8_t* dst){
    if(kind==3) {
        if(!within(at,4,b.size()))throw std::runtime_error("TIM2 CLUT RGBA32 truncated");
        dst[0]=b[at];dst[1]=b[at+1];dst[2]=b[at+2];dst[3]=gsAlpha(b[at+3]);
    } else if(kind==2){
        if(!within(at,3,b.size()))throw std::runtime_error("TIM2 CLUT RGB24 truncated");
        dst[0]=b[at];dst[1]=b[at+1];dst[2]=b[at+2];dst[3]=255;
    } else if(kind==1){
        color16(read16(b,at),dst);
    } else throw std::runtime_error("unsupported native TIM2 CLUT pixel format");
}
}
bool isTim2(const std::vector<std::uint8_t>& b){
    return b.size()>=16 && b[0]=='T' && b[1]=='I' && b[2]=='M' && b[3]=='2';
}

Texture decodeTim2(const std::vector<std::uint8_t>& b, unsigned picture){
    if(!isTim2(b))throw std::runtime_error("not a TIM2 texture");
    const auto imageCount=read16(b,6);
    if(picture>=imageCount || imageCount>1024)
        throw std::runtime_error("invalid TIM2 picture index");
    std::size_t cursor=b[5]==0?16:128;
    Texture result;
    for(unsigned index=0;index<=picture;++index) {
        if(!within(cursor,48,b.size()))throw std::runtime_error("truncated TIM2 picture header");
        const std::uint32_t totalSize=read32(b,cursor);
        const std::uint32_t clutSize=read32(b,cursor+4);
        const std::uint32_t imageSize=read32(b,cursor+8);
        const std::uint16_t headerSize=read16(b,cursor+12);
        const unsigned clutColors=read16(b,cursor+14);
        const unsigned clutType=b[cursor+18];
        const unsigned imageType=b[cursor+19];
        const unsigned width=read16(b,cursor+20);
        const unsigned height=read16(b,cursor+22);
        if(!width || !height || width>4096 || height>4096 ||
           std::uint64_t(width)*height>8*1024*1024)
            throw std::runtime_error("unreasonable TIM2 dimensions");
        if(headerSize<48 || totalSize<headerSize ||
           !within(cursor,totalSize,b.size()) ||
           imageSize>totalSize-headerSize ||
           clutSize>totalSize-headerSize-imageSize)
            throw std::runtime_error("invalid TIM2 picture bounds");
        if(index==picture) {
            const std::size_t pixels=std::size_t(width)*height;
            const std::size_t imagePos=cursor+headerSize;
            const std::size_t palettePos=imagePos+imageSize;
            result.width=width;result.height=height;
            result.rgba.resize(pixels*4);
            const bool indexed=imageType==4 || imageType==5;
            std::vector<std::uint8_t> palette;
            unsigned paletteColorSize=0;
            if(indexed) {
                const unsigned paletteKind=clutType&0x3f;
                paletteColorSize=paletteKind==3?4:paletteKind==2?3:paletteKind==1?2:0;
                if(!paletteColorSize || !clutColors || clutColors>256 ||
                    std::uint64_t(clutColors)*paletteColorSize>clutSize)
                    throw std::runtime_error("invalid TIM2 indexed palette");
                palette.resize(std::size_t(clutColors)*4);
                for(unsigned c=0;c<clutColors;++c)
                    decodePaletteEntry(b,palettePos+std::size_t(c)*paletteColorSize,
                                       paletteKind,palette.data()+std::size_t(c)*4);
            }
            const std::size_t required=imageType==3?pixels*4 :
                imageType==2?pixels*3 : imageType==1?pixels*2 :
                imageType==5?pixels : imageType==4?(pixels+1)/2 : 0;
            if(!required)throw std::runtime_error("unsupported native TIM2 image format");
            if(required>imageSize)throw std::runtime_error("TIM2 pixel image truncated");
            for(std::size_t p=0;p<pixels;++p){
                auto* color=result.rgba.data()+4*p;
                if(imageType==3){
                    auto at=imagePos+p*4;
                    color[0]=b[at];color[1]=b[at+1];color[2]=b[at+2];
                    color[3]=gsAlpha(b[at+3]);
                }else if(imageType==2){
                    auto at=imagePos+p*3;
                    color[0]=b[at];color[1]=b[at+1];color[2]=b[at+2];color[3]=255;
                }else if(imageType==1){
                    color16(read16(b,imagePos+p*2),color);
                }else{
                    unsigned id=imageType==5?b[imagePos+p]:
                       (p%2==0?b[imagePos+p/2]&15:b[imagePos+p/2]>>4);
                    if(imageType==5 && clutColors==256 && !(clutType&0x80))
                        id=csm1Index(id);
                    if(id>=clutColors)throw std::runtime_error("TIM2 pixel palette out of bounds");
                    std::copy_n(palette.data()+std::size_t(id)*4,4,color);
                }
            }
            return result;
        }
        cursor+=(std::size_t(totalSize)+15)&~std::size_t(15);
    }
    throw std::runtime_error("TIM2 image not found");
}
} // namespace ae3
