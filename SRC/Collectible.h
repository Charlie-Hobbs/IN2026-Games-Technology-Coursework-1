#ifndef __COLLECTIBLE_H__
#define __COLLECTIBLE_H__

#include "GameObject.h"

enum ECollectibleType
{
	ExtraLife,
	ExtraBullets,
	COLLECTIBLE_TYPE_LENGTH
};

class Collectible : public GameObject
{
	ECollectibleType mType;
public:
	Collectible(ECollectibleType type);
	Collectible(void);
	~Collectible(void);

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
	ECollectibleType GetCollectibleType() const { return mType; }
};

#endif