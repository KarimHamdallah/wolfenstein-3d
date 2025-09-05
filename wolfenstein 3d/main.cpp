#include <iostream>
#include "Engine/Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Engine/stb_image.h"
#include <Windows.h>
#include <math.h>

#pragma comment(lib, "winmm.lib")

using namespace Engine;






////////////////////////// MACROS ///////////////////////////



#define TILE_SIZE 64

#define COL_TILE_NUM 20
#define RAW_TILE_NUM 13

#define WINDOW_WIDTH (COL_TILE_NUM * TILE_SIZE)
#define WINDOW_HEIGHT (RAW_TILE_NUM * TILE_SIZE)

#define PI 3.14159265359
#define TORAD (PI / 180.0f)

float FOV_ANGLE = 60.0f * TORAD;
#define NUM_RAYS (WINDOW_WIDTH)

#define MAP_SCALING_FACTOR 0.3f

/////////////////////////////////////////////////////////////




/////////////////////////// MAP //////////////////////////////




const int map[RAW_TILE_NUM][COL_TILE_NUM] =
{
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 0, 0, 0, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 4, 0, 0, 1, 0, 0, 1},
	{1, 0, 0, 6, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

void RenderMap(GraphicsEngine* gfx)
{
	for (size_t raw = 0; raw < RAW_TILE_NUM; raw++)
	{
		for (size_t col = 0; col < COL_TILE_NUM; col++)
		{
			COLOR tile_color;
			switch (map[raw][col])
			{
			case 0: tile_color = BLACK_COLOR; break;
			case 1: tile_color = WHITE_COLOR; break;
			case 2: tile_color = BLUE_COLOR; break;
			case 3: tile_color = COLORSTONE_COLOR; break;
			case 4: tile_color = CYAN_COLOR; break;
			case 5: tile_color = OPAQUE_GRAY_COLOR; break;
			case 6: tile_color = GREEN_COLOR; break;
			case 7: tile_color = WOOD_COLOR; break;
			default: tile_color = WHITE_COLOR; break;
			}


			gfx->DrawOutlinedRect(
				col * TILE_SIZE * MAP_SCALING_FACTOR,
				raw * TILE_SIZE * MAP_SCALING_FACTOR,
				TILE_SIZE * MAP_SCALING_FACTOR,
				TILE_SIZE * MAP_SCALING_FACTOR,
				tile_color, GRAY_COLOR);
		}
	}
}

/////////////////////////////////////////////////////////////


/////////////////////////// PLAYER //////////////////////////


struct Player
{
	float x = WINDOW_WIDTH * 0.5f;
	float y = WINDOW_HEIGHT * 0.5f;
	float size = 10.0f;
	float rotation_angle = PI / 2.0f;
	float walk_direction = 0; // 1 or -1 walk forward, backward
	float turn_direction = 0; // 1 or -1 turn right, left
	float wlak_speed = 200.0f;
	float turn_speed = 90.0f * TORAD;


	void Update(float dt)
	{
		rotation_angle += turn_speed * turn_direction * dt;

		float new_x = x + cosf(rotation_angle) * wlak_speed * walk_direction * dt;
		float new_y = y + sinf(rotation_angle) * wlak_speed * walk_direction * dt;

		int raw = (int)(floor(new_y / TILE_SIZE));
		int col = (int)(floor(new_x / TILE_SIZE));

		if (map[raw][col] == 0)
		{
			x = new_x;
			y = new_y;
		}
	}

	void Render(GraphicsEngine* gfx)
	{
		gfx-> DrawCircle(
			x * MAP_SCALING_FACTOR,
			y * MAP_SCALING_FACTOR,
			size * MAP_SCALING_FACTOR,
			RED_COLOR);

		gfx->DrawLine(
			x * MAP_SCALING_FACTOR,
			y * MAP_SCALING_FACTOR,
			(x + cosf(rotation_angle) * 100.0f ) * MAP_SCALING_FACTOR,
			(y + sinf(rotation_angle) * 100.0f ) * MAP_SCALING_FACTOR,
			RED_COLOR);
	}
};
Player player;

/////////////////////////////////////////////////////////////////



//////////////////////////// RAY CASTING ////////////////////////


struct IntersectionData
{
	bool hit;
	float x, y;
};

IntersectionData RayToLineIntersection(
	float rx, float ry, float rdx, float rdy,
	float x1, float y1, float x2, float y2)
{
	IntersectionData result{ false, 0.0f, 0.0f };

	float sdx = x2 - x1;
	float sdy = y2 - y1;

	float denom = rdx * sdy - rdy * sdx;

	if (fabs(denom) < 1e-6f)
	{
		return result;
	}

	float dx = x1 - rx;
	float dy = y1 - ry;

	float t = (dx * sdy - dy * sdx) / denom;

	if (t >= 0.0f)
	{
		result.hit = true;
		result.x = rx + t * rdx;
		result.y = ry + t * rdy;
	}

	return result;
}

inline float Distance(float x1, float y1, float x2, float y2)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	return std::sqrt(dx * dx + dy * dy);
}

