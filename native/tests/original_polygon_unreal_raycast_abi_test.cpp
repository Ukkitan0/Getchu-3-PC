#include "ae3/ae3_unreal_lua_bridge.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    auto* owner = ae3_original_manager_init_create(5489);
    assert(owner);
    // Source polygon vector lane layout at +0x40, +0x50, +0x60.
    // The actual original raycast sees bytes, NOT Unreal StaticMesh.
    std::array<uint8_t, 0x70> sourcePolygon{};
    const std::array<Ae3OriginalVec4,3> vertices{{
        {0,0,0,1}, {0,0,10,1}, {10,0,0,1}
    }};
    std::memcpy(sourcePolygon.data()+0x40,vertices.data(),
                vertices.size()*sizeof(vertices[0]));
    const std::array<uint8_t,1> degree{0};
    const std::array<uint32_t,3> neighbors{0,0,0};
    const std::array<uint16_t,3> kinds{0,0,0};
    Ae3OriginalPolygonRaySourceV1 ray{};
    ray.original_records70=sourcePolygon.data();
    ray.original_record_count=1;
    ray.original_degree=degree.data();
    ray.original_neighbor3=neighbors.data();
    ray.original_action_kind3=kinds.data();
    ray.original_start_polygon=0;
    ray.original_ray_start={2,0,2,1};
    ray.original_ray_end={3,0,2,1};
    ray.original_max_recursions=64;
    Ae3OriginalPolygonRayResultV1 result{};
    result.last_original_polygon=UINT32_MAX;
    assert(ae3_original_polygon_project_4c1b3c(owner,&ray,&result)==1);
    assert(result.last_original_polygon==0);
    assert(result.original_polygons_tested==1);
    assert(result.original_edge_tests==3);
    assert(result.original_hit_point_available==1);
    assert(result.original_hit_point.x==3.0f &&
           result.original_hit_point.z==2.0f);
    // Source original 0x21EE00 rejects a ray outside x>=0.
    ray.original_ray_start={-2,0,2,1};
    ray.original_ray_end={-3,0,2,1};
    assert(ae3_original_polygon_project_4c1b3c(owner,&ray,&result)==1);
    assert(result.last_original_polygon==UINT32_MAX);
    assert(result.original_hit_point_available==0);
    assert(result.original_edge_tests==1);
    // Reject unknown polygon links before any output publication.
    std::array<uint8_t,1> corruptDegree{4};
    ray.original_degree=corruptDegree.data();
    const auto snapshot=result;
    assert(ae3_original_polygon_project_4c1b3c(owner,&ray,&result)==0);
    assert(result.last_original_polygon==snapshot.last_original_polygon);
    assert(std::strstr(ae3_original_manager_init_last_error(owner),
                       "degree")!=nullptr);
    ray.original_degree=degree.data();
    ray.original_start_polygon=5;
    assert(ae3_original_polygon_project_4c1b3c(owner,&ray,&result)==0);
    ray.original_start_polygon=0;
    ray.original_max_recursions=0;
    assert(ae3_original_polygon_project_4c1b3c(owner,&ray,&result)==0);
    ray.original_max_recursions=64;
    ray.original_records70=nullptr;
    assert(ae3_original_polygon_project_4c1b3c(owner,&ray,&result)==0);
    assert(ae3_original_manager_init_destroy(owner)==1);
    std::cout<<"ORIGINAL_POLYGON_4C1B3C_NATIVE_RAYCAST_ABI_PASSED\n";
    return 0;
}
