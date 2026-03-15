#ifndef pos2len_h
#define pos2len_h

#include <stdint.h>
#include <math.h>

/// @brief calculate the lengths of the two strings for a given position (x, y) of the pen and the distance L between the two motors
/// @param x gondola position x [mm]
/// @param y gondola position y [mm]
/// @param L length between the two motors [mm]
/// @param u length of left string [mm]
/// @param v length of right string [mm]
void pos2leni(const float x, const float y, const float L, float &u, float &v)
{
  const float xsqr{x * x};
  const float lxsqr{(L - x) * (L - x)};
  const float ysqr{y * y};
  u = sqrtf(xsqr + ysqr);
  v = sqrtf(lxsqr + ysqr);
}

/// @brief calculate the lengths of the two strings for a given position (x, y) of the pen and the distance L between the two motors
/// with stepper wheel radius correction
/// @param x gondola position x [mm]
/// @param y gondola position y [mm]
/// @param L length between the two motors [mm]
/// @param R radius of the stepper wheel [mm]
/// @param u length of left string [mm]
/// @param v length of right string [mm]
void pos2lenr(const float x, const float y, const float L, const float R, float &u, float &v)
{
  const float xsqr{x * x};
  const float lxsqr{(L - x) * (L - x)};
  const float ysqr{y * y};
  const float Rsqr{R * R};
  const float uIdeal{sqrtf(xsqr + ysqr)};
  const float vIdeal{sqrtf(lxsqr + ysqr)};
  const float uIdealInvSqr{1.0F / (xsqr + ysqr)};
  const float vIdealInvSqr{1.0F / (lxsqr + ysqr)};
  const float sqruPart1{(xsqr + ysqr + Rsqr) * uIdealInvSqr};
  const float sqruPart2{2.0F * R * uIdeal * uIdealInvSqr};
  const float sqrvPart1{(lxsqr + ysqr + Rsqr) * vIdealInvSqr};
  const float sqrvPart2{2.0F * R * vIdeal * vIdealInvSqr};
  const float xsqruCorr{sqruPart1 - sqruPart2};
  const float ysqruCorr{sqruPart1 + sqruPart2};
  const float xsqrvCorr{sqrvPart1 - sqrvPart2};
  const float ysqrvCorr{sqrvPart1 + sqrvPart2};
  u = sqrtf(xsqr * xsqruCorr + ysqr * ysqruCorr);
  v = sqrtf(lxsqr * xsqrvCorr + ysqr * ysqrvCorr);
}

