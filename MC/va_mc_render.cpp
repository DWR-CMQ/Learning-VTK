#include "va_mc_render.h"

MCRender::MCRender(vtkImageData* image, glm::ivec2 windowSize, glm::ivec2 fbSize)
{
    m_iWorkGroupX = m_iWorkGroupY = 0;
    m_spCamera = std::make_shared<MCCamera>(30);
    m_spVolume = std::make_shared<MCVolume>(image);
    alpha_scale = 1.0f;
    m_iCamUBOID = 0;
    m_WindowSize = windowSize;
    m_FrameBufferSize = fbSize;

    double range[2];
    image->GetScalarRange(range);
    max_dataset_val = static_cast<int>(range[1]);
    min_dataset_val = static_cast<int>(range[0]);
    max_val = max_dataset_val;
    min_val = min_dataset_val;

    double spacing[3];
    image->GetSpacing(spacing);
    voxel_size = glm::vec3(static_cast<float>(spacing[0]), static_cast<float>(spacing[1]), static_cast<float>(spacing[2]));

    int dims[3];
    image->GetDimensions(dims);
    tex3D_dim = glm::ivec3(dims[0], dims[1], dims[2]);
}

MCRender::~MCRender()
{
}

void MCRender::SetUp()
{
    SetUpFBO();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_iFBOID);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    
    //m_iVolumeTex3D = m_spVolume->ConvertImageDataToTexture3D();

    void* data = NULL;
    TextureInfo stInfo;
    stInfo = m_spVolume->GetDataType();
    stInfo.internalFormat = GL_R16I;
    stInfo.type = GL_SHORT;
    auto spData = m_spVolume->GetImageData();
    if (spData == nullptr)
    {
        std::cout << "SetUp data is nullptr" << std::endl;
        return;
    }
    std::cout << "SetUp data is normal" << std::endl;
    data = spData->GetScalarPointer();

    glGenTextures(1, &vol_tex3D);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, vol_tex3D);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (tex3D_dim.x % 4 != 0)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    glTexImage3D(GL_TEXTURE_3D,
        0,
        stInfo.internalFormat,
        stInfo.width,
        stInfo.height,
        stInfo.depth,
        0,
        stInfo.format,
        stInfo.type,
        data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    //glBindTexture(GL_TEXTURE_3D, 0);

}

void MCRender::SetUpFBO()
{
	glGenFramebuffers(1, &m_iFBOID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFBOID);

	glGenTextures(1, &m_iFBOTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_iFBOTexID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_FrameBufferSize.x, m_FrameBufferSize.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iFBOTexID, 0);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
            throw std::runtime_error("Framebuffer not complete. Error code: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
            throw std::runtime_error("Framebuffer not complete. Error code: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        else if (status == GL_FRAMEBUFFER_UNDEFINED)
            throw std::runtime_error("Framebuffer not complete. Error code: GL_FRAMEBUFFER_UNDEFINED");
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
            throw std::runtime_error("Framebuffer not complete. Error code: GL_FRAMEBUFFER_UNSUPPORTED");
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
            throw std::runtime_error("Framebuffer not complete. Error code: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
            throw std::runtime_error("Framebuffer not complete. Error code: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
        else
            throw std::runtime_error("Framebuffer not complete.");
    }
}

void MCRender::LoadShader()
{
    m_spShader = std::make_shared<MCShader>("F://ToGithub//Learning-VTK//MC//shaders//raycast.comp");
    int iWorkGroup[3];
    glGetProgramiv(m_spShader->ID, GL_COMPUTE_WORK_GROUP_SIZE, iWorkGroup);
    m_iWorkGroupX = m_WindowSize.x / iWorkGroup[0];
    m_iWorkGroupY = m_WindowSize.y / iWorkGroup[1];

    glUseProgram(m_spShader->ID);
    glUniform1f(0, alpha_scale);
    glUniform3f(1, voxel_size.x, voxel_size.y, voxel_size.z);
    glUniform1i(2, min_val);
    glUniform1i(3, max_val);
    glUniform1i(4, 1);

    m_spCamera->resetCamera();
    glUniform1i(5, 1);
    glUniform1i(6, 0);
    m_spCamera->resetCamera();
    //glBindTextureUnit(1, m_iVolumeTex3D);
}

void MCRender::SetupUBO(bool is_update)
{
    std::vector<float> cam_data;
    cam_data.clear();
    m_spCamera->setUBO(cam_data);
    bool bInit = false;
    if (!is_update || m_iCamUBOID == 0)
    {
        bInit = true;
    }

    if (bInit)
    {
        glGenBuffers(1, &m_iCamUBOID);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, m_iCamUBOID);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * cam_data.size(), cam_data.data(), GL_DYNAMIC_DRAW);
    if (bInit)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_iCamUBOID);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void MCRender::Render()
{
    glUseProgram(m_spShader->ID);
    if (m_spCamera->is_changed)
    {
        SetupUBO(true);
    }
    glBindImageTexture(0, m_iFBOTexID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(m_iWorkGroupX, m_iWorkGroupY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBlitFramebuffer(0, 0, m_FrameBufferSize.x, m_FrameBufferSize.y,
                        0, 0, m_FrameBufferSize.x, m_FrameBufferSize.y,
                        GL_COLOR_BUFFER_BIT,
                        GL_LINEAR);
}
