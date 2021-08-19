//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  Renderer_GLES10.cpp
//
//  OpenGL ES 1.0 rendering functions.
//

#if defined(USE_OPENGLES_10)

#include "Log.h"
#include "Settings.h"
#include "renderers/Renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles.h>

namespace Renderer
{
    static SDL_GLContext sdlContext = nullptr;
    static GLuint whiteTexture = 0;

    static GLenum convertBlendFactor(const Blend::Factor _blendFactor)
    {
        // clang-format off
        switch (_blendFactor) {
            case Blend::ZERO:                { return GL_ZERO;                } break;
            case Blend::ONE:                 { return GL_ONE;                 } break;
            case Blend::SRC_COLOR:           { return GL_SRC_COLOR;           } break;
            case Blend::ONE_MINUS_SRC_COLOR: { return GL_ONE_MINUS_SRC_COLOR; } break;
            case Blend::SRC_ALPHA:           { return GL_SRC_ALPHA;           } break;
            case Blend::ONE_MINUS_SRC_ALPHA: { return GL_ONE_MINUS_SRC_ALPHA; } break;
            case Blend::DST_COLOR:           { return GL_DST_COLOR;           } break;
            case Blend::ONE_MINUS_DST_COLOR: { return GL_ONE_MINUS_DST_COLOR; } break;
            case Blend::DST_ALPHA:           { return GL_DST_ALPHA;           } break;
            case Blend::ONE_MINUS_DST_ALPHA: { return GL_ONE_MINUS_DST_ALPHA; } break;
            default:                         { return GL_ZERO;                }
        }
        // clang-format on
    }

    static GLenum convertTextureType(const Texture::Type _type)
    {
        // clang-format off
        switch (_type) {
            case Texture::RGBA:  { return GL_RGBA;  } break;
            case Texture::ALPHA: { return GL_ALPHA; } break;
            default:             { return GL_ZERO;  }
        }
        // clang-format on
    }

