#include "Player.h"
#include <iostream>
#include <fstream>
#include <cmath>

int sgn(int x) {
  if (std::abs(x) <= 3 * tileSize) {
    return 0;
  }
  if (x < 0) {
    return -1;
  }
  if (x > 0) {
    return 1;
  }
}

std::pair<int, int> get_tile_coords(int x, int y) {
  return {(x - WINDOW_WIDTH / 2 + tileSize * LEVEL_X / 2) / tileSize, 
        (y - WINDOW_HEIGHT / 2 + tileSize * LEVEL_Y / 2) / tileSize};
}

void draw_img(Image &img, int x, int y, Image *screen) {
  for (int i = y; i < y + img.Height(); ++i) {
    for (int j = x; j < x + img.Width(); ++j) {
      screen->PutPixel(j, i, img.GetPixel(j - x, img.Height() - (i - y) - 1));
    }
  }
}

bool Player::Moved() const
{
  if(coords.x == old_coords.x && coords.y == old_coords.y)
    return false;
  else
    return true;
}

int Player::ProcessInput(MovementDir dir)
{
  int move_dist = move_speed * 1;
  int x = old_coords.x, y = old_coords.y;
  switch(dir)
  {
    case MovementDir::UP:
      old_coords.y = coords.y;
      coords.y += move_dist;
      break;
    case MovementDir::DOWN:
      old_coords.y = coords.y;
      coords.y -= move_dist;
      break;
    case MovementDir::LEFT:
      old_coords.x = coords.x;
      coords.x -= move_dist;
      break;
    case MovementDir::RIGHT:
      old_coords.x = coords.x;
      coords.x += move_dist;
      break;
    default:
      break;
  }
  std::pair<int, int> tile_coords = get_tile_coords(coords.x + tileSize / 2, coords.y + tileSize / 2);
  char c = lab->level[tile_coords.second][tile_coords.first];
  if (c == ' ') {
    Image gameover = Image("./template1_cpp/img/Gameover.png");
    draw_img(gameover, WINDOW_WIDTH / 2 - LEVEL_X * tileSize / 2, WINDOW_HEIGHT - 20 * tileSize, this->lab->screen);
    return -1;
  }
  if (c == '#') {
    coords.x = old_coords.x;
    coords.y = old_coords.y;
    old_coords.x = x;
    old_coords.y = y;
  }
  if (c == '@') {
    int dx = sgn(coords.x - WINDOW_WIDTH / 2);
    int dy = -sgn(coords.y - WINDOW_HEIGHT / 2);
    int change = lab->draw_level(x_global + dx, y_global + dy);
    if (change != -1) {
      x_global += dx;
      y_global += dy;
      coords.x = WINDOW_WIDTH / 2 - dx * (LEVEL_X / 2 - 3) * tileSize;
      coords.y = WINDOW_HEIGHT / 2 + dy * (LEVEL_Y / 2 - 3) * tileSize;
      return 1;
    }
  }
  if (c == 'G' && !this->lab->used[y_global][x_global]) {
    Image gold = Image("./template1_cpp/img/chest_full_open_anim_f0.png");
    draw_img(gold, this->lab->x_offs + this->counter * (tileSize + 3), WINDOW_HEIGHT / 2 - (LEVEL_Y / 2 + 2) * tileSize, this->lab->screen);
    this->counter += 1;
    this->lab->used[y_global][x_global] = true;
  }
  if (c == 'Q') {
    Image gameover = Image("./template1_cpp/img/win.png");
    draw_img(gameover, WINDOW_WIDTH / 2 - LEVEL_X * tileSize / 2, WINDOW_HEIGHT - 20 * tileSize, this->lab->screen);
    return -1;
  }
  return 0;
}

void Player::Draw(Image &screen)
{
  Image img = Image("./template1_cpp/img/knight_idle_anim_f0.png");
  if(Moved())
  {
    for(int i = old_coords.y; i <= old_coords.y + tileSize; ++i)
    {
      for(int j = old_coords.x; j <= old_coords.x + tileSize; ++j)
      {
        screen.PutPixel(j, i, this->lab->background->GetPixel(j, i));
      }
    }
    old_coords = coords;
  }
  for(int i = coords.y; i < coords.y + tileSize; ++i)
  {
    for(int j = coords.x; j < coords.x + tileSize; ++j)
    {
      Pixel curr = img.GetPixel(j - coords.x, i - coords.y);
      if (curr.a != 0)
        screen.PutPixel(j, i, curr);
    }
  }
}

