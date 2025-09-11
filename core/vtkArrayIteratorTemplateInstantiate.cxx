// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#define vtkArrayIteratorTemplateInstantiate_cxx

#include "vtkArrayIteratorTemplate.txx"

#include "vtkOStreamWrapper.h"

VTK_ABI_NAMESPACE_BEGIN
vtkInstantiateTemplateMacro(template class vtkArrayIteratorTemplate);
template class vtkArrayIteratorTemplate<vtkStdString>;
template class vtkArrayIteratorTemplate<vtkVariant>;
VTK_ABI_NAMESPACE_END
