# Inverse transformation with stepper wheel radius

Given measured string lengths $[u,v]$, the stepper distance $L$ and stepper wheel radius $R$, the problem is to recover gondola position $(x,y)$.

**Ideal inverse**

For the ideal case $R=0$, the transformation $T$ is

$$u=\sqrt{x^2+y^2}, \quad v=\sqrt{(L-x)^2+y^2}$$

The exact analytical inverse is

$$x=\frac{u^2-v^2+L^2}{2L}$$
$$y=\sqrt{u^2-x^2}$$

**Forward model with stepper wheel radius**

When the stepper wheel has a radius $R>0$, the string lengths are corrected by the tangential lift-off positions

$$u=\sqrt{x^2\cdot\left(1-\frac{R}{\sqrt{x^2+y^2}}\right)^2+y^2\cdot\left(1+\frac{R}{\sqrt{x^2+y^2}}\right)^2}$$

$$v=\sqrt{(L-x)^2\cdot\left(1-\frac{R}{\sqrt{(L-x)^2+y^2}}\right)^2+y^2\cdot\left(1+\frac{R}{\sqrt{(L-x)^2+y^2}}\right)^2}$$

This is a nonlinear transcendental system with no closed-form analytical inverse.

**Newton-Raphson solution**

To recover $(x,y)$ from measured $[u,v]$, we solve the nonlinear system

$$\mathbf{F}(x,y) = \begin{pmatrix} T_u(x,y,L,R) - u \\ T_v(x,y,L,R) - v \end{pmatrix} = \mathbf{0}$$

using the Newton-Raphson method

$$\begin{pmatrix} x_{k+1} \cr y_{k+1} \end{pmatrix} = \begin{pmatrix} x_k \cr y_k \end{pmatrix} - \mathbf{J}^{-1}(x_k, y_k) \cdot \mathbf{F}(x_k, y_k)$$

where the Jacobian is

$$\mathbf{J} = \Large\begin{pmatrix} \frac{\partial T_u}{\partial x} & \frac{\partial T_u}{\partial y} \cr \frac{\partial T_v}{\partial x} & \frac{\partial T_v}{\partial y} \end{pmatrix}$$

### Algorithm

**Step 1:** Initialize with ideal inverse

$$x_0 = \frac{u^2 - v^2 + L^2}{2L}, \quad y_0 = \sqrt{u^2 - x_0^2}$$

**Step 2:** For $k = 0, 1, 2, \dots$:

1. Evaluate forward model: $u_k = T_u(x_k, y_k, L, R)$, $v_k = T_v(x_k, y_k, L, R)$
2. Compute residual: $f_u = u_k - u$, $f_v = v_k - v$
3. Compute Jacobian (numerical differentiation with step $h \approx 10^{-8}$)
4. Solve: $\mathbf{J} \Delta\mathbf{x} = -\mathbf{F}$
5. Update: $(x_{k+1}, y_{k+1}) = (x_k, y_k) + \Delta\mathbf{x}$

**Step 3:** Stop when $\|\Delta\mathbf{x}\| < \text{tol}$ (typically $10^{-10}$)

## Convergence properties

- Quadratic convergence: Error decreases as $\|\mathbf{F}_{k+1}\| \approx C \|\mathbf{F}_k\|^2$
- Typical iterations: 3-5 iterations for convergence
- Accuracy: Machine precision (errors $\sim 10^{-15}$ or better)
- Robust: Works for all valid $R$ values and gondola positions

## Implementation notes

- Initial guess from ideal inverse is crucial for convergence
- Numerical Jacobian avoids complex symbolic differentiation
- Works for arrays (meshgrid) via element-wise operations
- See [Octave/MATLAB](../math/len2pos.m) implementation

[<- back](math.md)