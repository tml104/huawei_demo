#include "StdAfx.h"
#include "PinACIS.h"

BPSystem::PinClass::PinClass(Json::Value j)
{
	classId = j["ClassId"].asUInt();
	pinType = j["PinType"].asUInt();
	pinKind = j["PinKind"].asUInt();
}

BPSystem::PinInstance::PinInstance(Json::Value j)
{
	instanceId = j["InstanceId"].asUInt();
	classId = j["ClassId"].asUInt();
	nodeInstanceId = j["NodeInstanceId"].asUInt();
}
