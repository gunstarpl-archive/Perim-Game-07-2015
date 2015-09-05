#pragma once

#include "Precompiled.hpp"

//
// Context
//

namespace Lua
{
    // State class.
    class State
    {
    public:
        State();
        ~State();

        // Restores instance to it's original state.
        void Cleanup();

        // Initializes the Lua state.
        bool Initialize();

        // Loads a script file.
        bool Load(std::string filename);
        
        // Conversion operator.
        operator lua_State*();

    private:
        // Lua state.
        lua_State* m_state;

        // Initialization state.
        bool m_initialized;
    };
}
