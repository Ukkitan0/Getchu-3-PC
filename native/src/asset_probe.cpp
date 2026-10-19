#include "ae3/archive.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
namespace fs = std::filesystem;

int main(int argc,char** argv) {
    if(argc < 2) {
        std::cerr << "Usage: Getchu3AssetProbe DATA.BIN|ApeEscape3.iso [--list] "
                     "[--extract asset/path output/file]\n";
        return 2;
    }
    try {
        ae3::Archive archive(fs::u8path(argv[1]));
        const auto& entries=archive.entries();
        std::map<std::string,unsigned> types;
        std::uint64_t total=0;
        for(const auto& item:entries) {
            types[ae3::assetType(item.path)]++;
            total+=item.size;
        }
        std::cout<<"Getchu 3 native game-asset scan (NO PS2 simulation)\n"
                 <<"Source kind: "<<archive.sourceKind()<<"\n"
                 <<"Original assets: "<<entries.size()<<"\n"
                 <<"Indexed bytes: "<<total<<"\n"
                 <<"File categories:\n";
        for(auto const& [extension,count]:types)
            std::cout<<"  "<<extension<<": "<<count<<"\n";
        if(argc>2 && std::string(argv[2])=="--list"){
            for(auto const& e:entries)std::cout<<e.path<<"\t"<<e.size<<"\n";
        } else if(argc>4 && std::string(argv[2])=="--extract"){
            auto result=archive.find(argv[3]);
            if(!result)throw std::runtime_error("asset not found in verified VFI index");
            auto data=archive.read(entries[*result]);
            fs::path output=fs::u8path(argv[4]);
            std::ofstream out(output,std::ios::binary|std::ios::trunc);
            if(!out)throw std::runtime_error("cannot create output file");
            out.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size()));
            if(!out)throw std::runtime_error("asset output write failed");
            std::cout<<"Exported native asset: "<<entries[*result].path<<"\n";
        }
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"Native asset scan failed: "<<e.what()<<"\n";
        return 1;
    }
}
