"""Schedule registries for annotation operators."""
from .._lib import _reg

_reg.register_injective_schedule("mnm.op.tvm.compiler_begin")
_reg.register_injective_schedule("mnm.op.tvm.compiler_end")