/// @brief calculate the lengths of the two strings for a given position (x, y) of the pen and the distance L between the two motors
/// with stepper wheel radius correction and mass ratio correction
/// @param x gondola position x [mm]
/// @param y gondola position y [mm]
/// @param L length between the two motors [mm]
/// @param R radius of the stepper wheel [mm]
/// @param q gondola mass to string weight ratio [mm] (i.e. q = mG/rhoA)
/// @param u length of left string [mm]
/// @param v length of right string [mm]
void pos2len(const float x, const float y, const float L, const float R, const float q, float &u, float &v)
{
  const float xsqr{x * x};
  const float lxsqr{(L - x) * (L - x)};
  const float ysqr{y * y};
  const float Rsqr{R * R};
  const float uIdeal{sqrtf(xsqr + ysqr)};
  const float vIdeal{sqrtf(lxsqr + ysqr)};
  const float uIdealInvSqr{1.0F / (xsqr + ysqr)};
  const float vIdealInvSqr{1.0F / (lxsqr + ysqr)};
  const float sqruPart1{(xsqr + ysqr + Rsqr) * uIdealInvSqr};
  const float sqruPart2{2.0F * R * uIdeal * uIdealInvSqr};
  const float sqrvPart1{(lxsqr + ysqr + Rsqr) * vIdealInvSqr};
  const float sqrvPart2{2.0F * R * vIdeal * vIdealInvSqr};
  const float xsqruCorr{sqruPart1 - sqruPart2};
  const float ysqruCorr{sqruPart1 + sqruPart2};
  const float xsqrvCorr{sqrvPart1 - sqrvPart2};
  const float ysqrvCorr{sqrvPart1 + sqrvPart2};

  float uEst = sqrtf(xsqr * xsqruCorr + ysqr * ysqruCorr);
  float vEst = sqrtf(lxsqr * xsqrvCorr + ysqr * ysqrvCorr);
  const float uEstInv{1.0F / uEst};
  const float vEstInv{1.0F / vEst};

  // initial estimates for catenary model
  // mounting boundary conditions in gondola coordinates: yl(xl) = yl, yr(xr) = yr
  float xl = -x + R * x * uEstInv;
  float yl = y + R * y * uEstInv;
  float xr = L - x - R * (L - x) * vEstInv;
  float yr = y + R * y * vEstInv;

  // Solve using fixed-point iteration with Newton-Raphson for c1, c3:
  // Initial estimates
  float sinhfc1 = yl / xl;
  float sinhfc3 = yr / xr;
  float coshfc1 = sqrtf(1.0F + sinhfc1 * sinhfc1);
  float coshfc3 = sqrtf(1.0F + sinhfc3 * sinhfc3);
  float c1 = log(sinhfc1 + coshfc1);
  float c3 = log(sinhfc3 + coshfc3);
  float A = q / (sinhfc3 - sinhfc1);

  // constants for Newton-Raphson solver
  constexpr uint8_t max_iter = 20; // maximum number of iterations for Newton-Raphson solver
  constexpr float lenfac = 2.0F * M_PI / 200.0F / 32.0F;
  const float len_res{lenfac * R}; // string step resolution (200 steps/rev & 1/32 microstepping) [mm]

  bool uCalc = true;
  bool vCalc = true;

  for (uint8_t iter = 0; iter < max_iter && uCalc && vCalc; iter++)
  {
    // Solve for c1 using Newton-Raphson: yl = A*(coshf(xl/A+c1) - coshfc1)
    const float arg_l{xl / A + c1};
    const float sinhfarg_l{sinhf(arg_l)};
    const float coshfarg_l{sqrtf(1.0F + sinhfarg_l * sinhfarg_l)};
    const float residual_c1{yl - A * (coshfarg_l - coshfc1)};
    const float deriv_c1{A * (sinhfarg_l - sinhfc1)};
    if (fabsf(deriv_c1) > 1e-12)
    {
      c1 = c1 + residual_c1 / deriv_c1;
      sinhfc1 = sinhf(c1);
      coshfc1 = sqrtf(1.0F + sinhfc1 * sinhfc1);
      A = q / (sinhfc3 - sinhfc1);
      const float uNew{A * (sinhfc1 - sinhf(-x / A + c1))};
      const float uNewInv{1.0F / uNew};
      xl = -x + R * x * uNewInv;
      yl = y + R * y * uNewInv;
      uCalc = (fabsf(uNew - uEst) > len_res);
      uEst = uNew;
    }

    // Solve for c3 using Newton-Raphson: yr = A*(coshf(xr/A+c3) - coshfc3)
    const float arg_r{xr / A + c3};
    const float sinhfarg_r{sinhf(arg_r)};
    const float coshfarg_r{sqrtf(1 + sinhfarg_r * sinhfarg_r)};
    const float residual_c3{yr - A * (coshfarg_r - coshfc3)};
    const float deriv_c3{A * (sinhfarg_r - sinhfc3)};
    if (fabsf(deriv_c3) > 1e-12)
    {
      c3 = c3 + residual_c3 / deriv_c3;
      sinhfc3 = sinhf(c3);
      coshfc3 = sqrtf(1 + sinhfc3 * sinhfc3);
      A = q / (sinhfc3 - sinhfc1);
      const float vNew{A * (sinhf((L - x) / A + c3) - sinhfc3)};
      const float vNewInv{1.0F / vNew};
      xr = L - x - R * (L - x) * vNewInv;
      yr = y + R * y * vNewInv;
      vCalc = (fabsf(vNew - vEst) > len_res);
      vEst = vNew;
    }
  }
  u = uEst;
  v = vEst;
}

#endif
