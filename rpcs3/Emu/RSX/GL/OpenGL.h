#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <stddef.h>
#include <span>

// Primitive Type Aliases
using GLint        = int;
using GLuint       = unsigned int;
using GLenum       = unsigned int;
using GLsizei      = int;
using GLsync       = void*;
using GLchar       = char;
using GLboolean    = unsigned char;
using GLbitfield   = unsigned int;
using GLuint64     = uint64_t;
using GLuint64EXT  = uint64_t;
using GLsizeiptr   = ptrdiff_t;
using GLvoid       = void;
using GLubyte      = unsigned char;

// Type Aliases mapped straight to types.hpp/geometry.h internal equivalents
using handle64_t   = uint64_t;

// State Definitions & Constants
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_NONE                           0
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_SYNC_STATUS                    0x9114
#define GL_SIGNALED                       0x9119
#define GL_UNSIGNALED                     0x9118
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
#define GL_TIMEOUT_EXPIRED                0x911B
#define GL_CONDITION_SATISFIED            0x911C
#define GL_ALREADY_SIGNALED               0x911A
#define GL_SYNC_FLUSH_COMMANDS_BIT        0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
#define GL_WAIT_FAILED                    0x911D

// Texture Targets & Bindings
#define GL_TEXTURE                        0x1702

// Buffer Object Binding Targets
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_TEXTURE_BUFFER                 0x8C2A
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_COPY_READ_BUFFER               0x8F36
#define GL_COPY_WRITE_BUFFER              0x8F37

// Buffer State Queries (Binding Constants)
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#define GL_UNIFORM_BUFFER_BINDING         0x8A28
#define GL_TEXTURE_BUFFER_BINDING         0x8C2C
#define GL_SHADER_STORAGE_BUFFER_BINDING  0x90D3
#define GL_COPY_READ_BUFFER_BINDING       0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING      0x8F37

// Buffer Storage Mapping Bits & Usage Formats
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_PERSISTENT_BIT             0x0040
#define GL_STREAM_DRAW                    0x88E0
#define GL_R8UI                           0x8238

// Texture Filtering Modes
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_NEAREST_MIPMAP_LINEAR          0x2701
#define GL_LINEAR_MIPMAP_NEAREST          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

// Primitive Data Types
#define GL_BYTE                           0x1400
#define GL_SHORT                          0x1402
#define GL_INT                            0x1404
#define GL_HALF_FLOAT                     0x140B
#define GL_FLOAT                          0x1406
#define GL_DOUBLE                         0x140A

// Primitive Sized Pixel Types
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_UNSIGNED_INT                   0x1405
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366

// Standard GL Enums
#define GL_RED                 0x1903
#define GL_GREEN               0x1904
#define GL_BLUE                0x1905
#define GL_ALPHA               0x1906

#define GL_RG                  0x8227
#define GL_RGB                 0x1907
#define GL_RGBA                0x1908

#define GL_BGR                 0x80E0
#define GL_BGRA                0x80E1

#define GL_STENCIL_INDEX       0x1901
#define GL_DEPTH_COMPONENT     0x1902

// GL Pixel Format Constants
#define GL_UNSIGNED_INT_8_8_8_8                0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV            0x8367
#define GL_UNSIGNED_INT_10_10_10_2             0x8036
#define GL_UNSIGNED_INT_2_10_10_10_REV         0x8368
#define GL_UNSIGNED_INT_24_8                   0x84FA
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV      0x8DAD

// Depth / stencil formats
#define GL_DEPTH_STENCIL              0x84F9
#define GL_STENCIL_INDEX8             0x8D48
#define GL_DEPTH_COMPONENT16          0x81A5
#define GL_DEPTH_COMPONENT32F         0x8CAC
#define GL_DEPTH24_STENCIL8           0x88F0
#define GL_DEPTH32F_STENCIL8          0x8CAD

// Compressed S3TC formats
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

// Common internal formats
#define GL_RGBA8                    0x8058
#define GL_RGB565                   0x8D62
#define GL_RGB5_A1                  0x8057
#define GL_RGBA4                    0x8056

