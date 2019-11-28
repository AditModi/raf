/*!
 * Copyright (c) 2019 by Contributors
 * \file value.h
 * \brief Definition of MNM values
 */
#pragma once
#include <memory>
#include <vector>
#include <string>
#include "./ir.h"
#include "./tensor.h"

namespace mnm {
namespace op {
class OpEnv;
}  // namespace op
namespace executor {
class Executor;
}  // namespace executor
}  // namespace mnm

// Basic values used in tensor algebra
namespace mnm {
namespace value {

/* Value */
class ValueObj : public ir::Object {
 public:
  mutable std::shared_ptr<op::OpEnv> op_env{nullptr};
  static constexpr const char* _type_key = "mnm.value.Value";
  MNM_BASE_OBJECT(ValueObj, ir::Object);
};

class Value : public ir::ObjectRef {
 public:
  operator DLTensor*() const;
  operator tensor::Tensor&() const;
  template <typename TValue,
            typename = typename std::enable_if<std::is_base_of<Value, TValue>::value>::type>
  explicit operator TValue() const {
    return ir::Downcast<TValue>(*this);
  }
  MNM_OBJECT_REF(Value, ir::ObjectRef, ValueObj);
};

/* TensorValue */
class TensorValueObj final : public ValueObj {
 public:
  mutable tensor::Tensor tensor;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("_tensor", &tensor);
  }
  static constexpr const char* _type_key = "mnm.value.TensorValue";
  MNM_FINAL_OBJECT(TensorValueObj, ValueObj);
};

class TensorValue final : public Value {
 public:
  static TensorValue make(tensor::Tensor tensor);
  static TensorValue Assemble(const Context& ctx, const DType& dtype,
                              const std::vector<int64_t>& shape,
                              const std::vector<int64_t>& strides = {}, void* data = nullptr);
  MNM_OBJECT_REF(TensorValue, Value, TensorValueObj);
};

/* TupleValue */
class TupleValueObj final : public ValueObj {
 public:
  ir::Array<Value> fields;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("_fields", &fields);
  }
  static constexpr const char* _type_key = "mnm.value.TupleValue";
  MNM_FINAL_OBJECT(TupleValueObj, ValueObj);
};

class TupleValue final : public Value {
 public:
  static TupleValue make(ir::Array<Value> fields);
  MNM_OBJECT_REF(TupleValue, Value, TupleValueObj);
};

/* ClosureValue */
class ClosureValueObj final : public ValueObj {
 public:
  ir::Map<ir::Var, Value> env;
  ir::Function func;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("_env", &env);
    v->Visit("_func", &func);
  }
  static constexpr const char* _type_key = "mnm.value.ClosureValue";
  MNM_FINAL_OBJECT(ClosureValueObj, ValueObj);
};

class ClosureValue final : public Value {
 public:
  static ClosureValue make(ir::Map<ir::Var, Value> env, ir::Function func);
  MNM_OBJECT_REF(ClosureValue, Value, ClosureValueObj);
};

/* RefValue */
class RefValueObj final : public ValueObj {
 public:
  mutable Value value;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("_value", &value);
  }
  static constexpr const char* _type_key = "mnm.value.RefValue";
  MNM_FINAL_OBJECT(RefValueObj, ValueObj);
};

class RefValue final : public Value {
 public:
  static RefValue make(Value value);
  MNM_OBJECT_REF(RefValue, Value, RefValueObj);
};

/* OpValue */
class OpValueObj final : public ValueObj {
 public:
  ir::Op op;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("_op", &op);
  }
  static constexpr const char* _type_key = "mnm.value.OpValue";
  MNM_FINAL_OBJECT(OpValueObj, ValueObj);
};

class OpValue final : public Value {
 public:
  static OpValue make(ir::Op op);
  MNM_OBJECT_REF(OpValue, Value, OpValueObj);
};

/* ConstructorValue */
class ConstructorValueObj;
class ConstructorValue;

/* OpaqueValue */
class OpaqueValueObj : public ValueObj {
 public:
  mutable Value data{nullptr};
  static constexpr const char* _type_key = "mnm.value.OpaqueValue";
  MNM_FINAL_OBJECT(OpaqueValueObj, ValueObj);
};

class OpaqueValue : public Value {
 public:
  MNM_OBJECT_REF(OpaqueValue, Value, OpaqueValueObj);
};

}  // namespace value
}  // namespace mnm

// Scalar values

namespace mnm {
namespace value {

class IntValue;
class FloatValue;
class BoolValue;

/* ScalarValue */
class ScalarValueObj : public ValueObj {
 public:
  static constexpr const char* _type_key = "mnm.value.ScalarValue";
  MNM_BASE_OBJECT(ScalarValueObj, ValueObj);
};

class ScalarValue : public Value {
 public:
  static IntValue make(int data);
  static IntValue make(int64_t data);
  static FloatValue make(double data);
  static BoolValue make(bool data);
  MNM_OBJECT_REF(ScalarValue, Value, ScalarValueObj);
};

/* IntValue */
class IntValueObj : public ScalarValueObj {
 public:
  int64_t data;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("data", &data);
  }
  static constexpr const char* _type_key = "mnm.value.IntValue";
  MNM_FINAL_OBJECT(IntValueObj, ScalarValueObj);
};

class IntValue : public ScalarValue {
 public:
  static IntValue make(int64_t data);
  MNM_OBJECT_REF(IntValue, ScalarValue, IntValueObj);
};

/* FloatValue */
class FloatValueObj : public ScalarValueObj {
 public:
  double data;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("data", &data);
  }
  static constexpr const char* _type_key = "mnm.value.FloatValue";
  MNM_FINAL_OBJECT(FloatValueObj, ScalarValueObj);
};

class FloatValue : public ScalarValue {
 public:
  static FloatValue make(double data);
  MNM_OBJECT_REF(FloatValue, ScalarValue, FloatValueObj);
};

/* BoolValue */
class BoolValueObj : public ScalarValueObj {
 public:
  bool data;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("data", &data);
  }
  static constexpr const char* _type_key = "mnm.value.BoolValue";
  MNM_FINAL_OBJECT(BoolValueObj, ScalarValueObj);
};

class BoolValue : public ScalarValue {
 public:
  static BoolValue make(bool data);
  MNM_OBJECT_REF(BoolValue, ScalarValue, BoolValueObj);
};

/* StringValue */
class StringValueObj : public ValueObj {
 public:
  std::string data;
  void VisitAttrs(tvm::AttrVisitor* v) {
    v->Visit("data", &data);
  }
  static constexpr const char* _type_key = "mnm.value.StringValue";
  MNM_FINAL_OBJECT(StringValueObj, ValueObj);
};

class StringValue : public Value {
 public:
  static StringValue make(const std::string& data);
  MNM_OBJECT_REF(StringValue, Value, StringValueObj);
};

}  // namespace value
}  // namespace mnm

namespace mnm {
namespace value {
ir::Var BindNothing(const std::string& name_hint = "");
ir::Var BindValue(const Value& value, const std::string& name_hint = "");
ir::Var BindExpr(const ir::Expr& expr, const std::string& name_hint = "");
ir::Var BindExprValue(const ir::Expr& expr, const Value& value, const std::string& name_hint = "");
ir::Expr LookupBoundExpr(const ir::Var& var);
Value LookupBoundValue(const ir::Var& var);
ir::Expr ExtractLetList(const ir::Var& var);
}  // namespace value
}  // namespace mnm
