#include "Competitor.h"
#include "VectorMaths.h"

void Competitor::OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		mAsteroids.push_back(object);
	}
}

void Competitor::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	Player::OnObjectRemoved(world, object);

	if (object->GetType() == GameObjectType("Asteroid"))
	{
		auto it = std::find(mAsteroids.begin(), mAsteroids.end(), object);
		if (it != mAsteroids.end())
		{
			mAsteroids.erase(it);
		}
	}
}