// Tencent is pleased to support the open source community by making TNN available.
//
// Copyright (C) 2022 THL A29 Limited, a Tencent company. All rights reserved.
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

#include <cmath>
#include "cpu_binary_op_layer_acc.h"
#include "tnn/core/blob_int8.h"
#include "tnn/utils/naive_compute.h"

namespace TNN_NS {

DECLARE_CPU_BINARY_OP_ACC(Mod, LAYER_MOD);

Status CpuModLayerAcc::Calculate(const std::vector<Blob *> &input_blobs, const std::vector<void *> &input_ptrs,
                                 const std::vector<DimsVector> &input_shapes, Blob *output) {
    void *output_data       = output->GetHandle().base;
    const auto &output_dims = output->GetBlobDesc().dims;
    auto layer_res = dynamic_cast<EltwiseLayerResource *>(resource_);

    if ((input_blobs.size()==2 && input_blobs[0]->GetBlobDesc().data_type != input_blobs[1]->GetBlobDesc().data_type) ||
        (input_blobs.size()==1 && layer_res && layer_res->element_handle.GetDataType()!=input_blobs[0]->GetBlobDesc().data_type)) {
        DataType in0_dtype = input_blobs[0]->GetBlobDesc().data_type;
        DataType in1_dtype = input_blobs.size()==2 ? input_blobs[1]->GetBlobDesc().data_type : layer_res->element_handle.GetDataType();
 
        if (in0_dtype==DATA_TYPE_FLOAT && in1_dtype==DATA_TYPE_HALF) {
            CPU_ELEMENT_WISE_BINARY_TYPECAST<float, fp16_t, float>(input_ptrs, input_shapes, output_data, output_dims,
                                    [](float a, fp16_t b) -> float { return std::fmod(a, float(b)); });
        } else if (in0_dtype==DATA_TYPE_HALF && in1_dtype==DATA_TYPE_FLOAT) {
            CPU_ELEMENT_WISE_BINARY_TYPECAST<fp16_t, float, float>(input_ptrs, input_shapes, output_data, output_dims,
                                    [](fp16_t a, float b) -> float { return std::fmod(float(a), b); });
        } else if (in0_dtype==DATA_TYPE_FLOAT && in1_dtype==DATA_TYPE_INT32) {
            CPU_ELEMENT_WISE_BINARY_TYPECAST<float, int, float>(input_ptrs, input_shapes, output_data, output_dims,
                                    [](float a, int b) -> float { return std::fmod(a, float(b)); });
        } else if (in0_dtype==DATA_TYPE_INT32 && in1_dtype==DATA_TYPE_FLOAT) {
            CPU_ELEMENT_WISE_BINARY_TYPECAST<int, float, float>(input_ptrs, input_shapes, output_data, output_dims,
                                    [](int a, float b) -> float { return std::fmod(float(a), b); });
        } else if (in0_dtype==DATA_TYPE_HALF && in1_dtype==DATA_TYPE_INT32) {
            CPU_ELEMENT_WISE_BINARY_TYPECAST<fp16_t, int, fp16_t>(input_ptrs, input_shapes, output_data, output_dims,
                                    [](fp16_t a, int b) -> fp16_t { return fp16_t(std::fmod(a, float(b))); });
        } else if (in0_dtype==DATA_TYPE_INT32 && in1_dtype==DATA_TYPE_HALF) {
            CPU_ELEMENT_WISE_BINARY_TYPECAST<int, fp16_t, fp16_t>(input_ptrs, input_shapes, output_data, output_dims,
                                    [](int a, fp16_t b) -> fp16_t { return fp16_t(std::fmod(float(a), b)); });
        } else {
            LOGE("Error: CpuModLayerAcc don't support in0.type: %d and in1.type: %d\n", in0_dtype, in1_dtype);
            return Status(TNNERR_MODEL_ERR, "CpuModLayerAcc don't support in0, in1 data type combination");
        }
    } else {
        if (output->GetBlobDesc().data_type == DATA_TYPE_FLOAT) {
            CPU_ELEMENT_WISE<float, float>(input_ptrs, input_shapes, output_data, output_dims, [](float a, float b) -> float { return std::fmod(a,b); });
        } else if (output->GetBlobDesc().data_type == DATA_TYPE_INT32) {
            CPU_ELEMENT_WISE<fp16_t, fp16_t>(input_ptrs, input_shapes, output_data, output_dims, [](fp16_t a, fp16_t b) -> fp16_t { return fp16_t(std::fmod(float(a),float(b))); });
        } else if (output->GetBlobDesc().data_type == DATA_TYPE_INT32) {
            CPU_ELEMENT_WISE<int, int>(input_ptrs, input_shapes, output_data, output_dims, [](int a, int b) -> int { return a % b; });
        } else {
            LOGE("Error: CpuModLayerAcc don't support in0 data type: %d and in1 data type: %d\n", input_blobs[0]->GetBlobDesc().data_type, input_blobs[1]->GetBlobDesc().data_type);
            return Status(TNNERR_MODEL_ERR, "Error: CpuModLayerAcc don't support input data type combinations.");
        }
    }

    return TNN_OK;
}

REGISTER_CPU_ACC(Mod, LAYER_MOD);

}  // namespace TNN_NS