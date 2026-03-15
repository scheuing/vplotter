function [u, v] = pos2len(x, y, L, varargin)
% function [u, v] = pos2len(x, y, L, R, q)
%   transform gondola position (x,y) into string length(u,v)
%   inputs:
%     x,y   .. gondola position [mm]
%     L     .. stepper distance [mm]
%     R     .. stepper wheel radius [mm] (optional)
%     q     .. gondola mass to string weight ratio [mm] (i.e. q = mG/rhoA) (optional)
%  outputs:
%     u,v   .. string lengths (left, right) [mm]

  % ---------------------------------------------------
  % --- string lengths of ideal straight line model ---
  % ---------------------------------------------------

  % function [u, v] = pos2len(x, y, L)
  assert(isnumeric(L) && isscalar(L) && L > 0, 'pos2len: stepper distance L must be a positive scalar');
  assert(isnumeric(x) && x>=0 && x<=L, 'pos2len: x must be in range [0, L]');
  assert(isnumeric(y) && y>=0, 'pos2len: y must be non-negative');

  uIdeal = sqrt(x.*x + y.*y);
  vIdeal = sqrt((L-x).*(L-x) + y.*y);
  u = uIdeal;
  v = vIdeal;

  if (length(varargin)>=1)
    R = varargin{1};
    if (~isnumeric(R) || ~isscalar(R) || R < 0)
      warning('pos2len: R must be a positive scalar, setting to 0');
      R = 0;
    end

    % --------------------------------------------------------------------
    % --- string lengths with radius correction on straight line model ---
    % --------------------------------------------------------------------

    % function [u, v] = pos2len(x, y, L, R)
    u = sqrt(x.*x.*(1-R./uIdeal).*(1-R./uIdeal) + y.*y.*(1+R./uIdeal).*(1+R./uIdeal));
    v = sqrt((L-x).*(L-x).*(1-R./vIdeal).*(1-R./vIdeal) + y.*y.*(1+R./vIdeal).*(1+R./vIdeal));
  end

  if (length(varargin)>1)
    assert(length(varargin)==2, 'pos2len: wrong number of input arguments');
    q = varargin{2};
    assert(isnumeric(q) && isscalar(q) && q > 0, 'pos2len: mass ratio q must be a positive scalar');

    % ----------------------------------------------------------
    % --- string lengths with radius and catenary correction ---
    % ----------------------------------------------------------

    % function [u, v] = pos2len(x, y, L, R, q)

    if (isscalar(x)&&isscalar(y))
      % Approximation: solve catenary model
      % mounting boundary conditions in gondola coordinates: yl(xl) = yl, yr(xr) = yr
      xl = -x + R*x/u;
      yl = y + R*y/u;
      xr = L-x - R*(L-x)/v;
      yr = y + R*y/v;

      % Solve using fixed-point iteration with Newton-Raphson for c1, c3:
      % Initial estimates
      A = q / (-yl / xl + yr / xr);
      c1 = asinh(yl / xl);
      c3 = asinh(yr / xr);

      % constants for Newton-Raphson solver
      tol = 1e-10;      % tolerance for convergence of Newton-Raphson solver
      max_iter = 20;    % maximum number of iterations for Newton-Raphson solver
      pos_res = 0.05;           % position resolution for fixed-point iteration [mm]
      len_res = 2*pi*R/200/32;  % string step resolution (200 steps/rev & 1/32 microstepping) [mm]

      u_done = false;
      v_done = false;

      for iter = 1:max_iter
        prev_u = u;
        prev_v = v;
        prev_c1 = c1;
        prev_c3 = c3;

        % Solve for c1 using Newton-Raphson: yl = A*(cosh(xl/A+c1) - cosh(c1))
        arg_l = xl/A + c1;
        residual_c1 = yl - A*(cosh(arg_l) - cosh(c1));
        deriv_c1 = A*(sinh(arg_l) - sinh(c1));
        if (abs(deriv_c1) > 1e-12)  % Avoid division by zero
          c1 = c1 + residual_c1 / deriv_c1;
          A = q / (sinh(c3)-sinh(c1));
        end
        u_done = (abs(u-prev_u)<len_res);

        % Solve for c3 using Newton-Raphson: yr = A*(cosh(xr/A+c3) - cosh(c3))
        arg_r = xr/A + c3;
        residual_c3 = yr - A*(cosh(arg_r) - cosh(c3));
        deriv_c3 = A*(sinh(arg_r) - sinh(c3));
        if (abs(deriv_c3) > 1e-12)  % Avoid division by zero
          c3 = c3 + residual_c3 / deriv_c3;
          A = q / (sinh(c3)-sinh(c1));
        end

        % calculate string lengths based on catenary model
        u = A*(sinh(c1) - sinh(-x/A+c1));
        v = A*(sinh((L-x)/A+c3) - sinh(c3));

        v_done = (abs(v-prev_v)<len_res);

        if (u_done && v_done)
          break;
        end
        xl = -x + R*x/u;
        yl = y + R*y/u;
        xr = L-x - R*(L-x)/v;
        yr = y + R*y/v;

      end
    else
      % Vectorial input: recursively call with scalar elements and combine
      assert(size(x)==size(y), 'pos2len: x and y must have the same size');
      u = zeros(size(x));
      v = zeros(size(x));
      for idx = 1:numel(x)
        [u_scalar, v_scalar] = pos2len(x(idx), y(idx), L, R, q);
        u(idx) = u_scalar;
        v(idx) = v_scalar;
      end
    end
  end
end
