# pylint: disable=line-too-long
"""Operator-specific type hints for automatic mixed precision.

The type hint function for each operator should return a list of type hints,
and the list size should be the same as the argument number plus one (i.e., output).
Here are some type hint examples:

- PrimType("float32"): The argument must be in float32.
- PrimType("float16"): The argument should be in the specified AMP dtype (float16 in this case).
- PrimType(None): Do not change the dtype of this argument. It means if the argument has been
    casted to the AMP dtype, we need to cast it back.

The current list is based on TF:
github.com/tensorflow/tensorflow/blob/v2.5.0/tensorflow/core/grappler/optimizers/auto_mixed_precision_lists.h

Note that since the infer list is the majority, we make it as the default behavior
and do not need to specify them here.

TODO(@comaniac): We need to consider the accumulation dtype, which may be different to the output
dtype. However, we need to make sure the ops will use the desired dtype for accumulation in advance.
"""
# pylint: enable=line-too-long
# pylint: disable=unused-argument
from mnm._lib import tvm
from tvm import relay
from tvm.ir import PrimType, TupleType


def register_op_cast_rule(op_name, cast_rule=None, level=10):
    """Register a custom casting rule function for an op.

    Parameters
    ----------
    op_name : str
        The name of the operator

    cast_rule: Callable[[List[Expr], Type], List[Type]]
        The function for providing cast hint type. The last hint type is the return type.

    level : int
        The priority level
    """
    return tvm.ir.register_op_attr(op_name, "FMNMCastRule", cast_rule, level)


def gen_hint_helper(etype, cast_to_amp, amp_dtype):
    """A helper function to generate a type hint for the given type."""
    if isinstance(etype, tvm.ir.TensorType):
        return PrimType(amp_dtype) if cast_to_amp else PrimType(None)
    if isinstance(etype, tvm.ir.TupleType):
        return TupleType([gen_hint_helper(field, cast_to_amp, amp_dtype) for field in etype.fields])
    raise ValueError("Unsupported input type: %s" % str(etype))


def check_amp_dtype(ttype, amp_dtype):
    """Check whether the given type has AMP dtype."""
    if isinstance(ttype, tvm.ir.TupleType):
        return all([check_amp_dtype(field, amp_dtype) for field in ttype.fields])
    assert isinstance(ttype, tvm.ir.TensorType)
    return ttype.dtype == amp_dtype

def generic_cast(cast_to_amp, input_num):
    """The generic cast function that generates AMP type hints for inputs, and generates
    don't touch type hints for rest arguments.
    """
    def _gen(args, ret_type, amp_dtype):
        ret = [gen_hint_helper(arg.checked_type, cast_to_amp, amp_dtype)
               for arg in args[:input_num]]
        ret += [PrimType(None) for _ in range(len(args) - input_num)]
        ret += [gen_hint_helper(ret_type, cast_to_amp, amp_dtype)]
        return ret

    return _gen

# Always cast.
register_op_cast_rule("mnm.op.conv2d", generic_cast(True, 2))
register_op_cast_rule("mnm.op.conv2d_dx", generic_cast(True, 3))
register_op_cast_rule("mnm.op.conv2d_dw", generic_cast(True, 3))
register_op_cast_rule("mnm.op.conv2d_transpose", generic_cast(True, 2))
register_op_cast_rule("mnm.op.conv2d_transpose_dx", generic_cast(True, 3))
register_op_cast_rule("mnm.op.conv2d_transpose_dw", generic_cast(True, 3))
register_op_cast_rule("mnm.op.matmul", generic_cast(True, 2))
register_op_cast_rule("mnm.op.dense", generic_cast(True, 2))
register_op_cast_rule("mnm.op.matmul_nt", generic_cast(True, 2))
register_op_cast_rule("mnm.op.matmul_tn", generic_cast(True, 2))
register_op_cast_rule("mnm.op.matmul_tt", generic_cast(True, 2))
register_op_cast_rule("mnm.op.batch_matmul", generic_cast(True, 2))
register_op_cast_rule("mnm.op.batch_matmul_nt", generic_cast(True, 2))
register_op_cast_rule("mnm.op.batch_matmul_tn", generic_cast(True, 2))
register_op_cast_rule("mnm.op.batch_matmul_tt", generic_cast(True, 2))

