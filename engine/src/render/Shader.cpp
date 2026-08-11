#include "engine/render/Shader.h"
#include "engine/core/Log.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Engine {

namespace {

unsigned int CompileStage(unsigned int stage, const std::string& source){
	unsigned int id = glCreateShader(stage);

	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if(!success){
		char log[512];
		glGetShaderInfoLog(id, sizeof(log), nullptr, log);
		ENGINE_CORE_ERROR("Shader compile error: {}", log);
	}

	return id;
}

}

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource){
	unsigned int vertexId = CompileStage(GL_VERTEX_SHADER, vertexSource);
	unsigned int fragmentId = CompileStage(GL_FRAGMENT_SHADER, fragmentSource);

	programId = glCreateProgram();
	glAttachShader(programId, vertexId);
	glAttachShader(programId, fragmentId);
	glLinkProgram(programId);

	int success = 0;
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if(!success){
		char log[512];
		glGetProgramInfoLog(programId, sizeof(log), nullptr, log);
		ENGINE_CORE_ERROR("Shader link error: {}", log);
	}

	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);
}

Shader::~Shader(){
	glDeleteProgram(programId);
}

void Shader::Bind() const{
	glUseProgram(programId);
}

void Shader::Unbind() const{
	glUseProgram(0);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& matrix) const{
	int location = glGetUniformLocation(programId, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const{
	int location = glGetUniformLocation(programId, name.c_str());
	glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::SetInt(const std::string& name, int value) const{
	int location = glGetUniformLocation(programId, name.c_str());
	glUniform1i(location, value);
}

}
