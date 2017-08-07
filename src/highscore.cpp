#include "highscore.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include "init.hpp"
#include "highscore.hpp"
#include "actor_player.hpp"
#include "game.hpp"
#include "map.hpp"
#include "popup.hpp"
#include "io.hpp"

// -----------------------------------------------------------------------------
// Highscore entry
// -----------------------------------------------------------------------------
HighscoreEntry::HighscoreEntry(std::string game_summary_file_path,
                               std::string entry_date_and_time,
                               std::string player_name,
                               int player_xp,
                               int player_lvl,
                               int player_dlvl,
                               int player_insanity,
                               IsWin is_win,
                               Bg player_bg) :
    game_summary_file_path_ (game_summary_file_path),
    date_and_time_          (entry_date_and_time),
    name_                   (player_name),
    xp_                     (player_xp),
    lvl_                    (player_lvl),
    dlvl_                   (player_dlvl),
    ins_                    (player_insanity),
    is_win_                 (is_win),
    bg_                     (player_bg) {}

HighscoreEntry::~HighscoreEntry() {}

int HighscoreEntry::score() const
{
    const double dlvl_db = double(dlvl_);
    const double dlvl_last_db = double(dlvl_last);
    const double xp_db = double(xp_);

    const bool win = is_win_ == IsWin::yes;

    const double xp_factor = 1.0 + xp_db + (win ? (xp_db / 5.0) : 0.0);
    const double dlvl_factor = 1.0 + (dlvl_db / dlvl_last_db);

    return (int)(xp_factor * dlvl_factor);
}

// -----------------------------------------------------------------------------
// Highscore
// -----------------------------------------------------------------------------
namespace highscore
{

namespace
{

void sort_entries(std::vector<HighscoreEntry>& entries)
{
    auto cmp = [](const HighscoreEntry & e1, const HighscoreEntry & e2)
    {
        return e1.score() > e2.score();
    };

    sort(entries.begin(), entries.end(), cmp);
}

void write_file(std::vector<HighscoreEntry>& entries)
{
    std::ofstream file;

    file.open("res/data/highscores", std::ios::trunc);

    for (const auto entry : entries)
    {
        const std::string win_str = (entry.is_win() == IsWin::yes) ? "1" : "0";

        file << entry.game_summary_file_path() << std::endl;
        file << win_str << std::endl;
        file << entry.date_and_time() << std::endl;
        file << entry.name() << std::endl;
        file << entry.xp() << std::endl;
        file << entry.lvl() << std::endl;
        file << entry.dlvl() << std::endl;
        file << entry.ins() << std::endl;
        file << (int)entry.bg() << std::endl;
    }
}

std::vector<HighscoreEntry> read_file()
{
    TRACE_FUNC_BEGIN;

    std::vector<HighscoreEntry> entries;

    std::ifstream file;

    file.open("res/data/highscores");

    if (!file.is_open())
    {
        return entries;
    }

    std::string line = "";

    while (getline(file, line))
    {
        const std::string game_summary_file = line;

        getline(file, line);
        IsWin is_win =
            (line[0] == '1') ?
            IsWin::yes :
            IsWin::no;

        getline(file, line);
        const std::string date_and_time = line;

        getline(file, line);
        const std::string name = line;

        getline(file, line);
        const int xp = to_int(line);

        getline(file, line);
        const int lvl = to_int(line);

        getline(file, line);
        const int dlvl = to_int(line);

        getline(file, line);
        const int ins = to_int(line);

        getline(file, line);
        Bg bg = (Bg)to_int(line);

        entries.push_back(
            HighscoreEntry(game_summary_file,
                           date_and_time,
                           name,
                           xp,
                           lvl,
                           dlvl,
                           ins,
                           is_win,
                           bg));
    }

    file.close();

    TRACE_FUNC_END;

    return entries;
}

} // namespace

void init()
{

}

void cleanup()
{

}

HighscoreEntry mk_entry_from_current_game_data(
    const std::string game_summary_file_path,
    const IsWin is_win)
{
    const auto date = current_time().time_str(TimeType::day, true);

    HighscoreEntry entry(
        game_summary_file_path,
        date,
        map::player->name_a(),
        game::xp_accumulated(),
        game::clvl(),
        map::dlvl,
        map::player->ins(),
        is_win,
        player_bon::bg());

    return entry;
}

