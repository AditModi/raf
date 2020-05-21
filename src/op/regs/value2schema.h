/*!
 * Copyright (c) 2019 by Contributors
 * \file src/op/regs/value2schema.h
 * \brief Converters from values to MNM operator schemas
 */
#pragma once
#include <string>
#include <vector>
#include "mnm/value.h"
#include "./regs_utils.h"

namespace mnm {
namespace op {
namespace regs {
namespace value2schema {

#define MNM_PRELUDE_ALLOW_NULL() \
  using namespace mnm::value;    \
  using namespace mnm::ir;       \
  if (!a.defined()) {            \
    return {};                   \
  }

#define MNM_PRELUDE_DISALLOW_NULL(type)                                                       \
  using namespace mnm::value;                                                                 \
  using namespace mnm::ir;                                                                    \
  if (!a.defined()) {                                                                         \
    LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \""            \
               << a->GetTypeKey() << "\" is undefined (NULL), but is required to be of type " \
               << (type);                                                                     \
    throw;                                                                                    \
  }

inline value::Value ArrayLike(const value::Value& a) {
  MNM_PRELUDE_ALLOW_NULL();
  if (a->IsInstance<IntValueObj>() || a->IsInstance<FloatValueObj>() ||
      a->IsInstance<BoolValueObj>() || a->IsInstance<TensorValueObj>()) {
    return a;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is not array-like";
  throw;
}
inline value::TensorValue Tensor(const value::Value& a) {
  MNM_PRELUDE_ALLOW_NULL();
  if (const auto* v = a.as<TensorValueObj>()) {
    return GetRef<TensorValue>(v);
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is not a tensor";
  throw;
}
inline int64_t Int(const value::Value& a) {
  MNM_PRELUDE_DISALLOW_NULL("an integer");
  if (const auto* v = a.as<IntValueObj>()) {
    return v->data;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is not an integer";
  throw;
}
inline bool Bool(const value::Value& a) {
  MNM_PRELUDE_DISALLOW_NULL("boolean");
  if (const auto* v = a.as<BoolValueObj>()) {
    return v->data;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is not an integer";
  throw;
}
inline double Double(const value::Value& a) {
  MNM_PRELUDE_DISALLOW_NULL("double");
  if (const auto* v = a.as<FloatValueObj>()) {
    return v->data;
  }
  if (const auto* v = a.as<IntValueObj>()) {
    return v->data;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is double";
  throw;
}
inline std::string String(const value::Value& a) {
  MNM_PRELUDE_DISALLOW_NULL("string");
  if (const auto* v = a.as<StringValueObj>()) {
    return v->data;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is a string";
  throw;
}
inline std::vector<int64_t> TupleInt(const value::Value& a) {
  MNM_PRELUDE_DISALLOW_NULL("tuple of integers");
  if (const auto* v = a.as<TupleValueObj>()) {
    std::vector<int64_t> ret;
    ret.reserve(v->fields.size());
    for (const ObjectRef& i : v->fields) {
      if (const auto* e = i.as<IntValueObj>()) {
        ret.push_back(e->data);
        continue;
      }
      LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" is not tuple of integers, "
                 << "because the " << ToOrdinal(ret.size()) << " member is of type \""
                 << i->GetTypeKey() << '"';
      throw;
    }
    return ret;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is not tuple of integers";
  throw;
}
inline std::vector<int64_t> IntOrTupleInt(const value::Value& a) {
  MNM_PRELUDE_DISALLOW_NULL("an integer or tuple of integers");
  if (const auto* v = a.as<IntValueObj>()) {
    return {v->data};
  }
  if (const auto* v = a.as<TupleValueObj>()) {
    std::vector<int64_t> ret;
    ret.reserve(v->fields.size());
    for (const ObjectRef& i : v->fields) {
      if (const auto* e = i.as<IntValueObj>()) {
        ret.push_back(e->data);
        continue;
      }
      LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" is not an integer or "
                    "tuple of integers, because the "
                 << ToOrdinal(ret.size()) << " member is of type \"" << i->GetTypeKey() << '"';
      throw;
    }
    return ret;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is not an integer or tuple of integers";
  throw;
}
inline std::vector<value::TensorValue> TupleTensor(const value::Value& a) {
  MNM_PRELUDE_DISALLOW_NULL("tuple of tensors");
  if (const auto* v = a.as<TupleValueObj>()) {
    std::vector<TensorValue> ret;
    ret.reserve(v->fields.size());
    for (const ObjectRef& i : v->fields) {
      if (const auto* e = i.as<TensorValueObj>()) {
        ret.push_back(Downcast<TensorValue>(i));
        continue;
      }
      LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" is not tuple of tensors, "
                 << "because the " << ToOrdinal(ret.size()) << " member is of type \""
                 << i->GetTypeKey() << '"';
      throw;
    }
    return ret;
  }
  LOG(FATAL) << "TypeError: In operator \"{op}\", argument \"{arg}\" of type \"" << a->GetTypeKey()
             << "\" is not tuple of tensors";
  throw;
}

#undef MNM_PRELUDE_DISALLOW_NULL
#undef MNM_PRELUDE_ALLOW_NULL

}  // namespace value2schema
}  // namespace regs
}  // namespace op
}  // namespace mnm