inline float NormalizeAngle(float angle)
{
	angle = fmodf(angle, 2.0f * PI);   // wrap within [-2π, 2π]
	if (angle < 0)
		angle += 2.0f * PI;            // shift to [0, 2π)
	return angle;
}

struct Ray
{
	float x, y;
	float rotation_angle = PI / 2.0f;

	bool isRayFacingDown = 0;
	bool isRayFacingUp = 0;
	bool isRayFacingRight = 0;
	bool isRayFacingLeft = 0;

	float min_intersection_dist = INFINITY;
	float intersection_x = 0.0f;
	float intersection_y = 0.0f;
	int wall_texture_index = 1;

	bool was_vertical_hit = false;

	void Cast()
	{
		min_intersection_dist = INFINITY;

		float normalized_angle = NormalizeAngle(rotation_angle);
		isRayFacingDown = normalized_angle > 0 && normalized_angle < PI;
		isRayFacingUp = !isRayFacingDown;

		isRayFacingRight = normalized_angle < 0.5 * PI || normalized_angle > 1.5 * PI;
		isRayFacingLeft = !isRayFacingRight;

		// horizontal intersections
		for (size_t i = 1; i < RAW_TILE_NUM; i++)
		{
			auto hit = RayToLineIntersection(
				x, y,
				cosf(rotation_angle),
				sinf(rotation_angle),
				0.0f, TILE_SIZE * i, WINDOW_WIDTH, TILE_SIZE * i);

			if (hit.hit)
			{
				int col = floor(hit.x / TILE_SIZE);
				int raw = i;

				if (isRayFacingUp)
					raw = i - 1;

				if (raw >= 0 && col >= 0 && raw < RAW_TILE_NUM && col < COL_TILE_NUM)
				{
					if (map[raw][col] != 0)
					{
						float dist = Distance(x, y, hit.x, hit.y);
						if (min_intersection_dist > dist)
						{
							min_intersection_dist = dist;
							intersection_x = hit.x;
							intersection_y = hit.y;
							was_vertical_hit = false;
							wall_texture_index = map[raw][col];
						}
					}
				}
			}
		}

		// vertical intersections
		for (size_t i = 1; i < COL_TILE_NUM; i++)
		{
			auto hit = RayToLineIntersection(
				x, y,
				cosf(rotation_angle),
				sinf(rotation_angle),
				TILE_SIZE * i, 0.0f, TILE_SIZE * i, WINDOW_HEIGHT);

			if (hit.hit)
			{
				int raw = floor(hit.y / TILE_SIZE);
				int col = i;

				if (isRayFacingLeft)
					col = i - 1;

				if (raw >= 0 && col >= 0 && raw < RAW_TILE_NUM && col < COL_TILE_NUM)
				{
					if (map[raw][col] != 0)
					{
						float dist = Distance(x, y, hit.x, hit.y);
						if (min_intersection_dist > dist)
						{
							min_intersection_dist = dist;
							intersection_x = hit.x;
							intersection_y = hit.y;
							was_vertical_hit = true;
							wall_texture_index = map[raw][col];
						}
					}
				}
			}
		}
	}

