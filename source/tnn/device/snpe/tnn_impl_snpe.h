// Tencent is pleased to support the open source community by making TNN available.
//
// Copyright (C) 2023 THL A29 Limited, a Tencent company. All rights reserved.
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

#ifndef TNN_SOURCE_TNN_DEVICE_SNPE_TNN_IMPL_SNPE_H_
#define TNN_SOURCE_TNN_DEVICE_SNPE_TNN_IMPL_SNPE_H_

#include "tnn/core/tnn_impl.h"

namespace TNN_NS {

// @brief tnn impl with interpreter
class TNNImplSnpe : public TNNImpl {
public:
    // @brief tnn constructor
    TNNImplSnpe();

    // @brief tnn destructor
    virtual ~TNNImplSnpe();

    // @brief init the tnn, contruct model interpreter
    // @param config config model type and params
    // @return status code: Successful, returns zero. Otherwise, returns
    // error code.
    virtual Status Init(ModelConfig& config);

    // @brief release model interpreter
    virtual Status DeInit();

    //@brief Adds output to the layer. If layerName not found, then search
    // outputIndex.
    //@param output_name Name of the output blob
    //@param output_index Index of the output layer
    //@return status code: If successful, returns zero. Otherwise, returns
    // error
    // code.
    virtual Status AddOutput(const std::string& output_name,
                             int output_index = 0);

    // return input shapes map from model
    virtual Status GetModelInputShapesMap(InputShapesMap& shapes_map);
    
    //@brief get input data types map from model
    virtual Status GetModelInputDataTypeMap(InputDataTypeMap& data_type_map);

    // return input names from model
    virtual Status GetModelInputNames(std::vector<std::string>& input_names);

    // return output names from model
    virtual Status GetModelOutputNames(std::vector<std::string>& output_names);

    // @brief create an instance
    // @param instance: The instance to be created.
    // @param inputs_shape: modify input shape, or it will use the shape in the
    // proto
    // @param inputs_data_type: modify input data type
    // @param status code: If successful, returns zero. Otherwise, returns
    // error code.
    virtual std::shared_ptr<Instance> CreateInst(NetworkConfig& config, Status& status,
                                                 InputShapesMap inputs_shape = InputShapesMap(),
                                                 InputDataTypeMap inputs_data_type = InputDataTypeMap());


    // @brief create an instance
    // @param instance: The instance to be created.
    // @param min_inputs_shape: support min shape
    // @param max_inputs_shape: support max shape
    // @param inputs_data_type: modify input data type
    // @param status code: If successful, returns zero. Otherwise, returns
    // error code.
    virtual std::shared_ptr<Instance> CreateInst(NetworkConfig& config, Status& status,
                                                 InputShapesMap min_inputs_shape, InputShapesMap max_inputs_shape,
                                                 InputDataTypeMap inputs_data_type = InputDataTypeMap());

private:
    std::shared_ptr<AbstractModelInterpreter> interpreter_;
};

}  // namespace TNN_NS
#endif  // TNN_SOURCE_TNN_DEVICE_SNPE_TNN_IMPL_SNPE_H_