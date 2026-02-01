#include "va_render.h"
#include <glad/glad.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkDensifyPolyData.h>
#include <vtkUnsignedIntArray.h>
#include <vtkIdTypeArray.h>

#include "va_common_function.h"
VARender::VARender(std::shared_ptr<VAVolume> spVolume, std::shared_ptr<Camera> spCamera)
{
    m_uiVao = 0;
    m_uiVbo = 0;
    m_uiEbo = 0;
    m_spVolume = spVolume;
    m_spCamera = spCamera;
    m_mat4TempMatrix4x4->Identity();
    this->TotalNumberOfLights = 1;
    this->DefaultLighting = true;
    
    this->FinalColorWindow = 1.0;
    this->FinalColorLevel = 0.5;

    this->AverageIPScalarRange[0] = VTK_FLOAT_MIN;
    this->AverageIPScalarRange[1] = VTK_FLOAT_MAX;
    this->ActualSampleDistance = 1.0;
}

VARender::~VARender()
{
    glDeleteVertexArrays(1, &m_uiVao);
    glDeleteBuffers(1, &m_uiVbo);
    glDeleteBuffers(1, &m_uiEbo);
    m_uiVao = 0;
    m_uiVbo = 0;
    m_uiEbo = 0;
}

void VARender::Init(std::shared_ptr<VAWindow> spWindow)
{
    if (m_pDrawShader == NULL)
    {
        m_pDrawShader = new Shader("shaders//raycast.vs", "shaders//raycast.fs");
    }

    // 其它参数
    spWindow->GetTiledSizeAndOrigin(this->WindowSize, this->WindowSize + 1, this->WindowLowerLeft, this->WindowLowerLeft + 1);
}

