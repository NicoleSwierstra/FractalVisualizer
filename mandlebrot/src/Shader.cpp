#include "shader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

ShaderProgramSource Shader::ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, GEOMETRY = 1, FRAGMENT = 2
    };

    std::string line;
    std::stringstream ss[3];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else if (line.find("geometry") != std::string::npos) {
                type = ShaderType::GEOMETRY;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(), ss[1].str(), ss[2].str() };
}

unsigned int Shader::compileShader(const std::string source, unsigned int type) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)malloc(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << 
            (type == GL_VERTEX_SHADER ? "Vertex" : type == GL_FRAGMENT_SHADER ? "Fragment" : "Geometry") 
            << "Shader compiler error:" << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(vertexShader  , GL_VERTEX_SHADER  );
    unsigned int fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);
    unsigned int gs = compileShader(geometryShader, GL_GEOMETRY_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glAttachShader(program, gs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(gs);
    glDeleteShader(fs);

    return program;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fs = compileShader(fragmentShader, GL_FRAGMENT_SHADER);
    
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}


Shader::Shader(const std::string& filepath) : m_ShaderId(0) {
    ShaderProgramSource source = ParseShader(filepath);
    m_ShaderId = source.Geometry.length() > 1 ? CreateShader(source.Vertex, source.Fragment, source.Geometry) 
        : CreateShader(source.Vertex, source.Fragment);
}

Shader::Shader(const std::string& vertex, const std::string& fragment) : m_ShaderId(0) {
    CreateShader(vertex, fragment);
}

Shader::~Shader()
{
    glDeleteProgram(m_ShaderId);
}

void Shader::Bind() const
{
    try {
        glUseProgram(m_ShaderId);
    }
    catch (int i) {
        std::cout << i << std::endl;
    }
}

void Shader::UnBind() const
{
    glUseProgram(0);
}

void Shader::SetV2DUniforms(const std::string& name, double x, double y)
{
    glUniform2d(GetGLUniformLocation(name), x, y);
}

void Shader::SetV4DUniforms(const std::string& name, double x, double y, double z, double w)
{
    glUniform4d(GetGLUniformLocation(name), x, y, z, w);
}

void Shader::SetV4Uniforms(const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(GetGLUniformLocation(name), x, y, z, w);
}

void Shader::SetV2Uniforms(const std::string& name, float x, float y)
{
    glUniform2f(GetGLUniformLocation(name), x, y);
}

void Shader::SetMat4Uniforms(const std::string& name, glm::mat4& matrix)
{
    glUniformMatrix4fv(GetGLUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}
void Shader::SetIntUniforms(const std::string& name, int i) {
    glUniform1i(GetGLUniformLocation(name), i);
}

void Shader::SetFloatUniforms(const std::string& name, float i)
{
    glUniform1f(GetGLUniformLocation(name), i);
}

void Shader::SetDoubleUniforms(const std::string& name, double i)
{
    glUniform1d(GetGLUniformLocation(name), i);
}

unsigned int Shader::GetGLUniformLocation(const std::string& name)
{
    if (uniform_cache.find(name) != uniform_cache.end())
        return uniform_cache[name];
    unsigned int location = glGetUniformLocation(m_ShaderId, name.c_str());
    if (location != -1) {
        uniform_cache[name] = location;
        return location;
    }
    std::cout << "uniform " << name << " not found." << std::endl;
    return -1;
}


