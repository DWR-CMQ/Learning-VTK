
#ifndef _H
#define _H

//#ifdef VTKCOMMONDATAMODEL_STATIC_DEFINE
//#  define 
//#  define VTKCOMMONDATAMODEL_NO_EXPORT
//#else
//#  ifndef VTKCOMMONDATAMODEL_EXPORT
//#    ifdef CommonDataModel_EXPORTS
//        /* We are building this library */
//#      define  __declspec(dllexport)
//#    else
//        /* We are using this library */
//#      define  __declspec(dllimport)
//#    endif
//#  endif
//
//#  ifndef VTKCOMMONDATAMODEL_NO_EXPORT
//#    define VTKCOMMONDATAMODEL_NO_EXPORT 
//#  endif
//#endif
//
//#ifndef VTKCOMMONDATAMODEL_DEPRECATED
//#  define VTKCOMMONDATAMODEL_DEPRECATED __declspec(deprecated)
//#endif
//
//#ifndef VTKCOMMONDATAMODEL_DEPRECATED_EXPORT
//#  define VTKCOMMONDATAMODEL_DEPRECATED_EXPORT  VTKCOMMONDATAMODEL_DEPRECATED
//#endif
//
//#ifndef VTKCOMMONDATAMODEL_DEPRECATED_NO_EXPORT
//#  define VTKCOMMONDATAMODEL_DEPRECATED_NO_EXPORT VTKCOMMONDATAMODEL_NO_EXPORT VTKCOMMONDATAMODEL_DEPRECATED
//#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VTKCOMMONDATAMODEL_NO_DEPRECATED
#    define VTKCOMMONDATAMODEL_NO_DEPRECATED
#  endif
#endif

/* VTK-HeaderTest-Exclude: vtkCommonDataModelModule.h */

/* Include ABI Namespace */
#include "vtkABINamespace.h"

#endif /* _H */
