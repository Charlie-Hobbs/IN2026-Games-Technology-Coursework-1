#include "Collectible.h"
#include "BoundingShape.h"
#include "GameObjectType.h"

Collectible::Collectible(ECollectibleType type) : GameObject("Collectible")
{
	mAngle = 180;
	mRotation = 0;
	mPosition.x = rand() / 2.0f;
	mPosition.y = rand() / 2.0f;
	mPosition.z = 0.0f;
	mVelocity.x = 0.0f;
	mVelocity.y = 0.0f;
	mVelocity.z = 0.0f;

	mType = type;
}

Collectible::Collectible(void) : GameObject("Collectible")
{
	mAngle = 180;
	mRotation = 0;
	mPosition.x = rand() / 2.0f;
	mPosition.y = rand() / 2.0f;
	mPosition.z = 0.0f;
	mVelocity.x = 0.0f;
	mVelocity.y = 0.0f;
	mVelocity.z = 0.0f;

	mType = RandomType();
}

Collectible::~Collectible(void)
{

}

bool Collectible::CollisionTest(shared_ptr<GameObject> o)
{
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;

	if (o->GetType() == GameObjectType("Asteroid")) return false;
	if (o->GetType() == GameObjectType("Bullet")) return false;

	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Collectible::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}