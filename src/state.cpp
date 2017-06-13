#include "state.hpp"

#include "io.hpp"
#include "rl_utils.hpp"

//-----------------------------------------------------------------------------
// State keeping
//-----------------------------------------------------------------------------
namespace states
{

namespace
{

std::vector< std::unique_ptr<State> > states_;

} // namespace

void init()
{
    TRACE_FUNC_BEGIN;

    cleanup();

    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;

    states_.resize(0);

    TRACE_FUNC_END;
}

void start()
{
    while (!states_.empty() &&
           !states_.back()->has_started())
    {
        auto& state = states_.back();

        state->set_started();

        // NOTE: This may cause states to be pushed/popped - do not use the
        //       "state" pointer beyond this call!
        state->on_start();
    }
}

void draw()
{
    if (states_.empty())
    {
        return;
    }

    // Find the first state from the end which is NOT drawn overlayed
    auto draw_from = end(states_);

    while (draw_from != begin(states_))
    {
        --draw_from;

        const auto& state_ptr = *draw_from;

        // If not drawn overlayed, draw from this state as bottom layer
        // (but only if the state has been started, see note below)
        if (!state_ptr->draw_overlayed() &&
            state_ptr->has_started())
        {
            break;
        }
    }

    // Draw every state from this (non-overlayed) state onward
    for (/* No declaration */ ; draw_from != end(states_); ++draw_from)
    {
        const auto& state_ptr = *draw_from;

        // Do NOT draw states which are not yet started (they may need to
        // set up menus etc in their start function, and expect the chance
        // to do so before drawing is called)

        if (state_ptr->has_started())
        {
            state_ptr->draw();
        }
    }
}

void update()
{
    if (states_.empty())
    {
        return;
    }

    states_.back()->update();
}

void push(std::unique_ptr<State> state)
{
    // Pause the current state
    if (!states_.empty())
    {
        states_.back()->on_pause();
    }

    states_.push_back(std::move(state));

    states_.back()->on_pushed();

    io::flush_input();
}

void pop()
{
    if (states_.empty())
    {
        return;
    }

    states_.back()->on_popped();

    states_.pop_back();

    if (!states_.empty())
    {
        states_.back()->on_resume();
    }

    io::flush_input();
}

void pop_all()
{
    TRACE_FUNC_BEGIN;

    while (!states_.empty())
    {
        states_.back()->on_popped();

        states_.pop_back();
    }

    TRACE_FUNC_END;
}

bool check_states(const states::StateId state_given, const states::StateId state_to_check)
{                               //This is from the vector, and this is to check for.
    if (state_to_check == states::StateId::DEFAULT_STATE)
        return false;

    //^If it should pop 'till default state, then stop without popping a single state.

    if (state_given == state_to_check)
        return false;

    //^If the states are the same, just stop.
    
    return true;
    //^Returning true as default.
}

bool contains_state(const states::StateId state)
{
    for( auto&& p : states_ ) {
        if(p->id() == state)
            return true;
    }

    return false;
}

void pop_until(const states::StateId state)
{
    if (is_empty())
        return;

    if (check_states(states_.back().get()->id(), state))
    {
        pop();
        pop_until(state);
    }
}

bool is_empty()
{
    return states_.empty();
}

bool is_current_state(const State& state)
{
    if (states_.empty())
    {
        return false;
    }

    return &state == states_.back().get();
}

} // states
