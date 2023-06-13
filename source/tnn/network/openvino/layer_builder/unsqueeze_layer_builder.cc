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

#include "tnn/network/openvino/layer_builder/openvino_layer_builder.h"

namespace TNN_NS {

DECLARE_OPENVINO_LAYER_BUILDER(Unsqueeze, LAYER_UNSQUEEZE);

Status UnsqueezeOVLayerBuilder::Build() {
    auto paramlist  = dynamic_cast<UnsqueezeLayerParam *>(param_);
    auto input_node = GetInputNodes();

    auto axis_node     = std::make_shared<ngraph::op::Constant>(ngraph::element::Type_t::i32,
                                                            ngraph::Shape{paramlist->axes.size()}, paramlist->axes);
    auto unsqueezeNode = std::make_shared<ngraph::op::Unsqueeze>(input_node[0]->output(0), axis_node->output(0));

    unsqueezeNode->validate_and_infer_types();
    unsqueezeNode->set_friendly_name(param_->name);

    ngraph::NodeVector outputNodes;
    outputNodes.push_back(unsqueezeNode);
    SetOutputTensors(outputNodes);

    return TNN_OK;
}

REGISTER_OPENVINO_LAYER_BUILDER(Unsqueeze, LAYER_UNSQUEEZE);

}  // namespace TNN_NS
