#include "Precompiled.hpp"
#include "Graphics/Shader.hpp"
using namespace Graphics;

namespace
{
    // Log error message.
    #define LogLoadError(filename) "Failed to load a shader from \"" << filename << "\" file! "

    // Constant definitions.
    const GLuint InvalidHandle    = 0;
    const GLint  InvalidAttribute = -1;
    const GLint  InvalidUniform   = -1;

    // Available shader types.
    struct ShaderType
    {
        const char* name;
        GLenum      type;
    };

    const int ShaderTypeCount = 3;
    const ShaderType ShaderTypes[ShaderTypeCount] =
    {
        { "VERTEX_SHADER",   GL_VERTEX_SHADER   },
        { "GEOMETRY_SHADER", GL_GEOMETRY_SHADER },
        { "FRAGMENT_SHADER", GL_FRAGMENT_SHADER },
    };
}

Shader::Shader() :
    m_handle(InvalidHandle),
    m_initialized(false)
{
}

Shader::~Shader()
{
    if(m_handle != InvalidHandle)
    {
        glDeleteProgram(m_handle);
    }
}

bool Shader::Load(std::string filename)
{
    BOOST_ASSERT(!m_initialized);

    // Create an array of shader objects for each type that can be linked.
    GLuint shaderObjects[ShaderTypeCount] = { 0 };

    BOOST_SCOPE_EXIT(&)
    {
        for(int i = 0; i < ShaderTypeCount; ++i)
        {
            glDeleteShader(shaderObjects[i]);
        }
    };

    // Load the shader code from a file.
    std::string shaderCode = Utility::GetTextFileContent(filename);

    if(shaderCode.empty())
    {
        Log() << LogLoadError(filename) << "Couldn't read the file.";
        return false;
    }

    // Extract shader version.
    std::string shaderVersion;

    std::size_t versionStart = shaderCode.find("#version ");
    std::size_t versionEnd   = shaderCode.find('\n', versionStart);

    if(versionStart != std::string::npos)
    {
        shaderVersion = shaderCode.substr(versionStart, versionEnd - versionStart + 1);
        shaderCode.erase(versionStart, versionEnd + 1);
    }

    // Compile shader objects.
    bool shaderObjectsFound = false;

    for(unsigned int i = 0; i < ShaderTypeCount; ++i)
    {
        const ShaderType& shaderType   = ShaderTypes[i];
        GLuint&           shaderObject = shaderObjects[i];

        // Compile shader object if found.
        if(shaderCode.find(shaderType.name) != std::string::npos)
        {
            shaderObjectsFound = true;

            // Create a shader object.
            shaderObject = glCreateShader(shaderType.type);

            if(shaderObject == InvalidHandle)
            {
                Log() << LogLoadError(filename) << "Couldn't create a shader object.";
                return false;
            }

            // Prepare preprocessor define.
            std::string shaderDefine = "#define ";
            shaderDefine += shaderType.name;
            shaderDefine += "\n";

            // Compile shader object code.
            const char* shaderCodePtrs[3] =
            {
                shaderVersion.c_str(),
                shaderDefine.c_str(),
                shaderCode.c_str(),
            };

            glShaderSource(shaderObject, 3, (const GLchar**)&shaderCodePtrs, nullptr);
            glCompileShader(shaderObject);

            // Check compiling results.
            GLint compileStatus = 0;
            glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &compileStatus);

            if(compileStatus == GL_FALSE)
            {
                Log() << LogLoadError(filename) << "Couldn't compile a shader object.";

                GLint errorLength = 0;
                glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &errorLength);

                if(errorLength != 0)
                {
                    std::vector<char> errorText(errorLength);
                    glGetShaderInfoLog(shaderObject, errorLength, &errorLength, &errorText[0]);

                    Log() << "Shader compile errors: \"" << errorText.data() << "\".";
                }

                return false;
            }
        }
    }

    // Check if any shader objects were found.
    if(shaderObjectsFound == false)
    {
        Log() << LogLoadError(filename) << "Couldn't find any shader objects.";
        return false;
    }

    // Create a shader program.
    m_handle = glCreateProgram();

    if(m_handle == InvalidHandle)
    {
        Log() << LogLoadError(filename) << "Couldn't create a shader program.";
        return false;
    }

    BOOST_SCOPE_EXIT(&)
    {
        if(!m_initialized)
        {
            glDeleteProgram(m_handle);
            m_handle = InvalidHandle;
        }
    };

    // Attach compiled shader objects.
    for(unsigned int i = 0; i < ShaderTypeCount; ++i)
    {
        GLuint& shaderObject = shaderObjects[i];

        if(shaderObject != InvalidHandle)
        {
            glAttachShader(m_handle, shaderObject);
        }
    }

    // Link attached shader objects.
    glLinkProgram(m_handle);

    // Detach linked shader objects.
    for(unsigned int i = 0; i < ShaderTypeCount; ++i)
    {
        GLuint& shaderObject = shaderObjects[i];

        if(shaderObject != InvalidHandle)
        {
            glDetachShader(m_handle, shaderObject);
        }
    }

    // Check linking results.
    GLint linkStatus = 0;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &linkStatus);

    if(linkStatus == GL_FALSE)
    {
        Log() << LogLoadError(filename) << "Couldn't link a shader program.";

        GLint errorLength = 0;
        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &errorLength);

        if(errorLength != 0)
        {
            std::vector<char> errorText(errorLength);
            glGetProgramInfoLog(m_handle, errorLength, &errorLength, &errorText[0]);

            Log() << "Shader link errors: \"" << errorText.data() << "\".";
        }
        
        return false;
    }

    // Success!
    Log() << "Loaded a shader from \"" << filename << "\" file.";

    return m_initialized = true;
}

GLint Shader::GetAttribute(std::string name) const
{
    BOOST_ASSERT(m_initialized);

    if(m_handle == InvalidHandle)
        return InvalidAttribute;

    if(name.empty())
        return InvalidAttribute;

    return glGetAttribLocation(m_handle, name.c_str());
}

GLint Shader::GetUniform(std::string name) const
{
    BOOST_ASSERT(m_initialized);

    if(m_handle == InvalidHandle)
        return InvalidUniform;

    if(name.empty())
        return InvalidUniform;

    return glGetUniformLocation(m_handle, name.c_str());
}

bool Shader::IsValid() const
{
    return m_initialized;
}
