// Copyright 2023 David Lareau. This program is free software under the terms of the Zero Clause BSD.
// gcc --pedantic -Wall -Werror-implicit-function-declaration -Wno-pointer-sign -o zeldaish *.c $(pkg-config --libs --cflags libxml-2.0 raylib) -lm

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <math.h>
#include "data-util.h"
#include <raylib.h>

static void center_fit(double bounds_w, double bounds_h, double surface_w, double surface_h, double * out_scale, double * out_x, double * out_y) { if ((bounds_w / bounds_h) > (surface_w / surface_h)) *out_scale = bounds_h / surface_h; else *out_scale = bounds_w / surface_w; if(out_x) *out_x = (bounds_w - surface_w * *out_scale) / 2; if(out_y) *out_y = (bounds_h - surface_h * *out_scale) / 2; }

enum vk { LEFT, RIGHT, ACTION, UP, DOWN };
enum vk_filter { JOY_0 = 1, JOY_1 = 2, JOY_2 = 4, JOY_3 = 8, KEYBOARD = 16, ALL_INPUT = 0xFFFF };
static bool gamepad_trust[4];
double vk_key(enum vk k) {
  const int filter = ALL_INPUT;
  int key = -1, key2 = -1, button = -1, button2 = -1, axis = -1; double axis_min, axis_max;
  switch(k) {
    case LEFT: key = KEY_LEFT; key2 = KEY_A; button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; axis = GAMEPAD_AXIS_LEFT_X; axis_min = -1; axis_max = -.4; break;
    case RIGHT: key = KEY_RIGHT; key2 = KEY_D; button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; axis = GAMEPAD_AXIS_LEFT_X; axis_min = .4; axis_max = 1; break;
    case UP: key = KEY_UP; key2 = KEY_W; button = GAMEPAD_BUTTON_LEFT_FACE_UP; axis = GAMEPAD_AXIS_LEFT_Y; axis_min = -1; axis_max = -.4; break;
    case DOWN: key = KEY_DOWN; key2 = KEY_S; button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; axis = GAMEPAD_AXIS_LEFT_Y; axis_min = .4; axis_max = 1; break;
    case ACTION: key = KEY_SPACE; key2 = KEY_X; button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; button2 = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
  }
  if(filter & KEYBOARD) {
    if(key != -1 && IsKeyDown(key)) return 1;
    if(key2 != -1 && IsKeyDown(key2)) return 1;
  }
  for(int gamepad = 0; gamepad < 4; gamepad++) {
    if(!IsGamepadAvailable(gamepad)) continue;
    if(!gamepad_trust[gamepad]) gamepad_trust[gamepad] = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_UP) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    if(!gamepad_trust[gamepad]) continue;
    if(gamepad == 0 && !(filter & JOY_0)) continue;
    if(gamepad == 1 && !(filter & JOY_1)) continue;
    if(gamepad == 2 && !(filter & JOY_2)) continue;
    if(gamepad == 3 && !(filter & JOY_3)) continue;
    if(button != -1 && IsGamepadButtonDown(gamepad, button)) return 1;
    if(button2 != -1 && IsGamepadButtonDown(gamepad, button2)) return 1;
    if(axis != -1) {
      double value = GetGamepadAxisMovement(gamepad, axis);
      if(value >= axis_min && value <= axis_max) return value / (axis_max - axis_min);
    }
  }
  return 0;
}
double vk_key_released(enum vk k) {
  const int filter = ALL_INPUT;
  int key = -1, key2 = -1, button = -1, button2 = -1, axis = -1; double axis_min, axis_max;
  switch(k) {
    case LEFT: key = KEY_LEFT; key2 = KEY_A; button = GAMEPAD_BUTTON_LEFT_FACE_LEFT; axis = GAMEPAD_AXIS_LEFT_X; axis_min = -1; axis_max = -.4; break;
    case RIGHT: key = KEY_RIGHT; key2 = KEY_D; button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT; axis = GAMEPAD_AXIS_LEFT_X; axis_min = .4; axis_max = 1; break;
    case UP: key = KEY_UP; key2 = KEY_W; button = GAMEPAD_BUTTON_LEFT_FACE_UP; axis = GAMEPAD_AXIS_LEFT_Y; axis_min = -1; axis_max = -.4; break;
    case DOWN: key = KEY_DOWN; key2 = KEY_S; button = GAMEPAD_BUTTON_LEFT_FACE_DOWN; axis = GAMEPAD_AXIS_LEFT_Y; axis_min = .4; axis_max = 1; break;
    case ACTION: key = KEY_SPACE; key2 = KEY_X; button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN; button2 = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
  }
  if(filter & KEYBOARD) {
    if(key != -1 && IsKeyReleased(key)) return 1;
    if(key2 != -1 && IsKeyReleased(key2)) return 1;
  }
  for(int gamepad = 0; gamepad < 4; gamepad++) {
    if(!IsGamepadAvailable(gamepad)) continue;
    if(!gamepad_trust[gamepad]) gamepad_trust[gamepad] = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_UP) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT) | IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    if(!gamepad_trust[gamepad]) continue;
    if(gamepad == 0 && !(filter & JOY_0)) continue;
    if(gamepad == 1 && !(filter & JOY_1)) continue;
    if(gamepad == 2 && !(filter & JOY_2)) continue;
    if(gamepad == 3 && !(filter & JOY_3)) continue;
    if(button != -1 && IsGamepadButtonReleased(gamepad, button)) return 1;
    if(button2 != -1 && IsGamepadButtonReleased(gamepad, button2)) return 1;
    if(axis != -1) {
      double value = GetGamepadAxisMovement(gamepad, axis);
      if(value >= axis_min && value <= axis_max) return value / (axis_max - axis_min);
    }
  }
  return 0;
}

