#pragma once

#ifdef Core_EXPORTS
#define CORE_EXPORTS __declspec(dllexport)
#else
#define CORE_EXPORTS __declspec(dllimport)
#endif

#include "vtkABINamespace.h"