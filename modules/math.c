#include <math.h>

// Standard Constants
double get_pi() { return M_PI; }
double get_e() { return M_E; }
double get_phi() { return (1.0 + sqrt(5.0)) / 2.0; }

// Trigonometric Functions (Degrees to Radians)
#define DEG_TO_RAD(deg) (deg * (M_PI / 180.0))

double get_sin(double degrees) { return sin(DEG_TO_RAD(degrees)); }
double get_cos(double degrees) { return cos(DEG_TO_RAD(degrees)); }

// Roots and Logarithms
double get_sqrt(double val) { return sqrt(val); }
double get_ln(double val) { return log(val); }
double get_log10(double val) { return log10(val); }