 void append_entry_to_highscores_file(const HighscoreEntry& entry)
{
    TRACE_FUNC_BEGIN;

    std::vector<HighscoreEntry> entries = entries_sorted();

    entries.push_back(entry);

    sort_entries(entries);

    write_file(entries);

    TRACE_FUNC_END;
}

std::vector<HighscoreEntry> entries_sorted()
{
    auto entries = read_file();

    if (!entries.empty())
    {
        sort_entries(entries);
    }

    return entries;
}

} // highscore

// -----------------------------------------------------------------------------
// Browse highscore
// -----------------------------------------------------------------------------
namespace
{

const int top_more_y_ = 1;
const int btm_more_y_ = screen_h - 1;

const int entries_y0_ = top_more_y_ + 1;
const int entries_y1_ = btm_more_y_ - 1;

const int entries_h_ = entries_y1_ - entries_y0_ + 1;

} // namespace

BrowseHighscore::BrowseHighscore() :
        State       (),
        entries_    (),
        browser_    () {}

StateId BrowseHighscore::id()
{
    return StateId::highscore;
}

void BrowseHighscore::on_start()
{
    entries_ = highscore::read_file();

    highscore::sort_entries(entries_);

    browser_.reset(entries_.size(), entries_h_);
}

void BrowseHighscore::draw()
{
    if (entries_.empty())
    {
        return;
    }

    const Panel panel = Panel::screen;

    const std::string title =
        "Browsing high scores [select] to view game summary";

    io::draw_text_center(title,
                         panel,
                         P(map_w_half, 0),
                         clr_title,
                         clr_black,
                         true);

    const Clr& label_clr = clr_white;

    const int labels_y = 1;

    const int x_date = 0;
    const int x_name = x_date + 12;
    const int x_bg = player_name_max_len + 14;
    const int x_lvl = x_bg + 13;
    const int x_dlvl = x_lvl + 7;
    const int x_ins = x_dlvl + 7;
    const int x_win = x_ins + 10;
    const int x_score = x_win + 5;

    const std::vector< std::pair<std::string, int> > labels
    {
        {"Level", x_lvl},
        {"Depth", x_dlvl},
        {"Insanity", x_ins},
        {"Win", x_win},
        {"Score", x_score}
    };

    for (const auto& label : labels)
    {
        io::draw_text(label.first,
                      panel,
                      P(label.second, labels_y),
                      label_clr);
    }

    const int browser_y = browser_.y();

    int y = entries_y0_;

    const Range idx_range_shown = browser_.range_shown();

    for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
    {
        const auto& entry = entries_[i];

        const std::string date_and_time = entry.date_and_time();
        const std::string name = entry.name();
        const std::string bg = player_bon::bg_title(entry.bg());
        const std::string lvl = std::to_string(entry.lvl());
        const std::string dlvl = std::to_string(entry.dlvl());
        const std::string ins = std::to_string(entry.ins());
        const std::string win = (entry.is_win() == IsWin::yes) ? "Yes" : "No";
        const std::string score = std::to_string(entry.score());

        const bool is_idx_marked = browser_y == i;

        const Clr& clr =
            is_idx_marked ?
            clr_menu_highlight:
            clr_menu_drk;

        io::draw_text(date_and_time, panel, P(x_date, y), clr);
        io::draw_text(name, panel, P(x_name, y), clr);
        io::draw_text(bg, panel, P(x_bg, y), clr);
        io::draw_text(lvl, panel, P(x_lvl, y), clr);
        io::draw_text(dlvl, panel, P(x_dlvl, y), clr);
        io::draw_text(ins + "%", panel, P(x_ins, y), clr);
        io::draw_text(win, panel, P(x_win, y), clr);
        io::draw_text(score, panel, P(x_score, y), clr);

        ++y;
    }

    // Draw "more" labels
    if (!browser_.is_on_top_page())
    {
        io::draw_text("(More - Page Up)",
                      Panel::screen,
                      P(0, top_more_y_),
                      clr_white_lgt);
    }

    if (!browser_.is_on_btm_page())
    {
        io::draw_text("(More - Page Down)",
                      Panel::screen,
                      P(0, btm_more_y_),
                      clr_white_lgt);
    }
}

