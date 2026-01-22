#include "va_window.h"
#include <vtkMath.h>

Window::Window()
{
    this->Size[0] = m_iWidth;
    this->Size[1] = m_iHeight;
    this->TileSize[0] = 0;
    this->TileSize[1] = 0;
    this->TileScale[0] = 1;
    this->TileScale[1] = 1;
}

int* Window::GetSize()
{
    this->TileSize[0] = this->Size[0] * this->TileScale[0];
    this->TileSize[1] = this->Size[1] * this->TileScale[1];

    return this->TileSize;
}

void Window::NormalizedDisplayToDisplay(double& u, double& v)
{
    u = u * this->Size[0];
    v = v * this->Size[1];
}

void Window::GetTiledSizeAndOrigin(int* usize, int* vsize, int* lowerLeftU, int* lowerLeftV)
{
    double* vport = new double[4];
    vport[0] = 0.0;      
    vport[1] = 0.0;      
    vport[2] = 0.5;  
    vport[3] = 1.0;  

    // if there is no window assume 0 1
    double tileViewPort[4];

    tileViewPort[0] = 0;
    tileViewPort[1] = 0;
    tileViewPort[2] = 1;
    tileViewPort[3] = 1;
    
    // find the lower left corner of the viewport, taking into account the
    // lower left boundary of this tile
    double vpu = vtkMath::ClampValue(vport[0] - tileViewPort[0], 0.0, 1.0);
    double vpv = vtkMath::ClampValue(vport[1] - tileViewPort[1], 0.0, 1.0);
    // store the result as a pixel value
    this->NormalizedDisplayToDisplay(vpu, vpv);
    *lowerLeftU = static_cast<int>(vpu + 0.5);
    *lowerLeftV = static_cast<int>(vpv + 0.5);

    // find the upper right corner of the viewport, taking into account the
    // lower left boundary of this tile
    double vpu2 = vtkMath::ClampValue(vport[2] - tileViewPort[0], 0.0, 1.0);
    double vpv2 = vtkMath::ClampValue(vport[3] - tileViewPort[1], 0.0, 1.0);
    // also watch for the upper right boundary of the tile
    if (vpu2 > (tileViewPort[2] - tileViewPort[0]))
    {
        vpu2 = tileViewPort[2] - tileViewPort[0];
    }
    if (vpv2 > (tileViewPort[3] - tileViewPort[1]))
    {
        vpv2 = tileViewPort[3] - tileViewPort[1];
    }
    this->NormalizedDisplayToDisplay(vpu2, vpv2);
    // now compute the size of the intersection of the viewport with the
    // current tile
    *usize = static_cast<int>(vpu2 + 0.5) - *lowerLeftU;
    *vsize = static_cast<int>(vpv2 + 0.5) - *lowerLeftV;
    if (*usize < 0)
    {
        *usize = 0;
    }
    if (*vsize < 0)
    {
        *vsize = 0;
    }

    delete[] vport;
}