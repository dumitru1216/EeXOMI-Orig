#pragma once

class IPrediction
{
public:
  void Update( int startframe, bool validframe, int incoming_acknowledged, int outgoing_command );
  void SetupMove( C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move );
  void FinishMove( C_BasePlayer* player, CUserCmd* ucmd, CMoveData* move );
  int CheckMovingGround( C_BasePlayer * player, double unk );
};
