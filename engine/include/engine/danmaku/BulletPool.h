#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Engine {

class Renderer2D;

struct Bullet {
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 velocity{ 0.0f, 0.0f };
	glm::vec2 acceleration{ 0.0f, 0.0f };
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	float radius = 4.0f;
	float homingStrength = 0.0f;
	bool active = false;
};

// A fixed-capacity array of bullets, not EnTT entities, for performance
class BulletPool {
private:
	std::vector<Bullet> bullets;
	glm::vec2 boundsMin{ 0.0f, 0.0f };
	glm::vec2 boundsMax{ 0.0f, 0.0f };

public:
	BulletPool(std::size_t capacity, glm::vec2 boundsMin, glm::vec2 boundsMax);

	// acceleration covers speeding up/slowing down and parabolic curves.
	// homingStrength (0 = off) steers velocity toward Update()'s target each frame.
	void Spawn(glm::vec2 position, glm::vec2 velocity, float radius, glm::vec4 color, glm::vec2 acceleration = glm::vec2(0.0f), float homingStrength = 0.0f);
	void Update(float deltaTime, glm::vec2 targetPosition);
	void Draw(const Renderer2D& renderer) const;

	// Overlap test against active bullets; doesn't touch them
	bool CheckCollision(glm::vec2 boxCenter, glm::vec2 boxSize) const;

	// Same overlap test, but deactivates every bullet that hits
	bool ConsumeCollisions(glm::vec2 boxCenter, glm::vec2 boxSize);
};

}
