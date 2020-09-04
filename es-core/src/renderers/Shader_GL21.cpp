//
//  Shader_GL21.cpp
//
//  OpenGL 2.1 GLSL shader functions.
//

#if defined(USE_OPENGL_21)

#include "Shader_GL21.h"

#include "renderers/Renderer.h"
#include "resources/ResourceManager.h"
#include "Log.h"

namespace Renderer
{
    Renderer::Shader::Shader()
            : mProgramID(-1),
            shaderTextureSize(-1),
            shaderTextureCoord(-1),
            shaderColor(-1),
            shaderSaturation(-1)
    {
    }

    Renderer::Shader::~Shader()
    {
        deleteProgram(mProgramID);
    }

    void Renderer::Shader::loadShaderFile(const std::string& path, GLenum shaderType)
    {
        std::string preprocessorDefines;
        std::string shaderCode;

        // This will load the entire GLSL source code into the string variable.
        const ResourceData& shaderData = ResourceManager::getInstance()->getFileData(path);
        shaderCode.assign((const char*)shaderData.ptr.get(), shaderData.length);

        // Define the GLSL version (version 120 = OpenGL 2.1).
        preprocessorDefines = "#version 120\n";

        // Define the preprocessor constants that will let the shader compiler know whether
        // the VERTEX or FRAGMENT portion of the code should be used.
        if (shaderType == GL_VERTEX_SHADER)
            preprocessorDefines += "#define VERTEX\n";
        else if (shaderType == GL_FRAGMENT_SHADER)
            preprocessorDefines += "#define FRAGMENT\n";

        shaderVector.push_back(std::make_tuple(
                path, preprocessorDefines + shaderCode, shaderType));
    }

    bool Renderer::Shader::createProgram()
    {
        GLint programSuccess;

        mProgramID = glCreateProgram();

        // Compile and attach all shaders that have been loaded.
        for (auto it = shaderVector.cbegin(); it != shaderVector.cend(); it++) {
            GLuint currentShader = glCreateShader(std::get<2>(*it));
            GLchar const* shaderCodePtr = std::get<1>(*it).c_str();

            glShaderSource(currentShader, 1, (const GLchar**)&shaderCodePtr, nullptr);
            glCompileShader(currentShader);

            GLint shaderCompiled;
            glGetShaderiv(currentShader, GL_COMPILE_STATUS, &shaderCompiled);

            if (shaderCompiled != GL_TRUE) {
                LOG(LogError) << "OpenGL error: Unable to compile shader " <<
                        currentShader << " (" << std::get<0>(*it) << ").";
                printShaderInfoLog(currentShader, std::get<2>(*it));
                return false;
            }

            GL_CHECK_ERROR(glAttachShader(mProgramID, currentShader));
        }

        glLinkProgram(mProgramID);

        glGetProgramiv(mProgramID, GL_LINK_STATUS, &programSuccess);
        if (programSuccess != GL_TRUE) {
            LOG(LogError) << "OpenGL error: Unable to link program " << mProgramID << ".";
            printProgramInfoLog(mProgramID);
            return false;
        }

        return true;
    }

    void Renderer::Shader::deleteProgram(GLuint programID)
    {
        GL_CHECK_ERROR(glDeleteProgram(programID));
    }

    void Renderer::Shader::getVariableLocations(GLuint programID)
    {
        // Some of the variable names are chosen to be compatible with the RetroArch GLSL shaders.
        shaderTextureSize = glGetUniformLocation(mProgramID, "TextureSize");
        shaderTextureCoord = glGetAttribLocation(mProgramID, "TexCoord");
        shaderColor = glGetAttribLocation(mProgramID, "COLOR");
        shaderSaturation = glGetUniformLocation(mProgramID, "saturation");
    }

    void Renderer::Shader::setTextureSize(std::array<GLfloat, 2> shaderVec2)
    {
        GL_CHECK_ERROR(glUniform2f(shaderTextureSize, shaderVec2[0], shaderVec2[1]));
    }

    void Renderer::Shader::setTextureCoordinates(std::array<GLfloat, 4> shaderVec4)
    {
        glEnableVertexAttribArray(shaderTextureCoord);
        glVertexAttribPointer(shaderTextureCoord, 4, GL_FLOAT, GL_FALSE, 0,
                (const GLvoid*)(uintptr_t)&shaderVec4);
    }

    void Renderer::Shader::setColor(std::array<GLfloat, 4> shaderVec4)
    {
        GL_CHECK_ERROR(glUniform4f(shaderColor, shaderVec4[0],
                        shaderVec4[1], shaderVec4[2], shaderVec4[3]));
    }

    void Renderer::Shader::setSaturation(GLfloat saturation)
    {
        GL_CHECK_ERROR(glUniform1f(shaderSaturation, saturation));
    }

    void Renderer::Shader::activateShaders()
    {
        GL_CHECK_ERROR(glUseProgram(mProgramID));
    }

    void Renderer::Shader::deactivateShaders()
    {
        GL_CHECK_ERROR(glUseProgram(0));
    }

    GLuint Renderer::Shader::getProgramID()
    {
        return mProgramID;
    }

    void Renderer::Shader::printProgramInfoLog(GLuint programID)
    {
        if (glIsProgram(programID)) {
            int logLength;
            int maxLength;

            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<char> infoLog(maxLength);

            glGetProgramInfoLog(programID, maxLength, &logLength, &infoLog.front());

            if (logLength > 0) {
                LOG(LogDebug) << "Renderer_GL21::printProgramLog():\n" <<
                        std::string(infoLog.begin(), infoLog.end());
            }
        }
        else {
            LOG(LogError) << "OpenGL error: " << programID << " is not a program.";
        }
    }

    void Renderer::Shader::printShaderInfoLog(GLuint shaderID, GLenum shaderType)
    {
        if (glIsShader(shaderID)) {
            int logLength;
            int maxLength;

            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<char> infoLog(maxLength);

            glGetShaderInfoLog(shaderID, maxLength, &logLength, &infoLog.front());

            if (logLength > 0) {
                LOG(LogDebug) << "Renderer_GL21::printShaderLog(): Error in " <<
                        (shaderType == GL_VERTEX_SHADER ? "VERTEX section:\n" :
                        "FRAGMENT section:\n") << std::string(infoLog.begin(), infoLog.end());
            }
        }
        else {
            LOG(LogError) << "OpenGL error: " << shaderID << " is not a shader.";
        }
    }

} // Renderer

#endif // USE_OPENGL_21
