/*!
 * Copyright (c) 2019 by Contributors
 * \file ./src/op/dispatch/tvmjit/transform.cc
 * \brief NN-related operators bridged from TVM.
 */
#include <tvm/relay/attrs/transform.h>
#include <mnm/value.h>
#include <array>
#include "./tvmjit_utils.h"
#include "./tvm_attrs.h"
#include "../../schema/transform.h"
#include "../../schema/nn.h"

namespace mnm {
namespace op {
namespace tvmjit {

using namespace mnm::ir;
using namespace mnm::value;
using namespace mnm::op::schema;
using namespace tvm;
using namespace ::tvm::relay;

Attrs TakeNormalizer(TVMOpEnv* env, const TakeArgs* args) {
  CHECK_EQ(env->outputs.size(), 1U);
  env->inputs.resize(2);
  env->inputs[0] = GetDLTensor(args->x);
  env->inputs[1] = GetDLTensor(args->indices);
  auto attrs = make_object<TakeAttrs>();
  if (args->axis.defined()) {
    const auto* v = args->axis.as<IntValueObj>();
    CHECK(v != nullptr);
    attrs->axis = v->data;
  } else {
    attrs->axis = NullValue<Integer>();
  }
  attrs->mode = "clip";
  return Attrs(attrs);
}

void TakeTyper(TVMOpEnv* env, std::vector<Type>* param_types, Type* y_type) {
  y_type[0] = GetTensorType(env->outputs[0]);
  *param_types = {GetTensorType(env->inputs[0]),
                  GetTensorType(env->inputs[1])};
}

HashKey TakeHasher(const std::vector<Type>& param_types,
                   const Type &y_type,
                   const TakeArgs* args) {
  HashKey key = GenericHasher<nullptr_t>(param_types, y_type, nullptr);
  if (args->axis.defined()) {
    const auto* v = args->axis.as<IntValueObj>();
    CHECK(v != nullptr);
    key << v->data;
  }
  return key;
}

MNM_TVMJIT(Take, "mnm.op.take", TakeArgs, TakeNormalizer, TakeTyper, TakeHasher);

Attrs SequenceMaskNormalizer(TVMOpEnv* env, const SequenceMaskArgs* args) {
  CHECK_EQ(env->outputs.size(), 1U);
  env->inputs.resize(2);
  env->inputs[0] = GetDLTensor(args->x);
  env->inputs[1] = GetDLTensor(args->sequence_length);
  auto attrs = make_object<SequenceMaskAttrs>();
  attrs->mask_value = args->mask_value;
  attrs->axis = args->axis;
  return Attrs(attrs);
}

void SequenceMaskTyper(TVMOpEnv* env, std::vector<Type>* param_types, Type* y_type) {
  y_type[0] = GetTensorType(env->outputs[0]);
  *param_types = {GetTensorType(env->inputs[0]),
                  GetTensorType(env->inputs[1])};
}

HashKey SequenceMaskHasher(const std::vector<Type>& param_types,
                           const Type &y_type,
                           const SequenceMaskArgs* args) {
  HashKey key = GenericHasher<nullptr_t>(param_types, y_type, nullptr);
  key << args->mask_value;
  key << args->axis;
  return key;
}

MNM_TVMJIT(SequenceMask, "mnm.op.sequence_mask", SequenceMaskArgs,
           SequenceMaskNormalizer, SequenceMaskTyper, SequenceMaskHasher);

Attrs BroadcastToNormalizer(TVMOpEnv* env, const BroadcastToArgs* args) {
  CHECK_EQ(env->outputs.size(), 1U);
  env->inputs.resize(1);
  env->inputs[0] = GetDLTensor(args->x);
  auto attrs = make_object<InitOpAttrs>();
  std::vector<IndexExpr> shape;
  shape.reserve(args->shape.size());
  for (size_t i = 0; i < args->shape.size(); ++i) {
    shape.emplace_back(IntImm(ir::DataType::Int(32), args->shape[i]));
  }
  attrs->shape = Array<relay::IndexExpr>(shape.begin(), shape.end());
  return Attrs(attrs);
}

void BroadcastToTyper(TVMOpEnv* env, std::vector<Type>* param_types, Type* y_type) {
  y_type[0] = GetTensorType(env->outputs[0]);
  *param_types = {GetTensorType(env->inputs[0])};
}

MNM_TVMJIT(BroadcastTo, "mnm.op.broadcast_to", BroadcastToArgs,
           BroadcastToNormalizer, BroadcastToTyper, GenericHasher);

Attrs TransposeNormalizer(TVMOpEnv* env, const TransposeArgs* args) {
  CHECK_EQ(env->outputs.size(), 1U);
  env->inputs.resize(1);
  env->inputs[0] = GetDLTensor(args->x);
  auto attrs = make_object<TransposeAttrs>();
  std::vector<Integer> axes;
  axes.reserve(args->axes.size());
  for (size_t i = 0; i < args->axes.size(); ++i) {
    axes.emplace_back(args->axes[i]);
  }
  attrs->axes = Array<Integer>(axes.begin(), axes.end());
  return Attrs(attrs);
}

void TransposeTyper(TVMOpEnv* env, std::vector<Type>* param_types, Type* y_type) {
  *y_type = GetTensorType(env->outputs[0]);
  *param_types = {GetTensorType(env->inputs[0])};
}

HashKey TransposeHasher(const std::vector<Type>& param_types,
                        const Type& y_type,
                        const TransposeArgs *args) {
  HashKey key = GenericHasher<nullptr_t>(param_types, y_type, nullptr);
  key << args->axes;
  return key;
}

MNM_TVMJIT(Transpose, "mnm.op.transpose", TransposeArgs, TransposeNormalizer,
           TransposeTyper, TransposeHasher);

Attrs TransposeDxNormalizer(TVMOpEnv* env, const TransposeDxArgs* args) {
  CHECK_EQ(env->outputs.size(), 1U);
  env->inputs.resize(3);
  env->inputs[0] = GetDLTensor(args->x);
  env->inputs[1] = GetDLTensor(args->y);
  env->inputs[2] = GetDLTensor(args->dy);
  auto attrs = make_object<TransposeAttrs>();
  std::vector<Integer> axes;
  axes.reserve(args->axes.size());
  for (size_t i = 0; i < args->axes.size(); ++i) {
    axes.emplace_back(args->axes[i]);
  }
  attrs->axes = Array<Integer>(axes.begin(), axes.end());
  return Attrs(attrs);
}

void TransposeDxTyper(TVMOpEnv* env, std::vector<Type>* param_types, Type* y_type) {
  *y_type = GetTensorType(env->outputs[0]);
  *param_types = {
      GetTensorType(env->inputs[0]),
      GetTensorType(env->inputs[1]),
      GetTensorType(env->inputs[2]),
  };
}

HashKey TransposeDxHasher(const std::vector<Type>& param_types,
                        const Type& y_type,
                        const TransposeDxArgs *args) {
  HashKey key = GenericHasher<nullptr_t>(param_types, y_type, nullptr);
  key << args->axes;
  return key;
}

MNM_TVMJIT(TransposeDx, "mnm.op.transpose_dx", TransposeDxArgs, TransposeDxNormalizer,
           TransposeDxTyper, TransposeDxHasher);

Attrs BroadcastToLikeNormalizer(TVMOpEnv* env,
                                const BroadcastToLikeArgs* args) {
  CHECK_EQ(env->outputs.size(), 1U);
  env->inputs.resize(2);
  env->inputs[0] = GetDLTensor(args->x);
  env->inputs[1] = GetDLTensor(args->broadcast_type);
  auto attrs = make_object<InitOpAttrs>();
  return Attrs(attrs);
}

Attrs SplitNormalizer(TVMOpEnv* env, const SplitArgs* args) {
  using namespace tvm;
  const std::vector<int64_t>& indices_or_sections = args->indices_or_sections;
  CHECK_EQ(env->outputs.size(), indices_or_sections.size() + 1);
  env->inputs.resize(1);
  env->inputs[0] = GetDLTensor(args->x);
  auto attrs = make_object<SplitAttrs>();
  attrs->indices_or_sections = Array<Integer>(indices_or_sections.begin(),
                                              indices_or_sections.end());
  attrs->axis = args->axis;
  return Attrs(attrs);
}

void SplitTyper(TVMOpEnv* env, std::vector<Type>* param_types, Type* y_type) {
  *y_type = GetTupleType(env->outputs);
  *param_types = {GetTensorType(env->inputs[0])};
}

HashKey SplitHasher(const std::vector<Type>& param_types,
                        const Type& y_type,
                        const SplitArgs *args) {
  HashKey key = GenericHasher<nullptr_t>(param_types, y_type, nullptr);
  key << args->axis;
  return key;
}

MNM_TVMJIT(Split, "mnm.op.split", SplitArgs, SplitNormalizer,
           SplitTyper, SplitHasher);

void BroadcastToLikeTyper(TVMOpEnv* env,
                          std::vector<Type>* param_types, Type* y_type) {
  y_type[0] = GetTensorType(env->outputs[0]);
  *param_types = {
      GetTensorType(env->inputs[0]),
      GetTensorType(env->inputs[1]),
  };
}

MNM_TVMJIT(BroadcastToLike, "mnm.op.broadcast_to_like", BroadcastToLikeArgs,
           BroadcastToLikeNormalizer, BroadcastToLikeTyper, GenericHasher);

Attrs ConcatenateNormalizer(TVMOpEnv* env, const ConcatenateArgs* args) {
  using namespace tvm;
  CHECK_EQ(env->outputs.size(), 1U);
  const std::vector<TensorValue>& x = args->x;
  env->inputs.resize(x.size());
  for (size_t i = 0; i < x.size(); ++i) {
    env->inputs[i] = GetDLTensor(x[i]);
  }
  auto attrs = make_object<ConcatenateAttrs>();
  attrs->axis = args->axis;
  return Attrs(attrs);
}

void ConcatenateTyper(TVMOpEnv* env, std::vector<Type>* param_types, Type* y_type) {
  y_type[0] = GetTensorType(env->outputs[0]);
  std::vector<Type> types;
  for (size_t i = 0; i < env->inputs.size(); ++i) {
    types.push_back(GetTensorType(env->inputs[i]));
  }
  *param_types = types;
}

HashKey ConcatenateHasher(const std::vector<Type>& param_types,
                     const Type &y_type,
                     const ConcatenateArgs* args) {
  HashKey key = GenericHasher<nullptr_t>(param_types, y_type, nullptr);
  key << args->axis;
  return key;
}

MNM_TVMJIT(Concatenate, "mnm.op.concatenate", ConcatenateArgs,
           ConcatenateNormalizer, ConcatenateTyper, ConcatenateHasher);


}  // namespace tvmjit
}  // namespace op
}  // namespace mnm
