#include "explosion.h"

#include "render.h"
#include "map.h"
#include "log.h"
#include "map_parsing.h"
#include "sdl_wrapper.h"
#include "line_calc.h"
#include "actor_player.h"
#include "utils.h"
#include "sdl_wrapper.h"
#include "player_bon.h"
#include "feature_rigid.h"
#include "feature_mob.h"

using namespace std;

namespace
{

void draw(const vector< vector<Pos> >& pos_lists, bool blocked[MAP_W][MAP_H],
          const Clr* const clr_override)
{
    Render::draw_map_and_interface();

    const Clr& clr_inner = clr_override ? *clr_override : clr_yellow;
    const Clr& clr_outer = clr_override ? *clr_override : clr_red_lgt;

    const bool IS_TILES     = Config::is_tiles_mode();
    const int NR_ANIM_STEPS = IS_TILES ? 2 : 1;

    bool is_any_cell_seen_by_player = false;

    for (int i_anim = 0; i_anim < NR_ANIM_STEPS; i_anim++)
    {

        const Tile_id tile = i_anim == 0 ? Tile_id::blast1 : Tile_id::blast2;

        const int NR_OUTER = pos_lists.size();
        for (int i_outer = 0; i_outer < NR_OUTER; i_outer++)
        {
            const Clr& clr = i_outer == NR_OUTER - 1 ? clr_outer : clr_inner;
            const vector<Pos>& inner = pos_lists[i_outer];
            for (const Pos& pos : inner)
            {
                if (Map::cells[pos.x][pos.y].is_seen_by_player && !blocked[pos.x][pos.y])
                {
                    is_any_cell_seen_by_player = true;
                    if (IS_TILES)
                    {
                        Render::draw_tile(tile, Panel::map, pos, clr, clr_black);
                    }
                    else
                    {
                        Render::draw_glyph('*', Panel::map, pos, clr, true, clr_black);
                    }
                }
            }
        }
        if (is_any_cell_seen_by_player)
        {
            Render::update_screen();
            Sdl_wrapper::sleep(Config::get_delay_explosion() / NR_ANIM_STEPS);
        }
    }
}

void get_area(const Pos& c, const int RADI, Rect& rect_ref)
{
    rect_ref = Rect(Pos(max(c.x - RADI, 1),         max(c.y - RADI, 1)),
                   Pos(min(c.x + RADI, MAP_W - 2), min(c.y + RADI, MAP_H - 2)));
}

void get_cells_reached(const Rect& area, const Pos& origin,
                     bool blocked[MAP_W][MAP_H],
                     vector< vector<Pos> >& pos_list_ref)
{
    vector<Pos> line;
    for (int y = area.p0.y; y <= area.p1.y; ++y)
    {
        for (int x = area.p0.x; x <= area.p1.x; ++x)
        {
            const Pos pos(x, y);
            const int DIST = Utils::king_dist(pos, origin);
            bool is_reached = true;
            if (DIST > 1)
            {
                Line_calc::calc_new_line(origin, pos, true, 999, false, line);
                for (Pos& pos_check_block : line)
                {
                    if (blocked[pos_check_block.x][pos_check_block.y])
                    {
                        is_reached = false;
                        break;
                    }
                }
            }
            if (is_reached)
            {
                if (int(pos_list_ref.size()) <= DIST) {pos_list_ref.resize(DIST + 1);}
                pos_list_ref[DIST].push_back(pos);
            }
        }
    }
}

} //namespace


namespace Explosion
{

void run_explosion_at(const Pos& origin, const Expl_type expl_type,
                    const Expl_src expl_src, const int RADI_CHANGE,
                    const Sfx_id sfx, Prop* const prop, const Clr* const clr_override)
{
    Rect area;
    const int RADI = EXPLOSION_STD_RADI + RADI_CHANGE;
    get_area(origin, RADI, area);

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_projectiles(), blocked);

    vector< vector<Pos> > pos_lists;
    get_cells_reached(area, origin, blocked, pos_lists);

    Snd_vol vol = expl_type == Expl_type::expl ? Snd_vol::high : Snd_vol::low;

    Snd snd("I hear an explosion!", sfx, Ignore_msg_if_origin_seen::yes, origin,
            nullptr, vol, Alerts_mon::yes);
    Snd_emit::emit_snd(snd);

    draw(pos_lists, blocked, clr_override);

    //Do damage, apply effect

    Actor* living_actors[MAP_W][MAP_H];
    vector<Actor*> corpses[MAP_W][MAP_H];

