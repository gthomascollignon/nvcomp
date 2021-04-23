/*
 * Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NVCOMP_API_HPP
#define NVCOMP_API_HPP

#include "nvcomp.h"
#include "nvcomp/lz4.h"

#include <cstdint>
#include <cuda_runtime.h>
#include <stdexcept>
#include <string>

namespace nvcomp
{

/******************************************************************************
 * CLASSES ********************************************************************
 *****************************************************************************/

/**
 * @brief The top-level exception throw by nvcomp C++ methods.
 */
class NVCompException : public std::runtime_error
{
public:
  /**
   * @brief Create a new NVCompException.
   *
   * @param err The error associated with the exception.
   * @param msg The error message.
   */
  NVCompException(nvcompError_t err, const std::string& msg) :
      std::runtime_error(msg + " : code=" + std::to_string(err) + "."),
      m_err(err)
  {
    // do nothing
  }

  nvcompError_t get_error() const
  {
    return m_err;
  }

private:
  nvcompError_t m_err;
};

/**
 * @brief Top-level compressor class. This class takes in data on the device,
 * and compresses it to another location on the device.
 */
class Compressor
{
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~Compressor() = default;

  virtual void
  configure(const size_t in_bytes, size_t* temp_bytes, size_t* out_bytes)
      = 0;

  /**
   * @brief Launch asynchronous compression. If the `out_bytes` is pageable
   * memory, this method will block.
   *
   * @param temp_ptr The temporary workspace on the device.
   * @param temp_bytes The size of the temporary workspace.
   * @param out_ptr The output location the the device (for compressed data).
   * @param out_bytes The size of the output location on the device on input,
   * and the size of the compressed data on output.
   * @param stream The stream to operate on.
   *
   * @throw NVCompException If compression fails to launch on the stream.
   */
  virtual void compress_async(
      const void* in_ptr,
      const size_t in_bytes,
      void* temp_ptr,
      const size_t temp_bytes,
      void* out_ptr,
      size_t* out_bytes,
      cudaStream_t stream)
      = 0;
};

/**
 * @brief Top-level decompress class. The compression type is read from the
 * metadata at the start of the compressed data.
 */
class Decompressor
{

public:
  virtual ~Decompressor() = default;

  virtual void configure(
      const void* in_ptr,
      const size_t in_bytes,
      size_t* temp_bytes,
      size_t* out_bytes,
      cudaStream_t stream)
      = 0;

  virtual void decompress_async(
      const void* in_ptr,
      const size_t in_bytes,
      void* temp_ptr,
      const size_t temp_bytes,
      void* out_ptr,
      const size_t out_bytes,
      cudaStream_t stream)
      = 0;
};

/******************************************************************************
 * INLINE DEFINITIONS AND HELPER FUNCTIONS ************************************
 *****************************************************************************/

template <typename T>
inline nvcompType_t getnvcompType()
{
  if (std::is_same<T, int8_t>::value) {
    return NVCOMP_TYPE_CHAR;
  } else if (std::is_same<T, uint8_t>::value) {
    return NVCOMP_TYPE_UCHAR;
  } else if (std::is_same<T, int16_t>::value) {
    return NVCOMP_TYPE_SHORT;
  } else if (std::is_same<T, uint16_t>::value) {
    return NVCOMP_TYPE_USHORT;
  } else if (std::is_same<T, int32_t>::value) {
    return NVCOMP_TYPE_INT;
  } else if (std::is_same<T, uint32_t>::value) {
    return NVCOMP_TYPE_UINT;
  } else if (std::is_same<T, int64_t>::value) {
    return NVCOMP_TYPE_LONGLONG;
  } else if (std::is_same<T, uint64_t>::value) {
    return NVCOMP_TYPE_ULONGLONG;
  } else {
    throw NVCompException(
        nvcompErrorNotSupported, "nvcomp does not support the given type.");
  }
}

inline void throwExceptionIfError(nvcompError_t error, const std::string& msg)
{
  if (error != nvcompSuccess) {
    throw NVCompException(error, msg);
  }
}


} // namespace nvcomp

#endif
