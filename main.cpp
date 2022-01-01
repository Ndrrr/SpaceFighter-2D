#include <stdio.h>
#include <iostream>
#include <SDL_ttf.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <random>

using namespace std;

const int bulletX = 15, bulletY = 15;
const int windowX = 1200, windowY = 700;

int bullet_cnt, cooldown, damage_multiplier=1, skill_point=0, characther_HP=20;

int speed; 
struct fire{
	SDL_Rect pos;
	int life = 100;
	int damage = 1; 
	long long id;
	SDL_Texture* texture;
};

struct enemy {
	SDL_Rect pos;
	int HP = 4;
	int hit_tex_life = 11;
	set<long long> hit_by;
	SDL_Texture* texture; // change enemy color green->yellow->red by health FT510
};

struct button {
	SDL_Rect cord;
	int state = 0; // 0-normal, 1-mouseOver, 2 - Pressed
	SDL_Texture* texture,*texture2;
	int id;
	int check_pressed(int x, int y) {
		if (cord.x <= x && cord.x + cord.w >= x && cord.y <= y && cord.y + cord.h >= y) {
			//cout << cord.x << " " << cord.w << " " << cord.y << " " << cord.h << endl;
			return id;
		}
		return -1;
	}
};

struct skill {
	SDL_Rect cord, up_cord;
	vector<SDL_Texture*> textures;
	SDL_Texture* up_textures[3];
	int level = 0;
};


struct luck {
	int cooldown = 10;
	int id; // 1-bullet++, 2 - cooldown--, 3 - damage++, 4 cooldown++ FT510
};


bool check_hit(fire fr, enemy en) {
	if ((fr.pos.x<en.pos.x + 30 && fr.pos.x > en.pos.x && fr.pos.y<en.pos.y && fr.pos.y >en.pos.y - 30) ||
		(fr.pos.x + bulletX<en.pos.x + 30 && fr.pos.x + bulletX > en.pos.x && fr.pos.y<en.pos.y && fr.pos.y >en.pos.y - 30)) {
		return 1;
	}
	return 0;
}

void RenderHPBar(int x, int y, int w, int h, float Percent, SDL_Color FGColor, SDL_Color BGColor, SDL_Renderer* Renderer) {
	Percent = Percent > 1.f ? 1.f : Percent < 0.f ? 0.f : Percent;
	SDL_Color old;
	SDL_GetRenderDrawColor(Renderer, &old.r, &old.g, &old.g, &old.a);
	SDL_Rect bgrect = { x, y, w, h };
	SDL_SetRenderDrawColor(Renderer, BGColor.r, BGColor.g, BGColor.b, BGColor.a);
	SDL_RenderFillRect(Renderer, &bgrect);
	SDL_SetRenderDrawColor(Renderer, FGColor.r, FGColor.g, FGColor.b, FGColor.a);
	int pw = (int)((float)w * Percent);
	int px = x + (w - pw);
	SDL_Rect fgrect = { px, y, pw, h };
	SDL_RenderFillRect(Renderer, &fgrect);
	SDL_SetRenderDrawColor(Renderer, old.r, old.g, old.b, old.a);
}