Labirynth::Labirynth(const std::string &a_path, Image *screen, Image *background) 
{
  this->screen = screen;
  this->background = background;
  std::ifstream f;
  f.open(a_path);
  this->lab.resize(LAB_Y);
  this->used.resize(LAB_Y);
  for (int i = 0; i < LAB_Y; ++i) {
    getline(f, this->lab[i]);
    this->used[i].resize(LAB_X);
    std::fill(used[i].begin(), used[i].end(), false);
  }
  Image gold = Image("./template1_cpp/img/Gold.png");
  draw_img(gold, WINDOW_WIDTH / 2 - LEVEL_X * tileSize / 2, WINDOW_HEIGHT / 2 - (LEVEL_Y / 2 + 2) * tileSize - 2, screen);
  this->x_offs = WINDOW_WIDTH / 2 - LEVEL_X * tileSize / 2 + gold.Width();
  this->draw_level(0, 0);
}


int Labirynth::draw_level(int x, int y) 
{
  if (x < 0 || x >= LAB_X || y < 0 || y >= LAB_Y || this->lab[y][x] == '#') {
    return -1;
  }
  std::ifstream f;
  std::string name = "./template1_cpp/data/";
  name.push_back(this->lab[y][x]);
  name.append(".txt");
  f.open(name);
  this->level.resize(LEVEL_Y);
  for (int i = 0; i < LEVEL_Y; ++i) {
    getline(f, this->level[i]);
  }
  Image WALL = Image("./template1_cpp/img/wall_mid.png");
  Image LADDER = Image("./template1_cpp/img/floor_ladder.png");
  Image FLOOR = Image("./template1_cpp/img/floor_1.png");
  Image GOLD = Image("./template1_cpp/img/chest_full_open_anim_f0.png");
  Image DOOR = Image("./template1_cpp/img/doors_leaf_closed.png");
  int xbeg = WINDOW_WIDTH / 2 - tileSize * LEVEL_X / 2;
  int ybeg = WINDOW_HEIGHT / 2 - tileSize * LEVEL_Y / 2;
  for (int i = 0; i < tileSize * LEVEL_Y; ++i) {
    for (int j = 0; j < tileSize * LEVEL_X; ++j) {
      char c = this->level[i / tileSize][j / tileSize];
      if (c == '#') {
        screen->PutPixel(xbeg + j, ybeg + i, WALL.GetPixel(j % tileSize, i % tileSize));
        this->background->PutPixel(xbeg + j, ybeg + i, WALL.GetPixel(j % tileSize, i % tileSize));
      }
      else if (c == '.') {
        screen->PutPixel(xbeg + j, ybeg + i, FLOOR.GetPixel(j % tileSize, i % tileSize));
        this->background->PutPixel(xbeg + j, ybeg + i, FLOOR.GetPixel(j % tileSize, i % tileSize));
      }
      else if (c == '@') {
        screen->PutPixel(xbeg + j, ybeg + i, LADDER.GetPixel(j % tileSize, i % tileSize));
        this->background->PutPixel(xbeg + j, ybeg + i, LADDER.GetPixel(j % tileSize, i % tileSize));
      }
      else if (c == 'G') {
        screen->PutPixel(xbeg + j, ybeg + i, GOLD.GetPixel(j % tileSize, i % tileSize));
        this->background->PutPixel(xbeg + j, ybeg + i, GOLD.GetPixel(j % tileSize, i % tileSize));
      }
      else if (c == ' ') {
        screen->PutPixel(xbeg + j, ybeg + i, backgroundColor);
        this->background->PutPixel(xbeg + j, ybeg + i, backgroundColor);
      }
      else if (c == 'Q') {
        screen->PutPixel(xbeg + j, ybeg + i, DOOR.GetPixel(j % tileSize, i % tileSize));
        this->background->PutPixel(xbeg + j, ybeg + i, DOOR.GetPixel(j % tileSize, i % tileSize));
      }
    }
  }
  return 0;
}