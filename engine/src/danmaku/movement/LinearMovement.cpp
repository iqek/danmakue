#include "engine/danmaku/movement/LinearMovement.h"
#include "engine/scene/Components.h"

namespace Engine {

LinearMovement::LinearMovement(glm::vec2 velocity):
	velocity(velocity)
{
}

void LinearMovement::Update(float deltaTime, Transform& transform){
	transform.position += velocity * deltaTime;
}

}
