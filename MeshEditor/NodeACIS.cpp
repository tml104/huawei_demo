#include "StdAfx.h"
#include "NodeACIS.h"

BPSystem::NodeClass::NodeClass(Json::Value j)
{
	classId = j["ClassId"].asUInt();	
	nodeType = j["NodeType"].asUInt();
	auto input_pin_class_list_json = j["InputPinsClassId"];
	auto output_pin_class_list_json = j["OutputPinsClassId"];

	for (int i = 0; i < input_pin_class_list_json.size(); i++) {
		ull pin_class_id = input_pin_class_list_json[i].asUInt();
		inputPinsClassId.push_back(pin_class_id);
	}

	for (int i = 0; i < output_pin_class_list_json.size(); i++) {
		ull pin_class_id = output_pin_class_list_json[i].asUInt();
		outputPinsClassId.push_back(pin_class_id);
	}

}

BPSystem::NodeInstance::NodeInstance(Json::Value j)
{
	instanceId = j["InstanceId"].asUInt();
	classId = j["ClassId"].asUInt();

	auto input_pin_instance_list_json = j["InputPinsInstanceId"];
	auto output_pin_instance_list_json = j["OutputPinsInstanceId"];

	for (int i = 0; i < input_pin_instance_list_json.size(); i++) {
		ull pin_instance_id = input_pin_instance_list_json[i].asUInt();
		inputPinsInstaceId.push_back(pin_instance_id);
	}


	for (int i = 0; i < output_pin_instance_list_json.size(); i++) {
		ull pin_instance_id = output_pin_instance_list_json[i].asUInt();
		outputPinsInstaceId.push_back(pin_instance_id);
	}

}
