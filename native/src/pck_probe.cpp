#include "ae3/pck_archive.hpp"
#include <filesystem>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

int main(int argc,char** argv){
    if(argc!=2 && argc!=4){
        std::cerr<<"Usage: Getchu3PckProbe <your-own-level.pck> [--extract <private-output-folder>]\n";
        return 2;
    }
    try{
        ae3::PckArchive archive(std::filesystem::u8path(argv[1]));
        std::map<std::string,unsigned> types;
        std::uint64_t bytes=0;
        std::cout<<"NATIVE ORIGINAL AE3 PCK ARCHIVE: "<<archive.entries().size()<<" members\n";
        for(const auto& item:archive.entries()){
            ++types[ae3::pckExtension(item.kind)];
            bytes+=item.size;
            std::cout<<item.index<<"\t"<<item.name<<"\t"
                     <<ae3::pckExtension(item.kind)<<"\t"
                     <<item.size<<" bytes\n";
        }
        std::cout<<"Original asset bytes: "<<bytes<<"\n";
        for(const auto& [name,count]:types)
            std::cout<<name<<": "<<count<<"\n";
        if(argc==4) {
            if(std::string(argv[2])!="--extract")
                throw std::runtime_error("expected --extract");
            archive.extractTo(std::filesystem::u8path(argv[3]));
            std::cout<<"Original owner asset files extracted locally; no PS2 hardware emulation.\n";
        }
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"NATIVE_PCK_ERROR: "<<e.what()<<"\n";
        return 1;
    }
}
