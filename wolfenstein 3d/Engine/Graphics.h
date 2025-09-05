#pragma once
#include <SDL3/SDL.h>

namespace Engine
{
	struct COLOR
	{
		uint8_t r, g, b, a = 255;
	};

	static COLOR WHITE_COLOR = { 255, 255, 255, 255 };
	static COLOR GRAY_COLOR = { 87, 87, 87, 120 };
	static COLOR OPAQUE_GRAY_COLOR = { 87, 87, 87, 255 };
	static COLOR DARK_GRAY_COLOR = { 25, 25, 25, 255 };
	static COLOR BLACK_COLOR = { 0, 0, 0, 255 };
	static COLOR RED_COLOR = { 255, 0, 0, 255 };
	static COLOR GREEN_COLOR = { 0, 120, 0, 255 };
	static COLOR BLUE_COLOR = { 0, 0, 255, 255 };
	static COLOR YELLOW_COLOR = { 255, 255, 0, 255 };
	static COLOR CYAN_COLOR = { 0, 255, 255, 255 };
	static COLOR MAGENTA_COLOR = { 255, 0, 255, 255 };
	static COLOR COLORSTONE_COLOR = { 76, 76, 76, 255 };
	static COLOR WOOD_COLOR = { 92, 64, 32, 255 };
	static COLOR MAP_LINES_COLOR = { 87, 87, 87, 120 };


	class GraphicsEngine
	{
	public:
		inline uint32_t RGBtoUint(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
			return ((r << 24) | (g << 16) | (b << 8) | a);
		}

		void DrawPoint(int x, int y, uint32_t color)
		{
			framebuffer[window_width * y + x] = color;
		}
	private:
		SDL_Renderer* renderer;
		SDL_Texture* frame_buffer_texture;
		int window_width, window_height = 0;
	public:
		uint32_t* framebuffer;

	public:
		GraphicsEngine(SDL_Window* window, int window_width, int window_height)
		{
			renderer = SDL_CreateRenderer(window, NULL);
			if (!renderer)
			{
				std::cout << "Failed To Create SDL Renderer!\n";
				__debugbreak();
			}

			// alpha blending
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

			this->window_width = window_width;
			this->window_height = window_height;
			framebuffer = new uint32_t[window_width * (window_height + 1)];

			frame_buffer_texture = SDL_CreateTexture(
				renderer,
				SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_STREAMING,
				window_width,
				window_height);
		}

		void Clear(COLOR clear_color)
		{
			SDL_SetRenderDrawColor(renderer, clear_color.r, clear_color.g, clear_color.b, clear_color.a);
			SDL_RenderClear(renderer);
		}

		void ClearFramebuffer(COLOR color)
		{
			uint32_t c = RGBtoUint(color.r, color.g, color.b, color.a);
			for (size_t y = 0; y < window_height; y++)
			{
				for (size_t x = 0; x < window_width; x++)
				{
					framebuffer[(window_width * y) + x] = c;
				}
			}
		}

		void DrawFramebuffer()
		{
			SDL_UpdateTexture(
				frame_buffer_texture,
				nullptr,
				framebuffer,
				(int)((uint32_t)window_width * sizeof(uint32_t)));

			SDL_RenderTexture(renderer, frame_buffer_texture, nullptr, nullptr);
		}

		void Present()
		{
			SDL_RenderPresent(renderer);
		}

		void Destroy()
		{
			SDL_DestroyTexture(frame_buffer_texture);
			delete framebuffer;
			SDL_DestroyRenderer(renderer);
		}

		void DrawRect(float x, float y, float w, float h, COLOR color)
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_FRect rect = { x, y, w, h };
			SDL_RenderFillRect(renderer, &rect);
		}

		void DrawRectOutline(float x, float y, float w, float h, COLOR color)
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_FRect rect = { x, y, w, h };
			SDL_RenderRect(renderer, &rect);
		}

		void DrawOutlinedRect(float x, float y, float w, float h, COLOR color, COLOR outline_color)
		{
			SDL_FRect rect = { x, y, w, h };

			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_RenderFillRect(renderer, &rect);

			SDL_SetRenderDrawColor(renderer, outline_color.r, outline_color.g, outline_color.b, outline_color.a);
			SDL_RenderRect(renderer, &rect);
		}

		void DrawCircle(int x, int y, int r, COLOR color)
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

			int top_left_x = x - r;
			int top_left_y = y - r;

			for (int Y = top_left_y; Y < y + r; Y++)
			{
				for (int X = top_left_x; X < x + r; X++)
				{
					int dx = X - x;
					int dy = Y - y;

					if (dx * dx + dy * dy < r * r)
					{
						SDL_RenderPoint(renderer, x + dx, y + dy);
					}
				}
			}
		}

		void DrawLine(float s_x, float s_y, float e_x, float e_y, COLOR color)
		{
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

			SDL_RenderLine(renderer,
				s_x, s_y,
				e_x, e_y
			);
		}

		bool IsPointInsideRect(int px, int py, int rx, int ry, int rw, int rh)
		{
			bool left = px < rx;
			bool right = px > rx + rw;
			bool up = py < ry;
			bool down = py > ry + rh;

			if (left || right || up || down)
				return false;
			return true;
		}
	};
}