int main(int argc, char* argv[])
{
	mt19937 mt(time(NULL));

	// retutns zero on success else non-zero
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("error initializing SDL: %s\n", SDL_GetError());
	}
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* win = SDL_CreateWindow("Space Fighter", // creates a window
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowX, windowY, 0);

	// triggers the program that controls
	// graphics hardware and sets flags
	Uint32 render_flags = SDL_RENDERER_ACCELERATED;

	// creates a renderer to render our images
	SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);

	// creates a surface to load an image into the main memory
	SDL_Surface* surface;
	
	//
	map<string,button> buttons, gameOverButtons;
	map<string, skill> skills;

	// for saving temporarily
	button tmp;
	SDL_Texture* tmpTex;
	//import textures
	surface = IMG_Load("Textures\\spaceFighter.png");
	SDL_Texture* spaceship_texture = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	
	surface = IMG_Load("Textures\\sky.jpg");
	SDL_Texture* background_image = SDL_CreateTextureFromSurface(rend, surface), *background_image2 = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\laser.jpg");
	SDL_Texture* laser_tex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\green_enemies.png");
	SDL_Texture* green_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\yellow_enemies.png");
	SDL_Texture* yellow_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\red_enemies.png");
	SDL_Texture* red_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\lucky_enemy.png");
	SDL_Texture* lucky_enemy = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	surface = IMG_Load("Textures\\white.jpg");
	SDL_Texture* on_hit_tex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	


	// add information about keybinding to pause screen FT510
	surface = IMG_Load("Textures\\UI\\background.png");
	SDL_Texture* pauseBackgroundTexture = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);

	// import button textures and initialize buttons
	surface = IMG_Load("Textures\\UI\\playButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["play"].texture = tmpTex;
	buttons["play"].cord.x = buttons["play"].cord.y = 50;
	buttons["play"].cord.w = 250; buttons["play"].cord.h = 100;
	buttons["play"].id = 0;
	surface = IMG_Load("Textures\\UI\\playButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["play"].texture2 = tmpTex;

	surface = IMG_Load("Textures\\UI\\settingsButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["settings"].texture = tmpTex;
	buttons["settings"].texture = tmpTex;
	buttons["settings"].cord.x = 50,  buttons["settings"].cord.y = 175;
	buttons["settings"].cord.w = 250; buttons["settings"].cord.h = 100;
	buttons["settings"].id = 1;
	surface = IMG_Load("Textures\\UI\\settingsButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["settings"].texture2 = tmpTex;

	surface = IMG_Load("Textures\\UI\\quitButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["quit"].texture = tmpTex;
	buttons["quit"].texture = tmpTex;
	buttons["quit"].cord.x =50,  buttons["quit"].cord.y = 300;
	buttons["quit"].cord.w = 250; buttons["quit"].cord.h = 100;
	buttons["quit"].id = 2;
	surface = IMG_Load("Textures\\UI\\quitButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["quit"].texture2 = tmpTex;

	surface = IMG_Load("Textures\\UI\\continueButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["continue"].texture = tmpTex;
	buttons["continue"].cord.x = buttons["continue"].cord.y = 50;
	buttons["continue"].cord.w = 250; buttons["continue"].cord.h = 100;
	buttons["continue"].id = 3;
	surface = IMG_Load("Textures\\UI\\continueButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	buttons["continue"].texture2 = tmpTex;

	surface = IMG_Load("Textures\\UI\\restartButton.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	gameOverButtons["restart"].texture = tmpTex;
	gameOverButtons["restart"].cord.x = gameOverButtons["restart"].cord.y = 50;
	gameOverButtons["restart"].cord.w = 250; gameOverButtons["restart"].cord.h = 100;
	gameOverButtons["restart"].id = 0;
	surface = IMG_Load("Textures\\UI\\restartButton2.png");
	tmpTex = SDL_CreateTextureFromSurface(rend, surface);
	SDL_FreeSurface(surface);
	gameOverButtons["restart"].texture2 = tmpTex;

	gameOverButtons["quit"] = buttons["quit"];
	gameOverButtons["quit"].id = 1;
	gameOverButtons["quit"].cord.x = buttons["settings"].cord.x;
	gameOverButtons["quit"].cord.y = buttons["settings"].cord.y;
	
	skills["cooldown"].cord.x = windowX - 70;
	skills["cooldown"].cord.y = windowY - 70;
	skills["cooldown"].cord.h = skills["cooldown"].cord.w = 50;
	surface = IMG_Load("Textures\\Skills\\Cooldown\\blue.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend,surface));
	surface = IMG_Load("Textures\\Skills\\Cooldown\\green.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Cooldown\\yellow.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Cooldown\\red.png");
	skills["cooldown"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	
	skills["bulletCnt"].cord.x = windowX - 70;
	skills["bulletCnt"].cord.y = windowY - 125;
	skills["bulletCnt"].cord.h = skills["bulletCnt"].cord.w = 50;
	surface = IMG_Load("Textures\\Skills\\BulletCnt\\blue.png");
	skills["bulletCnt"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\BulletCnt\\green.png");
	skills["bulletCnt"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\BulletCnt\\yellow.png");
	skills["bulletCnt"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\BulletCnt\\red.png");
	skills["bulletCnt"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));

	skills["damage"].cord.x = windowX - 70;
	skills["damage"].cord.y = windowY - 180;
	skills["damage"].cord.h = skills["damage"].cord.w = 50;
	surface = IMG_Load("Textures\\Skills\\Damage\\blue.png");
	skills["damage"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Damage\\green.png");
	skills["damage"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Damage\\yellow.png");
	skills["damage"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Damage\\red.png");
	skills["damage"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));

	skills["speed"].cord.x = 20;
	skills["speed"].cord.y = windowY - 160;
	skills["speed"].cord.h = 140, skills["speed"].cord.w=195;
	surface = IMG_Load("Textures\\Skills\\Speed\\blue.png");
	skills["speed"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Speed\\green.png");
	skills["speed"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Speed\\yellow.png");
	skills["speed"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));
	surface = IMG_Load("Textures\\Skills\\Speed\\red.png");
	skills["speed"].textures.push_back(SDL_CreateTextureFromSurface(rend, surface));

	surface = IMG_Load("Textures\\Skills\\LevelUp\\green.png");
	skills["speed"].up_textures[0] = skills["damage"].up_textures[0] = skills["bulletCnt"].up_textures[0] = skills["cooldown"].up_textures[0] = SDL_CreateTextureFromSurface(rend, surface);

	surface = IMG_Load("Textures\\Skills\\LevelUp\\yellow.png");
	skills["speed"].up_textures[1] = skills["damage"].up_textures[1] = skills["bulletCnt"].up_textures[1] = skills["cooldown"].up_textures[1] = SDL_CreateTextureFromSurface(rend, surface);

	surface = IMG_Load("Textures\\Skills\\LevelUp\\red.png");
	skills["speed"].up_textures[2] = skills["damage"].up_textures[2] = skills["bulletCnt"].up_textures[2] = skills["cooldown"].up_textures[2] = SDL_CreateTextureFromSurface(rend, surface);
	for (auto &itr : skills) {
		itr.second.up_cord.h = 50;
		itr.second.up_cord.w = 50;
		itr.second.up_cord.x = itr.second.cord.x - 50;
		itr.second.up_cord.y = itr.second.cord.y;
	}
	skills["speed"].up_cord.h = skills["speed"].up_cord.w = 50;
	skills["speed"].up_cord.x += 270;
	skills["speed"].up_cord.y = windowY - 45;
	TTF_Font* font = TTF_OpenFont("Textures\\evilEmpire.ttf", 100);
	if (font == NULL) {
		printf("%s", TTF_GetError());
	}
	SDL_Color White = { 255,255,255,255 }, random = {255,0,0,255};

	long score = 0;
	string scr;
	SDL_Texture* scoreTextTexture;
	// let us control our image position
	// so that we can move it with our keyboard.
	SDL_Rect main_pos, cord,scoreTextPos,background1,background2;
	
	background1.x = background1.y = background2.x = 0; background2.y = -windowY;
	background1.w = background2.w= windowX, background1.h = background2.h = windowY;

	cord.x = cord.y = 0;
	
	scoreTextPos.x = 1000, scoreTextPos.y = 0;
	scoreTextPos.w = 150;
	scoreTextPos.h = 60;
	// connects our texture with main_pos to control position
	SDL_QueryTexture(spaceship_texture, NULL, NULL, &main_pos.w, &main_pos.h);

	// adjust height and width of our spaceship.
	main_pos.w = 80;
	main_pos.h = 80;

	// sets initial x-position of object
	main_pos.x = (windowX - main_pos.w) / 2;

	// sets initial y-position of object
	main_pos.y = (windowY - main_pos.h) / 2;

	// controls annimation loop
	int close = 0;

	

	vector <fire> fires,fires_tmp; 
	
	fire fr;

	enemy en;
	vector<enemy> enemies,enemies_tmp;
	
	int counter = -1; 
	long long fire_id = 0; // for assigning unique id
	int pause = 1, stop_debug=0;
	bool startFlag = 0;

	speed = 600; // spaceship
	//skills
	//add these skills -> bullet cnt ++, cooldown --, movement speed ++, bullet damage ++( multiple bullet under 1 image max 3) FT510
	// add these extras -> lucky enemies with special ammos - 4 bullet in one , unlucky bullet increase enemy health, big bullet 
	bullet_cnt = 0; // 3 bullet
	cooldown = 25; 
	damage_multiplier = 1; // explosion 
	int cooldown_cnt = 30, used_skill_point=0, escapedEnemies=0; 
	int bullet_crd[4][4] = {{33,0,0,0},{0,66,0,0},{0,33,66,0},{-8,18,44,70}};
	long long starting_time = time(NULL);
	//animation loop
	while (!close) {
		
		skill_point = score / 50;

		cooldown_cnt++;
		SDL_Event event;
		if (escapedEnemies >= characther_HP) {
			while (SDL_PollEvent(&event)) {

				switch (event.type) {
				case SDL_QUIT:
					// handling of close button
					close = 1;
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT) {
						int x, y;
						SDL_GetMouseState(&x, &y);
						map<string, button>::iterator itr;
						//printf("%d %d\n", x, y);
						for (itr = gameOverButtons.begin(); itr != gameOverButtons.end(); itr++) {
							if ((*itr).second.check_pressed(x, y) != -1) {
								//cout << (*itr).second.check_pressed(x, y) << " "<<(*itr).second.id << endl;
								switch ((*itr).second.check_pressed(x, y))
								{
								case 0://restart everything
									main_pos.x = (windowX - main_pos.w) / 2;
									main_pos.y = (windowY - main_pos.h) / 2;

									fires.clear();
									enemies.clear();
									skill_point = 0;
									cooldown = 25, damage_multiplier = 1, bullet_cnt = 0,speed=600;
									cooldown_cnt = 30, used_skill_point = 0, escapedEnemies = 0;
									for (auto &itr : skills) {
										itr.second.level = 0;
									}
									startFlag = 0;
									fire_id = 0;
									counter = -1;
									score = 0;
									break;
								case 1:
									close = 1;
									break;
								default:
									break;
								}
							}
						}
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_P:
						pause = 1 - pause;
						break;
					default:
						break;
					}
				default:
					break;
				}

			}
			SDL_RenderClear(rend);
			SDL_RenderCopy(rend, pauseBackgroundTexture, NULL, NULL);
			SDL_RenderCopy(rend, gameOverButtons["restart"].texture, NULL, &gameOverButtons["restart"].cord);
			SDL_RenderCopy(rend, gameOverButtons["quit"].texture, NULL, &gameOverButtons["quit"].cord);
			int x, y;
			SDL_GetMouseState(&x, &y);
			for (auto itr : buttons) {
				switch (itr.second.check_pressed(x, y))
				{
				case 0:
					SDL_RenderCopy(rend, gameOverButtons["restart"].texture2, NULL, &gameOverButtons["restart"].cord);
					break;
				case 1:
					SDL_RenderCopy(rend, gameOverButtons["quit"].texture2, NULL, &gameOverButtons["quit"].cord);
					break;
				default:
					break;
				}
			}
			SDL_RenderPresent(rend);

			// calculates to 60 fps
			SDL_Delay(windowX / 60);
		}
		else if (!pause) {
			//Main Game
			if (counter > 10000) counter = -1;
			counter++;
			if (counter % 100 == 0) {

				
				en.pos = cord;
				en.pos.x = mt() % (windowX - 200) + 15;
				en.texture = green_enemy;
				
				// luck block enemies FT510
				//if (mt() % 10 == 0) en.texture = lucky_enemy;
				
				
				enemies.push_back(en);
				
				SDL_QueryTexture(en.texture, NULL, NULL, &enemies[enemies.size() - 1].pos.w, &enemies[enemies.size() - 1].pos.h);
				enemies[enemies.size() - 1].pos.w = 30;
				enemies[enemies.size() - 1].pos.h = 30;
			}
			// Events management
			while (SDL_PollEvent(&event)) {
				switch (event.type) {

				case SDL_QUIT:
					// handling of close button
					close = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT && cooldown_cnt>=cooldown) {
						for (int i = 0; i < bullet_cnt%4+1; i++) {
							for (int j = 0; j < damage_multiplier; j++) {
								fr.pos = main_pos;
								fr.pos.x += bullet_crd[bullet_cnt % 4][i];
								fires.push_back(fr);
								fr.id = fire_id++;
								SDL_QueryTexture(laser_tex, NULL, NULL, &fires[fires.size() - 1].pos.w, &fires[fires.size() - 1].pos.h);
								fires[fires.size() - 1].pos.w = bulletX;
								fires[fires.size() - 1].pos.h = bulletY;
							}
						}
						cooldown_cnt = 0;
					}
					break;


				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_E:
						stop_debug = 1 - stop_debug; // for debug
 						break;
					case SDL_SCANCODE_G:
						score += 10; // for not wasting time
						break;
					case SDL_SCANCODE_T:
						cout << time(NULL) - starting_time << endl; // for fixing some time related bugs
						break;
					case SDL_SCANCODE_ESCAPE:
					case SDL_SCANCODE_P:
						pause = 1-pause;
						break;
					case SDL_SCANCODE_W:
					case SDL_SCANCODE_UP:
						main_pos.y -= speed / 30;
						break;
					case SDL_SCANCODE_A:
					case SDL_SCANCODE_LEFT:
						main_pos.x -= speed / 30;
						break;
					case SDL_SCANCODE_S:
					case SDL_SCANCODE_DOWN:
						main_pos.y += speed / 30;
						break;
					case SDL_SCANCODE_D:
					case SDL_SCANCODE_RIGHT:
						main_pos.x += speed / 30;
						break;
					case SDL_SCANCODE_1:
						if (skill_point - used_skill_point > 0 && skills["bulletCnt"].level < 3) {
							bullet_cnt++;
							skills["bulletCnt"].level++;
							used_skill_point++;
						}
						break;
					case SDL_SCANCODE_2:
						if (skill_point - used_skill_point > 0 && skills["cooldown"].level < 3) {
							skills["cooldown"].level++;
							cooldown -= 5;
							used_skill_point++;
						}
						break;
					case SDL_SCANCODE_3:
						if (skill_point - used_skill_point > 0 && skills["speed"].level < 3) {
							skills["speed"].level++;
							speed += 30;
							used_skill_point++;
						}
						break;
					case SDL_SCANCODE_4:
						if (skill_point - used_skill_point > 0 && skills["damage"].level < 3) {
							damage_multiplier++;
							skills["damage"].level++;
							used_skill_point++;
						}
						break;
					default:
						break;
					}
				}
			}

			// right boundary
			if (main_pos.x + main_pos.w > windowX)
				main_pos.x = windowX - main_pos.w;

			// left boundary
			if (main_pos.x < 0)
				main_pos.x = 0;

			// bottom boundary
			if (main_pos.y + main_pos.h > windowY)
				main_pos.y = windowY - main_pos.h;

			// upper boundary
			if (main_pos.y < 0)
				main_pos.y = 0;

			// clears the screen
			if (!stop_debug) {
				SDL_RenderClear(rend);

				//Background movement
				background1.y++;
				background2.y++;

				if (background1.y == windowY) background1.y = -windowY;
				if (background2.y == windowY) background2.y = -windowY;

				SDL_RenderCopy(rend, background_image, NULL, &background1);
				SDL_RenderCopy(rend, background_image2, NULL, &background2);


				SDL_RenderCopy(rend, spaceship_texture, NULL, &main_pos);
				//score update
				scr = "Score: " + to_string(score);
				surface = TTF_RenderText_Solid(font, scr.c_str(), White);
				if (surface == NULL) {
					cout <<time(NULL)-starting_time<<" "<< TTF_GetError() << endl;
				}

				scoreTextTexture = SDL_CreateTextureFromSurface(rend, surface);
				SDL_FreeSurface(surface);
				SDL_RenderCopy(rend, scoreTextTexture, NULL, &scoreTextPos);


				for (int i = 0; i < fires.size(); i++) {
					if (fires[i].life > 0) {
						SDL_RenderCopy(rend, laser_tex, NULL, &fires[i].pos);
						for (int j = 0; j < enemies.size(); j++) {
							if (check_hit(fires[i], enemies[j])) {
								//cout << enemies[j].hit_by.size() << endl;
								enemies[j].hit_tex_life = 10;
								enemies[j].hit_by.insert(fires[i].id);
							}
						}
						//fires[i].pos.x += 10;
						fires[i].pos.y -= 10;
						fires[i].life--;
						if (fires[i].life > 0)
							fires_tmp.push_back(fires[i]);
					}
				}

				fires.clear();
				for (int i = 0; i < fires_tmp.size(); i++) fires.push_back(fires_tmp[i]);
				fires_tmp.clear();
				int escaped_enemies = 0; // escaped enemies for 1 unit of time
				for (int i = 0; i < enemies.size(); i++) {
					if (enemies[i].pos.y >= 800) escaped_enemies++;
					if (enemies[i].hit_by.size() < enemies[i].HP && enemies[i].pos.y < 800) {

						// change color by health
						if (enemies[i].hit_by.size() >= enemies[i].HP / 3 * 2) enemies[i].texture = red_enemy;
						else if (enemies[i].hit_by.size() >= enemies[i].HP / 3) enemies[i].texture = yellow_enemy;
						
						

						if (enemies[i].hit_tex_life <= 10 && enemies[i].hit_tex_life > 0) {
							SDL_RenderCopy(rend, on_hit_tex, NULL, &enemies[i].pos);
							enemies[i].hit_tex_life--;
						}
						else if (enemies[i].hit_tex_life <= 0 || enemies[i].hit_tex_life == 11) {
							enemies[i].hit_tex_life = 11;
							SDL_RenderCopy(rend, enemies[i].texture, NULL, &enemies[i].pos);
						}
						//fires[i].pos.x += 10;
						enemies[i].pos.y += 2;
						enemies_tmp.push_back(enemies[i]);
					}
					else if (enemies[i].hit_by.size() >= 4 && enemies[i].texture == lucky_enemy) {
						//lucky_func();
					}

				}
				escapedEnemies += escaped_enemies;
				score += (enemies.size() - enemies_tmp.size()-escaped_enemies) * 10;
				
				enemies.clear();
				for (int i = 0; i < enemies_tmp.size(); i++) {
					enemies.push_back(enemies_tmp[i]);
				}
				enemies_tmp.clear();

				for (auto itr : skills) {
					SDL_RenderCopy(rend, itr.second.textures[itr.second.level], NULL, &itr.second.cord);
					if(skill_point - used_skill_point > 0 && itr.second.level<3)
					SDL_RenderCopy(rend, itr.second.up_textures[itr.second.level], NULL, &itr.second.up_cord);
					//cout << itr.first << endl;
				}
				RenderHPBar(windowX - 300, windowY-25, 200, 20, escapedEnemies*1.0/characther_HP, White, random, rend);
				SDL_RenderPresent(rend);
			}

			// calculates to 60 fps
			SDL_Delay(1000 / 60);
		}
		else {
			while (SDL_PollEvent(&event)) {
				
				switch (event.type) {
					case SDL_QUIT:
					// handling of close button
					close = 1;
					break;
					case SDL_MOUSEBUTTONUP:
						if (event.button.button == SDL_BUTTON_LEFT) {
							int x, y;
							SDL_GetMouseState(&x, &y);
							map<string, button>::iterator itr;
							//printf("%d %d\n", x, y);
							for (itr = buttons.begin(); itr != buttons.end(); itr++) {
								if ((*itr).second.check_pressed(x, y) != -1) {
									//cout << (*itr).second.check_pressed(x, y) << " "<<(*itr).second.id << endl;
									switch ((*itr).second.check_pressed(x, y))
									{
									case 0:
										if (startFlag == 0) {
											startFlag++;
											buttons["play"].texture = buttons["continue"].texture;
											buttons["play"].texture2 = buttons["continue"].texture2;
										}
										pause = pause - 1;
										break;
									case 2:
										close = 1;
										break;
									default:
										break;
									}
								}
							}
						}
						break;
					case SDL_KEYDOWN:
						switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_P:
							pause = 1 - pause;
							break;
						default:
							break;
						}
					default:
						break;
				}

			}
			SDL_RenderClear(rend);
			SDL_RenderCopy(rend, pauseBackgroundTexture, NULL, NULL);
			SDL_RenderCopy(rend, buttons["play"].texture, NULL, &buttons["play"].cord);
			SDL_RenderCopy(rend, buttons["settings"].texture, NULL, &buttons["settings"].cord);
			SDL_RenderCopy(rend, buttons["quit"].texture, NULL, &buttons["quit"].cord);
			int x, y;
			SDL_GetMouseState(&x, &y);
			for (auto itr : buttons) {
				switch (itr.second.check_pressed(x, y))
				{
				case 0:
					SDL_RenderCopy(rend, buttons["play"].texture2, NULL, &buttons["play"].cord);
					break;
				case 1:
					SDL_RenderCopy(rend, buttons["settings"].texture2, NULL, &buttons["settings"].cord);
					break;
				case 2:
					SDL_RenderCopy(rend, buttons["quit"].texture2, NULL, &buttons["quit"].cord);
					break;
				default:
					break;
				}
			}
			SDL_RenderPresent(rend);

			// calculates to 60 fps
			SDL_Delay(windowX / 60);
		}
	}

	// posroy texture
	SDL_DestroyTexture(spaceship_texture);
	SDL_DestroyTexture(background_image);
	SDL_DestroyTexture(background_image2);
	SDL_DestroyTexture(lucky_enemy);
	SDL_DestroyTexture(green_enemy);
	SDL_DestroyTexture(laser_tex);
	SDL_DestroyTexture(on_hit_tex);
	for (auto itr : buttons) {
		SDL_DestroyTexture(itr.second.texture);
	}
	// destroy renderer
	SDL_DestroyRenderer(rend);
	// destroy window
	SDL_DestroyWindow(win);

	// close SDL
	SDL_Quit();

	return 0;
}
