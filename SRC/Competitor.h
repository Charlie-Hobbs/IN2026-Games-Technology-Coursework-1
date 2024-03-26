#ifndef  __COMPETITOR_H__
#define __COMPETITOR_H__

#include "Player.h"

class Competitor : public Player
{
	shared_ptr<GameObject> ClosestAsteroid();

	GameObjectList mAsteroids;

	shared_ptr<GameObject> mTarget;

	void LookAtTarget();

public:
	virtual void OnWorldUpdated(GameWorld* world);
	virtual void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object);
	virtual void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);
};

#endif 
