// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class vtkOpenGLGPUVolumeRayCastMapper
 * @brief OpenGL implementation of volume rendering through ray-casting.
 *
 * @section multi Multiple Inputs

 * When multiple inputs are rendered simultaneously, it is possible to
 * composite overlapping areas correctly. Inputs are connected directly to
 * the mapper and their parameters (transfer functions, transformations, etc.)
 * are specified through standard vtkVolume instances. These vtkVolume
 * instances are to be registered in a special vtkProp3D, vtkMultiVolume.
 *
 * Structures related to a particular active input are stored in a helper
 * class (vtkVolumeInputHelper) and helper structures are kept in a
 * port-referenced map (VolumeInputMap). The order of the inputs in the
 * map is important as it defines the order in which parameters are
 * bound to uniform variables (transformation matrices, bias, scale and every
 * other required rendering parameter).
 *
 * A separate code path is used when rendering multiple-inputs in order to
 * facilitate the co-existance of these two modes (single/multiple), due to
 * current feature incompatibilities with multiple inputs (e.g. texture-streaming,
 * cropping, etc.).
 *
 * @note A limited set of the mapper features are currently supported for
 * multiple inputs:
 *
 * - Blending
 *   - Composite (front-to-back)
 *
 * - Transfer functions (defined separately for per input)
 *   - 1D color
 *   - 1D scalar opacity
 *   - 1D gradient magnitude opacity
 *   - 2D scalar-gradient magnitude
 *
 * - Point and cell data
 *   - With the limitation that all of the inputs are assumed to share the same
 *     name/id.
 *
 * - Inputs
 *   - 1-component inputs with vtkVolumeProperty::IndependentComponentsOn()
 *   - 4-component inputs with vtkVolumeProperty::IndependentComponentsOff()
 *
 * @sa vtkGPUVolumeRayCastMapper vtkVolumeInputHelper vtkVolumeTexture
 * vtkMultiVolume
 *
 */

#ifndef vtkOpenGLGPUVolumeRayCastMapper_h
#define vtkOpenGLGPUVolumeRayCastMapper_h
#include <map> // For methods

#include "vtk_glad.h"

#include "vtkGPUVolumeRayCastMapper.h"
#include "vtkNew.h"                          // For vtkNew
#include "vtkRenderingVolumeOpenGL2Module.h" // For export macro
#include "vtkShader.h"                       // For methods
#include "vtkSmartPointer.h"                 // For smartptr
#include "vtkWrappingHints.h"                // For VTK_MARSHALAUTO
#include "vtkVolumeInputHelper.h"
#include "vtkHardwareSelector.h"
#include "vtkVolumeProperty.h"
#include "vtkVolumeTexture.h"
#include "vtkLightCollection.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLVolumeGradientOpacityTable.h"
#include "vtkOpenGLVolumeMaskGradientOpacityTransferFunction2D.h"
#include "vtkOpenGLVolumeMaskTransferFunction2D.h"
#include "vtkOpenGLVolumeOpacityTable.h"
#include "vtkOpenGLVolumeRGBTable.h"
#include "vtkOpenGLVolumeTransferFunction2D.h"
#include "vtkOpenGLFramebufferObject.h"
#include "vtkPolyData.h"
#include "vtkOpenGLShaderCache.h"
#include "vtkPolyDataMapper.h"
#include "vtkMultiVolume.h"
#include "vtkTextureObject.h"
#include "vtkOpenGLVertexArrayObject.h"

 // C/C++ includes
#include <cassert>
#include <limits>
#include <map>
#include <sstream>
#include <string>

VTK_ABI_NAMESPACE_BEGIN
class vtkGenericOpenGLResourceFreeCallback;
class vtkImplicitFunction;
class vtkOpenGLCamera;
class vtkOpenGLTransferFunctions2D;
class vtkOpenGLVolumeGradientOpacityTables;
class vtkOpenGLVolumeOpacityTables;
class vtkOpenGLVolumeRGBTables;
class vtkShaderProgram;
class vtkTextureObject;
class vtkVolume;
class vtkVolumeInputHelper;
class vtkVolumeTexture;
class vtkOpenGLShaderProperty;

