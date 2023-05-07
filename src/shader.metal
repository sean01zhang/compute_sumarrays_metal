kernel void add_arrays(device const int * a, device const int * b,
                       device int * result, uint index [[thread_position_in_grid]]) {
  result[index] = a[index] + b[index];
}
