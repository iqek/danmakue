#pragma once

#include <string>
#include <glm/glm.hpp>

namespace Engine {

// Compiles and links a vertex+fragment GLSL pair
class Shader {
private:
	unsigned int programId = 0;

public:
	Shader(const std::string& vertexSource, const std::string& fragmentSource);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	void Bind() const;
	void Unbind() const;

	void SetMat4(const std::string& name, const glm::mat4& matrix) const;
	void SetVec4(const std::string& name, const glm::vec4& value) const;
	void SetInt(const std::string& name, int value) const;
};

}