class CORE_EXPORTS VTK_MARSHALAUTO vtkOpenGLGPUVolumeRayCastMapper
  : public vtkGPUVolumeRayCastMapper
{
public:
  static vtkOpenGLGPUVolumeRayCastMapper* New();

  enum Passes
  {
    RenderPass,
    DepthPass = 1
  };

  vtkTypeMacro(vtkOpenGLGPUVolumeRayCastMapper, vtkGPUVolumeRayCastMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Low level API to enable access to depth texture in
  // RenderToTexture mode. It will return either nullptr if
  // RenderToImage was never turned on or texture captured
  // the last time RenderToImage was on.
  vtkTextureObject* GetDepthTexture();

  // Description:
  // Low level API to enable access to color texture in
  // RenderToTexture mode. It will return either nullptr if
  // RenderToImage was never turned on or texture captured
  // the last time RenderToImage was on.
  vtkTextureObject* GetColorTexture();

  // Description:
  // Low level API to export the depth texture as vtkImageData in
  // RenderToImage mode.
  void GetDepthImage(vtkImageData* im) override;

  // Description:
  // Low level API to export the color texture as vtkImageData in
  // RenderToImage mode.
  void GetColorImage(vtkImageData* im) override;

  // Description:
  // Mapper can have multiple passes and internally it will set
  // the state. The state can not be set externally explicitly
  // but can be set indirectly depending on the options set by
  // the user.
  vtkGetMacro(CurrentPass, int);

  // Sets a depth texture for this mapper to use
  // This allows many mappers to use the same
  // texture reducing GPU usage. If this is set
  // the standard depth texture code is skipped
  // The depth texture should be activated
  // and deactivated outside of this class
  void SetSharedDepthTexture(vtkTextureObject* nt);

  /**
   * Set a fixed number of partitions in which to split the volume
   * during rendering. This will force by-block rendering without
   * trying to compute an optimum number of partitions.
   */
  void SetPartitions(unsigned short x, unsigned short y, unsigned short z);

  /**
   *  Load the volume texture into GPU memory.  Actual loading occurs
   *  in vtkVolumeTexture::LoadVolume.  The mapper by default loads data
   *  lazily (at render time), so it is most commonly not necessary to call
   *  this function.  This method is only exposed in order to support on-site
   *  loading which is useful in cases where the user needs to know a-priori
   *  whether loading will succeed or not.
   */
  bool PreLoadData(vtkRenderer* ren, vtkVolume* vol);

  // Description:
  // Delete OpenGL objects.
  // \post done: this->OpenGLObjectsCreated==0
  void ReleaseGraphicsResources(vtkWindow* window) override;

protected:
  vtkOpenGLGPUVolumeRayCastMapper();
  ~vtkOpenGLGPUVolumeRayCastMapper() override;

  vtkGenericOpenGLResourceFreeCallback* ResourceCallback;

  // Description:
  // Build vertex and fragment shader for the volume rendering
  void BuildDepthPassShader(
    vtkRenderer* ren, vtkVolume* vol, int noOfComponents, int independentComponents);

  // Description:
  // Build vertex and fragment shader for the volume rendering
  void BuildShader(vtkRenderer* ren);

  // TODO Take these out as these are no longer needed
  // Methods called by the AMR Volume Mapper.
  void PreRender(vtkRenderer* vtkNotUsed(ren), vtkVolume* vtkNotUsed(vol),
    double vtkNotUsed(datasetBounds)[6], double vtkNotUsed(scalarRange)[2],
    int vtkNotUsed(noOfComponents), unsigned int vtkNotUsed(numberOfLevels)) override
  {
  }

  // \pre input is up-to-date
  void RenderBlock(vtkRenderer* vtkNotUsed(ren), vtkVolume* vtkNotUsed(vol),
    unsigned int vtkNotUsed(level)) override
  {
  }

  void PostRender(vtkRenderer* vtkNotUsed(ren), int vtkNotUsed(noOfComponents)) override {}

  // Description:
  // Rendering volume on GPU
  void GPURender(vtkRenderer* ren, vtkVolume* vol) override;

  // Description:
  // Method that performs the actual rendering given a volume and a shader
  void DoGPURender(vtkRenderer* ren, vtkOpenGLCamera* cam, vtkShaderProgram* shaderProgram,
    vtkOpenGLShaderProperty* shaderProperty);

  // Description:
  // Update the reduction factor of the render viewport (this->ReductionFactor)
  // according to the time spent in seconds to render the previous frame
  // (this->TimeToDraw) and a time in seconds allocated to render the next
  // frame (allocatedTime).
  // \pre valid_current_reduction_range: this->ReductionFactor>0.0 && this->ReductionFactor<=1.0
  // \pre positive_TimeToDraw: this->TimeToDraw>=0.0
  // \pre positive_time: allocatedTime>0
  // \post valid_new_reduction_range: this->ReductionFactor>0.0 && this->ReductionFactor<=1.0
  void ComputeReductionFactor(double allocatedTime);

  // Description:
  // Returns a reduction ratio for each dimension
  // This ratio is computed from MaxMemoryInBytes and MaxMemoryFraction so that the total
  // memory usage of the resampled image, by the returned ratio, does not exceed
  // `MaxMemoryInBytes * MaxMemoryFraction`
  // \pre input is up-to-date
  // \post Aspect ratio of image is always kept
  // - for a 1D image `ratio[1] == ratio[2] == 1`
  // - for a 2D image `ratio[0] == ratio[1]` and `ratio[2] == 1`
  // - for a 3D image `ratio[0] == ratio[1] == ratio[2]`
  void GetReductionRatio(double* ratio) override;

  // Description:
  // Empty implementation.
  int IsRenderSupported(
    vtkRenderWindow* vtkNotUsed(window), vtkVolumeProperty* vtkNotUsed(property)) override
  {
    return 1;
  }

  ///@{
  /**
   *  \brief vtkOpenGLRenderPass API
   */
  vtkMTimeType GetRenderPassStageMTime(vtkVolume* vol);

  /**
   * Create the basic shader template strings before substitutions
   */
  void GetShaderTemplate(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkOpenGLShaderProperty* p);

  /**
   * Perform string replacements on the shader templates
   */
  void ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);

  /**
   *  RenderPass string replacements on shader templates called from
   *  ReplaceShaderValues.
   */
  void ReplaceShaderCustomUniforms(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkOpenGLShaderProperty* p);
  void ReplaceShaderBase(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderTermination(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderShading(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderCompute(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderCropping(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderClipping(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderMasking(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderPicking(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderRTT(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkRenderer* ren, vtkVolume* vol, int numComps);
  void ReplaceShaderRenderPass(
    std::map<vtkShader::Type, vtkShader*>& shaders, vtkVolume* vol, bool prePass);

  /**
   *  Update parameters from RenderPass
   */
  void SetShaderParametersRenderPass();

  /**
   *  Caches the vtkOpenGLRenderPass::RenderPasses() information.
   *  Note: Do not dereference the pointers held by this object. There is no
   *  guarantee that they are still valid!
   */
  vtkNew<vtkInformation> LastRenderPassInfo;
  ///@}

  double ReductionFactor;
  int CurrentPass;

public:
    //------------------------------------------------------------------------------
    class CORE_EXPORTS vtkInternal
    {
    public:
        // Constructor
        //--------------------------------------------------------------------------
        vtkInternal(vtkOpenGLGPUVolumeRayCastMapper* parent)
        {
            this->Parent = parent;
            this->ValidTransferFunction = false;
            this->LoadDepthTextureExtensionsSucceeded = false;
            this->CameraWasInsideInLastUpdate = false;
            this->CubeVBOId = 0;
            this->CubeVAOId = 0;
            this->CubeIndicesId = 0;
            this->DepthTextureObject = nullptr;
            this->DepthCopyColorTextureObject = nullptr;
            this->DepthCopyFBO = nullptr;
            this->SharedDepthTextureObject = false;
            this->TextureWidth = 1024;
            this->ActualSampleDistance = 1.0;
            this->CurrentMask = nullptr;
            this->TextureSize[0] = this->TextureSize[1] = this->TextureSize[2] = -1;
            this->WindowLowerLeft[0] = this->WindowLowerLeft[1] = 0;
            this->WindowSize[0] = this->WindowSize[1] = 0;
            this->LastDepthPassWindowSize[0] = this->LastDepthPassWindowSize[1] = 0;
            this->LastRenderToImageWindowSize[0] = 0;
            this->LastRenderToImageWindowSize[1] = 0;
            this->CurrentSelectionPass = vtkHardwareSelector::MIN_KNOWN_PASS - 1;

            this->TotalNumberOfLights = 0;
            this->NumberPositionalLights = 0;
            this->DefaultLighting = true;

            this->NeedToInitializeResources = false;
            this->ShaderCache = nullptr;

            this->FBO = nullptr;
            this->RTTDepthBufferTextureObject = nullptr;
            this->RTTDepthTextureObject = nullptr;
            this->RTTColorTextureObject = nullptr;
            this->RTTDepthTextureType = -1;

            this->DPFBO = nullptr;
            this->DPDepthBufferTextureObject = nullptr;
            this->DPColorTextureObject = nullptr;
            this->PreserveViewport = false;
            this->PreserveGLState = false;
            this->DepthMaskOverride = false;

            this->Partitions[0] = this->Partitions[1] = this->Partitions[2] = 1;
        }

        // Destructor
        //--------------------------------------------------------------------------
        ~vtkInternal()
        {
            if (this->DepthTextureObject)
            {
                this->DepthTextureObject->Delete();
                this->DepthTextureObject = nullptr;
            }

            if (this->FBO)
            {
                this->FBO->Delete();
                this->FBO = nullptr;
            }

            if (this->RTTDepthBufferTextureObject)
            {
                this->RTTDepthBufferTextureObject->Delete();
                this->RTTDepthBufferTextureObject = nullptr;
            }

            if (this->RTTDepthTextureObject)
            {
                this->RTTDepthTextureObject->Delete();
                this->RTTDepthTextureObject = nullptr;
            }

            if (this->RTTColorTextureObject)
            {
                this->RTTColorTextureObject->Delete();
                this->RTTColorTextureObject = nullptr;
            }

            if (this->ImageSampleFBO)
            {
                this->ImageSampleFBO->Delete();
                this->ImageSampleFBO = nullptr;
            }

            for (auto& tex : this->ImageSampleTexture)
            {
                tex = nullptr;
            }
            this->ImageSampleTexture.clear();
            this->ImageSampleTexNames.clear();

            if (this->ImageSampleVAO)
            {
                this->ImageSampleVAO->Delete();
                this->ImageSampleVAO = nullptr;
            }
            this->DeleteMaskTransfer();

            // Do not delete the shader programs - Let the cache clean them up.
            this->ImageSampleProg = nullptr;
        }

        // Helper methods
        //--------------------------------------------------------------------------
        template <typename T>
        static void ToFloat(const T& in1, const T& in2, float(&out)[2]);
        template <typename T>
        static void ToFloat(const T& in1, const T& in2, const T& in3, float(&out)[3]);
        template <typename T>
        static void ToFloat(T* in, float* out, int noOfComponents);
        template <typename T>
        static void ToFloat(T(&in)[3], float(&out)[3]);
        template <unsigned int N, typename T>
        static std::array<float, N> ToFloat(T* in);
        template <typename T>
        static void ToFloat(T(&in)[2], float(&out)[2]);
        template <typename T>
        static void ToFloat(T& in, float& out);
        template <typename T>
        static void ToFloat(T(&in)[4][2], float(&out)[4][2]);
        template <typename T, int SizeX, int SizeY>
        static void CopyMatrixToVector(T* matrix, float* matrixVec, int offset);
        template <typename T, int SizeSrc>
        static void CopyVector(T* srcVec, T* dstVec, int offset);

        ///@{
        /**
        * \brief Setup and clean-up transfer functions for each vtkVolumeInputHelper
        * and masks.
        */
        void UpdateTransferFunctions(vtkRenderer* ren);

        void RefreshMaskTransfer(vtkRenderer* ren, vtkVolumeInputHelper& input);
        int UpdateMaskTransfer(vtkRenderer* ren, vtkVolume* vol, unsigned int component);
        void SetupMaskTransfer(vtkRenderer* ren);
        void ReleaseGraphicsMaskTransfer(vtkWindow* window);
        void DeleteMaskTransfer();
        ///@}

        void UpdateTransfer2DYAxisArray(vtkRenderer* ren, vtkVolume* vol);

        bool LoadMask(vtkRenderer* ren, vtkVolume* vol);

        // Update the depth sampler with the current state of the z-buffer. The
        // sampler is used for z-buffer compositing with opaque geometry during
        // ray-casting (rays are early-terminated if hidden begin opaque geometry).
        void CaptureDepthTexture(vtkRenderer* ren);

        // Test if camera is inside the volume geometry
        bool IsCameraInside(vtkRenderer* ren, vtkVolume* vol, double geometry[24]);

        ///@{
        /**
        * Update volume's proxy-geometry and draw it
        */
        bool IsGeometryUpdateRequired(vtkRenderer* ren, vtkVolume* vol, double geometry[24]);
        void RenderVolumeGeometry(
            vtkRenderer* ren, vtkShaderProgram* prog, vtkVolume* vol, double geometry[24]);
        ///@}

        // Update cropping params to shader
        void SetCroppingRegions(vtkShaderProgram* prog, double loadedBounds[6]);

        // Update clipping params to shader
        void SetClippingPlanes(vtkRenderer* ren, vtkShaderProgram* prog, vtkVolume* vol);

        // Update the ray sampling distance. Sampling distance should be updated
        // before updating opacity transfer functions.
        void UpdateSamplingDistance(vtkRenderer* ren);

        // Check if the mapper should enter picking mode.
        void CheckPickingState(vtkRenderer* ren);

        // Look for property keys used to control the mapper's state.
        // This is necessary for some render passes which need to ensure
        // a specific OpenGL state when rendering through this mapper.
        void CheckPropertyKeys(vtkVolume* vol);

        // Configure the vtkHardwareSelector to begin a picking pass. This call
        // changes GL_BLEND, so it needs to be called before constructing
        // vtkVolumeStateRAII.
        void BeginPicking(vtkRenderer* ren);

        // Update the prop Id if hardware selection is enabled.
        void SetPickingId(vtkRenderer* ren);

        // Configure the vtkHardwareSelector to end a picking pass.
        void EndPicking(vtkRenderer* ren);

        // Load OpenGL extensiosn required to grab depth sampler buffer
        void LoadRequireDepthTextureExtensions(vtkRenderWindow* renWin);

        // Create GL buffers
        void CreateBufferObjects();

        // Dispose / free GL buffers
        void DeleteBufferObjects();

        // Convert vtkTextureObject to vtkImageData
        void ConvertTextureToImageData(vtkTextureObject* texture, vtkImageData* output);

        // Render to texture for final rendering
        void SetupRenderToTexture(vtkRenderer* ren);
        void ExitRenderToTexture(vtkRenderer* ren);

        // Render to texture for depth pass
        void SetupDepthPass(vtkRenderer* ren);
        void RenderContourPass(vtkRenderer* ren);
        void ExitDepthPass(vtkRenderer* ren);
        void RenderWithDepthPass(vtkRenderer* ren, vtkOpenGLCamera* cam, vtkMTimeType renderPassTime);

        void RenderSingleInput(vtkRenderer* ren, vtkOpenGLCamera* cam, vtkShaderProgram* prog);

        void RenderMultipleInputs(vtkRenderer* ren, vtkOpenGLCamera* cam, vtkShaderProgram* prog);

        ///@{
        /**
        * Update shader parameters.
        */
        void SetLightingShaderParameters(
            vtkRenderer* ren, vtkShaderProgram* prog, vtkVolume* vol, int numberOfSamplers);

        /**
        * Global parameters.
        */
        void SetMapperShaderParameters(
            vtkShaderProgram* prog, vtkRenderer* ren, int independent, int numComponents);

        /**
        * Per input data/ per component parameters.
        */
        void SetVolumeShaderParameters(
            vtkShaderProgram* prog, int independent, int noOfComponents, vtkMatrix4x4* modelViewMat);
        void BindTransformations(vtkShaderProgram* prog, vtkMatrix4x4* modelViewMat);

        /**
        * Transformation parameters.
        */
        void SetCameraShaderParameters(vtkShaderProgram* prog, vtkRenderer* ren, vtkOpenGLCamera* cam);

        /**
        * Feature specific.
        */
        void SetMaskShaderParameters(vtkShaderProgram* prog, vtkVolumeProperty* prop, int noOfComponents);
        void SetRenderToImageParameters(vtkShaderProgram* prog);
        void SetAdvancedShaderParameters(vtkRenderer* ren, vtkShaderProgram* prog, vtkVolume* vol,
            vtkVolumeTexture::VolumeBlock* block, int numComp);
        ///@}

        void FinishRendering(int numComponents);

        vtkMTimeType LastModifiedLightTime(vtkLightCollection* lights);

        inline bool ShaderRebuildNeeded(
            vtkCamera* cam, vtkVolume* vol, vtkMTimeType renderPassTime, vtkRenderer* ren);
        bool VolumePropertyChanged = true;

        ///@{
        /**
        * Image XY-Sampling
        * Render to an internal framebuffer with lower resolution than the currently
        * bound one (hence casting less rays and improving performance). The rendered
        * image is subsequently rendered as a texture-mapped quad (linearly
        * interpolated) to the default (or previously attached) framebuffer. If a
        * vtkOpenGLRenderPass is attached, a variable number of render targets are
        * supported (as specified by the RenderPass). The render targets are assumed
        * to be ordered from GL_COLOR_ATTACHMENT0 to GL_COLOR_ATTACHMENT$N$, where
        * $N$ is the number of targets specified (targets of the previously bound
        * framebuffer as activated through ActivateDrawBuffers(int)). Without a
        * RenderPass attached, it relies on FramebufferObject to re-activate the
        * appropriate previous DrawBuffer.
        *
        * \sa vtkOpenGLRenderPass vtkOpenGLFramebufferObject
        */
        void BeginImageSample(vtkRenderer* ren);
        bool InitializeImageSampleFBO(vtkRenderer* ren);
        void EndImageSample(vtkRenderer* ren);
        size_t GetNumImageSampleDrawBuffers(vtkVolume* vol);
        ///@}

        ///@{
        /**
        * Allocate and update input data. A list of active ports is maintained
        * by the parent class. This list is traversed to update internal structures
        * used during rendering.
        */
        bool UpdateInputs(vtkRenderer* ren, vtkVolume* vol);

        /**
        * Cleanup resources of inputs that have been removed.
        */
        void ClearRemovedInputs(vtkWindow* win);

        /**
        * Forces transfer functions in all of the active vtkVolumeInputHelpers to
        * re-initialize in the next update. This is essential if the order in
        * AssembledInputs changes (inputs are added or removed), given that variable
        * names cached in vtkVolumeInputHelper instances are indexed.
        */
        void ForceTransferInit();
        ///@}

        vtkVolume* GetActiveVolume()
        {
            return this->MultiVolume ? this->MultiVolume : this->Parent->AssembledInputs[0].Volume;
        }
        int GetComponentMode(vtkVolumeProperty* prop, vtkDataArray* array) const;

        void ReleaseRenderToTextureGraphicsResources(vtkWindow* win);
        void ReleaseImageSampleGraphicsResources(vtkWindow* win);
        void ReleaseDepthPassGraphicsResources(vtkWindow* win);

        // Private member variables
        //--------------------------------------------------------------------------
        vtkOpenGLGPUVolumeRayCastMapper* Parent;

        bool ValidTransferFunction;
        bool LoadDepthTextureExtensionsSucceeded;
        bool CameraWasInsideInLastUpdate;

        GLuint CubeVBOId;
        GLuint CubeVAOId;
        GLuint CubeIndicesId;

        vtkTextureObject* DepthTextureObject;
        vtkTextureObject* DepthCopyColorTextureObject;
        vtkOpenGLFramebufferObject* DepthCopyFBO;
        bool SharedDepthTextureObject;

        int TextureWidth;

        float ActualSampleDistance;

        int LastProjectionParallel;
        int TextureSize[3];
        int WindowLowerLeft[2];
        int WindowSize[2];
        int LastDepthPassWindowSize[2];
        int LastRenderToImageWindowSize[2];

        int TotalNumberOfLights;
        bool DefaultLighting;
        int NumberPositionalLights;

        std::ostringstream ExtensionsStringStream;

        vtkSmartPointer<vtkOpenGLVolumeMaskTransferFunction2D> LabelMapTransfer2D;
        vtkSmartPointer<vtkOpenGLVolumeMaskGradientOpacityTransferFunction2D> LabelMapGradientOpacity;

        vtkTimeStamp ShaderBuildTime;

        vtkNew<vtkMatrix4x4> InverseProjectionMat;
        vtkNew<vtkMatrix4x4> InverseModelViewMat;
        vtkNew<vtkMatrix4x4> InverseVolumeMat;

        vtkNew<vtkMatrix4x4> TempMatrix4x4;

        vtkSmartPointer<vtkPolyData> BBoxPolyData;
        vtkSmartPointer<vtkVolumeTexture> CurrentMask;

        vtkTimeStamp InitializationTime;
        vtkTimeStamp MaskUpdateTime;
        vtkTimeStamp ReleaseResourcesTime;
        vtkTimeStamp DepthPassTime;
        vtkTimeStamp DepthPassSetupTime;
        vtkTimeStamp SelectionStateTime;
        int CurrentSelectionPass;
        bool IsPicking;

        bool NeedToInitializeResources;
        bool PreserveViewport;
        bool PreserveGLState;
        bool DepthMaskOverride;

        vtkShaderProgram* ShaderProgram;
        vtkOpenGLShaderCache* ShaderCache;

        vtkOpenGLFramebufferObject* FBO;
        vtkTextureObject* RTTDepthBufferTextureObject;
        vtkTextureObject* RTTDepthTextureObject;
        vtkTextureObject* RTTColorTextureObject;
        int RTTDepthTextureType;

        vtkOpenGLFramebufferObject* DPFBO;
        vtkTextureObject* DPDepthBufferTextureObject;
        vtkTextureObject* DPColorTextureObject;

        vtkOpenGLFramebufferObject* ImageSampleFBO = nullptr;
        std::vector<vtkSmartPointer<vtkTextureObject>> ImageSampleTexture;
        std::vector<std::string> ImageSampleTexNames;
        vtkShaderProgram* ImageSampleProg = nullptr;
        vtkOpenGLVertexArrayObject* ImageSampleVAO = nullptr;
        size_t NumImageSampleDrawBuffers = 0;
        bool RebuildImageSampleProg = false;
        bool RenderPassAttached = false;

        bool Transfer2DUseGradient = true;
        vtkSmartPointer<vtkVolumeTexture> Transfer2DYAxisScalars;
        vtkTimeStamp Transfer2DYAxisScalarsUpdateTime;

        //vtkNew<vtkContourFilter> ContourFilter;
        vtkNew<vtkPolyDataMapper> ContourMapper;
        vtkNew<vtkActor> ContourActor;

        unsigned short Partitions[3];
        vtkMultiVolume* MultiVolume = nullptr;

        std::vector<float> VolMatVec, InvMatVec, TexMatVec, InvTexMatVec, TexEyeMatVec, CellToPointVec,
            TexMinVec, TexMaxVec, EyePosVec, ScaleVec, BiasVec, StepVec, SpacingVec, RangeVec;
    };

public:
  using VolumeInput = vtkVolumeInputHelper;
  using VolumeInputMap = std::map<int, vtkVolumeInputHelper>;
  VolumeInputMap AssembledInputs;

  vtkInternal* GetImpl() { return Impl; }
  const vtkInternal* GetImpl() const { return Impl; }

private:
  class vtkInternal;
  vtkInternal* Impl;

  friend class vtkVolumeTexture;

  vtkOpenGLGPUVolumeRayCastMapper(const vtkOpenGLGPUVolumeRayCastMapper&) = delete;
  void operator=(const vtkOpenGLGPUVolumeRayCastMapper&) = delete;
};

VTK_ABI_NAMESPACE_END
#endif // vtkOpenGLGPUVolumeRayCastMapper_h