// NOTES: cane / elf / key / chest / bottle / fountain / fire / staff / wizard / spell / dragon / heart

struct tile_animation {
  int size;
  int * ids;
  uint64_t * durations;
  uint64_t total_duration;
};

struct map_node {
  const char * filename;
  struct map_node * north;
  struct map_node * south;
  struct map_node * east;
  struct map_node * west;
};

struct rect {
  double x;
  double y;
  double w;
  double h;
};

bool collides_1D(double p, double pl, double q, double ql) {
  return p + pl >= q && p <= q + ql;
}

bool collides_2D(struct rect * p, struct rect * q) {
  return collides_1D(p->x, p->w, q->x, q->w) && collides_1D(p->y, p->h, q->y, q->h);
}

bool collides_2D_dx(double px, double py, double pw, double ph, struct rect * q) {
  struct rect p = {px, py, pw, ph};
  return collides_2D(&p, q);
}

//[0, 1[
double bound_cyclic_normalized(double x) {
  if (x < 0) {
    if ((int) x - x == 0) return 0;
    return 1 - bound_cyclic_normalized(-x);
  }
  return x - (int) x;
}

//[0, 1]
double bound_cyclic_back_and_forth_normalized(double x) {
  if (x < 0) return bound_cyclic_back_and_forth_normalized(-x);
  int i = (int) x;
  double cyclic = bound_cyclic_normalized(x);
  // if pair
  if (i % 2 == 0) {
    return cyclic;
  }
  // if odd
  else {
    return 1 - cyclic; // this is where if it was 0, now its exactly 1, so the max limit is inclusive
  }
}

struct axis {
  double lx, ly;
};