	void Render(GraphicsEngine* gfx)
	{
		gfx->DrawLine(
			x * MAP_SCALING_FACTOR,
			y * MAP_SCALING_FACTOR,
			intersection_x * MAP_SCALING_FACTOR,
			intersection_y * MAP_SCALING_FACTOR,
			BLUE_COLOR);
	}
};
Ray rays[NUM_RAYS];

/////////////////////////////////////////////////////////////////

struct Texture
{
	int w, h, bpp = 0;
	uint8_t* data = nullptr;

	void load(const char* path)
	{
		data = (uint8_t*)stbi_load(path, &w, &h, &bpp, 0);
	}

	void free()
	{
		stbi_image_free(data);
		w = h = bpp = 0;
	}
};
Texture WallTextures[8];
Texture GuardTexture;


void Render3DProjectWalls(GraphicsEngine* gfx)
{
	for (int i = 0; i < NUM_RAYS; i++)
	{
		float ray_distance = rays[i].min_intersection_dist;
		float corrected_distance = ray_distance * cosf(rays[i].rotation_angle - player.rotation_angle);
		float distance_proj_plane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
		float projected_wall_height = (TILE_SIZE / corrected_distance) * distance_proj_plane;

		int wallStripHeight = (int)projected_wall_height;

		int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
		int wallTopPixel_no_clamp = wallTopPixel;
		wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

		int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
		wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

		// ceiling
		for (int y = 0; y < wallTopPixel; y++)
		{
			gfx->framebuffer[(WINDOW_WIDTH * y) + i] = 0x333333FF;
		}

		// walls
		int textureOffsetX;
		if (rays[i].was_vertical_hit)
			textureOffsetX = (int)rays[i].intersection_y % TILE_SIZE;
		else
			textureOffsetX = (int)rays[i].intersection_x % TILE_SIZE;

		for (int y = wallTopPixel; y < wallBottomPixel; y++)
		{
			auto WallTexture = WallTextures[rays[i].wall_texture_index];

			int textureOffsetY = (y - wallTopPixel_no_clamp) * ((float)WallTexture.h / wallStripHeight);

			int index = ((WallTexture.w * textureOffsetY) + textureOffsetX) * 4;

			uint8_t r = WallTexture.data[index + 0];
			uint8_t g = WallTexture.data[index + 1];
			uint8_t b = WallTexture.data[index + 2];
			uint8_t a = WallTexture.data[index + 3];
			
			gfx->framebuffer[(WINDOW_WIDTH * y) + i] = gfx->RGBtoUint(r, g, b, a);

			//gfx->framebuffer[(WINDOW_WIDTH * y) + i] = rays[i].was_vertical_hit ? 0xCCCCCCFF : 0xFFFFFFFF;
		}
	}
}

///////////////////////////////////////////////////////////////////

struct PlayerSpriteSheet
{
	int w, h, bpp = 0;
	int frame_width = 0;
	int frame_count = 0;
	int frames_per_sprite = 0;
	uint8_t* data = nullptr;

	int frame_counter = 0;
	int current_frame = 0;
	bool animation_finished = true;
	bool is_playing = false;

	void load(const char* path, int framewidth, int framecount, int framespersprite)
	{
		data = (uint8_t*)stbi_load(path, &w, &h, &bpp, 0);
		frame_width = framewidth;
		frame_count = framecount;
		frames_per_sprite = framespersprite;
	}

	void free()
	{
		stbi_image_free(data);
		w = h = bpp = 0;
		frame_width = 0;
		frame_count = 0;

		frame_counter = 0;
		current_frame = 0;
		animation_finished = true;
		is_playing = false;
	}

	void PlayAnimation()
	{
		frame_counter = 0;
		current_frame = 0;
		is_playing = true;
	}

	void Update()
	{
		if (is_playing)
		{
			frame_counter++;
			int frame_index = frame_counter / frames_per_sprite;

			if (frame_index >= frame_count) // exceeded the frame count in sprite sheet
			{
				current_frame = 0;
				animation_finished = true;
				is_playing = false;
			}
			else
			{
				current_frame = frame_index;
				animation_finished = false;
			}
		}
	}

