#ifndef MAPBUILD_HPP
#define MAPBUILD_HPP

#include <vector>

#include "map_templates.hpp"

class Room;

struct Region
{
public:
    Region(const R& r) :
        main_room_  (nullptr),
        r_          (r),
        is_free_    (true) {}

    Region() :
        main_room_  (nullptr),
        r_          (),
        is_free_    (true) {}

    R rnd_room_rect() const;

    Room* main_room_;
    R r_;
    bool is_free_;
};

namespace mapgen
{

//This variable is checked at certain points to see if the current map
//has been flagged as "failed". Setting is_map_valid to false will generally
//stop map generation, discard the map, and trigger generation of a new map.
extern bool is_map_valid;

//All cells marked as true in this array will be considered for door placement
extern bool door_proposals[map_w][map_h];

//------------------------------------------------------------------------------
//Generate maps
//------------------------------------------------------------------------------
//Standard dungeon level
bool mk_std_lvl();

//"Special" levels
bool mk_intro_lvl();
bool mk_egypt_lvl();
bool mk_leng_lvl();
bool mk_rats_in_the_walls_lvl();
bool mk_trapez_lvl();
bool mk_boss_lvl();

//------------------------------------------------------------------------------
//Map generation steps (in no particular order)
//------------------------------------------------------------------------------
void mk_merged_regions_and_rooms(Region regions[3][3]);

void randomly_block_regions(Region regions[3][3]);

void mk_aux_rooms(Region regions[3][3]);

void reserve_river(Region regions[3][3]);

void mk_sub_rooms();

void decorate();

//------------------------------------------------------------------------------
//Room reshaping utils (called by the room objects)
//------------------------------------------------------------------------------
//NOTE: Some reshape functions below will not change the boundaries of the room,
//but may affect which cells belong to the room. This only affects the room map
//(in the "map" namespace), so the room parameter should be a const reference.
//For other reshape functions, the room may expand beyond its initial rectangle,
//so in those cases the functions need to modify the data of the room object.
void cut_room_corners(const Room& room);
void mk_pillars_in_room(const Room& room);
void cavify_room(Room& room);

//------------------------------------------------------------------------------
//Misc utils
//------------------------------------------------------------------------------
bool is_all_rooms_connected();

void register_room(Room& room);

void mk_floor_in_room(const Room& room);

void valid_room_corr_entries(const Room& room, std::vector<P>& out);

void mk_pathfind_cor(Room& r0, Room& r1,
                      bool door_proposals[map_w][map_h] = nullptr);

void rnd_walk(const P& p0,
              int len,
              std::vector<P>& pos_list_ref,
              const bool allow_diagonal = true,
              R area = R(1, 1, map_w - 2, map_h - 2));

void pathfinder_walk(const P& p0,
                     const P& p1,
                     std::vector<P>& pos_list_ref,
                     const bool is_smooth);

} //mapgen

#endif