#define GL_R8                       0x8229
#define GL_R16                      0x822A
#define GL_R32F                     0x822E
#define GL_RG8                      0x822B
#define GL_RG16                     0x822C

// Extra scalar type
using GLfloat = float;

// More internal formats
#define GL_RG16F                    0x822F
#define GL_RGBA16F                  0x881A
#define GL_RGBA32F                  0x8814
#define GL_RG8_SNORM                0x8F95

// Sampler wrap modes
#define GL_REPEAT                   0x2901
#define GL_MIRRORED_REPEAT          0x8370
#define GL_CLAMP_TO_EDGE            0x812F
#define GL_CLAMP_TO_BORDER          0x812D
#define GL_MIRROR_CLAMP_EXT         0x8742
#define GL_MIRROR_CLAMP_TO_BORDER_EXT 0x8912

// Compare mode
#define GL_COMPARE_REF_TO_TEXTURE   0x884E

// Texture targets
#define GL_TEXTURE_1D               0x0DE0
#define GL_TEXTURE_2D               0x0DE1
#define GL_TEXTURE_3D               0x806F
#define GL_TEXTURE_CUBE_MAP         0x8513
#define GL_TEXTURE_2D_ARRAY         0x8C1A
#define GL_TEXTURE_2D_MULTISAMPLE   0x9100

// --- Sampler API ---
inline void glGenSamplers(GLsizei, GLuint*) {}
inline void glDeleteSamplers(GLsizei, const GLuint*) {}
inline void glBindSampler(GLuint, GLuint) {}
inline void glSamplerParameteri(GLuint, GLenum, GLint) {}
inline void glSamplerParameterf(GLuint, GLenum, GLfloat) {}

// Active texture
inline void glActiveTexture(GLenum) {}

// Sampler-related enums
#define GL_TEXTURE0             0x84C0
#define GL_SAMPLER_BINDING      0x8919

// --- Pixel store API ---
inline void glPixelStorei(GLenum, GLint) {}

// PACK pixel-store enums
#define GL_PACK_SWAP_BYTES      0x0D00
#define GL_PACK_LSB_FIRST       0x0D01
#define GL_PACK_ROW_LENGTH      0x0D02
#define GL_PACK_IMAGE_HEIGHT    0x806C
#define GL_PACK_SKIP_ROWS       0x0D03
#define GL_PACK_SKIP_PIXELS     0x0D04
#define GL_PACK_SKIP_IMAGES     0x806B
#define GL_PACK_ALIGNMENT       0x0D05

// UNPACK pixel-store enums
#define GL_UNPACK_SWAP_BYTES    0x0CF0
#define GL_UNPACK_LSB_FIRST     0x0CF1
#define GL_UNPACK_ROW_LENGTH    0x0CF2
#define GL_UNPACK_IMAGE_HEIGHT  0x806E
#define GL_UNPACK_SKIP_ROWS     0x0CF3
#define GL_UNPACK_SKIP_PIXELS   0x0CF4
#define GL_UNPACK_SKIP_IMAGES   0x806D
#define GL_UNPACK_ALIGNMENT     0x0CF5

// Extra GL float type
using GLclampf = float;

// Enable / disable
inline void glEnable(GLenum) {}
inline void glDisable(GLenum) {}
inline void glEnablei(GLenum, GLuint) {}
inline void glDisablei(GLenum, GLuint) {}

// Depth state
#define GL_DEPTH_FUNC            0x0B74
#define GL_DEPTH_WRITEMASK       0x0B72
#define GL_DEPTH_CLEAR_VALUE     0x0B73

inline void glDepthFunc(GLenum) {}
inline void glDepthMask(GLboolean) {}
inline void glClearDepth(double) {}

// Stencil state
#define GL_STENCIL_WRITEMASK     0x0B98
#define GL_STENCIL_CLEAR_VALUE   0x0B91
#define GL_BACK                  0x0405

inline void glStencilMask(GLuint) {}
inline void glStencilMaskSeparate(GLenum, GLuint) {}
inline void glClearStencil(GLint) {}
inline void glStencilFunc(GLenum, GLint, GLuint) {}

