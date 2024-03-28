#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "Collectible.h"
#include "VectorMaths.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mCollectibleCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	mGameStarted = false;
	mObjectsBeingRemoved = false;

	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");
	Animation *life_anim = AnimationManager::GetInstance().CreateAnimationFromFile("extralife", 128, 128, 128, 128, "extralife.png");
	Animation *ammo_anim = AnimationManager::GetInstance().CreateAnimationFromFile("extrabullets", 128, 128, 128, 128, "ammo.png");
	Animation *enemy_anim = AnimationManager::GetInstance().CreateAnimationFromFile("enemy", 128, 8192, 128, 128, "enemy_fs.png");
	Animation *armour_anim = AnimationManager::GetInstance().CreateAnimationFromFile("armour", 128, 128, 128, 128, "armour.png");

	//Create the GUI
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);


	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	InitialiseDemo();

	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	switch (key)
	{
	case ' ':
		if (!mGameStarted) return;
		mSpaceship->Shoot();
		UpdateAmmoLabel(mSpaceship->GetBulletCount());
		break;
	case START_GAME_KEY:
		if (!mGameStarted)
		{
			OnGameStart();
		}
		break;
	default:
		break;
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (!mGameStarted) return;
	switch (key)
	{
	// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
	// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
	// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
	// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (!mGameStarted) return;
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	} 
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (mObjectsBeingRemoved) return; // objects being removed deliberately shouldnt result in this method running

	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		world->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0) 
		{ 
			SetTimer(500, START_NEXT_LEVEL); 
		}
	}
	else if (object->GetType() == GameObjectType("Collectible"))
	{
		--mCollectibleCount;
		Collectible* col = (Collectible*)object.get();
		switch (col->GetCollectibleType())
		{
		case ECollectibleType(ExtraLife):
			mPlayer.AddLife();
			UpdateLivesLabel(mPlayer.GetLives());
			break;
		case ECollectibleType(ExtraBullets):
			mSpaceship->ReplenishAmmo(100); // maybe add variable for the number of bullets to give?
			UpdateAmmoLabel(mSpaceship->GetBulletCount());
			break;
		case ECollectibleType(Armour):
			mSpaceship->SetAmoured(true); // maybe change colour?
			break;
		default:
			break;
		}
	}

}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = NUM_START_ASTEROIDS + 2 * mLevel;
		CreateAsteroids(num_asteroids);

		uint numCollectiblesToCreate = NUM_POWERUPS - mCollectibleCount;
		CreateCollectibles(numCollectiblesToCreate);
	}

	if (value == ON_GAME_OVER)
	{
		if (!mGameStarted)
		{
			RemoveAllObjects();

			mSpaceship->Reset();
			mGameWorld->AddObject(mSpaceship);

			mLevel = 0;
			CreateAsteroids(NUM_START_ASTEROIDS);
			CreateCollectibles(NUM_POWERUPS);
			mScoreKeeper.ResetScore();
			mPlayer.SetLives(PLAYER_START_LIVES);
			mSpaceship->SetAmmo(PLAYER_START_BULLETS);
			UpdateAmmoLabel(mSpaceship->GetBulletCount());
			UpdateLivesLabel(mPlayer.GetLives());
		}
		else OnGameOver();
	}

	if (value == ALLOW_DEMO_SHOOT) mAllowDemoShoot = true;
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();

	mSpaceship->SetAmmo(PLAYER_START_BULLETS);

	mSpaceship->SetBulletSpeed(30.0f);

	// Return the spaceship so it can be added to the world
	return mSpaceship;
}


void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount += num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
		mGameObjectList.push_back(asteroid);
	}
}

