#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>

int main() {
  // create device
  MTL::Device * device = MTL::CreateSystemDefaultDevice();
  NS::Error * error;

  // create command queue
  MTL::CommandQueue * command_queue = device->newCommandQueue();
  // create command buffer
  MTL::CommandBuffer * command_buffer = command_queue->commandBuffer();
  // create command encoder
  MTL::ComputeCommandEncoder * command_encoder = command_buffer->computeCommandEncoder();

  // ** Create pipeline state object
  NS::String* libPath = NS::String::string("./shader.metallib", NS::UTF8StringEncoding);
  auto default_library = device->newLibrary(libPath, &error);
  if (!default_library) {
    std::cerr << "Failed to load default library.";
    std::exit(-1);
  }

  auto add_arrays_function_name = NS::String::string("add_arrays", NS::ASCIIStringEncoding);
  auto add_function = default_library->newFunction(add_arrays_function_name);
  if (!add_function) {
    std::cerr << "failed to find the adder function";
  }


  auto gen_randnums_fn_name = NS::String::string("generate_randomnumbers", NS::ASCIIStringEncoding);
  auto gen_randnums_function = default_library->newFunction(gen_randnums_fn_name);
  if (!add_function) {
    std::cerr << "failed to find the adder function";
  }

  auto pso = device->newComputePipelineState(gen_randnums_function, &error);
  // free defualt library and add function
  add_arrays_function_name->release();
  default_library->release();
  add_function->release();

  // pass pipeline state object created
  // into the command encoder
  command_encoder->setComputePipelineState(pso);


  // ** Create data buffers
  // TODO: Make data and determine its size
  int array1[] = {1, 2, 3, 4, 5, 6};
  int array2[] = {1, 1, 1, 1, 1, 1};
  size_t arraySize = 6;
  size_t bufferSize = arraySize * sizeof(int);

  MTL::Buffer * a = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
  MTL::Buffer * b = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
  MTL::Buffer * out = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);

  // copy data into buffers
  memcpy(a->contents(), array1, bufferSize);
  memcpy(b->contents(), array2, bufferSize);

  // pass argument data into the command encoder
  command_encoder->setBuffer(a, 0, 0);
  command_encoder->setBuffer(b, 0, 1);
  command_encoder->setBuffer(out,0, 2);

  // set thread count and organization, then run the damn thing
  MTL::Size gridSize = MTL::Size(arraySize, 1, 1);

  NS::UInteger threadsPerThreadgroup = pso->maxTotalThreadsPerThreadgroup();
  MTL::Size threadgroupSize(threadsPerThreadgroup, 1, 1);

  command_encoder->dispatchThreads(gridSize, threadgroupSize);
  command_encoder->endEncoding();

  command_buffer->commit();

  // wait for the GPU work is done
  command_buffer->waitUntilCompleted();

  // read results from buffer
  float * result = (float *)out->contents();

  std::cout << "results:" << std::endl;
  for (size_t i = 0; i < arraySize; ++i) {
    std::cout << result[i] << std::endl;
  }

  a->release();
  b->release();
  out->release();
  pso->release();
  command_queue->release();
  device->release();

  return 0;
}