// Sample coverage / shading
inline void glSampleCoverage(GLclampf, GLboolean) {}
inline void glMinSampleShading(GLclampf) {}


//
// ---- GL FUNCTION STUBS REQUIRED BY RPCS3 ----
//

// Basic state queries
inline GLenum glGetError() { return GL_NONE; }
inline void   glGetIntegerv(GLenum, GLint*) {}

// Buffer operations
inline void   glBindBuffer(GLenum, GLuint) {}

// Debug / labeling
inline void   glObjectLabel(GLenum, GLuint, GLsizei, const char*) {}
inline void   glInsertEventMarkerEXT(GLsizei, const char*) {}

// Sync objects
inline GLsync glFenceSync(GLenum, GLbitfield) { return nullptr; }
inline void   glDeleteSync(GLsync) {}
inline GLenum glClientWaitSync(GLsync, GLbitfield, GLuint64) { return GL_ALREADY_SIGNALED; }
inline void   glGetSynciv(GLsync, GLenum, GLsizei, GLsizei*, GLint*) {}
inline void   glWaitSync(GLsync, GLbitfield, GLuint64) {}

// Shader/program management
inline void   glDeleteShader(GLuint) {}
inline GLuint glCreateProgram() { return 0; }
inline void   glDeleteProgram(GLuint) {}

// Uniform setters
inline void glProgramUniform1i(GLuint, GLint, GLint) {}
inline void glProgramUniform1ui(GLuint, GLint, GLuint) {}
inline void glProgramUniform1f(GLuint, GLint, float) {}
inline void glProgramUniform2i(GLuint, GLint, GLint, GLint) {}
inline void glProgramUniform2f(GLuint, GLint, float, float) {}
inline void glProgramUniform3i(GLuint, GLint, GLint, GLint, GLint) {}
inline void glProgramUniform3f(GLuint, GLint, float, float, float) {}
inline void glProgramUniform4i(GLuint, GLint, GLint, GLint, GLint, GLint) {}
inline void glProgramUniform4f(GLuint, GLint, float, float, float, float) {}
inline void glProgramUniformMatrix3fv(GLuint, GLint, GLsizei, GLboolean, const float*) {}
inline void glProgramUniform1iv(GLuint, GLint, GLsizei, const GLint*) {}

// Bindless texture extensions
inline void glProgramUniformHandleui64ARB(GLuint, GLint, GLuint64) {}
inline void glProgramUniformHandleui64vARB(GLuint, GLint, GLsizei, const GLuint64*) {}

// Shader attachment and linking
inline void glAttachShader(GLuint, GLuint) {}
inline void glBindAttribLocation(GLuint, GLuint, const char*) {}
inline void glBindFragDataLocation(GLuint, GLuint, const char*) {}

// Make Texture Handle Resident ARB
inline void glMakeTextureHandleResidentARB(GLuint64) {}

//
// -----------------------------------------------
//

// ---- Stencil ops ----
inline void glStencilOp(GLenum, GLenum, GLenum) {}
inline void glStencilOpSeparate(GLenum, GLenum, GLenum, GLenum) {}

// ---- Color write / clear ----
#define GL_COLOR_WRITEMASK        0x0C23
#define GL_COLOR_CLEAR_VALUE      0x0C22

inline void glColorMaski(GLuint, GLboolean, GLboolean, GLboolean, GLboolean) {}
inline void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat) {}

// ---- Depth bounds / range ----
inline void glDepthBoundsdNV(double, double) {}
inline void glDepthBoundsEXT(double, double) {}
inline void glDepthRangedNV(double, double) {}
inline void glDepthRange(double, double) {}

// ---- Logic op ----
#define GL_COLOR_LOGIC_OP         0x0BF2
inline void glLogicOp(GLenum) {}

// ---- Line width ----
#define GL_LINE_WIDTH             0x0B21
inline void glLineWidth(GLfloat) {}

