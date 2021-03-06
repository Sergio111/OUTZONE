#include <math.h>
#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "SDL/include/SDL_render.h"
#include "SDL/include/SDL_timer.h"
#include "Module_lvl_1.h"
#include "Module_Welcome.h"
#include "Module_Congrats.h"
#include "ModuleGameOver.h"
#include "ModuleChangeScene.h"
#include "ModulePlayer.h"
#include "ModuleInterfice.h"

ModuleChangeScene::ModuleChangeScene()
{
	screen = { 0, 0, SCREEN_WIDTH * SCREEN_SIZE, SCREEN_HEIGHT * SCREEN_SIZE };
}

ModuleChangeScene::~ModuleChangeScene()
{}

// Load assets
bool ModuleChangeScene::Start()
{
	LOG("Preparing Fade Screen");
	SDL_SetRenderDrawBlendMode(App->render->renderer, SDL_BLENDMODE_BLEND);
	return true;
}

// Update: draw background
update_status ModuleChangeScene::Update()
{ 
	if (current_step == fade_step::none)
		return UPDATE_CONTINUE;

	Uint32 now = SDL_GetTicks() - start_time;
	float normalized = MIN(1.0f, (float)now / (float)total_time);
	

	switch (current_step)
	{
	case fade_step::fade_to_black:
	{
		if (now >= total_time)
		{
			
			if (App->welcome->IsEnabled())
			{
				App->welcome->Disable();
				App->lvl_1->Enable();
			}

			else {
				if (App->lvl_1->IsEnabled() && App->player->destroyed == false && App->player->result == false){
					App->lvl_1->Disable();
					App->welcome->Enable();
				}
				else if (App->lvl_1->IsEnabled()&&App->player->destroyed==false&& App->player->result)
				{
					App->lvl_1->Disable();
					App->congrats->Enable();
				}
				else if (App->lvl_1->IsEnabled() && App->player->destroyed){
					if (App->player->fully_destroyed){
						App->lvl_1->Disable();
						App->gameover->Enable();
					}
					else{
						App->lvl_1->Disable();
						App->lvl_1->Enable();
					}
				}
				else{
					if (App->congrats->IsEnabled())
					{
						App->congrats->Disable();
						App->welcome->Enable();
					}
					else{
						if (App->gameover->IsEnabled()){
							App->gameover->Disable();
							App->welcome->Enable();
						}
					}
				}
			}
			
			// ---
			total_time += total_time;
			start_time = SDL_GetTicks();

			current_step = fade_step::fade_from_black;
		}
	} break;

	case fade_step::fade_from_black:
	{
		normalized = 1.0f - normalized;

		if (now >= total_time)
			current_step = fade_step::none;
	} break;
	}

	// Finally render the black square with alpha on the screen
	SDL_SetRenderDrawColor(App->render->renderer, 0, 0, 0, (Uint8)(normalized * 255.0f));
	SDL_RenderFillRect(App->render->renderer, &screen);

	return UPDATE_CONTINUE;
}

// Fade to black. At mid point deactivate one module, then activate the other
bool ModuleChangeScene::ChangeScene(Module* module_off, Module* module_on, float time)
{
	bool ret = false;

	if (current_step == fade_step::none)
	{
		current_step = fade_step::fade_to_black;
		start_time = SDL_GetTicks();
		total_time = (Uint32)(time * 0.5f * 1000.0f);
		ret = true;
	}

	return ret;
}

bool ModuleChangeScene::IsFading() const
{
	return current_step != fade_step::none;
}