# Never cast.
register_op_cast_rule("mnm.op.arange", generic_cast(False, 3))
register_op_cast_rule("mnm.op.exp", generic_cast(False, 1))
register_op_cast_rule("mnm.op.power", generic_cast(False, 1))
register_op_cast_rule("mnm.op.softmax", generic_cast(False, 1))
register_op_cast_rule("mnm.op.softmax_dx", generic_cast(False, 2))
register_op_cast_rule("mnm.op.log_softmax", generic_cast(False, 1))
register_op_cast_rule("mnm.op.log_softmax_dx", generic_cast(False, 2))
register_op_cast_rule("mnm.op.erf", generic_cast(False, 1))
register_op_cast_rule("mnm.op.erf_dx", generic_cast(False, 3))
register_op_cast_rule("mnm.op.gelu", generic_cast(False, 1))
register_op_cast_rule("mnm.op.gelu_dx", generic_cast(False, 3))
register_op_cast_rule("mnm.op.smooth_l1_loss", generic_cast(False, 2))
register_op_cast_rule("mnm.op.smooth_l1_loss_dpred", generic_cast(False, 2))
register_op_cast_rule("mnm.op.smooth_l1_loss_dtrue", generic_cast(False, 2))
register_op_cast_rule("mnm.op.nll_loss", generic_cast(False, 2))
register_op_cast_rule("mnm.op.nll_loss_dpred", generic_cast(False, 3))
register_op_cast_rule("mnm.op.nll_loss_dtrue", generic_cast(False, 3))
register_op_cast_rule("mnm.op.cross_entropy", generic_cast(False, 2))
register_op_cast_rule("mnm.op.cross_entropy_dpred", generic_cast(False, 2))
register_op_cast_rule("mnm.op.cross_entropy_dtrue", generic_cast(False, 2))

# embedding_dx has accuracy issue and its performance does not improve significantly
# over float32, so never cast.
register_op_cast_rule("mnm.op.embedding_dx", generic_cast(False, 2))

# FIXME: These ops should support float16, but the current TVM code results in
# either runtime error or mismatch outputs.
register_op_cast_rule("mnm.op.atan", generic_cast(False, 1))
register_op_cast_rule("mnm.op.tanh", generic_cast(False, 1))
register_op_cast_rule("mnm.op.tanh_dx", generic_cast(False, 3))
register_op_cast_rule("mnm.op.rsqrt", generic_cast(False, 1))

# These ops needs to accumulate the result in float32, so we never cast them,
# and expect they will be fused with the cast ops.
register_op_cast_rule("mnm.op.take_dx", generic_cast(False, 3))
register_op_cast_rule("mnm.op.gather_dx", generic_cast(False, 4))
register_op_cast_rule("mnm.op.gather_nd", generic_cast(False, 1))
register_op_cast_rule("mnm.op.gather_nd_dx", generic_cast(False, 3))


def infer_cast(input_num):
    """The cast rule is inferred by the dtype of current arguments and output:
    1. If the original output dtype is not float32 (e.g., int32/bool/tuple), then do not touch.
    2. If some arguments are casted to the AMP dtype, then cast all arguments to the AMP dtype.
    3. Otherwise keep all arguments untouched.
    """
    def _gen(args, ret_type, amp_dtype):
        cast_to_amp = not isinstance(ret_type, tvm.ir.TupleType) and ret_type.dtype == "float32"
        if cast_to_amp:
            cast_to_amp = any([check_amp_dtype(arg.checked_type, amp_dtype)
                               for arg in args[:input_num]])

        ret = []
        for arg in args[:input_num]:
            ret.append(gen_hint_helper(arg.checked_type, cast_to_amp, amp_dtype))
        ret += [PrimType(None) for _ in range(len(args) - input_num)]
        ret += [gen_hint_helper(ret_type, cast_to_amp, amp_dtype)]
        return ret

    return _gen

