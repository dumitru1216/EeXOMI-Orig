#pragma once
#include "Protobuf\protobufs\cstrike15_usermessages.pb.h"
#include "Protobuf\protobufs\cstrike15_gcmessages.pb.h"
#include "Protobuf\protobufs\gcsystemmsgs.pb.h"
#include "Protobuf\protobufs\gcsdk_gcmessages.pb.h"
#include "Protobuf\protobufs\econ_gcmessages.pb.h"
#include "Protobuf\protobufs\base_gcmessages.pb.h"
#ifdef _DEBUG
#pragma comment(lib, "libprotobufd.lib")
#else
#pragma comment(lib, "libprotobuf_release.lib")
#endif



namespace Protobuf
{
	bool SendClientHello();

	bool SendClientReport(uint32_t account_id, uint64_t match_id);

	bool SendMatchmakingHello();
}