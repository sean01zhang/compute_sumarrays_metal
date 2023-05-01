#include <metal_stdlib>
#include "mt.h"

using namespace metal;


kernel void generate_randomnumbers(device const int * a, device const int * b,
                       device float * result, uint2 index [[thread_position_in_grid]]) {
  mt19937 mt;
  mt.srand(index);
  result[index] += mt.rand();
  tex.write()
}

kernel void generate_randomnumbers_2d(texture2d<half, access:write> tex [[texture(0)]],
                   device const float * a, uint2 index [[thread_position_in_grid]]) {
  mt19937 mt;
  mt.srand(index);
  tex.write(float3(mt.rand() + a[index[0]], mt.rand(), mt.rand()), index, 0);
}

kernel void add_arrays(device const int * a, device const int * b,
                       device int * result, uint index [[thread_position_in_grid]]) {
  result[index] = a[index] + b[index];
}


