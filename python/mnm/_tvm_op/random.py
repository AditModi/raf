"""Compute definition and schedules for random operators"""
from .._lib import _reg
from .._lib import strategy

_reg.register_strategy("mnm.op.tvm.threefry_generate", strategy.threefry_generate_strategy)
_reg.register_strategy("mnm.op.tvm.threefry_split", strategy.threefry_split_strategy)
