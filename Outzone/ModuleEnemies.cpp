#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleEnemies.h"
#include "ModuleParticles.h"
#include "ModuleTextures.h"
#include "Enemy.h"
#include "ENEMY_Basic_Robot.h"
#include "ENEMY_Tiny_Turret.h"
#include "ENEMY_Big_Turret_Left.h"
#include "ENEMY_Big_Turret_Right.h"
#include "ENEMY_Golden_Turret.h"
#include "ENEMY_Car.h"
#include "ENEMY_Red_Robot.h"
#include "ENEMY_Blue_Robot.h"
#include "ENEMY_Beatle.h"
#include "ModuleObjects.h"
#include "OBJECT_Upgrade.h"
#include "ModulePlayer.h"�
#include "ModuleAudio.h"
#include "ModuleInterfice.h"

#define SPAWN_MARGIN 200

ModuleEnemies::ModuleEnemies()
{
	for (uint i = 0; i < MAX_ENEMIES; ++i)
	{
		enemies[i] = nullptr;
		
	}
}

// Destructor
ModuleEnemies::~ModuleEnemies()
{}

bool ModuleEnemies::Start()
{
	sprites = App->textures->Load("Game/enemies_sprites.png");
	enemy_die_fx = App->audio->Load_chunk("Game/death_small_enemies.wav");
	
	return true;
}

update_status ModuleEnemies::PreUpdate()
{
	// check camera position to decide what to spawn
	for (uint i = 0; i < MAX_ENEMIES; ++i)
	{
		if (queue[i].type != ENEMY_TYPES::NO_TYPE)
		{
			if (queue[i].type == ENEMY_TYPES::CAR || queue[i].type == ENEMY_TYPES::BIG_TURRET_LEFT || queue[i].type == ENEMY_TYPES::BIG_TURRET_RIGHT)
			{
				if (queue[i].y * (-SCREEN_SIZE) < App->render->camera.y + SPAWN_MARGIN)
				{
					SpawnEnemy(queue[i]);
					queue[i].type = ENEMY_TYPES::NO_TYPE;
					LOG("Spawning enemy at %d", queue[i].x * SCREEN_SIZE);
				}
			}
			else
			{
				if (queue[i].y * (-SCREEN_SIZE) < App->render->camera.y + 100)
				{
					SpawnEnemy(queue[i]);
					queue[i].type = ENEMY_TYPES::NO_TYPE;
					LOG("Spawning enemy at %d", queue[i].x * SCREEN_SIZE);
				}
			}
		}
	}

	return UPDATE_CONTINUE;
}

// Called before render is available
update_status ModuleEnemies::Update()
{
	
	for (uint i = 0; i < MAX_ENEMIES; ++i)
	if (enemies[i] != nullptr) enemies[i]->Move();

	for (uint i = 0; i < MAX_ENEMIES; ++i)
	if (enemies[i] != nullptr) enemies[i]->Draw(sprites);

	for(uint i = 0; i < MAX_ENEMIES; ++i)
	if (enemies[i] != nullptr) enemies[i]->Attack();

	for (uint i = 0; i < MAX_ENEMIES; ++i)
		if (enemies[i] != nullptr) enemies[i]->return_from_hitmarker();

	for (uint i = 0; i < MAX_ENEMIES; ++i)
	{
		if (enemies[i] != nullptr)
		{
 			if(enemies[i]->Is_Dead())
			{
				Mix_PlayChannel(-1, enemy_die_fx, 0);
				delete enemies[i];
				enemies[i] = nullptr;
			}
		}
			
	}

	

	return UPDATE_CONTINUE;
}

