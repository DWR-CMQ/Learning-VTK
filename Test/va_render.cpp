#include "va_render.h"
#include <vtkCellArray.h>

#include <vtkPoints.h>
#include <vtkDensifyPolyData.h>
#include <vtkUnsignedIntArray.h>
#include <vtkIdTypeArray.h>

#include "va_common_function.h"
Render::Render(std::shared_ptr<SetVolumeParameter> spParameter, std::shared_ptr<Camera> spCamera)
{
    m_uiVao = 0;
    m_uiVbo = 0;
    m_uiEbo = 0;
    m_spVolumePara = spParameter;
    m_spCamera = spCamera;
    m_mat4TempMatrix4x4->Identity();
}

Render::~Render()
{
    glDeleteVertexArrays(1, &m_uiVao);
    glDeleteBuffers(1, &m_uiVbo);
    glDeleteBuffers(1, &m_uiEbo);
    m_uiVao = 0;
    m_uiVbo = 0;
    m_uiEbo = 0;
}

void Render::Init()
{
    if (m_pDrawShader == NULL)
    {
        m_pDrawShader = new Shader("shaders//raycast.vs", "shaders//raycast.fs");
    }

    vtkNew<vtkPolyData> boxSource;

    // 顶点着色器
    {
        vtkNew<vtkCellArray> cells;
        vtkNew<vtkPoints> points;
        points->SetDataTypeToDouble();
        for (int i = 0; i < 8; ++i)
        {
            points->InsertNextPoint(this->m_spVolumePara->m_dVolumeGeometry + i * 3);
        }
        // 6 faces 12 triangles
        int tris[36] =
        {
          0, 1, 2, //
          1, 3, 2, //
          1, 5, 3, //
          5, 7, 3, //
          5, 4, 7, //
          4, 6, 7, //
          4, 0, 6, //
          0, 2, 6, //
          2, 3, 6, //
          3, 7, 6, //
          0, 4, 1, //
          1, 4, 5  //
        };
        for (int i = 0; i < 12; ++i)
        {
            cells->InsertNextCell(3);
            // this code uses a clockwise convention for some reason
            // no clue why but the ClipConvexPolyData assumes the same
            // so we add verts as 0 2 1 instead of 0 1 2
            cells->InsertCellPoint(tris[i * 3]);
            cells->InsertCellPoint(tris[i * 3 + 2]);
            cells->InsertCellPoint(tris[i * 3 + 1]);
        }
        boxSource->SetPoints(points);
        boxSource->SetPolys(cells);

        vtkNew<vtkDensifyPolyData> densifyPolyData;
        densifyPolyData->SetInputData(boxSource);
        densifyPolyData->SetNumberOfSubdivisions(2);
        densifyPolyData->Update();

        this->BBoxPolyData = vtkSmartPointer<vtkPolyData>::New();
        this->BBoxPolyData->ShallowCopy(densifyPolyData->GetOutput());
        vtkPoints* boxPoints = this->BBoxPolyData->GetPoints();
        vtkCellArray* boxCells = this->BBoxPolyData->GetPolys();

        vtkNew<vtkUnsignedIntArray> polys;
        polys->SetNumberOfComponents(3);
        vtkIdType npts;
        const vtkIdType* pts;

        bool preservesOrientation = true;
        const vtkIdType indexMap[3] = { preservesOrientation ? 0 : 2, 1, preservesOrientation ? 2 : 0 };
        while (boxCells->GetNextCell(npts, pts))
        {
            polys->InsertNextTuple3(pts[indexMap[0]], pts[indexMap[1]], pts[indexMap[2]]);
        }

        glGenVertexArrays(1, &m_uiVao);
        glGenBuffers(1, &m_uiVbo);
        glGenBuffers(1, &m_uiEbo);
        m_pDrawShader->use();

        glBindVertexArray(this->m_uiVao);
        glBindBuffer(GL_ARRAY_BUFFER, this->m_uiVbo);
        glBufferData(GL_ARRAY_BUFFER, boxPoints->GetData()->GetDataSize() * boxPoints->GetData()->GetDataTypeSize(),
            boxPoints->GetData()->GetVoidPointer(0), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_uiEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, polys->GetDataSize() * polys->GetDataTypeSize(), polys->GetVoidPointer(0), GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

}

void Render::InitShaderInput()
{
    int numVolumes = 1;
    this->m_vecVolMat.resize(numVolumes * 16, 0);
    this->m_vecInvMat.resize(numVolumes * 16, 0);
    this->m_vecTexMat.resize(numVolumes * 16, 0);
    this->m_vecInvTexMat.resize(numVolumes * 16, 0);
    this->m_vecTexEyeMat.resize(numVolumes * 16, 0);
    this->m_vecCellToPoint.resize(numVolumes * 16, 0);
    this->m_vecTexMin.resize(numVolumes * 3, 0);
    this->m_vecTexMax.resize(numVolumes * 3, 0);
    this->m_vecEyePos.resize(numVolumes * 3, 0);

    vtkNew<vtkMatrix4x4> dataToWorld;
    vtkNew<vtkMatrix4x4> dataToView;
    vtkNew<vtkMatrix4x4> texToDataMat;
    vtkNew<vtkMatrix4x4> texToViewMat;
    vtkNew<vtkMatrix4x4> cellToPointMat;

    float defaultTexMin[3] = { 0.0f, 0.0f, 0.0f };
    float defaultTexMax[3] = { 1.0f, 1.0f, 1.0f };
    float eyePos[3] = { 0.0f, 0.0f, 0.0f };

    vtkMatrix4x4* wcvc, * vcdc, * wcdc;
    vtkMatrix3x3* norm;
    m_spCamera->GetKeyMatrices(wcvc, norm, vcdc, wcdc);

    for (int i = 0; i < numVolumes; i++)
    {
        const int vecOfffset = i * 16;
        float* texMin;
        float* texMax;

        if (numVolumes > 1)
        {
        }
        else
        {
            vtkMatrix4x4* volMatrix = this->m_mat4TempMatrix4x4;
            // DeepCopy的参数是source 
            dataToWorld->DeepCopy(volMatrix);
            texToDataMat->DeepCopy(this->m_spVolumePara->m_mat4TextureToDataset.GetPointer());

            // Texture matrices (texture to view)
            // Multiply4x4 => a * b = c
            vtkMatrix4x4::Multiply4x4(volMatrix, texToDataMat.GetPointer(), texToViewMat.GetPointer());
            vtkMatrix4x4::Multiply4x4(wcvc, texToViewMat.GetPointer(), texToViewMat.GetPointer());

            CommonFunction::CopyMatrixToVector<vtkMatrix4x4, 4, 4>(texToViewMat.GetPointer(), this->m_vecTexEyeMat.data(), vecOfffset);
            cellToPointMat->DeepCopy(this->m_spVolumePara->CellToPointMatrix.GetPointer());
        }
    }
}