void VARender::RenderVolumeGeometry()
{
    vtkNew<vtkPolyData> boxSource;

    // 顶点着色器
    {
        vtkNew<vtkCellArray> cells;
        vtkNew<vtkPoints> points;
        points->SetDataTypeToDouble();
        for (int i = 0; i < 8; ++i)
        {
            points->InsertNextPoint(this->m_spVolume->m_dVolumeGeometry + i * 3);
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

void VARender::BindTransformations(vtkMatrix4x4* modelViewMat)
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

    for (int i = 0; i < numVolumes; i++)
    {
        const int vecOffset = i * 16;
        float* texMin;
        float* texMax;

        if (numVolumes > 1)
        {
            cellToPointMat->Identity();
            texMin = defaultTexMin;
            texMax = defaultTexMax;
        }
        else
        {
            vtkMatrix4x4* volMatrix = this->m_mat4TempMatrix4x4;
            // DeepCopy的参数是source 
            dataToWorld->DeepCopy(volMatrix);
            texToDataMat->DeepCopy(this->m_spVolume->m_mat4TextureToDataset.GetPointer());

            // Texture matrices (texture to view)
            // Multiply4x4 => a * b = c
            vtkMatrix4x4::Multiply4x4(volMatrix, texToDataMat.GetPointer(), texToViewMat.GetPointer());
            vtkMatrix4x4::Multiply4x4(modelViewMat, texToViewMat.GetPointer(), texToViewMat.GetPointer());

            CommonFunction::CopyMatrixToVector<vtkMatrix4x4, 4, 4>(texToViewMat.GetPointer(), this->m_vecTexEyeMat.data(), vecOffset);
            cellToPointMat->DeepCopy(this->m_spVolume->CellToPointMatrix.GetPointer());
            texMin = this->m_spVolume->AdjustedTexMin;
            texMax = this->m_spVolume->AdjustedTexMax;
        }

        // Volume matrices (dataset to world)
        dataToWorld->Transpose();

        // Get the effective position of the eye in world coordinates for this
        // volume (or the bbox).
        // This multiply may look backwards, but dataToWorld and modelViewMat are
        // both already transposed to send to OpenGL.
        vtkMatrix4x4::Multiply4x4(dataToWorld.GetPointer(), modelViewMat, dataToView.GetPointer());
        dataToView->Invert();
        eyePos[0] = dataToView->GetElement(3, 0);
        eyePos[1] = dataToView->GetElement(3, 1);
        eyePos[2] = dataToView->GetElement(3, 2);
        CommonFunction::CopyVector<float, 3>(eyePos, this->m_vecEyePos.data(), i * 3);

        CommonFunction::CopyMatrixToVector<vtkMatrix4x4, 4, 4>(
            dataToWorld.GetPointer(), this->m_vecVolMat.data(), vecOffset);

        this->m_mat4InverseVolume->DeepCopy(dataToWorld.GetPointer());
        this->m_mat4InverseVolume->Invert();
        CommonFunction::CopyMatrixToVector<vtkMatrix4x4, 4, 4>(
            this->m_mat4InverseVolume.GetPointer(), this->m_vecInvMat.data(), vecOffset);

        // Texture matrices (texture to dataset)
        texToDataMat->Transpose();
        CommonFunction::CopyMatrixToVector<vtkMatrix4x4, 4, 4>(
            texToDataMat.GetPointer(), this->m_vecTexMat.data(), vecOffset);

        texToDataMat->Invert();
        CommonFunction::CopyMatrixToVector<vtkMatrix4x4, 4, 4>(
            texToDataMat.GetPointer(), this->m_vecInvTexMat.data(), vecOffset);

        // Cell to Point (texture adjustment)
        cellToPointMat->Transpose();
        CommonFunction::CopyMatrixToVector<vtkMatrix4x4, 4, 4>(
            cellToPointMat.GetPointer(), this->m_vecCellToPoint.data(), vecOffset);
        CommonFunction::CopyVector<float, 3>(texMin, this->m_vecTexMin.data(), i * 3);
        CommonFunction::CopyVector<float, 3>(texMax, this->m_vecTexMax.data(), i * 3);
    }

    // the matrix from data to world
    m_pDrawShader->setMat4("in_volumeMatrix", this->m_vecVolMat.data());
    m_pDrawShader->setMat4("in_inverseVolumeMatrix", this->m_vecInvMat.data());
    m_pDrawShader->setMat4("in_textureDatasetMatrix", this->m_vecTexMat.data());
    m_pDrawShader->setMat4("in_inverseTextureDatasetMatrix", this->m_vecInvTexMat.data());

    // matrix from texture to view coordinates
    m_pDrawShader->setMat4("in_textureToEye", this->m_vecTexEyeMat.data());

    // handle cell/point differences in tcoords
    m_pDrawShader->setMat4("in_cellToPoint", this->m_vecCellToPoint.data());

    m_pDrawShader->setVec3("in_texMin", this->m_vecTexMin.data());
    m_pDrawShader->setVec3("in_texMax", this->m_vecTexMax.data());
    m_pDrawShader->setVec3("in_eyePosObjs", this->m_vecEyePos.data());
}

void VARender::SetMapperShaderParameters(int independent, int numComp)
{
    m_pDrawShader->setInt("in_noOfComponents", numComp);
    // 当体渲染出现更新时,ActualSampleDistance会重新计算,它直接影响步进长度
    // 不出现更新时,它默认是1.0
    m_pDrawShader->setFloat("in_sampleDistance", this->ActualSampleDistance);
    m_pDrawShader->setFloat("in_scale", 1.0 / this->FinalColorWindow);
    m_pDrawShader->setFloat("in_bias", (0.5 - (this->FinalColorLevel / this->FinalColorWindow)));
    m_pDrawShader->setInt("in_transfer2DYAxis", 0);
}

void VARender::SetVolumeShaderParameters(int independent, int noOfComponents, vtkMatrix4x4* modelViewMat)
{
    this->BindTransformations(modelViewMat);

    const int numInputs = 1;
    this->m_vecScale.resize(numInputs * 4, 0);
    this->m_vecBias.resize(numInputs * 4, 0);
    this->m_vecStep.resize(numInputs * 3, 0);
    this->m_vecSpacing.resize(numInputs * 3, 0);
    this->m_vecRange.resize(numInputs * 8, 0);

    int index = 0;
    // Volume纹理激活

    float tscale[4] = { 1.0, 1.0, 1.0, 1.0 };
    float tbias[4] = { 0.0, 0.0, 0.0, 0.0 };
    float(*scalePtr)[4] = &tscale;
    float(*biasPtr)[4] = &tbias;
    if (noOfComponents == 1 || noOfComponents == 2)
    {
        scalePtr = &m_spVolume->Scale;
        biasPtr = &m_spVolume->Bias;
    }
    CommonFunction::CopyVector<float, 4>(*scalePtr, this->m_vecScale.data(), index * 4);
    CommonFunction::CopyVector<float, 4>(*biasPtr, this->m_vecBias.data(), index * 4);
    CommonFunction::CopyVector<float, 3>(m_spVolume->m_fCellStep, this->m_vecStep.data(), index * 3);
    CommonFunction::CopyVector<float, 3>(m_spVolume->m_fCellSpacing, this->m_vecSpacing.data(), index * 3);
    // 8 elements stands for [min, max] per 4-components
    CommonFunction::CopyVector<float, 8>(reinterpret_cast<float*>(this->m_spVolume->ScalarRange), this->m_vecRange.data(), index * 8);
    // 激活传输函数纹理

    m_pDrawShader->setVec4("in_volume_scale", this->m_vecScale.data());
    m_pDrawShader->setVec4("in_volume_bias", this->m_vecBias.data());
    m_pDrawShader->setVec4("in_scalarsRange", this->m_vecRange.data());
    m_pDrawShader->setVec4("in_cellStep", this->m_vecStep.data());
    m_pDrawShader->setVec4("in_cellSpacing", this->m_vecSpacing.data());
}

void VARender::SetLightingShaderParameters(int numberOfSamplers)
{
    if (m_pDrawShader == NULL || m_spVolume == nullptr)
    {
        return;
    }
    auto volumeProperty = m_spVolume->GetVolumeProperty();
    float ambient[4][3];
    float diffuse[4][3];
    float specular[4][3];
    float specularPower[4];

    // 目前numberOfSamplers强制设置为1
    for (int i = 0; i < numberOfSamplers; i++)
    {
        ambient[i][0] = ambient[i][1] = ambient[i][2] = volumeProperty->GetAmbient(i);
        diffuse[i][0] = diffuse[i][1] = diffuse[i][2] = volumeProperty->GetDiffuse(i);
        specular[i][0] = specular[i][1] = specular[i][2] = volumeProperty->GetSpecular(i);
        specularPower[i] = volumeProperty->GetSpecularPower(i);
    }
    m_pDrawShader->setVec3("in_ambient", ambient);
    m_pDrawShader->setVec3("in_diffuse", diffuse);
    m_pDrawShader->setVec3("in_specular", specular);
    m_pDrawShader->setVec1("in_shininess", specularPower);

    if ((m_spVolume != nullptr && volumeProperty->GetShade(0)) || this->TotalNumberOfLights == 0)
    {
        return;
    }

    // in_twoSidedLighting参数强行置为1
    m_pDrawShader->setInt("in_twoSidedLighting", 1);

    // in_lightAmbientColor/in_lightDiffuseColor/in_lightSpecularColor/in_lightDirection强制置为对应的值
    float lightAmbientColor[3] = { 0.0f,0.0f,0.0f };
    float lightDiffuseColor[3] = { 1.0f,1.0f,1.0f };
    float lightSpecularColor[3] = { 1.0f,1.0f,1.0f };
    float lightDirection[3] = { 0.0f,0.0f,-1.0f };
    m_pDrawShader->setVec3("in_lightAmbientColor", lightAmbientColor);
    m_pDrawShader->setVec3("in_lightDiffuseColor", lightDiffuseColor);
    m_pDrawShader->setVec3("in_lightSpecularColor", lightSpecularColor);
    m_pDrawShader->setVec3("in_lightDirection", lightDirection);
    if (this->DefaultLighting)
    {
        return;
    }
    // 下面还有光照强度/位置/圆锥角的变量 因为上面return 所以暂时不用设置
}

void VARender::SetCameraShaderParameters()
{
    vtkMatrix4x4* glTransformMatrix;
    vtkMatrix4x4* modelViewMatrix;
    vtkMatrix3x3* normalMatrix;
    vtkMatrix4x4* projectionMatrix;
    m_spCamera->GetKeyMatrices(modelViewMatrix, normalMatrix, projectionMatrix, glTransformMatrix);

    this->m_mat4InverseProjection->DeepCopy(projectionMatrix);
    this->m_mat4InverseProjection->Invert();
    m_pDrawShader->SetUniformMatrix("in_projectionMatrix", projectionMatrix);
    m_pDrawShader->SetUniformMatrix("in_inverseProjectionMatrix", this->m_mat4InverseProjection.GetPointer());

    this->m_mat4InverseModelView->DeepCopy(modelViewMatrix);
    this->m_mat4InverseModelView->Invert();
    m_pDrawShader->SetUniformMatrix("in_modelViewMatrix", modelViewMatrix);
    m_pDrawShader->SetUniformMatrix("in_inverseModelViewMatrix", this->m_mat4InverseModelView.GetPointer());

    // TODO Take consideration of reduction factor
    float fvalue2[2];
    CommonFunction::ToFloat(this->WindowLowerLeft, fvalue2);
    m_pDrawShader->setVec2("in_windowLowerLeftCorner", fvalue2);

    CommonFunction::ToFloat(1.0 / this->WindowSize[0], 1.0 / this->WindowSize[1], fvalue2);
    m_pDrawShader->setVec2("in_inverseOriginalWindowSize", fvalue2);

    CommonFunction::ToFloat(1.0 / this->WindowSize[0], 1.0 / this->WindowSize[1], fvalue2);
    m_pDrawShader->setVec2("in_inverseWindowSize", fvalue2);
}

void VARender::SetAdvancedShaderParameters(int numComp)
{
    auto blockExt = m_spVolume->GetExtent();
    float fvalue3[3];
    CommonFunction::ToFloat(blockExt[0], blockExt[2], blockExt[4], fvalue3);
    m_pDrawShader->setVec3("in_textureExtentsMin" , &fvalue3);

    CommonFunction::ToFloat(blockExt[1], blockExt[3], blockExt[5], fvalue3);
    m_pDrawShader->setVec3("in_textureExtentsMax",  &fvalue3);

    double avgRange[2] = { VTK_FLOAT_MIN , VTK_FLOAT_MAX };
    float fvalue2[2];
    //this->GetAverageIPScalarRange(avgRange);
    if (avgRange[1] < avgRange[0])
    {
        double tmp = avgRange[1];
        avgRange[1] = avgRange[0];
        avgRange[0] = tmp;
    }
    CommonFunction::ToFloat(avgRange[0], avgRange[1], fvalue2);
    m_pDrawShader->setVec2("in_averageIPRange", &fvalue2);
}

void VARender::GPURender(std::shared_ptr<VAWindow> spWindow)
{
    if (m_pDrawShader == NULL)
    {
        return;
    }
    this->UpdateSamplingDistance();
    RenderSingleInput(spWindow);
}

void VARender::RenderSingleInput(std::shared_ptr<VAWindow> spWindow)
{
    const int independent = m_spVolume->GetVolumeProperty()->GetIndependentComponents();
    const int numComp = m_spVolume->GetLoadedScalars()->GetNumberOfComponents();

    const int numSamplers = (independent ? numComp : 1);
    this->SetMapperShaderParameters(independent, numComp);

    vtkMatrix4x4* wcvc, * vcdc, * wcdc;
    vtkMatrix3x3* norm;
    m_spCamera->GetKeyMatrices(wcvc, norm, vcdc, wcdc);

    this->SetVolumeShaderParameters(independent, numComp, wcvc);
    this->SetLightingShaderParameters(numSamplers);
    this->SetCameraShaderParameters();
    this->SetAdvancedShaderParameters(numComp);
    this->RenderVolumeGeometry();
}

void VARender::UpdateSamplingDistance()
{
    double cellSpacing[3] = { 0.5,0.5,0.5 };
    if (m_spVolume != nullptr && m_spVolume->GetImageData() != nullptr)
    {
        m_spVolume->GetImageData()->GetSpacing(cellSpacing);
    }
    else
    {
        return;
    }

    vtkMatrix4x4* worldToDataset = vtkMatrix4x4::New();
    worldToDataset->Identity();
    double minWorldSpacing = VTK_DOUBLE_MAX;
    int i = 0;
    while (i < 3)
    {
        double tmp = worldToDataset->GetElement(0, i);
        double tmp2 = tmp * tmp;
        tmp = worldToDataset->GetElement(1, i);
        tmp2 += tmp * tmp;
        tmp = worldToDataset->GetElement(2, i);
        tmp2 += tmp * tmp;

        // We use fabs() in case the spacing is negative.
        double worldSpacing = fabs(cellSpacing[i] * sqrt(tmp2));
        if (worldSpacing < minWorldSpacing)
        {
            minWorldSpacing = worldSpacing;
        }
        ++i;
    }

    // minWorldSpacing is the optimal sample distance in world space.
    // To go faster (reduceFactor<1.0), we multiply this distance
    // by 1/reduceFactor.
    this->ActualSampleDistance = static_cast<float>(minWorldSpacing);
    
}

void VARender::RendermultipleInputs()
{
}

void VARender::FinishRendering()
{

}