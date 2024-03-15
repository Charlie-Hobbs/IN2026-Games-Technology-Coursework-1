#ifndef __COLLECTIBLE_H__
#define __COLLECTIBLE_H__

#include "GameObject.h"

enum ECollectibleType
{
	ExtraLife,
	ExtraBullets,
	Armour,
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
	void SetCollectibleType(ECollectibleType type) { mType = type; }

	static ECollectibleType RandomType()
	{
		return (ECollectibleType)(rand() % COLLECTIBLE_TYPE_LENGTH);
	}
};

#endif