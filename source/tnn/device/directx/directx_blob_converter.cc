// Tencent is pleased to support the open source community by making TNN available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "tnn/device/directx/directx_blob_converter.h"

#include "tnn/core/macro.h"
#include "tnn/core/blob_int8.h"
#include "tnn/utils/data_format_converter.h"
#include "tnn/utils/naive_compute.h"
#include "tnn/utils/string_utils_inner.h"
#include "tnn/device/directx/directx_device.h"
#include "tnn/device/directx/directx_util.h"
#include "tnn/device/directx/directx_common.h"
#include "tnn/device/directx/directx_runtime.h"

namespace TNN_NS {
namespace directx {

DirectXBlobConverterAcc::DirectXBlobConverterAcc(Blob *blob) : DefaultBlobConverterAcc(blob) {

#if TNN_PROFILE
    profiling_data = std::shared_ptr<DirectXProfilingData>(new DirectXProfilingData());
    profiling_data->Init();
    profiling_data->op_name    = "BlobConverter";
#endif
}

DirectXBlobConverterAcc::~DirectXBlobConverterAcc() { }

std::string DirectXBlobConverterAcc::GetUniqueBlobConvertKey(MatType mat_type, DataType data_type,
                                                         BlobConvertDirection cvt_dir) {
    return ToString(mat_type) + "_" + ToString(data_type) + "_" + ToString(cvt_dir);
}

std::map<std::string, DirectXBlobConvertFunc>& DirectXBlobConverterAcc::GetBlobConvertFuncMap() {
    static std::map<std::string, DirectXBlobConvertFunc> cvt_map;
    return cvt_map;
}

Status DirectXBlobConverterAcc::RegisterBlobConvertFunc(MatType mat_type, DataType data_type,
                                                    BlobConvertDirection cvt_dir, DirectXBlobConvertFunc cvt_func) {
    auto& cvt_map       = GetBlobConvertFuncMap();
    const auto& cvt_key = GetUniqueBlobConvertKey(mat_type, data_type, cvt_dir);
    cvt_map[cvt_key] = cvt_func;
    return TNN_OK;
}

Status DirectXBlobConverterAcc::GetBlobConvertFunc(MatType mat_type, DataType data_type,
                                               BlobConvertDirection cvt_dir, DirectXBlobConvertFunc& cvt_func) {
    const auto& cvt_map = GetBlobConvertFuncMap();
    const auto& cvt_key = GetUniqueBlobConvertKey(mat_type, data_type, cvt_dir);
    if (cvt_map.find(cvt_key) == cvt_map.end() || cvt_map.at(cvt_key) == nullptr) {
        LOGE("DirectXBlobConverterAcc::GetBlobConvertFunc, convert type not support yet. mat_type:%d data_type:%d cvt_dir:%d\n", mat_type, data_type, cvt_dir);
        return Status(TNNERR_PARAM_ERR, "DirectXBlobConverterAcc::GetBlobConvertFunc, convert type not support yet");
    }
    cvt_func = cvt_map.at(cvt_key);
    return TNN_OK;
}

Status DirectXBlobConverterAcc::ConvertToMatAsync(Mat &image, MatConvertParam param, void *command_queue) {
    Status ret = TNN_OK;
    if (blob_ == nullptr) {
        return Status(TNNERR_NULL_PARAM, "input/output blob is null");
    }


#if TNN_PROFILE
    profiling_data->layer_name = "ToMatAsync";
    profiling_data->input_dims = blob_->GetBlobDesc().dims;
    profiling_data->output_dims = image.GetDims();

    // context only accepts profilingdata after the StartProfile() call
    // DirectXProfilingResult will remove duplicated datas, on worry 
    std::shared_ptr<DirectXContext> tnn_context = nullptr;
    auto runtime = DirectXRuntime::GetInstance();
    RETURN_ON_NEQ(runtime->GetTNNContext(tnn_context), TNN_OK);
    tnn_context->AddProfilingData(profiling_data);

    profiling_data->Begin();
#endif

    ret = GetBlobConvertFunc(image.GetMatType(), DATA_TYPE_FLOAT, CVT_DIR_BLOB2MAT, cvt_func_);
    RETURN_ON_NEQ(ret, TNN_OK);

    ret = cvt_func_(image, blob_, param, command_queue);
    RETURN_ON_NEQ(ret, TNN_OK);

#if TNN_PROFILE
    profiling_data->End();
#endif

    return TNN_OK;
}

Status DirectXBlobConverterAcc::ConvertFromMatAsync(Mat &image, MatConvertParam param, void *command_queue) {
    Status ret = TNN_OK;
    if (blob_ == nullptr) {
        return Status(TNNERR_NULL_PARAM, "input/output blob is null");
    }

#if TNN_PROFILE
    profiling_data->layer_name = "FromMatAsync";
    profiling_data->input_dims = blob_->GetBlobDesc().dims;
    profiling_data->output_dims = image.GetDims();

    // context only accepts profilingdata after the StartProfile() call
    // DirectXProfilingResult will remove duplicated datas, on worry 
    std::shared_ptr<DirectXContext> tnn_context = nullptr;
    auto runtime = DirectXRuntime::GetInstance();
    RETURN_ON_NEQ(runtime->GetTNNContext(tnn_context), TNN_OK);
    tnn_context->AddProfilingData(profiling_data);

    profiling_data->Begin();
#endif

    ret = GetBlobConvertFunc(image.GetMatType(), DATA_TYPE_FLOAT, CVT_DIR_MAT2BLOB, cvt_func_);
    RETURN_ON_NEQ(ret, TNN_OK);

    ret = cvt_func_(image, blob_, param, command_queue);
    RETURN_ON_NEQ(ret, TNN_OK);

#if TNN_PROFILE
    profiling_data->End();
#endif

    return TNN_OK;
}

Status DirectXBlobConverterAcc::ConvertToMat(Mat &image, MatConvertParam param, void *command_queue) {
    auto ret = ConvertToMatAsync(image, param, command_queue);
    RETURN_ON_NEQ(ret, TNN_OK);
    // TODO: add synchronization
    return TNN_OK;
}

Status DirectXBlobConverterAcc::ConvertFromMat(Mat &image, MatConvertParam param, void *command_queue) {
    auto ret = ConvertFromMatAsync(image, param, command_queue);
    RETURN_ON_NEQ(ret, TNN_OK);
    // TODO: add synchronization
    return TNN_OK;
}

DECLARE_BLOB_CONVERTER_CREATER(DirectX);
REGISTER_BLOB_CONVERTER(DirectX, DEVICE_DIRECTX);

static Status NCHWToBlob(Mat& image,
                         Blob * blob,
                         const MatConvertParam& param,
                         void * command_queue) {

    auto tnn_device = dynamic_cast<DirectXDevice*>(GetDevice(DEVICE_DIRECTX));
    if (!tnn_device) {
        LOGE("Got null directx device");
        return Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    auto device = tnn_device->GetID3DDevice();
    if (!device) {
        LOGE("Got null ID3Ddevice");
        return  Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    std::shared_ptr<DirectXMemory> blob_memory;
    RETURN_ON_NEQ(DirectXMemoryManager::GetInstance()->GetRefMemoryFromBlob(blob, blob_memory), TNN_OK);
//    auto blob_memory = DirectXMemory::CreateRefMemoryFromBlob(blob);
    DimsVector dims = image.GetDims();

    shared_ptr<DirectXMemory> mat_buffer = DirectXMemory::CreateBufferMemoryFromHost(
        image.GetData(), dims, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW);
    if (!mat_buffer) {
        LOGE("param transfer to GPU failed.");
        return Status(TNNERR_DX_BUFFER_ALOCATE_ERR, "param transfer to GPU failed.");
    }

    auto mat_srv = mat_buffer->GetSRV();
    auto blob_uav = blob_memory->GetUAV();

    ParamCB param_cb_host = {param.scale[0], param.scale[1], param.scale[2], param.scale[3],
                             param.bias[0], param.bias[1],param.bias[2],param.bias[3],
                             DimsFunctionUtils::GetDim(dims, 0), DimsFunctionUtils::GetDim(dims, 1),
                             DimsFunctionUtils::GetDim(dims, 2), DimsFunctionUtils::GetDim(dims, 3)};

    std::shared_ptr<ID3D11Buffer> param_cb;
    Status status = CreateConstBuffer<ParamCB>(param_cb_host, device, param_cb);
    RETURN_ON_NEQ(status, TNN_OK);

//    LOGD("kernel name: NCHWToNHC4W4\n");
    std::shared_ptr<ID3D11ComputeShader> cs;
    status = GetShaderByName("NCHWToNHC4W4", cs);
    RETURN_ON_NEQ(status, TNN_OK);

    const int THREADS_PER_BLOCK = 128;
    const int ELE_PER_THREAD    = 4;

    int batch, channel, height, width;
    batch            = DimsFunctionUtils::GetDim(dims, 0);
    channel          = DimsFunctionUtils::GetDim(dims, 1);
    height           = DimsFunctionUtils::GetDim(dims, 2);
    width            = DimsFunctionUtils::GetDim(dims, 3);
    int image_width  = UP_DIV(channel, 4) * width;
    int image_height = batch * height;
    Status  ret = DispatchShader(cs, {mat_srv}, {blob_uav}, {param_cb.get()}, {UP_DIV(image_width, 4),UP_DIV(image_height, 4),1});

    return ret;
}

static Status N8UC3ToBlob(Mat& image,
                          Blob * blob,
                          const MatConvertParam& param,
                          void * command_queue) {

    auto tnn_device = dynamic_cast<DirectXDevice*>(GetDevice(DEVICE_DIRECTX));
    if (!tnn_device) {
        LOGE("Got null directx device");
        return Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    auto device = tnn_device->GetID3DDevice();
    if (!device) {
        LOGE("Got null ID3Ddevice");
        return  Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    auto blob_memory = DirectXMemory::CreateRefMemoryFromBlob(blob);
    DimsVector dims = image.GetDims();

    shared_ptr<DirectXMemory> mat_buffer = DirectXMemory::CreateBufferMemoryFromHost(
        image.GetData(), dims, DATA_TYPE_INT8, DATA_FORMAT_NCHW);
    if (!mat_buffer) {
        LOGE("param transfer to GPU failed.");
        return Status(TNNERR_DX_BUFFER_ALOCATE_ERR, "param transfer to GPU failed.");
    }

    auto mat_srv = mat_buffer->GetSRV();
    auto blob_uav = blob_memory->GetUAV();

    ParamCB param_cb_host = {param.scale[0], param.scale[1], param.scale[2], param.scale[3],
                             param.bias[0], param.bias[1],param.bias[2],param.bias[3],
                             DimsFunctionUtils::GetDim(dims, 0), DimsFunctionUtils::GetDim(dims, 1),
                             DimsFunctionUtils::GetDim(dims, 2), DimsFunctionUtils::GetDim(dims, 3)};

    std::shared_ptr<ID3D11Buffer> param_cb;
    Status status = CreateConstBuffer<ParamCB>(param_cb_host, device, param_cb);
    RETURN_ON_NEQ(status, TNN_OK);

    LOGD("kernel name: N8UC3ToNHC4W4\n");
    std::shared_ptr<ID3D11ComputeShader> cs;
    status = GetShaderByName("N8UC3ToNHC4W4", cs);
    RETURN_ON_NEQ(status, TNN_OK);

    const int THREADS_PER_BLOCK = 128;
    const int ELE_PER_THREAD    = 4;

    int batch, channel, height, width;
    batch            = DimsFunctionUtils::GetDim(dims, 0);
    channel          = DimsFunctionUtils::GetDim(dims, 1);
    height           = DimsFunctionUtils::GetDim(dims, 2);
    width            = DimsFunctionUtils::GetDim(dims, 3);
    int image_width  = UP_DIV(channel, 4) * width;
    int image_height = batch * height;
    Status  ret = DispatchShader(cs, {mat_srv}, {blob_uav}, {param_cb.get()}, {UP_DIV(image_width,4),image_height,1});

    return ret;
}

static Status N8UC4ToBlob(Mat& image,
                          Blob * blob,
                          const MatConvertParam& param,
                          void * command_queue) {

    auto tnn_device = dynamic_cast<DirectXDevice*>(GetDevice(DEVICE_DIRECTX));
    if (!tnn_device) {
        LOGE("Got null directx device");
        return Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    auto device = tnn_device->GetID3DDevice();
    if (!device) {
        LOGE("Got null ID3Ddevice");
        return  Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    auto blob_memory = DirectXMemory::CreateRefMemoryFromBlob(blob);
    DimsVector dims = image.GetDims();

    shared_ptr<DirectXMemory> mat_buffer = DirectXMemory::CreateBufferMemoryFromHost(
        image.GetData(), dims, DATA_TYPE_INT8, DATA_FORMAT_NCHW);
    if (!mat_buffer) {
        LOGE("param transfer to GPU failed.");
        return Status(TNNERR_DX_BUFFER_ALOCATE_ERR, "param transfer to GPU failed.");
    }

    auto mat_srv = mat_buffer->GetSRV();
    auto blob_uav = blob_memory->GetUAV();

    ParamCB param_cb_host = {param.scale[0], param.scale[1], param.scale[2], param.scale[3],
                             param.bias[0], param.bias[1],param.bias[2],param.bias[3],
                             DimsFunctionUtils::GetDim(dims, 0), DimsFunctionUtils::GetDim(dims, 1),
                             DimsFunctionUtils::GetDim(dims, 2), DimsFunctionUtils::GetDim(dims, 3)};

    std::shared_ptr<ID3D11Buffer> param_cb;
    Status status = CreateConstBuffer<ParamCB>(param_cb_host, device, param_cb);
    RETURN_ON_NEQ(status, TNN_OK);

    LOGD("kernel name: N8UC4ToNHC4W4\n");
    std::shared_ptr<ID3D11ComputeShader> cs;
    status = GetShaderByName("N8UC4ToNHC4W4", cs);
    RETURN_ON_NEQ(status, TNN_OK);

    const int THREADS_PER_BLOCK = 128;
    const int ELE_PER_THREAD    = 4;

    int batch, channel, height, width;
    batch            = DimsFunctionUtils::GetDim(dims, 0);
    channel          = DimsFunctionUtils::GetDim(dims, 1);
    height           = DimsFunctionUtils::GetDim(dims, 2);
    width            = DimsFunctionUtils::GetDim(dims, 3);
    int image_width  = UP_DIV(channel, 4) * width;
    int image_height = batch * height;
    Status  ret = DispatchShader(cs, {mat_srv}, {blob_uav}, {param_cb.get()}, {image_width,image_height,1});

    return ret;
}

static Status BlobToNCHW(Mat& image,
                         Blob * blob,
                         const MatConvertParam& param,
                         void * command_queue) {

    auto tnn_device = dynamic_cast<DirectXDevice*>(GetDevice(DEVICE_DIRECTX));
    if (!tnn_device) {
        LOGE("Got null directx device");
        return Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    auto device = tnn_device->GetID3DDevice();
    if (!device) {
        LOGE("Got null ID3Ddevice");
        return  Status(TNNERR_CONTEXT_ERR, "got null directx device");
    }

    std::shared_ptr<DirectXMemory> blob_memory;
    RETURN_ON_NEQ(DirectXMemoryManager::GetInstance()->GetRefMemoryFromBlob(blob, blob_memory), TNN_OK);
//    auto blob_memory = DirectXMemory::CreateRefMemoryFromBlob(blob);
    DimsVector dims = image.GetDims();

    shared_ptr<DirectXMemory> mat_buffer = DirectXMemory::CreateBufferMemoryFromHost(
        nullptr, dims, DATA_TYPE_FLOAT, DATA_FORMAT_NCHW);
    if (!mat_buffer) {
        LOGE("param transfer to GPU failed.");
        return Status(TNNERR_DX_BUFFER_ALOCATE_ERR, "param transfer to GPU failed.");
    }

    auto blob_srv = blob_memory->GetSRV();
    auto mat_uav = mat_buffer->GetUAV();

    ParamCB param_cb_host = {param.scale[0], param.scale[1], param.scale[2], param.scale[3],
                             param.bias[0], param.bias[1],param.bias[2],param.bias[3],
                             DimsFunctionUtils::GetDim(dims, 0), DimsFunctionUtils::GetDim(dims, 1),
                             DimsFunctionUtils::GetDim(dims, 2), DimsFunctionUtils::GetDim(dims, 3)};

    std::shared_ptr<ID3D11Buffer> param_cb;
    Status status = CreateConstBuffer<ParamCB>(param_cb_host, device, param_cb);
    RETURN_ON_NEQ(status, TNN_OK);

//    LOGD("kernel name: NHC4W4ToNCHW\n");
    std::shared_ptr<ID3D11ComputeShader> cs;
    status = GetShaderByName("NHC4W4ToNCHW", cs);
    RETURN_ON_NEQ(status, TNN_OK);

    const int THREADS_PER_BLOCK = 128;
    const int ELE_PER_THREAD    = 4;

    int batch, channel, height, width;
    batch            = DimsFunctionUtils::GetDim(dims, 0);
    channel          = DimsFunctionUtils::GetDim(dims, 1);
    height           = DimsFunctionUtils::GetDim(dims, 2);
    width            = DimsFunctionUtils::GetDim(dims, 3);
    int image_width  = UP_DIV(channel, 4) * width;
    int image_height = batch * height;
    Status  ret = DispatchShader(cs, {blob_srv}, {mat_uav}, {param_cb.get()}, {UP_DIV(image_width, 4),UP_DIV(image_height, 4),1});

    BlobHandle cpu_handle;
    cpu_handle.base = image.GetData();

    BlobHandle mat_handle;
    mat_handle.base = mat_buffer->GetData();

    tnn_device->CopyFromDevice(&cpu_handle, &mat_handle, blob->GetBlobDesc(), command_queue);

    return ret;
}

REGISTER_DIRECTX_BLOB_CONVERT_FUNC(NCHW_FLOAT, DATA_TYPE_FLOAT,  CVT_DIR_MAT2BLOB, NCHWToBlob)
REGISTER_DIRECTX_BLOB_CONVERT_FUNC(N8UC3, DATA_TYPE_FLOAT,  CVT_DIR_MAT2BLOB, N8UC3ToBlob)
REGISTER_DIRECTX_BLOB_CONVERT_FUNC(N8UC4, DATA_TYPE_FLOAT,  CVT_DIR_MAT2BLOB, N8UC4ToBlob)

REGISTER_DIRECTX_BLOB_CONVERT_FUNC(NCHW_FLOAT, DATA_TYPE_FLOAT,  CVT_DIR_BLOB2MAT, BlobToNCHW)

}
}  // namespace TNN_NS