	void Render(GraphicsEngine* gfx)
	{
		int rect_w = frame_width;
		int rect_h = h;
		int start_x = WINDOW_WIDTH * 0.5f - rect_w * 0.5f;
		int start_y = WINDOW_HEIGHT - rect_h;

		int image_x = current_frame * frame_width;
		int image_y = 0;

		for (size_t y = start_y; y < start_y + rect_h; y++)
		{
			for (size_t x = start_x; x < start_x + rect_w; x++)
			{
				int src_index = ((w * image_y) + image_x) * 4;

				uint8_t r = data[src_index + 0];
				uint8_t g = data[src_index + 1];
				uint8_t b = data[src_index + 2];
				uint8_t a = data[src_index + 3];

				if (a > 10)
					gfx->framebuffer[(WINDOW_WIDTH * y) + x] = gfx->RGBtoUint(r, g, b, a);
				image_x++;
			}
			image_x = current_frame * frame_width;
			image_y++;
		}
	}
};
PlayerSpriteSheet PlayerGunSpriteSheet;

///////////////////////////////// Sprite (Enemy, Doors, ... etc) ///////////////////////////

struct Sprite
{
	int x = WINDOW_WIDTH * 0.5f;
	int y = WINDOW_HEIGHT * 0.5f;
	int map_size = 10;
	bool visible = false;
	COLOR map_color = GRAY_COLOR;

	void Render(GraphicsEngine* gfx)
	{
		float dx = x - player.x;
		float dy = y - player.y;

		float angle_player_sprite = player.rotation_angle - atan2(dy, dx);

		// clamp angle between 0 and 180
		if (angle_player_sprite > PI)
			angle_player_sprite -= 2.0f * PI;
		if (angle_player_sprite < -PI)
			angle_player_sprite += 2.0f * PI;

		visible = false;
		map_color = GRAY_COLOR;

		if (fabs(angle_player_sprite) < FOV_ANGLE / 2)
		{
			map_color = YELLOW_COLOR;

			visible = true;
			float distance = sqrtf(dx * dx + dy * dy);

			float distance_proj_plane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);

			float sprite_h = (TILE_SIZE / distance) * distance_proj_plane;
			float sprite_w = sprite_h;

			int spriteTopPixel = (WINDOW_HEIGHT / 2) - (sprite_h / 2);
			int spriteTopPixel_no_clamp = spriteTopPixel;
			spriteTopPixel = spriteTopPixel < 0 ? 0 : spriteTopPixel;

			int spriteBottomPixel = (WINDOW_HEIGHT / 2) + (sprite_h / 2);
			spriteBottomPixel = spriteBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : spriteBottomPixel;

			float spriteScreenPosX = tanf(angle_player_sprite) * distance_proj_plane;

			float spriteLeftX = (WINDOW_WIDTH / 2) - spriteScreenPosX;

			float spriteRightX = spriteLeftX + sprite_w;

			for (int x = spriteLeftX; x < spriteRightX; x++)
			{
				int texture_x_offset = (x - spriteLeftX) * ((float)GuardTexture.w / sprite_w);

				for (size_t y = spriteTopPixel; y < spriteBottomPixel; y++)
				{
					if (x > 0 && y > 0 && x < WINDOW_WIDTH && y < WINDOW_HEIGHT)
					{
						int texture_y_offset = (y - spriteTopPixel_no_clamp) * ((float)GuardTexture.h / sprite_h);

						int src_index = ((GuardTexture.w * texture_y_offset) + texture_x_offset) * 4;

						uint8_t r = GuardTexture.data[src_index + 0];
						uint8_t g = GuardTexture.data[src_index + 1];
						uint8_t b = GuardTexture.data[src_index + 2];
						uint8_t a = GuardTexture.data[src_index + 3];

						uint32_t color = gfx->RGBtoUint(r, g, b, a);

						bool is_pink = color == 0xFF00FFFF;
						if(!is_pink && distance < rays[x].min_intersection_dist)
							gfx->framebuffer[(WINDOW_WIDTH * y) + x] = color;
					}
				}
			}
		}
	}

	void RenderMapSprite(GraphicsEngine* gfx)
	{
		gfx->DrawCircle(
			x * MAP_SCALING_FACTOR,
			y * MAP_SCALING_FACTOR,
			map_size * MAP_SCALING_FACTOR, map_color);
	}
};
Sprite Enemy;