int main(int argc, char * argv[]) {
  // window
  int W = 256;
  int H = 224;
  bool fullscreen = false; Vector2 stored_window_position, stored_window_size;
  InitWindow(W, H, argv[0]); SetWindowState(FLAG_WINDOW_RESIZABLE); SetWindowState(FLAG_VSYNC_HINT);
  HideCursor();
  RenderTexture2D framebuffer = LoadRenderTexture(W, H);
  
  // audio
  InitAudioDevice();
  double bg_volume = .7;
  Music bg = LoadMusicStream("bg.ogg");
  SetMusicVolume(bg, bg_volume);
  PlayMusicStream(bg);
  Sound snd_elf_0 = LoadSound("elf_0.ogg"); if(!snd_elf_0.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_elf_2 = LoadSound("elf_2.ogg"); if(!snd_elf_2.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_elf_1 = LoadSound("elf_1.ogg"); if(!snd_elf_1.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_open = LoadSound("open.ogg"); if(!snd_open.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_locked = LoadSound("locked.ogg"); if(!snd_locked.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_empty = LoadSound("empty.ogg"); if(!snd_empty.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_flame = LoadSound("flame.ogg"); if(!snd_flame.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_wiz_1 = LoadSound("wiz_1.ogg"); if(!snd_wiz_1.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_wiz_2 = LoadSound("wiz_2.ogg"); if(!snd_wiz_2.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_wiz_0 = LoadSound("wiz_0.ogg"); if(!snd_wiz_0.stream.buffer) { exit(EXIT_FAILURE); }
  Sound snd_garden = LoadSound("garden.ogg"); if(!snd_garden.stream.buffer) { exit(EXIT_FAILURE); }
  
  // font
  Font font = LoadFont("DejaVuSans-Bold.ttf");

  // world
  struct map_node fountain = {"fountain.tmx"};
  struct map_node forest = {"forest.tmx"};
  struct map_node elf = {"elf.tmx"};
  struct map_node fire = {"fire.tmx"};
  struct map_node dragon = {"dragon.tmx"};
  struct map_node wizard = {"wizard.tmx"};
  struct map_node cave = {"cave.tmx"};
  fountain.west = &elf; elf.east = &fountain;
  fountain.north = &dragon; dragon.south = &fountain;
  dragon.west = &fire; fire.east = &dragon;
  dragon.east = &wizard; wizard.west = &dragon;
  wizard.south = &forest; forest.north = &wizard;
  struct dict warps;
  dict_init(&warps, 0, true, false);
  dict_set(&warps, "cave", &cave);
  dict_set(&warps, "wizard", &wizard);
  struct map_node * map = NULL;
  struct map_node * next_map = &fountain;

  // tileset
  struct dict animated_tiles;
  struct dict blocking_tiles;
  int tileset_columns;
  xmlChar * tileset_image = NULL;
  dict_init(&animated_tiles, sizeof(struct tile_animation), false, false);
  dict_init(&blocking_tiles, 0, false, false);

  // images
  struct dict npc_res; dict_init(&npc_res, 0, true, false);
  Texture2D texture_elf = LoadTexture("boggart.CC0.crawl-tiles.png"); dict_set(&npc_res, "elf", &texture_elf);
  Texture2D texture_dragon = LoadTexture("dragon.CC0.crawl-tiles.png"); dict_set(&npc_res, "dragon", &texture_dragon);
  Texture2D texture_wizard = LoadTexture("human.CC0.crawl-tiles.png"); dict_set(&npc_res, "wizard", &texture_wizard);
  Texture2D texture_chest = LoadTexture("chest_2_closed.CC0.crawl-tiles.png"); dict_set(&npc_res, "bottle", &texture_chest);
  Texture2D texture_kaboom = LoadTexture("8.CC0.pixel-boy.png"); dict_set(&npc_res, "kaboom", &texture_kaboom);
  Texture2D texture_chest_2 = LoadTexture("chest_2_open.CC0.crawl-tiles.png");
  Texture2D texture_flame[8] = {
    LoadTexture("dngn_altar_makhleb_flame1.CC0.crawl-tiles.png"),
    LoadTexture("dngn_altar_makhleb_flame2.CC0.crawl-tiles.png"),
    LoadTexture("dngn_altar_makhleb_flame3.CC0.crawl-tiles.png"),
    LoadTexture("dngn_altar_makhleb_flame4.CC0.crawl-tiles.png"),
    LoadTexture("dngn_altar_makhleb_flame5.CC0.crawl-tiles.png"),
    LoadTexture("dngn_altar_makhleb_flame6.CC0.crawl-tiles.png"),
    LoadTexture("dngn_altar_makhleb_flame7.CC0.crawl-tiles.png"),
    LoadTexture("dngn_altar_makhleb_flame8.CC0.crawl-tiles.png")
  };
  dict_set(&npc_res, "flame", texture_flame); // 1 to 8
  Texture2D texture_princess = LoadTexture("princess.clamp.png");
  struct rect collision = {1, 14, 12, 8}; // hard-coded princess collision box
  struct dict items;
  dict_init(&items, 0, true, false);
  Texture2D texture_cane = LoadTexture("cane.resized.CC0.7soul1.png"); dict_set(&items, "cane", &texture_cane);
  Texture2D texture_key = LoadTexture("key.resized.CC0.7soul1.png"); dict_set(&items, "key", &texture_key);
  Texture2D texture_bottle = LoadTexture("bottle.resized.CC0.7soul1.png"); dict_set(&items, "bottle", &texture_bottle);
  Texture2D texture_water = LoadTexture("water.resized.CC0.7soul1.png"); dict_set(&items, "water", &texture_water);
  Texture2D texture_heart = LoadTexture("heart.resized.CC0.7soul1.png"); dict_set(&items, "heart", &texture_heart);
  Texture2D texture_staff = LoadTexture("staff02.CC0.crawl-tiles.png"); dict_set(&items, "staff", &texture_staff);
  Texture2D texture_spell = LoadTexture("scroll-thunder.CC0.pixel-boy.png"); dict_set(&items, "spell", &texture_spell);
  // for sake of demo, also preload the known tileset file
  Texture2D texture_map;

  // map
  const int TS = 16;
  const int MAP_COL = 16;
  const int MAP_ROW = 11;
  const int LAYERS_CAPACITY = 2;
  int layers[LAYERS_CAPACITY][MAP_ROW][MAP_COL];
  int layers_size;
  bool warping = false;
  struct rect warp;
  struct map_node * warp_map;
  struct rect item;
  Texture2D * item_id = NULL;
  struct rect npc;
  char * npc_id = NULL;

  // states
  double px = 0;
  double py = 0;
  struct rect forward;
  forward.w = TS;
  forward.h = TS;
  Texture2D * held_item = NULL;
  uint64_t kaboom_t0 = -1;
  const char * message = NULL;
  struct dict npc_state;
  dict_init(&npc_state, 0, true, false);
  struct dict ignore;
  dict_init(&ignore, 0, true, false);
  bool running = true;
  uint64_t winner_t0 = -1;

  // game loop
  double delta_time = 0;
  uint64_t tick = 0;
  double step_per_seconds = 125;
  int facing_index = 0;
  bool facing_mirror = false;
  int facing_frame = 0;
  uint64_t walking_t0;
  const int walking_period = 300;
  SetTargetFPS(60);
  bool go_fullscreen = true;
  double t0 = GetTime();
  while(running && !WindowShouldClose()) {
    double t = GetTime(); delta_time = t - t0; t0 = t;
    //printf("DAVE tick %f\n", t);
    tick = (uint64_t)(t * 1000); // TODO is this ported right?
    
    UpdateMusicStream(bg);

    // parse map created with Tiled (https://www.mapeditor.org/)
    // [with many assumptions like tile size, single tileset across all maps, single warp rect, single npc]
    if(next_map) {
      //printf("DAVE LOADING NEXT MAP BEGIN\n");
      layers_size = 0;
      warp_map = NULL;
      item_id = NULL;
      if(npc_id) free(npc_id);
      npc_id = NULL;
      xmlDoc * doc = xmlParseFile(next_map->filename); if(!doc) { printf("xmlParseFile(%s) failed.\n", next_map->filename); exit(EXIT_FAILURE); }
      xmlNode * mcur = xmlDocGetRootElement(doc); if(!mcur) { printf("xmlDocGetRootElement() is null.\n"); exit(EXIT_FAILURE); }
      mcur = mcur->xmlChildrenNode;
      while(mcur != NULL) {
        // load tileset
        if(!tileset_image && xmlStrcmp(mcur->name, "tileset") == 0) {
          xmlChar * source = xmlGetProp(mcur, "source");
          xmlDoc * tileset = xmlParseFile(source); if(!tileset) { printf("xmlParseFile(%s) failed.\n", source); exit(EXIT_FAILURE); }
          xmlNode * tcur = xmlDocGetRootElement(tileset); if(!tcur) { printf("xmlDocGetRootElement() is null.\n"); exit(EXIT_FAILURE); }
          xmlFree(source);
          xmlChar * str_columns = xmlGetProp(tcur, "columns");
          tileset_columns = strtol(str_columns, NULL, 10);
          xmlFree(str_columns);
          tcur = tcur->xmlChildrenNode;
          while(tcur != NULL) {
            if(xmlStrcmp(tcur->name, "image") == 0) {
              tileset_image = xmlGetProp(tcur, "source");
              texture_map = LoadTexture(tileset_image);
            }
            else if(xmlStrcmp(tcur->name, "tile") == 0) {
              xmlChar * id = xmlGetProp(tcur, "id");
              // store blocking tiles
              xmlChar * type = xmlGetProp(tcur, "type");
              if(type && xmlStrcmp(type, "block") == 0) dict_set(&blocking_tiles, strtol(id, NULL, 10), true);
              xmlFree(type);
              // store animations
              xmlNode * acur = tcur->xmlChildrenNode;
              while(acur != NULL) {
                if(xmlStrcmp(acur->name, "animation") == 0) {
                  // count how many frames
                  struct tile_animation anim = {0};
                  xmlNode * fcur = acur->xmlChildrenNode;
                  while(fcur != NULL) {
                    if(xmlStrcmp(fcur->name, "frame") == 0) anim.size++;
                    fcur = fcur->next;
                  }
                  // alloc and store in dictionary
                  anim.ids = malloc(sizeof(int) * anim.size);
                  anim.durations = malloc(sizeof(uint64_t) * anim.size);
                  // populate ids/durations
                  fcur = acur->xmlChildrenNode;
                  int i = 0;
                  while(fcur != NULL) {
                    if(xmlStrcmp(fcur->name, "frame") == 0) {
                      xmlChar * t = xmlGetProp(fcur, "tileid");
                      xmlChar * d = xmlGetProp(fcur, "duration");
                      anim.ids[i] = strtol(t, NULL, 10);
                      anim.durations[i] = strtol(d, NULL, 10);
                      anim.total_duration += anim.durations[i];
                      i++;
                      xmlFree(d);
                      xmlFree(t);
                    }
                    fcur = fcur->next;
                  }
                  dict_set(&animated_tiles, strtol(id, NULL, 10), (intptr_t)&anim);
                }
                acur = acur->next;
              }
              xmlFree(id);
            }
            tcur = tcur->next;
          }
        }
        // layers
        else if(xmlStrcmp(mcur->name, "layer") == 0) {
          xmlNode * node = mcur->xmlChildrenNode;
          while(node != NULL) {
            if(xmlStrcmp(node->name, "data") == 0) {
              if(layers_size == LAYERS_CAPACITY) { printf("layers array full\n"); exit(EXIT_FAILURE); }
              int index = layers_size++;
              xmlChar * data = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
              char * p = data;
              int row = 0, col = 0;
              while(*p) {
                // change row
                if(*p == '\n' && col > 0) {
                  row++;
                  col = 0;
                  p++;
                } else {
                  char * end;
                  int tile = strtol(p, &end, 10);
                  // if the number is valid, store it
                  if(end != p) {
                    if(row >= MAP_ROW) { printf("too many row in map data\n"); exit(EXIT_FAILURE); }
                    if(col >= MAP_COL) { printf("too many col in map data\n"); exit(EXIT_FAILURE); }
                    layers[index][row][col++] = tile;
                    p = end;
                  }
                  // if it wasn't a number, skip over
                  else { p++; }
                }
              }
              xmlFree(data);
            }
            node = node->next;
          }
        }
        // object
        else if(xmlStrcmp(mcur->name, "objectgroup") == 0) {
          xmlNode * node = mcur->xmlChildrenNode;
          while(node != NULL) {
            if(xmlStrcmp(node->name, "object") == 0) {
              xmlChar * type = xmlGetProp(node, "type");
              if(type) {
                if(xmlStrcmp(type, "spawn") == 0) {
                  xmlChar * x = xmlGetProp(node, "x");
                  xmlChar * y = xmlGetProp(node, "y");
                  if(warping || !map) {
                    px = strtod(x, NULL) - collision.w/2 - collision.x;
                    py = strtod(y, NULL) - collision.h/2 - collision.y;
                  }
                  xmlFree(y);
                  xmlFree(x);
                }
                else if(xmlStrcmp(type, "warp") == 0) {
                  xmlChar * x = xmlGetProp(node, "x");
                  xmlChar * y = xmlGetProp(node, "y");
                  xmlChar * w = xmlGetProp(node, "width");
                  xmlChar * h = xmlGetProp(node, "height");
                  xmlChar * name = xmlGetProp(node, "name");
                  warp.x = strtod(x, NULL);
                  warp.w = strtod(w, NULL);
                  warp.y = strtod(y, NULL);
                  warp.h = strtod(h, NULL);
                  warp_map = dict_get(&warps, name);
                  if(!warp_map) { printf("invalid warp name %s\n", name); exit(EXIT_FAILURE); }
                  xmlFree(name);
                  xmlFree(h);
                  xmlFree(w);
                  xmlFree(y);
                  xmlFree(x);
                }
                else if(xmlStrcmp(type, "item") == 0) {
                  xmlChar * x = xmlGetProp(node, "x");
                  xmlChar * y = xmlGetProp(node, "y");
                  xmlChar * w = xmlGetProp(node, "width");
                  xmlChar * h = xmlGetProp(node, "height");
                  xmlChar * name = xmlGetProp(node, "name");
                  item_id = dict_get(&items, name);
                  if(dict_has(&ignore, item_id)) item_id = NULL;
                  item.x = strtod(x, NULL);
                  if(w) {
                    item.w = strtod(w, NULL);
                  } else {
                    item.w = TS;
                    item.x -= TS / 2;
                  }
                  item.y = strtod(y, NULL);
                  if(h) {
                    item.h = strtod(h, NULL);
                  } else {
                    item.h = TS;
                    item.y -= TS / 2;
                  }
                  xmlFree(name);
                  xmlFree(h);
                  xmlFree(w);
                  xmlFree(y);
                  xmlFree(x);
                } else if(xmlStrcmp(type, "npc") == 0) {
                  xmlChar * x = xmlGetProp(node, "x");
                  xmlChar * y = xmlGetProp(node, "y");
                  xmlChar * w = xmlGetProp(node, "width");
                  xmlChar * h = xmlGetProp(node, "height");
                  xmlChar * name = xmlGetProp(node, "name");
                  npc.x = strtod(x, NULL);
                  if(w) {
                    npc.w = strtod(w, NULL);
                  } else {
                    npc.w = TS;
                    npc.x -= TS / 2;
                  }
                  npc.y = strtod(y, NULL);
                  if(h) {
                    npc.h = strtod(h, NULL);
                  } else {
                    npc.h = TS;
                    npc.y -= TS / 2;
                  }
                  if(!dict_has(&ignore, name)) npc_id = strdup(name);
                  xmlFree(name);
                  xmlFree(h);
                  xmlFree(w);
                  xmlFree(y);
                  xmlFree(x);
                }
              }
              xmlFree(type);
            }
            node = node->next;
          }
        }
        mcur = mcur->next;
      }
      xmlFreeDoc(doc);
      if(!tileset_image) { printf("did not find anything tileset image while parsing map\n"); exit(EXIT_FAILURE); }
      map = next_map;
      next_map = NULL;
      warping = false;
      //printf("DAVE LOADING NEXT MAP DONE\n");
    }

    // input
    if(IsKeyPressed(KEY_F) || go_fullscreen) { if((fullscreen = !fullscreen)) { stored_window_position = GetWindowPosition(); stored_window_size = (Vector2){GetScreenWidth(),GetScreenHeight()}; SetWindowState(FLAG_WINDOW_UNDECORATED); SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())); } else { ClearWindowState(FLAG_WINDOW_UNDECORATED); SetWindowPosition(stored_window_position.x, stored_window_position.y); SetWindowSize(stored_window_size.x, stored_window_size.y); } } go_fullscreen = false;
    running &= !IsKeyPressed(KEY_ESCAPE);
    
    // walking
    struct axis axis = {0, 0};
    if(vk_key(DOWN)) axis.ly = fmin(1, axis.ly + 1);
    if(vk_key(UP)) axis.ly = fmax(-1, axis.ly - 1);
    if(vk_key(RIGHT)) axis.lx = fmin(1, axis.lx + 1);
    if(vk_key(LEFT)) axis.lx = fmax(-1, axis.lx - 1);
    if(axis.lx != 0 || axis.ly != 0) {
      // up/down
      if(fabs(axis.ly) > fabs(axis.lx)) {
        facing_index = (axis.ly < 0)? 2 : 0;
        // double up number of animation frame by mirroring half the time
        facing_mirror = (tick - walking_t0) % (walking_period * 2) < walking_period;
        forward.y = py + collision.y + ((axis.ly < 0)? -forward.h : collision.h);
        forward.x = px + collision.x - (forward.w - collision.w) / 2;
      }
      // left/right
      else {
        facing_index = 1;
        facing_mirror = axis.lx < 0; // left is right mirrored
        forward.y = py + collision.y - (forward.h - collision.h) / 2;
        forward.x = px + collision.x + ((axis.lx < 0)? -forward.w: collision.w);
      }
      // two-frame animation
      facing_frame = ((tick - walking_t0) % walking_period < walking_period/2)? 1 : 0;
    } else {
      facing_frame = 0;
      walking_t0 = tick;
    }
    // collision
    {
      // tentative new position
      double nx = px + delta_time * step_per_seconds * axis.lx;
      double ny = py + delta_time * step_per_seconds * axis.ly;
      // test dimensions separately to allow sliding
      // simply test the corners, and assume speed is low so I don't need collision response
      bool blocked_x = false;
      bool break_x = false;
      bool test_npc = npc_id && dict_get(&npc_res, npc_id);
      if(warp_map && collides_2D_dx(nx + collision.x, py + collision.y, collision.w, collision.h, &warp)) { break_x = true; next_map = warp_map; warping = true; }
      if(!break_x && item_id) blocked_x = collides_2D_dx(nx + collision.x, py + collision.y, collision.w, collision.h, &item);
      if(test_npc && !break_x && !blocked_x) blocked_x = collides_2D_dx(nx + collision.x, py + collision.y, collision.w, collision.h, &npc);
      for(int i = 0, x = nx + collision.x; !break_x && !blocked_x && i < 2; i++, x += collision.w) {
        for(int j = 0, y = py + collision.y; !break_x && !blocked_x && j < 2; j++, y += collision.h) {
          if(x < 0 && map->west) { break_x = true; next_map = map->west; nx += MAP_COL * TS - collision.w; }
          else if(x >= MAP_COL * TS && map->east) { break_x = true; next_map = map->east; nx -= MAP_COL * TS - collision.w; }
          else {
            blocked_x |= y < 0 || y >= MAP_ROW * TS || x < 0 || x >= MAP_COL * TS;
            int col = (int)(x / TS);
            int row = (int)(y / TS);
            for(int k = 0; !blocked_x && k < layers_size; k++) {
              int tile = layers[k][row][col] - 1;
              blocked_x |= dict_get(&blocking_tiles, tile);
            }
          }
        }
      }
      bool blocked_y = false;
      bool break_y = false;
      if(warp_map && collides_2D_dx(px + collision.x, ny + collision.y, collision.w, collision.h, &warp)) { break_y = true; next_map = warp_map; warping = true; }
      if(!break_y && item_id) blocked_y = collides_2D_dx(px + collision.x, ny + collision.y, collision.w, collision.h, &item);
      if(test_npc && !break_y && !blocked_y) blocked_y = collides_2D_dx(px + collision.x, ny + collision.y, collision.w, collision.h, &npc);
      for(int i = 0, x = px + collision.x; !break_y && !blocked_y && i < 2; i++, x += collision.w) {
        for(int j = 0, y = ny + collision.y; !break_y && !blocked_y && j < 2; j++, y += collision.h) {
          if(y < 0 && map->north) { break_y = true; next_map = map->north; ny += MAP_ROW * TS - collision.h; }
          else if(y >= MAP_ROW * TS && map->south) { break_y = true; next_map = map->south; ny -= MAP_ROW * TS - collision.h; }
          else {
            blocked_y |= y < 0 || y >= MAP_ROW * TS || x < 0 || x >= MAP_COL * TS;
            int col = (int)(x / TS);
            int row = (int)(y / TS);
            for(int k = 0; !blocked_y && k < layers_size; k++) {
              int tile = layers[k][row][col] - 1;
              blocked_y |= dict_get(&blocking_tiles, tile);
            }
          }
        }
      }
      if(!blocked_x) px = nx;
      if(!blocked_y) py = ny;
    }
    // action button (activate stuff forward, dismiss message box)
    if(vk_key_released(ACTION)) {
      // dismiss dialog
      if(message) {
        message = NULL;
        //dprintf(snd, "channel stop 0\n");
        SetMusicVolume(bg, bg_volume);
      }
      // pickup items
      else if(item_id && collides_2D(&forward, &item)) {
        if(item_id != (Texture2D *)dict_get(&items, "water") || (held_item && held_item == (Texture2D *)dict_get(&items, "bottle"))) {
          held_item = item_id;
          item_id = NULL;
          dict_set(&ignore, held_item, true);
          if(held_item == (Texture2D *)dict_get(&items, "heart")) {
            winner_t0 = tick;
          }
        }
      }
      // npc interaction
      else if(npc_id && collides_2D(&forward, &npc)) {
        int state = dict_get(&npc_state, npc_id);
        if(strcmp(npc_id, "elf") == 0) {
          if(state == 0) {
            message = "I'm hungry. I want candy.";
            PlaySound(snd_elf_0);
            dict_set(&npc_state, "elf", 1);
          } else {
            if(held_item && held_item == (Texture2D *)dict_get(&items, "cane")) {
              message = "A candy cane! Thank you so much. You may pass.";
              PlaySound(snd_elf_2);
              held_item = NULL;
              dict_set(&ignore, "elf", true); free(npc_id); npc_id = NULL;
              dict_set(&npc_state, "elf", 2);
            } else {
              message = "I'm so hungry. I really want candy!";
              PlaySound(snd_elf_1);
            }
          }
        } else if(strcmp(npc_id, "bottle") == 0) {
          if(state == 0) {
            if(held_item && held_item == (Texture2D *)dict_get(&items, "key")) {
              message = "You open the chest with the key, and find an empty bottle.";
              PlaySound(snd_open);
              held_item = dict_get(&items, npc_id);
              dict_set(&npc_res, "bottle", &texture_chest_2);
              dict_set(&npc_state, "bottle", 1);
            } else {
              message = "The chest is locked.";
              PlaySound(snd_locked);
            }
          } else if(state == 1) {
            message = "The chest is empty.";
            PlaySound(snd_empty);
          }
        } else if(strcmp(npc_id, "flame") == 0) {
          if(state == 0) {
            if(held_item && held_item == (Texture2D *)dict_get(&items, "water")) {
              message = "You douse the flame with your water bottle, and find a magic staff.";
              PlaySound(snd_flame);
              held_item = dict_get(&items, "staff");
              dict_set(&ignore, "flame", true); free(npc_id); npc_id = NULL;
              dict_set(&npc_state, "flame", 1);
            }
          }
        } else if(strcmp(npc_id, "wizard") == 0) {
          if(state == 1 && held_item && held_item == (Texture2D *)dict_get(&items, "staff")) {
            message = "You found my staff. Thank you. Let me teach you the magic spell 'Kaboom'.";
            PlaySound(snd_wiz_1);
            dict_set(&npc_state, "wizard", 2);
            held_item = dict_get(&items, "spell");
          } else {
            if(state == 2) {
              message = "Thank you for returning my staff.";
              PlaySound(snd_wiz_2);
            } else {
              message = "I cannot find my magic staff. Will you help?";
              PlaySound(snd_wiz_0);
              if(state == 0) dict_set(&npc_state, "wizard", 1);
            }
          }
        } else if(strcmp(npc_id, "garden") == 0) {
          message = "This is princess Purple Dress's garden, and don't go pass it or eat the carrots please.";
          PlaySound(snd_garden);
        } else if(strcmp(npc_id, "dragon") == 0) {
          if(held_item && held_item == (Texture2D *)dict_get(&items, "spell")) {
            dict_set(&ignore, "dragon", true); free(npc_id);
            npc_id = strdup("kaboom");
            held_item = NULL;
          }
        }
      }
      SetMusicVolume(bg, .3);
    }

    //printf("DAVE draw %f\n", t);
    BeginTextureMode(framebuffer);
    ClearBackground(BLACK);
    // hud
    const int HUD_H = 3 * TS;
    // draw tilemap
    //printf("DAVE draw tilemap\n");
    for(int i = 0; i < layers_size; i++) {
      for(int row = 0; row < MAP_ROW; row++) {
        for(int col = 0; col < MAP_COL; col++) {
          int tile = layers[i][row][col];
          if(tile != 0) {
            tile = tile - 1;
            // handle animated tiles
            struct tile_animation * anim = dict_get(&animated_tiles, tile);
            if(anim) {
              uint64_t t = tick % anim->total_duration;
              for(int i = 0; i < anim->size; i++) {
                if(t < anim->durations[i]) { tile = anim->ids[i]; break; }
                t-= anim->durations[i];
              }
            }
            int x = TS * col;
            int y = TS * row + HUD_H;
            const int margin = 1;
            int tx = margin + (TS + 2 * margin) * (tile % tileset_columns);
            int ty = margin + (TS + 2 * margin) * (tile / tileset_columns);
            DrawTextureRec(texture_map, (Rectangle){tx,ty,TS,TS}, (Vector2){x,y}, WHITE);
          }
        }
      }
    }
    // draw item
    if(item_id && item_id != (Texture2D *)dict_get(&items, "water")) {
      //printf("DAVE draw item\n");
      DrawTexture(*item_id, item.x, item.y + HUD_H, WHITE);
    }
    if(held_item) {
      //printf("DAVE draw held item\n");
      DrawTexture(*held_item, (W - TS) / 2.0, HUD_H / 2.0 - TS, WHITE);
    }
    // draw npc
    if(npc_id) {
      //printf("DAVE draw npc\n");
      Texture2D * res = dict_get(&npc_res, npc_id);
      if(res) {
        // case flame animation
        if(strcmp(npc_id, "flame") == 0) {
          uint64_t flame_period = 400;
          res = &res[(int)((tick % flame_period) / (double)flame_period) * 8];
        }
        double w = npc.w;
        double h = npc.h;
        double x = npc.x;
        double y = npc.y;
        // case dragon dimensions are his patrol region, not draw size, and neither is drawn position
        if(strcmp(npc_id, "dragon") == 0 || strcmp(npc_id, "kaboom") == 0) {
          w = h = 2 * TS;
          x = fmin(fmax(npc.x, px), npc.x + npc.w - w);
          y = npc.y + npc.h - h;
        }
        if(strcmp(npc_id, "kaboom") == 0) {
          if(kaboom_t0 == -1) {
            kaboom_t0 = tick;
          }
          uint64_t kaboom_duration = 1000;
          if(tick >= kaboom_t0 + kaboom_duration) {
            free(npc_id); npc_id = NULL;
          } else {
            double sx = (int)((tick - kaboom_t0) / (double)kaboom_duration * 5) * 16;
            double sy = 0;
            DrawTexturePro(*res, (Rectangle){sx,sy,16,16}, (Rectangle){x, y + HUD_H, w, h}, (Vector2){0,0}, 0, WHITE);  
          }
        } else {
          DrawTexturePro(*res, (Rectangle){0,0,w,h}, (Rectangle){x, y + HUD_H, w, h}, (Vector2){0,0}, 0, WHITE);  
        }
      }
    }
    // draw player
    //printf("DAVE draw player\n");
    DrawTexturePro(texture_princess, (Rectangle){1 + facing_frame * (14 + 2), 1 + facing_index * (24 + 2),facing_mirror?-14:14,24}, (Rectangle){px, py + HUD_H, 14, 24}, (Vector2){0,0}, 0, WHITE);  
    
    // message box
    if(message) {
      //printf("DAVE draw message\n");
      double w = W * .8;
      double h = (H - HUD_H) * .3;
      int n = h / 10;
      double x = (W - w) / 2;
      double y = (H - HUD_H - h) / 2 + HUD_H;
      //dprintf(gfx, "fill 88888888 %f %f %f %f\n", x, y, w, h);
      //dprintf(gfx, "text DejaVuSans-Bold.ttf %f %f %f %f center left %d noclip 0 ffffff 000000 .2 %s\n", x, y, w, h, n, message);
    }

    // winner animation
    if(winner_t0 != -1) {
      //printf("DAVE draw winner\n");
      double t = (tick - winner_t0) / 1000.0;
      t = bound_cyclic_back_and_forth_normalized(t);
      double cy = (H - HUD_H - TS) / 2 + HUD_H;
      double cx = (W - TS) / 2;
      double hw = W / 2;
      double hh = (H - HUD_H) / 2;
      for(double theta = 0; theta < 2 * M_PI; theta += M_PI / 5) {
        //dprintf(gfx, "draw %s %f %f\n", held_item, cx + hw * cos(theta) * t, cy + hh * sin(theta) * t);
      }
    }

    // fps
    { char tmp_buff[256]; snprintf(tmp_buff, sizeof(tmp_buff), "ms:%d", (int)(delta_time * 1000)); DrawTextEx(font, tmp_buff, (Vector2){1,0}, 16, 1, WHITE); }

    // flush
    EndMode2D();
    EndTextureMode();
    BeginDrawing();
    double scale, x, y; center_fit(GetScreenWidth(), GetScreenHeight(), W, H, &scale, &x, &y);
    if(x || y) { ClearBackground(BLACK); }
    DrawTexturePro(framebuffer.texture, (Rectangle){0,0,W,-H}, (Rectangle){x,y,W*scale,H*scale}, (Vector2){0,0}, 0, WHITE);
    EndDrawing();
  }

  // cleanup
  UnloadFont(font);
  UnloadMusicStream(bg);
  UnloadSound(snd_elf_0);
  UnloadSound(snd_elf_2);
  UnloadSound(snd_elf_1);
  UnloadSound(snd_open);
  UnloadSound(snd_locked);
  UnloadSound(snd_empty);
  UnloadSound(snd_flame);
  UnloadSound(snd_wiz_1);
  UnloadSound(snd_wiz_2);
  UnloadSound(snd_wiz_0);
  UnloadSound(snd_garden);

  CloseAudioDevice();
  CloseWindow();
  if(npc_id) free(npc_id);
  dict_free(&warps);
  dict_free(&npc_state);
  dict_free(&npc_res);
  dict_free(&ignore);
  dict_free(&items);
  dict_free(&blocking_tiles);
  dict_free(&animated_tiles);
  xmlFree(tileset_image);
  return EXIT_SUCCESS;
}