# Infer cast.
register_op_cast_rule("mnm.op.max_pool2d", infer_cast(1))
register_op_cast_rule("mnm.op.avg_pool2d", infer_cast(1))
register_op_cast_rule("mnm.op.adaptive_max_pool2d", infer_cast(1))
register_op_cast_rule("mnm.op.adaptive_avg_pool2d", infer_cast(1))
register_op_cast_rule("mnm.op.pad", infer_cast(1))
register_op_cast_rule("mnm.op.max_pool2d_dx", infer_cast(3))
register_op_cast_rule("mnm.op.avg_pool2d_dx", infer_cast(3))
register_op_cast_rule("mnm.op.adaptive_max_pool2d_dx", infer_cast(3))
register_op_cast_rule("mnm.op.adaptive_avg_pool2d_dx", infer_cast(3))
register_op_cast_rule("mnm.op.batch_flatten", infer_cast(1))
register_op_cast_rule("mnm.op.negative", infer_cast(1))
register_op_cast_rule("mnm.op.logical_not", infer_cast(1))
register_op_cast_rule("mnm.op.relu", infer_cast(1))
register_op_cast_rule("mnm.op.copy", infer_cast(1))
register_op_cast_rule("mnm.op.abs", infer_cast(1))
register_op_cast_rule("mnm.op.all", infer_cast(1))
register_op_cast_rule("mnm.op.any", infer_cast(1))
register_op_cast_rule("mnm.op.ceil", infer_cast(1))
register_op_cast_rule("mnm.op.cos", infer_cast(1))
register_op_cast_rule("mnm.op.sin", infer_cast(1))
register_op_cast_rule("mnm.op.sign", infer_cast(1))
register_op_cast_rule("mnm.op.round", infer_cast(1))
register_op_cast_rule("mnm.op.floor", infer_cast(1))
register_op_cast_rule("mnm.op.log", infer_cast(1))
register_op_cast_rule("mnm.op.log2", infer_cast(1))
register_op_cast_rule("mnm.op.sigmoid", infer_cast(1))
register_op_cast_rule("mnm.op.sqrt", infer_cast(1))
register_op_cast_rule("mnm.op.relu_dx", infer_cast(3))
register_op_cast_rule("mnm.op.sigmoid_dx", infer_cast(3))
register_op_cast_rule("mnm.op.sqrt_dx", infer_cast(3))
register_op_cast_rule("mnm.op.floor_divide", infer_cast(2))
register_op_cast_rule("mnm.op.mod", infer_cast(2))
register_op_cast_rule("mnm.op.less", infer_cast(2))
register_op_cast_rule("mnm.op.greater", infer_cast(2))
register_op_cast_rule("mnm.op.less_equal", infer_cast(2))
register_op_cast_rule("mnm.op.greater_equal", infer_cast(2))
register_op_cast_rule("mnm.op.equal", infer_cast(2))
register_op_cast_rule("mnm.op.not_equal", infer_cast(2))
register_op_cast_rule("mnm.op.maximum", infer_cast(2))
register_op_cast_rule("mnm.op.minimum", infer_cast(2))
register_op_cast_rule("mnm.op.right_shift", infer_cast(2))
register_op_cast_rule("mnm.op.left_shift", infer_cast(2))
register_op_cast_rule("mnm.op.trunc", infer_cast(2))
register_op_cast_rule("mnm.op.mesh_grid", infer_cast(2))
register_op_cast_rule("mnm.op.reshape", infer_cast(1))
register_op_cast_rule("mnm.op.resize", infer_cast(1))
register_op_cast_rule("mnm.op.ndarray_size", infer_cast(1))
register_op_cast_rule("mnm.op.transpose", infer_cast(1))
register_op_cast_rule("mnm.op.transpose_dx", infer_cast(1))
register_op_cast_rule("mnm.op.collapse_sum_like", infer_cast(1))
register_op_cast_rule("mnm.op.sum", infer_cast(1))
register_op_cast_rule("mnm.op.sum_dx", infer_cast(3))
register_op_cast_rule("mnm.op.argmax", infer_cast(1))
register_op_cast_rule("mnm.op.argmin", infer_cast(1))
register_op_cast_rule("mnm.op.prod", infer_cast(1))
register_op_cast_rule("mnm.op.prod_dx", infer_cast(3))
register_op_cast_rule("mnm.op.max", infer_cast(1))
register_op_cast_rule("mnm.op.min", infer_cast(1))
register_op_cast_rule("mnm.op.mean", infer_cast(1))
register_op_cast_rule("mnm.op.mean_dx", infer_cast(2))
register_op_cast_rule("mnm.op.get_reduce_axis", infer_cast(2))
register_op_cast_rule("mnm.op.get_kept_dims", infer_cast(2))
register_op_cast_rule("mnm.op.sgd", infer_cast(1))
register_op_cast_rule("mnm.op.shape", infer_cast(1))
register_op_cast_rule("mnm.op.swap_axis", infer_cast(1))
register_op_cast_rule("mnm.op.take", infer_cast(1))
register_op_cast_rule("mnm.op.repeat", infer_cast(1))
register_op_cast_rule("mnm.op.repeat_dx", infer_cast(3))
register_op_cast_rule("mnm.op.expand_dims", infer_cast(1))
register_op_cast_rule("mnm.op.threefry_generate", infer_cast(1))
register_op_cast_rule("mnm.op.threefry_split", infer_cast(1))
register_op_cast_rule("mnm.op.strided_slice", infer_cast(1))
register_op_cast_rule("mnm.op.strided_slice_dx", infer_cast(1))
register_op_cast_rule("mnm.op.sequence_mask", infer_cast(1))
register_op_cast_rule("mnm.op.reverse_sequence", infer_cast(1))
register_op_cast_rule("mnm.op.reverse", infer_cast(1))
register_op_cast_rule("mnm.op.broadcast_to", infer_cast(1))
register_op_cast_rule("mnm.op.broadcast_to_like", infer_cast(1))
register_op_cast_rule("mnm.op.concatenate", infer_cast(1))
register_op_cast_rule("mnm.op.squeeze", infer_cast(1))
register_op_cast_rule("mnm.op.stack", infer_cast(1))
register_op_cast_rule("mnm.op.split", infer_cast(1))
register_op_cast_rule("mnm.op.scatter", infer_cast(1))
register_op_cast_rule("mnm.op.scatter_dx", infer_cast(3))
register_op_cast_rule("mnm.op.concatenate_dx", infer_cast(1))
register_op_cast_rule("mnm.op.clip", infer_cast(1))
register_op_cast_rule("mnm.op.clip_dx", infer_cast(2))
register_op_cast_rule("mnm.op.get_valid_counts", infer_cast(1))
register_op_cast_rule("mnm.op.bias_add", infer_cast(2))
register_op_cast_rule("mnm.op._contrib_dropout", infer_cast(1))
register_op_cast_rule("mnm.op._contrib_dropout_dx", infer_cast(1))
register_op_cast_rule("mnm.op.non_max_suppression", infer_cast(1))
register_op_cast_rule("mnm.op._allreduce", infer_cast(1))
register_op_cast_rule("mnm.op._allgather", infer_cast(1))
register_op_cast_rule("mnm.op._reduce_scatter", infer_cast(1))
register_op_cast_rule("mnm.op.cast", infer_cast(1))
register_op_cast_rule("mnm.op.cast_like", infer_cast(1))
register_op_cast_rule("mnm.op.argsort", infer_cast(1))
register_op_cast_rule("mnm.op.sort", infer_cast(1))
register_op_cast_rule("mnm.op.full", infer_cast(0))
register_op_cast_rule("mnm.op.full_like", infer_cast(1))
register_op_cast_rule("mnm.op.where", infer_cast(3))
register_op_cast_rule("mnm.op.logical_and", infer_cast(2))
register_op_cast_rule("mnm.op.topk", infer_cast(1))
register_op_cast_rule("mnm.op.zeros", infer_cast(0))
register_op_cast_rule("mnm.op.zeros_like", infer_cast(1))
register_op_cast_rule("mnm.op.ones", infer_cast(0))
register_op_cast_rule("mnm.op.ones_like", infer_cast(1))
register_op_cast_rule("mnm.op.one_hot", infer_cast(0))
register_op_cast_rule("mnm.op.argwhere", infer_cast(1))
register_op_cast_rule("mnm.op.upper_bound.argwhere", infer_cast(1))
register_op_cast_rule("mnm.op.roi_align", infer_cast(2))
register_op_cast_rule("mnm.op.roi_align_dx", infer_cast(2))
register_op_cast_rule("mnm.op.layer_norm", infer_cast(3))
register_op_cast_rule("mnm.op.layer_norm_dx", infer_cast(3))
register_op_cast_rule("mnm.op.embedding", infer_cast(2))
register_op_cast_rule("mnm.op.gather", infer_cast(1))
register_op_cast_rule("mnm.op.multiply", infer_cast(2))
register_op_cast_rule("mnm.op.divide", infer_cast(2))

