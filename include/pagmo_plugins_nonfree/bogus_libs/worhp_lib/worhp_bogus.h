#ifndef WORHP_BOGUS_H
#define WORHP_BOGUS_H

#ifdef __cplusplus
extern "C" {
#endif

// When in windows symbols must be explictly marked for export
#ifdef _WIN32
#define __PAGMO_VISIBLE __declspec(dllexport)
#else
#define __PAGMO_VISIBLE __attribute__((visibility("default")))
#endif

double closed_interval_rand(double x0, double x1);

#ifdef __cplusplus
}
#endif

// The original headers from worhp
#include "worhp_headers/worhp.h"

#endif // WORHP_BOGUS_H
