#pragma once

#ifdef Commoncore_EXPORTS
#define COMMONCORE_EXPORTS __declspec(dllexport)
#else
#define COMMONCORE_EXPORTS __declspec(dllimport)
#endif

#include "vtkABINamespace.h"