# Special cases.

def op_cast_binary_ufunc(args, ret_type, amp_dtype):
    """The 3rd and 4th arguments of binary ufunc scheme are out and where, which may be
    nullptr that cannot be casted. On the other hand, if they are not nullptr (constant node),
    then we need to treat them as the first two arguments.
    """
    cast_to_amp = not isinstance(ret_type, tvm.ir.TupleType) and ret_type.dtype == "float32"
    if cast_to_amp:
        cast_to_amp = any([check_amp_dtype(arg.checked_type, amp_dtype)
                           for arg in args[:3]])

    ret = []
    for arg in args[:2]:
        ret.append(gen_hint_helper(arg.checked_type, cast_to_amp, amp_dtype))
    for arg in args[2:]:
        if isinstance(arg, relay.Constant):
            ret.append(PrimType(None))
        else:
            ret.append(gen_hint_helper(arg.checked_type, cast_to_amp, amp_dtype))
    ret += [gen_hint_helper(ret_type, cast_to_amp, amp_dtype)]
    return ret

register_op_cast_rule("mnm.op.add", op_cast_binary_ufunc)
register_op_cast_rule("mnm.op.subtract", op_cast_binary_ufunc)

def op_cast_adv_index(args, ret_type, amp_dtype):
    """adv_index/adv_index_dx are the only ops that need to take a tuple with different dtypes
    as an input.
    """
    return [TupleType([PrimType(amp_dtype), PrimType(None), PrimType(None)]), PrimType(amp_dtype)]

