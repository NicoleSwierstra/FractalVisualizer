#pragma once

#include <string>
#include <unordered_map>
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"

struct ShaderProgramSource {
	std::string Vertex;
	std::string Geometry;
	std::string Fragment;
};

class Shader {
private:
	unsigned int m_ShaderId;
	unsigned int texture;
	std::unordered_map<std::string, unsigned int> uniform_cache;
public:
	std::unordered_map<std::string, unsigned int> uniformstrings;

	Shader(const std::string& filepath);
	Shader(const std::string& vertex, const std::string& fragment);
	~Shader();

	void Bind() const;
	void UnBind() const;	

	void SetV2DUniforms(const std::string& name, double x, double y);
	void SetV4DUniforms(const std::string& name, double x, double y, double z, double w);
	void SetV2Uniforms(const std::string& name, float x, float y);
	void SetV4Uniforms(const std::string& name, float x, float y, float z, float w);
	void SetMat4Uniforms(const std::string& name, glm::mat4& matrix);
	void SetIntUniforms(const std::string& name, int i);
	void SetFloatUniforms(const std::string& name, float i);
	void SetDoubleUniforms(const std::string& name, double i);
private:
	unsigned int GetGLUniformLocation(const std::string& name);
	ShaderProgramSource ParseShader(const std::string& filepath);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	unsigned int compileShader(const std::string source, unsigned int type);
};