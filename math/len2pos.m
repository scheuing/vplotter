function [x, y] = len2pos(u, v, L, varargin)
% function [x, y] = len2pos(u, v, L, R, mG, rhoA)
%   transform string length(u,v) into gondola position (x,y)
%   inputs:
%     u,v  .. string lengths (left, right) [mm]
%     L    .. stepper distance [mm]
%     R    .. stepper wheel radius [mm] (optional)
%     q    .. gondola mass to string weight ratio [mm] (i.e. q = mG/rhoA) (optional)
%  outputs:
%     x,y  .. gondola position [m]

  % -------------------------------------------------------------
  % --- exact analytical inverse of ideal straight line model ---
  % -------------------------------------------------------------

  % function [x, y] = len2pos(u, v, L)
  assert(isnumeric(L) && isscalar(L) && L > 0, 'len2pos: stepper distance L must be a positive scalar');
  assert(isnumeric(u) && u>=0, 'len2pos: string length u must be non-negative');
  assert(isnumeric(v) && v>=0, 'len2pos: string length v must be non-negative');
  
  x = 0.5/L * (u.*u - v.*v + L*L);
  y = sqrt(max(u.*u - x.*x, 0));

  if (length(varargin)>0)
    % ----------------------------------------------------
    % --- approximate inverse by Newton-Raphson solver ---
    % ----------------------------------------------------

    % function [x, y] = len2pos(u, v, L, R)

    R = varargin{1};
    if (~isnumeric(R) || ~isscalar(R) || R < 0)
      warning('pos2len: R must be a positive scalar, setting to 0');
      R = 0;
    end

    % Solve: F(x,y) = [pos2len_u(x,y,L,R) - u; pos2len_v(x,y,L,R) - v] = 0
    % abbreviate forward function for radius correction on straight line model
    pos2lenCorr = @(x,y) pos2len(x, y, L, R);

    if (length(varargin)>1)

      % function [x, y] = len2pos(u, v, L, R, q)

      assert(length(varargin)==2, 'pos2len: wrong number of input arguments');
      q = varargin{2};
      assert(isnumeric(q) && isscalar(q) && q > 0, 'pos2len: mass ratio q must be a positive scalar');

      % Solve: F(x,y) = [pos2len_u(x,y,L,R,q) - u; pos2len_v(x,y,L,R,q) - v] = 0
      % abbreviate forward function for radius correction on catenary model
      pos2lenCorr = @(x,y) pos2len(x, y, L, R, q);
    end

    % constants for Newton-Raphson solver
    tol = 1e-10;      % tolerance for convergence of Newton-Raphson solver
    max_iter = 20;    % maximum number of iterations for Newton-Raphson solver
    h = 1e-8;         % finite difference step for numerical Jacobian

    for iter = 1:max_iter
      % compute forward model at current (x, y)
      [u0, v0] = pos2lenCorr(x, y);
      
      % residuals
      fu = u0 - u;
      fv = v0 - v;

      % Newton step: [dx; dy] = -J^{-1} * [fu; fv]
      % for scalar fields: J = [[dfu_dx, dfu_dy]; [dfv_dx, dfv_dy]]
      
      % Jacobian computation (numerical differentiation)
      [u0_xh, v0_xh] = pos2lenCorr(x+h, y);
      [u0_yh, v0_yh] = pos2lenCorr(x, y+h);
      dfu_dx = (u0_xh - u0) / h;
      dfu_dy = (u0_yh - u0) / h;
      dfv_dx = (v0_xh - v0) / h;
      dfv_dy = (v0_yh - v0) / h;
      
      det_J = dfu_dx.*dfv_dy - dfu_dy.*dfv_dx;
      % avoid division by zero
      det_J = max(abs(det_J), 1e-12) .* sign(det_J + (det_J==0));
      
      dx = -(dfv_dy.*fu - dfu_dy.*fv) ./ det_J;
      dy = -(dfu_dx.*fv - dfv_dx.*fu) ./ det_J;
      
      % update
      x = x + dx;
      y = y + dy;
      
      % residual for convergence check
      residual_x = max(abs(dx(:)));
      residual_y = max(abs(dy(:)));
      if (residual_x<tol && residual_y<tol)
        break;
      end
    end    
  end
end