register_op_cast_rule("mnm.op.adv_index", op_cast_adv_index)

def op_cast_adv_index_dx(args, ret_type, amp_dtype):
    """adv_index/adv_index_dx are the only ops that need to take a tuple with different dtypes
    as an input.
    """
    return [PrimType(amp_dtype), TupleType([PrimType(amp_dtype), PrimType(None), PrimType(None)]),
            TupleType([PrimType(amp_dtype)])]

register_op_cast_rule("mnm.op.adv_index_dx", op_cast_adv_index_dx)

def op_cast_norm(data_num, out_num):
    """Scale/bias tensors of normalization layers have to be in float32."""

    def _gen_rules(args, ret_type, amp_dtype):
        ret = [PrimType(amp_dtype) for _ in range(data_num)]
        ret += [PrimType(None) for _ in range(len(args) - data_num)]
        if out_num == 1:
            ret.append(PrimType(amp_dtype))
        else:
            assert out_num == 3
            ret += [TupleType([PrimType(amp_dtype), PrimType("float32"), PrimType("float32")])]
        return ret

    return _gen_rules


register_op_cast_rule("mnm.op.batch_norm_infer", op_cast_norm(1, 1))
register_op_cast_rule("mnm.op.batch_norm_train", op_cast_norm(1, 3))

# TODO(@comaniac): batch_norm_train_dxwb produces different results as PyTorch BatchNorm backward
# and we have not figured out the reason, so currently we limit batch_norm_train_dxwb to be
# executed with float32 to guarantee the correctness.
register_op_cast_rule("mnm.op.batch_norm_train_dxwb", generic_cast(False, 2))
