#pragma once

//class CClientState
//{
//public:
//  int& m_nDeltaTick( );
//  int& m_nLastOutgoingCommand( );
//  int& m_nChokedCommands( );
//  int& m_nLastCommandAck( );
//  bool& m_bIsHLTV( );
//};

class CEventInfo;
class INetChannel;

class CClientState {
public:
	char			pad0[ 156u ]{};
	INetChannel* net_channel{};
	int& m_nDeltaTick( );
	int& m_nLastOutgoingCommand( );
	int& m_nChokedCommands( );
	int& m_nLastCommandAck( );
	bool& m_bIsHLTV( );
};