// ---- Front face / cull ----
#define GL_FRONT_FACE             0x0B46
#define GL_CULL_FACE_MODE         0x0B45
#define GL_POLYGON_OFFSET_FILL    0x8037

inline void glFrontFace(GLenum) {}
inline void glCullFace(GLenum) {}

// ---- Polygon offset ----
inline void glPolygonOffset(GLfloat, GLfloat) {}

// ---- Sample mask ----
#define GL_SAMPLE_MASK_VALUE          0x8E51
inline void glSampleMaski(GLuint, GLbitfield) {}

// ---- Sample coverage value ----
#define GL_SAMPLE_COVERAGE_VALUE      0x80AA

// ---- Min sample shading value ----
#define GL_MIN_SAMPLE_SHADING_VALUE   0x8C37

// ---- Clip distances ----
#define GL_CLIP_DISTANCE0             0x3000

// ---- Polygon mode ----
#define GL_FRONT_AND_BACK             0x0408
inline void glPolygonMode(GLenum, GLenum) {}

// ---- Program binding ----
inline void glUseProgram(GLuint) {}

// ---- Texture binding ----
inline void glBindTexture(GLenum, GLuint) {}

// ---- FBO bits ----
#define GL_COLOR_BUFFER_BIT           0x00004000
#define GL_DEPTH_BUFFER_BIT           0x00000100
#define GL_STENCIL_BUFFER_BIT         0x00000400

// ---- FBO attachments ----
#define GL_COLOR_ATTACHMENT0          0x8CE0
#define GL_DEPTH_ATTACHMENT           0x8D00
#define GL_STENCIL_ATTACHMENT         0x8D20
#define GL_DEPTH_STENCIL_ATTACHMENT   0x821A

// ---- Framebuffer targets ----
#define GL_READ_FRAMEBUFFER           0x8CA8
#define GL_DRAW_FRAMEBUFFER           0x8CA9

// ---- Missing Fixes Added For gui_application.cpp ----
#define GL_FRONT                          0x0404
#define GL_FRAMEBUFFER                    0x8D40
#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_BUFFER_UPDATE_BARRIER_BIT      0x00000200
#define GL_FIXED                          0x140C
#define GL_INT_2_10_10_10_REV             0x8D9F
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B

using GLushort = unsigned short;

// ---- these stubs have warnings ----
// ---- warning: unused parameter ---- gui_application.cpp

// 1. Tell Clang to save the current warning settings
#pragma clang diagnostic push
// 2. Explicitly ignore unused parameter warnings for this specific block
#pragma clang diagnostic ignored "-Wunused-parameter"

// ---- warning stubs ----
inline void glBindFramebuffer(GLenum target, GLuint framebuffer) {}
inline void glMemoryBarrier(GLbitfield barriers) {}
inline void glTextureBarrier() {}
inline void glTextureBarrierNV() {}
inline void glBindVertexArray(GLuint array) {}
inline void glGenVertexArrays(GLsizei n, GLuint* arrays) {}
inline void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) {}
inline void glNamedFramebufferTextureEXT(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) {}

// ---- VAO management ----
inline void glDeleteVertexArrays(GLsizei, const GLuint*) {}
inline void glEnableVertexAttribArray(GLuint) {}
inline void glDisableVertexAttribArray(GLuint) {}

// ---- VAO-related enums ----
#define GL_VERTEX_ARRAY_BINDING 0x85B5

// ---- Missing Fixes Added For GLOverlays and Occlusion Queries ----
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_ANY_SAMPLES_PASSED             0x8C2F

// ---- Vertex attribute setters ----
inline void glVertexAttrib1f(GLuint index, GLfloat x) {}
inline void glVertexAttrib1d(GLuint index, double x) {}

inline void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {}
inline void glVertexAttrib2d(GLuint index, double x, double y) {}

inline void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {}
inline void glVertexAttrib3d(GLuint index, double x, double y, double z) {}

inline void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {}
inline void glVertexAttrib4d(GLuint index, double x, double y, double z, double w) {}

inline void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {}

// ---- Restore original warning rules right after the stubs end
#pragma clang diagnostic pop