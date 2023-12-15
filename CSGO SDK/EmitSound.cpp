#include "hooked.hpp"
#include "FnvHash.hpp"
#include "prediction.hpp"

void __fastcall Hooked::hkEmitSound( IEngineSound* thisptr, uint32_t, void* filter, int ent_index, int channel, const char* sound_entry, unsigned int sound_entry_hash,
									 const char* sample, float volume, float attenuation, int seed, int flags, int pitch, const Vector* origin, const Vector* direction,
									 void* vec_origins, bool update_positions, float sound_time, int speaker_entity, int test ) {
   g_Vars.globals.szLastHookCalled = XorStr( "7" );

   auto& prediction = Engine::Prediction::Instance( );
   if ( prediction.InPrediction( ) ) {
	  flags |= 1 << 2;
	  goto end;
   }

   if ( strstr( sample, ( XorStr( "weapon" ) ) ) && ( strstr( sample, ( XorStr( "draw" ) ) ) || strstr( sample, ( XorStr( "deploy" ) ) ) ) ) {
	  static uint32_t prev_hash = 0;
	  const uint32_t hash = hash_32_fnv1a_const( sample );

	  if ( prev_hash == hash ) {
		 flags |= 1 << 2;
		 goto end;
	  }

	  prev_hash = hash;
   }

end:
   oEmitSound( thisptr, filter, ent_index, channel, sound_entry, sound_entry_hash, sample, volume, attenuation, seed, flags,
			   pitch, origin, direction, vec_origins, update_positions, sound_time, speaker_entity, test );
}