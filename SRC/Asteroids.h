#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include "Competitor.h"

class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world);
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

private:
	GameObjectList mGameObjectList;

	bool mObjectsBeingRemoved;

	bool mGameStarted;

	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;
	shared_ptr<GUILabel> mStartScreenLabel;
	shared_ptr<GUILabel> mAmmoCountLabel;

	uint mLevel;
	uint mAsteroidCount;
	uint mCollectibleCount;

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	void CreateCollectibles(const uint num_collectibles);
	void UpdateLivesLabel(const int lives);
	void UpdateAmmoLabel(const uint bullets);
	void OnGameOver();
	void OnGameStart();
	void InitialiseDemo();

	shared_ptr<GameObject> CreateExplosion();

	const static uint ON_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;

	const static uchar START_GAME_KEY = 's';

	const static uint NUM_START_ASTEROIDS = 3;
	const static uint NUM_POWERUPS = 3;
	const static uint PLAYER_START_LIVES = 3;
	const static uint PLAYER_START_BULLETS = 100;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;

	void RemoveAllObjects()
	{
		mObjectsBeingRemoved = true;
		for each (auto g in mGameObjectList)
		{
			if (g.get() != NULL) mGameWorld->RemoveObject(g);
		}

		mGameObjectList.clear();

		mAsteroidCount = 0;
		mCollectibleCount = 0;

		mObjectsBeingRemoved = false;
	}
};

#endif