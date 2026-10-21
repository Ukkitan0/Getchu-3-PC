#include "ae3/archive.hpp"
#include "ae3/original_navgraph_loader.hpp"
#include "ae3/original_navgraph_topology.hpp"
#include <span>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc,char** argv) {
    if(argc<2){
        std::cerr<<"Usage: Getchu3NavGraphProbe <original AE3 USA ISO or DATA.BIN> [path-filter]\n";
        return 2;
    }
    try{
        ae3::Archive game(std::filesystem::u8path(argv[1]));
        const std::string filter=argc>2 ? argv[2] : "";
        std::uint64_t members=0,records=0,links=0,unlinked=0;
        std::size_t packageCount=0;
        for(const auto& item:game.entries()){
            const std::string_view path(item.path);
            if(!path.ends_with("navigation.pck.sz") ||
               (!filter.empty()&&path.find(filter)==std::string_view::npos))continue;
            const auto originalNav=ae3::originalNavReadPackage(game,item);
            ++packageCount;
            for(const auto& nav:originalNav){
                if(nav.originalNavBytes.size()<0x28)
                    throw std::runtime_error("Original .nav header truncated");
                const auto& b=nav.originalNavBytes;
                const std::uint32_t count=std::uint32_t(b[0x24])|
                    (std::uint32_t(b[0x25])<<8)|
                    (std::uint32_t(b[0x26])<<16)|
                    (std::uint32_t(b[0x27])<<24);
                const ae3::OriginalNavGraphTopology graph(
                    std::as_bytes(std::span(b)));
                if(graph.size()!=count)
                    throw std::runtime_error("Original nav adjacency count diverged");
                std::uint64_t memberLinks=0;
                for(std::size_t index=0;index<graph.size();index++){
                    const auto adjacent=graph.links(index);
                    memberLinks+=adjacent.size();
                    if(adjacent.empty())++unlinked;
                }
                ++members;
                records+=count;
                links+=memberLinks;
                if(!filter.empty())
                    std::cout<<item.path<<"/"<<nav.originalFilename
                             <<" original_slot="<<nav.originalSlot
                             <<" original_records="<<count
                             <<" source_neighbor_links="<<memberLinks<<"\n";
            }
        }
        std::cout<<"Original retail navigation packages: "<<packageCount<<"\n"
                 <<"Original retail .nav members: "<<members<<"\n"
                 <<"Original navigation record headers: "<<records<<"\n"
                 <<"Validated original neighbor indices: "<<links<<"\n"
                 <<"Zero-link original records: "<<unlinked<<"\n"
                 <<"Native archive parsing only; gameplay not booted.\n";
        return packageCount?0:1;
    }catch(const std::exception& err){
        std::cerr<<"Original NavGraph archive error: "<<err.what()<<"\n";
        return 1;
    }
}