update_status ModuleEnemies::PostUpdate()
{
	// check camera position to decide what to spawn
	for (uint i = 0; i < MAX_ENEMIES; ++i)
	{
		if (enemies[i] != nullptr)
		{
			if ((enemies[i]->position.y * -2) < (App->render->camera.y - (App->render->camera.h * 2)))
			{
				LOG("DeSpawning enemy at %d", enemies[i]->position.x * SCREEN_SIZE);
				delete enemies[i];
				enemies[i] = nullptr;
			}
		}
	}

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleEnemies::CleanUp()
{
	LOG("Freeing all enemies");

	App->textures->Unload(sprites);

	for (uint i = 0; i < MAX_ENEMIES; ++i)
	{
		if (enemies[i] != nullptr)
		{
			delete enemies[i];
			enemies[i] = nullptr;
		}
		
		queue[i].type = NO_TYPE;

	}

	return true;
}

bool ModuleEnemies::AddEnemy(ENEMY_TYPES type, int x, int y, MOVEMENT_TYPES m_type)
{
	bool ret = false;
	App->interfice->enemies_init_spawns_count++;
	for (uint i = 0; i < MAX_ENEMIES; ++i)
	{
		if (queue[i].type == ENEMY_TYPES::NO_TYPE)
		{
			queue[i].type = type;
			queue[i].x = x;
			queue[i].y = y;
			queue[i].movement_type = m_type;
			ret = true;
			break;
		}
	}

	return ret;
}

void ModuleEnemies::SpawnEnemy(const EnemyInfo& info)
{
	// find room for the new enemy
	uint i = 0;
	for (; enemies[i] != nullptr && i < MAX_ENEMIES; ++i);

	if (i != MAX_ENEMIES)
	{
		switch (info.type)
		{
		case ENEMY_TYPES::CAR:
			enemies[i] = new ENEMY_Car(info.x, info.y, info.movement_type);
			break;
		case ENEMY_TYPES::BASIC_ROBOT:
			enemies[i] = new ENEMY_Basic_Robot(info.x, info.y, info.movement_type);
			break;
		case ENEMY_TYPES::TINY_TURRET:
			enemies[i] = new ENEMY_Tiny_Turret(info.x, info.y);
			break;
		case ENEMY_TYPES::BIG_TURRET_LEFT:
			enemies[i] = new ENEMY_Big_Turret_Left(info.x, info.y);
			break;
		case ENEMY_TYPES::BIG_TURRET_RIGHT:
			enemies[i] = new ENEMY_Big_Turret_Right(info.x, info.y);
			break;
		case ENEMY_TYPES::GOLDEN_TURRET:
			enemies[i] = new ENEMY_Golden_Turret(info.x, info.y);
			break;
		case ENEMY_TYPES::RED_ROBOT:
			enemies[i] = new ENEMY_Red_Robot(info.x, info.y, info.movement_type);
			break;
		case ENEMY_TYPES::BEATLE:
			enemies[i] = new ENEMY_Beatle(info.x, info.y, info.movement_type);
			break;
		case ENEMY_TYPES::BLUE_ROBOT:
			enemies[i] = new ENEMY_Blue_Robot(info.x, info.y, info.movement_type);
			break;
		}
	}
}

void ModuleEnemies::OnCollision(Collider* c1, Collider* c2)
{
	for (uint i = 0; i < MAX_ENEMIES; ++i)
	{
		if (enemies[i] != nullptr && enemies[i]->GetCollider() == c1)
		{
			 if (c1->type == COLLIDER_ENEMY  && (c2->type == COLLIDER_PLAYER || c2->type == COLLIDER_PLAYER_SHOT))
			{
				if (enemies[i]->live > 0)
				{
					if (App->player->shotgun == true)
					{
						if (App->player->shotgun_lvl == 1) enemies[i]->live--;
						if (App->player->shotgun_lvl == 2) enemies[i]->live -= 2;
						if (App->player->shotgun_lvl == 3) enemies[i]->live -= 4;
					}
					else 
					{
						if (App->player->shotgun_lvl == 1) enemies[i]->live -= 2;
						if (App->player->shotgun_lvl == 2) enemies[i]->live -= 3;
						if (App->player->shotgun_lvl == 3) enemies[i]->live -= 5;
					}
				    //enemies change to green

					if (enemies[i]->enemy_type == BIG_TURRET_RIGHT) enemies[i]->hitmarker();
					if (enemies[i]->enemy_type == BIG_TURRET_LEFT) enemies[i]->hitmarker();
					if (enemies[i]->enemy_type == BASIC_ROBOT) enemies[i]->hitmarker();
					if (enemies[i]->enemy_type == BEATLE) enemies[i]->hitmarker();
					if (enemies[i]->enemy_type == GOLDEN_TURRET) enemies[i]->hitmarker();
					if (enemies[i]->enemy_type == TINY_TURRET) enemies[i]->hitmarker();
					if (enemies[i]->enemy_type == BLUE_ROBOT) enemies[i]->hitmarker();
					if (enemies[i]->enemy_type == RED_ROBOT) enemies[i]->hitmarker();
				
				}
			}
			 else  if (c1->type == COLLIDER_SHIELD && c2->type == COLLIDER_PLAYER_SHOT)
			 {
				 if (enemies[i]->live > 0)
				 {
					 if (App->player->shotgun == true)
					 {
						 if (App->player->shotgun_lvl == 1) enemies[i]->live--;
						 if (App->player->shotgun_lvl == 2) enemies[i]->live -= 2;
						 if (App->player->shotgun_lvl == 3) enemies[i]->live -= 4;
					 }
					 else
					 {
						 if (App->player->shotgun_lvl == 1) enemies[i]->live -= 2;
						 if (App->player->shotgun_lvl == 2) enemies[i]->live -= 3;
						 if (App->player->shotgun_lvl == 3) enemies[i]->live -= 5;
					 }
					 if (enemies[i]->enemy_type == CAR)
					 {
						 App->interfice->score += 20;
					 }
				 }
			 }
		}
	}

}
