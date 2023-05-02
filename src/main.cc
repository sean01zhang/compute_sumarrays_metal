#include <climits>
#include <simd/vector_types.h>
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <iostream>
#include <limits>
#include <simd/simd.h>

int main() {
  // create device
  MTL::Device *device = MTL::CreateSystemDefaultDevice();
  NS::Error *error;

  // create command queue
  MTL::CommandQueue *command_queue = device->newCommandQueue();
  // create command buffer
  MTL::CommandBuffer *command_buffer = command_queue->commandBuffer();
  // create command encoder
  MTL::ComputeCommandEncoder *command_encoder =
      command_buffer->computeCommandEncoder();

  // ** Create pipeline state object
  NS::String *libPath =
      NS::String::string("./shader.metallib", NS::UTF8StringEncoding);
  auto default_library = device->newLibrary(libPath, &error);
  if (!default_library) {
    std::cerr << "Failed to load default library.";
    std::exit(-1);
  }

  auto gen_randnums_fn_name =
      NS::String::string("generate_randomnumbers_2d", NS::ASCIIStringEncoding);
  auto gen_randnums_function =
      default_library->newFunction(gen_randnums_fn_name);
  if (!gen_randnums_function) {
    std::cerr << "failed to find the adder function";
  }

  auto pso = device->newComputePipelineState(gen_randnums_function, &error);
  // free defualt library and add function
  gen_randnums_fn_name->release();
  default_library->release();

  // pass pipeline state object created
  // into the command encoder
  command_encoder->setComputePipelineState(pso);

  // ** Create texture
  // TODO: Make data and determine its size
  MTL::TextureDescriptor *desc = MTL::TextureDescriptor::alloc()->init();
  desc->setWidth(2);
  desc->setHeight(3);
  desc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
  desc->setTextureType(MTL::TextureType2D);
  desc->setStorageMode(MTL::StorageModeManaged);
  desc->setUsage(MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
  MTL::Texture *out = device->newTexture(desc);
  desc->release();

  // add argument to cmd encoder
  command_encoder->setTexture(out, 0);

  MTL::Size gridSize = MTL::Size(out->width(), out->height(), 1);
  NS::UInteger threadsPerThreadgroup = pso->maxTotalThreadsPerThreadgroup();
  MTL::Size threadgroupSize(threadsPerThreadgroup, 1, 1);

  command_encoder->dispatchThreads(gridSize, threadgroupSize);
  command_encoder->endEncoding();

  command_buffer->commit();
  // wait for the GPU work is done
  command_buffer->waitUntilCompleted();

  // ** read results from buffer
  int bytesPerRow = out->width() * sizeof(simd::uchar4);
  int bytesPerImage = out->height() * bytesPerRow;

  MTL::Region destinationRegion = MTL::Region::Make2D(0, 0, out->width(), out->height());

  simd::uchar4 *pixelBytes = (simd::uchar4 *)malloc(bytesPerImage);
  out->getBytes(pixelBytes, bytesPerRow, destinationRegion, 0);

  // simd::uchar4 * rgbaPixel = (simd::uchar4 *)pixelBytes;

  int length = out->width() * out->height();
  std::cout << "results:" << std::endl;
  for (size_t i = 0; i < length; ++i) {
    for (size_t j = 0; j < 4; ++j) {
      std::cout << (float)pixelBytes[i][j]/ UCHAR_MAX  << " ";
    }
    std::cout << std::endl;

    // std::cout << rgbaPixel[i] << std::endl;
  }

  out->release();
  pso->release();
  command_queue->release();
  device->release();
  free(pixelBytes);

  return 0;
}