void Asteroids::CreateCollectibles(const uint num_collectibles)
{
	mCollectibleCount += num_collectibles;
	for (uint i = 0; i < num_collectibles; ++i)
	{
		shared_ptr<GameObject> collectible = make_shared<Collectible>();
		collectible->SetBoundingShape(make_shared<BoundingSphere>(collectible->GetThisPtr(), 5.0f));
		collectible->SetScale(0.1f);

		std::string animName;
		Collectible* col = (Collectible*) collectible.get();
		col->SetCollectibleType(Collectible::RandomType());
		switch (col->GetCollectibleType())
		{
		case ECollectibleType(ExtraLife):
			animName = "extralife";
			break;
		case ECollectibleType(ExtraBullets):
			animName = "extrabullets";
			break;
		case ECollectibleType(Armour):
			animName = "armour";
			break;
		default:
			animName = "extralife";
			break;
		}

		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName(animName);
		shared_ptr<Sprite> extraLife_sprite =
			make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);

		collectible->SetSprite(extraLife_sprite); // change sprite and animation

		mGameWorld->AddObject(collectible);
		mGameObjectList.push_back(collectible);
	}
}

void Asteroids::CreateGUI()
{
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	mAmmoCountLabel = shared_ptr<GUILabel>(new GUILabel("Ammo: 100"));
	mAmmoCountLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_RIGHT);
	mAmmoCountLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	shared_ptr<GUIComponent> ammo_count_component
		= static_pointer_cast<GUIComponent>(mAmmoCountLabel);
	mGameDisplay->GetContainer()->AddComponent(ammo_count_component, GLVector2f(1.0f, 0.0f));

	std::string gameOverText = "GAME OVER \n Press ";
	gameOverText += START_GAME_KEY;
	gameOverText += " to restart.";

	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel(gameOverText));
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameOverLabel->SetVisible(false);
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

	// so i can use the START_GAME_KEY in the string
	std::string startText = "Press ";
	startText += START_GAME_KEY;
	startText += " to start game!";

	mStartScreenLabel = shared_ptr<GUILabel>(new GUILabel(startText));
	mStartScreenLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartScreenLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mStartScreenLabel->SetVisible(true);
	shared_ptr<GUIComponent> start_screen_component
		= static_pointer_cast<GUIComponent>(mStartScreenLabel);
	mGameDisplay->GetContainer()->AddComponent(start_screen_component, GLVector2f(0.5f, 0.5f));

	// so they arent visible before game start
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mAmmoCountLabel->SetVisible(false);
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	UpdateLivesLabel(lives_left);

	if (lives_left > 0) 
	{ 
		SetTimer(1000, CREATE_NEW_PLAYER); 
	}
	else
	{
		SetTimer(500, ON_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}

void Asteroids::UpdateLivesLabel(const int lives)
{
	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);
}

void Asteroids::UpdateAmmoLabel(const uint bullets)
{
	// Format the bullets left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Ammo: " << bullets;
	// Get the lives left message as a string
	std::string ammo_msg = msg_stream.str();
	mAmmoCountLabel->SetText(ammo_msg);
}

void Asteroids::OnGameOver()
{
	mLivesLabel->SetVisible(false);
	mScoreLabel->SetVisible(false);
	mAmmoCountLabel->SetVisible(false);
	mGameOverLabel->SetVisible(true);
}

void Asteroids::OnGameStart()
{
	mStartScreenLabel->SetVisible(false);

	int collectibleAmount = NUM_POWERUPS;
	int asteroidAmount = NUM_START_ASTEROIDS;

	bool shouldGenerateCollectibles = true;
	bool shouldGenerateAsteroids = true;
	
	RemoveAllObjects();

	mLevel = 0;

	mSpaceship->Reset();
	mGameWorld->AddObject(mSpaceship);

	mGameOverLabel->SetVisible(false);
	mScoreKeeper.ResetScore();

	mSpaceship->SetAmmo(PLAYER_START_BULLETS);
	UpdateAmmoLabel(mSpaceship->GetBulletCount());

	mPlayer.SetLives(PLAYER_START_LIVES);
	UpdateLivesLabel(mPlayer.GetLives());

	if (mCollectibleCount < collectibleAmount) collectibleAmount -= mCollectibleCount;
	else shouldGenerateCollectibles = false;

	if (mAsteroidCount < asteroidAmount) asteroidAmount -= mAsteroidCount;
	else shouldGenerateAsteroids = false;

	mGameStarted = true;

	if (shouldGenerateAsteroids) CreateAsteroids(asteroidAmount);
	if (shouldGenerateCollectibles) CreateCollectibles(collectibleAmount);

	mScoreLabel->SetVisible(true);
	mLivesLabel->SetVisible(true);
	mAmmoCountLabel->SetVisible(true);

	mSpaceship->SetAmoured(false); // player is armoured at the start for some reason, this fixes it
}

