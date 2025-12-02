
#ifndef VTKRENDERINGCORE_EXPORT_H
#define VTKRENDERINGCORE_EXPORT_H

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VTKRENDERINGCORE_NO_DEPRECATED
#    define VTKRENDERINGCORE_NO_DEPRECATED
#  endif
#endif

/* VTK-HeaderTest-Exclude: vtkRenderingCoreModule.h */

/* Include ABI Namespace */
#include "vtkABINamespace.h"
/* AutoInit dependencies. */
#include "vtkFiltersCoreModule.h"
#include "Export.h"

/* AutoInit implementations. */
#ifdef vtkRenderingCore_AUTOINIT_INCLUDE
#include vtkRenderingCore_AUTOINIT_INCLUDE
#endif
#ifdef vtkRenderingCore_AUTOINIT
#include "vtkAutoInit.h"
VTK_MODULE_AUTOINIT(vtkRenderingCore)
#endif

#endif /* VTKRENDERINGCORE_EXPORT_H */