void BrowseHighscore::update()
{
    if (entries_.empty())
    {
        popup::show_msg("No high score entries found.");

        //
        // Exit screen
        //
        states::pop();

        return;
    }

    const auto input = io::get(false);

    const MenuAction action =
        browser_.read(input,
                      MenuInputMode::scrolling);

    switch (action)
    {
    case MenuAction::selected:
    case MenuAction::selected_shift:
    {
        const int browser_y = browser_.y();

        ASSERT(browser_y < (int)entries_.size());

        const auto& entry_marked = entries_[browser_y];

        const std::string file_path = entry_marked.game_summary_file_path();

        auto state = std::unique_ptr<BrowseHighscoreEntry>(
            new BrowseHighscoreEntry(file_path));

        states::push(std::move(state));
    }
    break;

    case MenuAction::space:
    case MenuAction::esc:
    {
        //
        // Exit screen
        //
        states::pop();
    }
    break;

    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Browse highscore entry game summary file
// -----------------------------------------------------------------------------
namespace
{

const int max_nr_lines_on_scr_ = screen_h - 2;

} // namespace

BrowseHighscoreEntry::BrowseHighscoreEntry(
    const std::string& file_path) :
        State       (),
        file_path_  (file_path),
        lines_      (),
        top_idx_    (0) {}

StateId BrowseHighscoreEntry::id()
{
    return StateId::browse_highscore_entry;
}

void BrowseHighscoreEntry::on_start()
{
    read_file();
}

void BrowseHighscoreEntry::draw()
{
    io::draw_info_scr_interface("Game summary",
                                InfScreenType::scrolling);

    const int nr_lines_tot = lines_.size();

    int btm_nr =
        std::min(top_idx_ + max_nr_lines_on_scr_ - 1,
                 nr_lines_tot - 1);

    int screen_y = 1;

    for (int i = top_idx_; i <= btm_nr; ++i)
    {
        io::draw_text(lines_[i],
                      Panel::screen,
                      P(0, screen_y),
                      clr_text);

        ++screen_y;
    }
}

void BrowseHighscoreEntry::update()
{
    const int line_jump = 3;

    const int nr_lines_tot = lines_.size();

    const auto input = io::get(false);

    switch (input.key)
    {
    case '2':
    case SDLK_DOWN:
    case 'j':
        top_idx_ += line_jump;

        if (nr_lines_tot <= max_nr_lines_on_scr_)
        {
            top_idx_ = 0;
        }
        else
        {
            top_idx_ = std::min(nr_lines_tot - max_nr_lines_on_scr_, top_idx_);
        }
        break;

    case '8':
    case SDLK_UP:
    case 'k':
        top_idx_ = std::max(0, top_idx_ - line_jump);
        break;

    case SDLK_SPACE:
    case SDLK_ESCAPE:
        //
        // Exit screen
        //
        states::pop();
        break;

    default:
        break;
    }
}

void BrowseHighscoreEntry::read_file()
{
    lines_.clear();

    std::ifstream file(file_path_);

    if (!file.is_open())
    {
        popup::show_msg("Path: \"" + file_path_ + "\"",
                        "Game summary file could not be opened",
                        SfxId::END,
                        20);

        states::pop();

        return;
    }

    std::string current_line;

    std::vector<std::string> formatted;

    while (getline(file, current_line))
    {
        lines_.push_back(current_line);
    }

    file.close();
}
