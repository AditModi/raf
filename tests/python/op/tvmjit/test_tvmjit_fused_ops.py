import numpy as np
import pytest

import mnm
from mnm.testing import get_ctx_list, run_infer_type, run_vm_model


def ir_fuser(func):
    # pylint: disable=protected-access
    func = run_infer_type(func)
    func = mnm._ffi.pass_.FuseOps(func, 3)
    func = run_infer_type(func)
    return func


@pytest.mark.parametrize("ctx", get_ctx_list())
def test_avg_pool2d_dx_fuse_relu_dx(ctx):
    # pylint: disable=attribute-defined-outside-init, unnecessary-pass
    class AvgPoolDxReLUDx(mnm.Model):
        def build(self, y, dy, relu_x, relu_dy):
            self.y = y
            self.dy = dy
            self.relu_x = relu_x
            self.relu_dy = relu_dy
            pass

        @mnm.model.trace
        def forward(self, x):
            pooldx = mnm.avg_pool2d_dx(x, self.y, self.dy, kernel=3, stride=1, padding=0,
                                       dilation=1, ceil_mode=False, include_pad=True)
            out = mnm.relu_dx(pooldx, self.relu_x, self.relu_dy)
            return out

    x = mnm.array(np.random.randn(8, 3, 32, 32), dtype="float64", ctx=ctx)
    relu_x = mnm.relu(x)
    relu_dy = mnm.array(np.random.randn(*relu_x.shape), dtype="float64", ctx=ctx)
    y = mnm.avg_pool2d(relu_x, kernel=3, stride=1, padding=0,
                       dilation=1, ceil_mode=False, include_pad=True)
    dy = mnm.array(np.random.randn(*y.shape), dtype="float64", ctx=ctx)
    model = AvgPoolDxReLUDx(y, dy, relu_x, relu_dy)
    run_vm_model(model, ctx, [x], ir_fuser)


if __name__ == "__main__":
    pytest.main([__file__])