void Asteroids::InitialiseDemo()
{
	int collectibleAmount = NUM_POWERUPS;
	int asteroidAmount = NUM_START_ASTEROIDS;
	
	mGameWorld->AddObject(CreateSpaceship());

	CreateAsteroids(asteroidAmount);
	CreateCollectibles(collectibleAmount);

	mSpaceship->SetAmoured(false); // player is armoured at the start for some reason, this fixes it

	mScoreLabel->SetVisible(true);
	mLivesLabel->SetVisible(true);
	mAmmoCountLabel->SetVisible(true);

	mAllowDemoShoot = true;
	mDemoShipState = EDemoShipState(TargetAsteroid);
}

void Asteroids::OnWorldUpdated(GameWorld* world)
{
	if (mGameStarted) return; // only run demo mode if the game isnt started

	mDemoShipState = EDemoShipState(TargetAsteroid);
	if (mSpaceship->GetBulletCount() < PLAYER_START_BULLETS / 2) mDemoShipState = EDemoShipState(SeekAmmo);
	if (mPlayer.GetLives() <= 2) mDemoShipState = EDemoShipState(SeekHealth);
	mSpaceship->Thrust(0);
	switch (mDemoShipState)
	{
	case TargetAsteroid:
		TargetAsteroidState(world);
		break;
	case SeekHealth:
		if (!SeekHealthState(world))
		{
			TargetAsteroidState(world);
		}
		break;
	case SeekAmmo:
		if (!SeekAmmoState(world))
		{
			TargetAsteroidState(world);
		}
		break;
	default:
		TargetAsteroidState(world);
		break;
	}
}

shared_ptr<GameObject> Asteroids::FindAsteroid(const GameWorld* world)
{
	shared_ptr<GameObject> target;
	float closestSqrDist = 99999999999999999999999999999.0f;

	for each (shared_ptr<GameObject> go in mGameObjectList)
	{
		if (go->GetType() != GameObjectType("Asteroid") || go->GetWorld() != world) continue;

		GLVector3f pos = go->GetPosition();
		float sqrDist = (pos - mSpaceship->GetPosition()).lengthSqr();

		if (sqrDist < closestSqrDist)
		{
			closestSqrDist = sqrDist;
			target = go;
		}
	}

	return target;
}

shared_ptr<GameObject> Asteroids::FindCollectible(const GameWorld* world, const int typeToFind)
{
	shared_ptr<GameObject> target;
	float closestSqrDist = 99999999999999999999999999999.0f;

	for each (shared_ptr<GameObject> go in mGameObjectList)
	{
		if (go.get() == nullptr) continue;
		if (go->GetType() != GameObjectType("Collectible") || go->GetWorld() != world) continue;

		Collectible* col = (Collectible*)go.get();
		if (col->GetCollectibleType() != typeToFind) continue;

		GLVector3f pos = go->GetPosition();
		float sqrDist = (pos - mSpaceship->GetPosition()).lengthSqr();

		if (sqrDist < closestSqrDist)
		{
			closestSqrDist = sqrDist;
			target = go;
		}
	}

	return target;
}

float Asteroids::AngleToFaceTarget(const GLVector3f& targetPos)
{
	GLVector3f dir = VectorMaths::Direction(mSpaceship->GetPosition(), targetPos);
	float angle = VectorMaths::Angle(GLVector3f(1.0f, 0.0f, 0.0f), dir);

	if (dir.y < 0.0f) angle = 360.0f - angle;

	return angle;
}