    for (int x = 0; x < MAP_W; ++x)
    {
        for (int y = 0; y < MAP_H; ++y)
        {
            living_actors[x][y] = nullptr;
            corpses[x][y].clear();
        }
    }

    for (Actor* actor : Game_time::actors_)
    {
        const Pos& pos = actor->pos;
        if (actor->is_alive())
        {
            living_actors[pos.x][pos.y] = actor;
        }
        else if (actor->is_corpse())
        {
            corpses[pos.x][pos.y].push_back(actor);
        }
    }

    const bool IS_DEM_EXP = Player_bon::traits[int(Trait::dem_expert)];

    const int NR_OUTER = pos_lists.size();
    for (int cur_radi = 0; cur_radi < NR_OUTER; cur_radi++)
    {
        const vector<Pos>& positions_at_cur_radi = pos_lists[cur_radi];

        for (const Pos& pos : positions_at_cur_radi)
        {

            Actor* living_actor          = living_actors[pos.x][pos.y];
            vector<Actor*> corpses_here  = corpses[pos.x][pos.y];

            if (expl_type == Expl_type::expl)
            {
                //Damage environment
                Cell& cell = Map::cells[pos.x][pos.y];
                cell.rigid->hit(Dmg_type::physical, Dmg_method::explosion, nullptr);

                const int ROLLS = EXPL_DMG_ROLLS - cur_radi;
                const int DMG   = Rnd::dice(ROLLS, EXPL_DMG_SIDES) + EXPL_DMG_PLUS;

                //Damage living actor
                if (living_actor)
                {
                    if (living_actor == Map::player)
                    {
                        Log::add_msg("I am hit by an explosion!", clr_msg_bad);
                    }
                    living_actor->hit(DMG, Dmg_type::physical);
                }
                //Damage dead actors
                for (Actor* corpse : corpses_here) {corpse->hit(DMG, Dmg_type::physical);}

                //Add smoke
                if (Rnd::fraction(6, 10)) {Game_time::add_mob(new Smoke(pos, Rnd::range(2, 4)));}
            }

            //Apply property
            if (prop)
            {
                bool should_apply_on_living_actor = living_actor;

                //Do not apply burning if actor is player with the demolition expert trait, and
                //intentionally throwing a Molotov
                if (
                    living_actor    == Map::player &&
                    prop->get_id()  == Prop_id::burning &&
                    IS_DEM_EXP                    &&
                    expl_src        == Expl_src::player_use_moltv_intended)
                {
                    should_apply_on_living_actor = false;
                }

                if (should_apply_on_living_actor)
                {
                    Prop_handler& prop_hlr = living_actor->get_prop_handler();
                    Prop* prop_cpy = prop_hlr.mk_prop(prop->get_id(), Prop_turns::specific,
                                                   prop->turns_left_);
                    prop_hlr.try_apply_prop(prop_cpy);
                }

                //If property is burning, also apply it to corpses and environment
                if (prop->get_id() == Prop_id::burning)
                {
                    Cell& cell = Map::cells[pos.x][pos.y];
                    cell.rigid->hit(Dmg_type::fire, Dmg_method::elemental, nullptr);

                    for (Actor* corpse : corpses_here)
                    {
                        Prop_handler& prop_hlr = corpse->get_prop_handler();
                        Prop* prop_cpy = prop_hlr.mk_prop(prop->get_id(), Prop_turns::specific,
                                                       prop->turns_left_);
                        prop_hlr.try_apply_prop(prop_cpy);
                    }
                }
            }
        }
    }

    Map::player->update_fov();
    Render::draw_map_and_interface();

    if (prop) {delete prop;}
}

void run_smoke_explosion_at(const Pos& origin/*, const int SMOKE_DURATION*/)
{
    Rect area;
    const int RADI = EXPLOSION_STD_RADI;
    get_area(origin, RADI, area);

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_projectiles(), blocked);

    vector< vector<Pos> > pos_lists;
    get_cells_reached(area, origin, blocked, pos_lists);

    //TODO: Sound message?
    Snd snd("", Sfx_id::END, Ignore_msg_if_origin_seen::yes, origin, nullptr,
            Snd_vol::low, Alerts_mon::yes);
    Snd_emit::emit_snd(snd);

    for (const vector<Pos>& inner : pos_lists)
    {
        for (const Pos& pos : inner)
        {
            if (!blocked[pos.x][pos.y])
            {
                Game_time::add_mob(new Smoke(pos, Rnd::range(25, 30)));
            }
        }
    }

    Map::player->update_fov();
    Render::draw_map_and_interface();
}

} //Explosion
