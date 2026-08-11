#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Engine {

class Renderer2D;

struct Bullet {
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 velocity{ 0.0f, 0.0f };
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	float radius = 4.0f;
	bool active = false;
};

// A fixed-capacity array of bullets, not EnTT entities. 
// Spawning reuses the first inactive slot instead of allocating
// and Update/Draw are linear scans, this is performance-critical
class BulletPool {
private:
	std::vector<Bullet> bullets;
	glm::vec2 boundsMin{ 0.0f, 0.0f };
	glm::vec2 boundsMax{ 0.0f, 0.0f };

public:
	BulletPool(std::size_t capacity, glm::vec2 boundsMin, glm::vec2 boundsMax);

	void Spawn(glm::vec2 position, glm::vec2 velocity, float radius, glm::vec4 color);
	void Update(float deltaTime);
	void Draw(const Renderer2D& renderer) const;
};

}