    void setupWindow()
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }

    bool createContext()
    {
        sdlContext = SDL_GL_CreateContext(getSDLWindow());
        SDL_GL_MakeCurrent(getSDLWindow(), sdlContext);

        std::string vendor =
            glGetString(GL_VENDOR) ? reinterpret_cast<const char*>(glGetString(GL_VENDOR)) : "";
        std::string renderer =
            glGetString(GL_RENDERER) ? reinterpret_cast<const char*>(glGetString(GL_RENDERER)) : "";
        std::string version =
            glGetString(GL_VERSION) ? reinterpret_cast<const char*>(glGetString(GL_VERSION)) : "";
        std::string extensions = glGetString(GL_EXTENSIONS) ?
                                     reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)) :
                                     "";

        LOG(LogInfo) << "GL vendor: " << vendor;
        LOG(LogInfo) << "GL renderer: " << renderer;
        LOG(LogInfo) << "GL version: " << version;
        LOG(LogInfo) << "EmulationStation renderer: OpenGL ES 1.0";
        LOG(LogInfo) << "Checking available OpenGL ES extensions...";
        std::string glExts = glGetString(GL_EXTENSIONS) ?
                                 reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)) :
                                 "";
        LOG(LogInfo) << "GL_OES_texture_npot: "
                     << (extensions.find("GL_OES_texture_npot") != std::string::npos ? "OK" :
                                                                                       "MISSING");

        uint8_t data[4] = {255, 255, 255, 255};
        whiteTexture = createTexture(Texture::RGBA, false, false, true, 1, 1, data);

        GL_CHECK_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GL_CHECK_ERROR(glEnable(GL_TEXTURE_2D));
        GL_CHECK_ERROR(glEnable(GL_BLEND));
        GL_CHECK_ERROR(glPixelStorei(GL_PACK_ALIGNMENT, 1));
        GL_CHECK_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        GL_CHECK_ERROR(glEnableClientState(GL_VERTEX_ARRAY));
        GL_CHECK_ERROR(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
        GL_CHECK_ERROR(glEnableClientState(GL_COLOR_ARRAY));

        return true;
    }

    void destroyContext()
    {
        SDL_GL_DeleteContext(sdlContext);
        sdlContext = nullptr;
    }

    unsigned int createTexture(const Texture::Type type,
                               const bool linearMinify,
                               const bool linearMagnify,
                               const bool repeat,
                               const unsigned int width,
                               const unsigned int height,
                               void* data)
    {
        const GLenum textureType = convertTextureType(type);
        unsigned int texture;

        GL_CHECK_ERROR(glGenTextures(1, &texture));
        GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, texture));

        GL_CHECK_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                       repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));
        GL_CHECK_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                       repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE));
        GL_CHECK_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                       linearMinify ? GL_LINEAR : GL_NEAREST));
        GL_CHECK_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                       linearMagnify ? GL_LINEAR : GL_NEAREST));

        GL_CHECK_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, textureType, width, height, 0, textureType,
                                    GL_UNSIGNED_BYTE, data));

        return texture;
    }

    void destroyTexture(const unsigned int _texture)
    {
        GL_CHECK_ERROR(glDeleteTextures(1, &_texture));
    }

    void updateTexture(const unsigned int _texture,
                       const Texture::Type _type,
                       const unsigned int _x,
                       const unsigned _y,
                       const unsigned int _width,
                       const unsigned int _height,
                       void* _data)
    {
        const GLenum type = convertTextureType(_type);

        GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, _texture));
        GL_CHECK_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, _x, _y, _width, _height, type,
                                       GL_UNSIGNED_BYTE, _data));
        GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, whiteTexture));
    }

    void bindTexture(const unsigned int _texture)
    {
        if (_texture == 0)
            GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, whiteTexture));
        else
            GL_CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, _texture));
    }

    void drawLines(const Vertex* _vertices,
                   const unsigned int _numVertices,
                   const Blend::Factor _srcBlendFactor,
                   const Blend::Factor _dstBlendFactor)
    {
        GL_CHECK_ERROR(glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].pos));
        GL_CHECK_ERROR(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].tex));
        GL_CHECK_ERROR(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &_vertices[0].col));

        GL_CHECK_ERROR(
            glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor)));

        GL_CHECK_ERROR(glDrawArrays(GL_LINES, 0, _numVertices));
    }

    void drawTriangleStrips(const Vertex* _vertices,
                            const unsigned int _numVertices,
                            const glm::mat4& _trans,
                            const Blend::Factor _srcBlendFactor,
                            const Blend::Factor _dstBlendFactor,
                            const shaderParameters& _parameters)
    {
        GL_CHECK_ERROR(glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].pos));
        GL_CHECK_ERROR(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].tex));
        GL_CHECK_ERROR(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &_vertices[0].col));

        GL_CHECK_ERROR(
            glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor)));

        GL_CHECK_ERROR(glDrawArrays(GL_TRIANGLE_STRIP, 0, _numVertices));
    }

    void setProjection(const glm::mat4& _projection)
    {
        GL_CHECK_ERROR(glMatrixMode(GL_PROJECTION));
        GL_CHECK_ERROR(glLoadMatrixf((GLfloat*)&_projection));
    }

    void setMatrix(const glm::mat4& _matrix)
    {
        glm::mat4 matrix{_matrix};
        matrix[3] = glm::round(matrix[3]);

        GL_CHECK_ERROR(glMatrixMode(GL_MODELVIEW));
        GL_CHECK_ERROR(glLoadMatrixf((GLfloat*)&matrix));
    }

    void setViewport(const Rect& _viewport)
    {
        // glViewport starts at the bottom left of the window.
        GL_CHECK_ERROR(glViewport(_viewport.x, getWindowHeight() - _viewport.y - _viewport.h,
                                  _viewport.w, _viewport.h));
    }

    void setScissor(const Rect& _scissor)
    {
        if ((_scissor.x == 0) && (_scissor.y == 0) && (_scissor.w == 0) && (_scissor.h == 0)) {
            GL_CHECK_ERROR(glDisable(GL_SCISSOR_TEST));
        }
        else {
            // glScissor starts at the bottom left of the window.
            GL_CHECK_ERROR(glScissor(_scissor.x, getWindowHeight() - _scissor.y - _scissor.h,
                                     _scissor.w, _scissor.h));
            GL_CHECK_ERROR(glEnable(GL_SCISSOR_TEST));
        }
    }

    void setSwapInterval()
    {
        if (Settings::getInstance()->getBool("VSync")) {
            // Adaptive VSync seems to be nonfunctional or having issues on some hardware
            // and drivers, so only attempt to apply regular VSync.
            if (SDL_GL_SetSwapInterval(1) == 0) {
                LOG(LogInfo) << "Enabling VSync...";
            }
            else {
                LOG(LogWarning) << "Could not enable VSync: " << SDL_GetError();
            }
        }
        else {
            SDL_GL_SetSwapInterval(0);
            LOG(LogInfo) << "Disabling VSync...";
        }
    }

    void swapBuffers()
    {
        SDL_GL_SwapWindow(getSDLWindow());
        GL_CHECK_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

} // namespace Renderer

#endif // USE_OPENGLES_10