void Asteroids::RemoveAllObjects()
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

void Asteroids::TargetAsteroidState(GameWorld* world)
{
	shared_ptr<GameObject> target = FindAsteroid(world);

	if (target.get() == nullptr) return;
	if (target->GetWorld() != world) return;

	bool slowingDown = false;
	if (mSpaceship->GetVelocity().length() > 3.0f)
	{
		float reverseAngle = AngleToFaceTarget(mSpaceship->GetPosition() - mSpaceship->GetVelocity());
		mSpaceship->SetAngle(reverseAngle);
		mSpaceship->Thrust(10.0f);
		slowingDown = true;
	}
	else
	{
		mSpaceship->Thrust(0);
	}

	GLVector3f directionToTarget = VectorMaths::Direction(mSpaceship->GetPosition(), target->GetPosition());
	float velocityInDirectionOfTarget = mSpaceship->GetVelocity().dot(directionToTarget);

	float bulletSpeed = mSpaceship->GetBulletSpeed() + velocityInDirectionOfTarget;
	float timeForBulletToHitTarget = (mSpaceship->GetPosition() - target->GetPosition()).length() / bulletSpeed;

	GLVector3f predictedTargetPos = target->GetPosition() + (target->GetVelocity() * timeForBulletToHitTarget);

	float angle = AngleToFaceTarget(predictedTargetPos);

	mSpaceship->SetAngle(angle);
	if (mAllowDemoShoot)
	{
		mSpaceship->Shoot();
		SetTimer(500, ALLOW_DEMO_SHOOT);
		mAllowDemoShoot = false;
		UpdateAmmoLabel(mSpaceship->GetBulletCount());
	}
	if ((mSpaceship->GetPosition() - predictedTargetPos).length() > 5.0f && !slowingDown) mSpaceship->Thrust(10);
}

bool Asteroids::SeekHealthState(GameWorld* world)
{
	shared_ptr<GameObject> target = FindCollectible(world, ECollectibleType(ExtraLife));

	if (target.get() == nullptr) return false;
	if (target->GetWorld() != world) return false;

	GLVector3f directionToTarget = VectorMaths::Direction(mSpaceship->GetPosition(), target->GetPosition());

	for each (shared_ptr<GameObject> go in mGameObjectList)
	{
		if (go.get() == nullptr) continue;
		if (go->GetType() != GameObjectType("Asteroid") || go->GetWorld() != world) continue;

		GLVector3f dir = VectorMaths::Direction(mSpaceship->GetPosition(), go->GetPosition());
		float dot = directionToTarget.dot(dir);
		if (dot > 0.8f) return false; // asteroid in the way
	}


	float angle = AngleToFaceTarget(target->GetPosition());

	mSpaceship->SetAngle(angle);
	mSpaceship->Thrust(10);
	return true;
}

bool Asteroids::SeekAmmoState(GameWorld* world)
{
	shared_ptr<GameObject> target = FindCollectible(world, ECollectibleType(ExtraBullets));

	if (target.get() == nullptr) return false;
	if (target->GetWorld() != world) return false;

	GLVector3f directionToTarget = VectorMaths::Direction(mSpaceship->GetPosition(), target->GetPosition());

	for each (shared_ptr<GameObject> go in mGameObjectList)
	{
		if (go.get() == nullptr) continue;
		if (go->GetType() != GameObjectType("Asteroid") || go->GetWorld() != world) continue;

		GLVector3f dir = VectorMaths::Direction(mSpaceship->GetPosition(), go->GetPosition());
		float dot = directionToTarget.dot(dir);
		if (dot > 0.8f) return false; // asteroid in the way
	}

	float angle = AngleToFaceTarget(target->GetPosition());

	mSpaceship->SetAngle(angle);
	mSpaceship->Thrust(10);

	return true;
}