////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
		std::cout << "Failed To Init SDL!\n";

	SDL_Window* window = SDL_CreateWindow("wolf3d", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (!window)
	{
		std::cout << "Failed To Create SDL Window!\n";
		__debugbreak();
	}

	GraphicsEngine* GFX = new GraphicsEngine(window, WINDOW_WIDTH, WINDOW_HEIGHT);

	float mouse_x = 0.0f;
	float mouse_y = 0.0f;

	uint64_t lastTime = 0.0f;
	double deltaTime = 0.0f;

	PlayerGunSpriteSheet.load("assets/pistol.png", 256, 6, 5);
	
	WallTextures[1].load("assets/redbrick.png");
	WallTextures[2].load("assets/bluestone.png");
	WallTextures[3].load("assets/colorstone.png");
	WallTextures[4].load("assets/eagle.png");
	WallTextures[5].load("assets/graystone.png");
	WallTextures[6].load("assets/mossystone.png");
	WallTextures[7].load("assets/wood.png");

	GuardTexture.load("assets/guard.png");

	SDL_Event e;
	bool is_game_running = true;
	while (is_game_running)
	{
		uint64_t currentTime = SDL_GetTicks();
		deltaTime = ((double)currentTime - (double)lastTime) / 1000.0;
		lastTime = currentTime;

		//std::cout << "fps " << 1.0 / deltaTime << "\n";

		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				is_game_running = false;
			}
			break;
			case SDL_EVENT_KEY_DOWN:
			{
				if (e.key.key == SDLK_W)
					player.walk_direction = +1;
				if (e.key.key == SDLK_S)
					player.walk_direction = -1;
				if (e.key.key == SDLK_D)
					player.turn_direction = +1;
				if (e.key.key == SDLK_A)
					player.turn_direction = -1;
			}
			break;
			case SDL_EVENT_KEY_UP:
			{
				if (e.key.key == SDLK_W)
					player.walk_direction = 0;
				if (e.key.key == SDLK_S)
					player.walk_direction = 0;
				if (e.key.key == SDLK_D)
					player.turn_direction = 0;
				if (e.key.key == SDLK_A)
					player.turn_direction = 0;
			}
			break;
			case SDL_EVENT_MOUSE_MOTION:
			{
				mouse_x = e.motion.x;
				mouse_y = e.motion.y;
				//player.rotation_angle += e.motion.xrel * 0.1f * deltaTime;
			}
			break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				if (e.button.button == SDL_BUTTON_LEFT && PlayerGunSpriteSheet.animation_finished)
				{
					PlayerGunSpriteSheet.PlayAnimation();
					PlaySound(TEXT("assets/gun shoot.wav"), NULL, SND_ASYNC | SND_FILENAME);
				}
			}
			break;
			default:
				break;
			}
		}

		// update
		player.Update(deltaTime);
		PlayerGunSpriteSheet.Update();

		// render
		GFX->Clear(BLACK_COLOR);

		GFX->ClearFramebuffer(DARK_GRAY_COLOR);
		Render3DProjectWalls(GFX);
		Enemy.Render(GFX);
		PlayerGunSpriteSheet.Render(GFX);
		GFX->DrawFramebuffer();

		RenderMap(GFX);
		player.Render(GFX);

		// cast all rays
		{
			float rayAngle = player.rotation_angle - (FOV_ANGLE / 2.0f);

			for (int stripId = 0; stripId < NUM_RAYS; stripId++)
			{
				rays[stripId].x = player.x;
				rays[stripId].y = player.y;
				rays[stripId].rotation_angle = rayAngle;

				rays[stripId].Cast();
				rays[stripId].Render(GFX);

				rayAngle += FOV_ANGLE / NUM_RAYS;
			}
		}
		Enemy.RenderMapSprite(GFX);

		GFX->Present();
	}

	PlayerGunSpriteSheet.free();
	GFX->Destroy();
	delete GFX;
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
