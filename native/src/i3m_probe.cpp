#include "ae3/i3m_animation.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc,char** argv){
    if(argc!=2){
        std::cerr<<"Usage: Getchu3I3mProbe <your-extracted-original-animation.i3m>\n";
        return 2;
    }
    try{
        const auto animation=ae3::parseI3m(std::filesystem::u8path(argv[1]));
        std::size_t keyCount=0;
        std::cout<<"NATIVE_AE3_ANIMATION_DATA_LOADED\n"
                 <<"Original duration (seconds): "<<animation.duration<<"\n"
                 <<"Named quaternion tracks: "<<animation.tracks.size()<<"\n";
        for(const auto& track:animation.tracks){
            keyCount+=track.keys.size();
            std::cout<<track.name<<": "<<track.keys.size()<<" rotation keys";
            if(!track.keys.empty())
                std::cout<<", first pose W="<<track.keys.front().xyzw[3];
            std::cout<<"\n";
        }
        std::cout<<"Original rotation keyframes: "<<keyCount<<"\n"
                 <<"All quaternions sampled on native host CPU; "
                   "track-to-bone binding in original skeleton still under investigation.\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"NATIVE_AE3_I3M_ERROR: "<<error.what()<<"\n";
        return 1;
    }
}
