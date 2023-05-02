#include <metal_stdlib>
#include "mt.h"

using namespace metal;


kernel void generate_randomnumbers_2d(texture2d<float, access::write> tex [[texture(0)]],
                   uint2 index [[thread_position_in_grid]]) {
  mt19937 mt;
  mt.srand(index[0] + index[1]);
  tex.write(float4(
    (float)index[0] / tex.get_width(),
    (float) index[1] / tex.get_height(),
    mt.rand(), 1), index, 0);
}

kernel void add_arrays(device const int * a, device const int * b,
                       device int * result, uint index [[thread_position_in_grid]]) {
  result[index] = a[index] + b[index];
}


