#ifndef __COLLECTIBLE_H__
#define __COLLECTIBLE_H__

#include "GameObject.h"
#include <random>

enum ECollectibleType
{
	ExtraLife,
	COLLECTIBLE_TYPE_LENGTH
};

class Collectible : public GameObject
{
	ECollectibleType mType;
public:
	Collectible(ECollectibleType type);
	~Collectible(void);

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
	ECollectibleType GetCollectibleType() const { return mType; }
};

#endif