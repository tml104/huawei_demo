#include "StdAfx.h"
#include "LinkACIS.h"

BPSystem::Link::Link(Json::Value j)
{
	linkId = j["LinkId"].asUInt();
	startPinInstanceId = j["StartPinInstanceId"].asUInt();
	endPinInstanceId = j["EndPinInstanceId"].asUInt();
}
