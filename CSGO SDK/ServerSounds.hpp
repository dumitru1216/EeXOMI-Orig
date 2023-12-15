#pragma once
#include "player.hpp"
#include <deque>

class C_Sound {
public:
   int guid;
   int tickcount;
   const char* name;

   Vector origin;
   Vector direction;

   StartSoundParams_t params;
};

class C_PlayerSounds {
public:
   std::deque<C_Sound> m_Sounds;
};

class C_ServerSounds {
private:
   class RestoreSounds {
   public:
      Vector m_vecOrigin = Vector( 0, 0, 0 );
      Vector m_vecAbsOrigin = Vector( 0, 0, 0 );
      int m_index = 0;
      bool m_bDormant = false;
   };
public:
   void ProcessSound( int guid, StartSoundParams_t& params, char const* soundname );
   void Start( );
   void Finish( );
   std::map<int, C_PlayerSounds> m_Players;
   std::deque< RestoreSounds > m_RestorePlayers;
};

extern C_ServerSounds g_ServerSounds;