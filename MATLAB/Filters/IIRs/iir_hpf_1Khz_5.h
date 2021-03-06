/*
 * Filter Coefficients (C Source) generated by the Filter Design and Analysis Tool
 * Generated by MATLAB(R) 9.10 and Signal Processing Toolbox 8.6.
 * Generated on: 22-Jun-2021 17:55:55
 */

/*
 * Discrete-Time IIR Filter (real)
 * -------------------------------
 * Filter Structure    : Direct-Form II, Second-Order Sections
 * Number of Sections  : 3
 * Stable              : Yes
 * Linear Phase        : No
 */

/* General type conversion for MATLAB generated C-code  */
#include "tmwtypes.h"
/* 
 * Expected path to tmwtypes.h 
 * C:\Program Files\Polyspace\R2021a\extern\include\tmwtypes.h 
 */
/*
 * Warning - Filter coefficients were truncated to fit specified data type.  
 *   The resulting response may not match generated theoretical response.
 *   Use the Filter Design & Analysis Tool to design accurate
 *   single-precision filter coefficients.
 */
#define MWSPT_NSEC 7
const int NL[MWSPT_NSEC] = { 1,3,1,3,1,2,1 };
const real32_T NUM[MWSPT_NSEC][3] = {
  {
     0.9531124234,              0,              0 
  },
  {
                1,             -2,              1 
  },
  {
      0.892416656,              0,              0 
  },
  {
                1,             -2,              1 
  },
  {
     0.9333941936,              0,              0 
  },
  {
                1,             -1,              0 
  },
  {
                1,              0,              0 
  }
};
const int DL[MWSPT_NSEC] = { 1,3,1,3,1,2,1 };
const real32_T DEN[MWSPT_NSEC][3] = {
  {
                1,              0,              0 
  },
  {
                1,    -1.89651823,   0.9159315228 
  },
  {
                1,              0,              0 
  },
  {
                1,   -1.775744796,   0.7939217687 
  },
  {
                1,              0,              0 
  },
  {
                1,  -0.8667884469,              0 
  },
  {
                1,              0,              0 
  }
};
