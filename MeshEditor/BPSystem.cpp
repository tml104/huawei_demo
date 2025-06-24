#include "StdAfx.h"
#include "BPSystem.h"

void BPSystem::BPSystem::InitPlay()
{
	LOG_INFO("Start");

	auto init_params = [](NodeParamsBase* params, int count) {
		params->ptrs.clear();
		params->ptrs.resize(count);
	};

	auto init_input_output_params = [&](ull node_instance_id, NodeParamsBase* input_params, NodeParamsBase* output_params) {
		NodeInstance node_instance = nodeInstaceMap[node_instance_id];
		init_params(input_params, node_instance.inputPinsInstaceId.size());
		init_params(output_params, node_instance.outputPinsInstaceId.size());
	};

	for (auto it = nodeInstaceMap.begin(); it != nodeInstaceMap.end(); it++) {

		ull instance_id = it->first;
		
		ull class_id = it->second.classId;

		NodeParamsBase* input_params = new NodeParamsBase();
		inputNodeParamsMap[instance_id] = input_params;

		NodeParamsBase* output_params = new NodeParamsBase();
		outputNodeParamsMap[instance_id] = output_params;

		init_input_output_params(instance_id, input_params, output_params);

		if (class_id == 3) {
			StartLoadNodeExecute* node_execute = new StartLoadNodeExecute();
			nodeExecuteMap[instance_id] = node_execute;

			LOG_INFO("Init for class_id: %llu", class_id);
		}
		else if (class_id == 8) {
			LoadEntityNodeExecute* node_execute = new LoadEntityNodeExecute();
			nodeExecuteMap[instance_id] = node_execute;

			LOG_INFO("Init for class_id: %llu", class_id);
		}
		else if (class_id == 9) {

			InitMarkNumNodeExecute* node_execute = new InitMarkNumNodeExecute();
			nodeExecuteMap[instance_id] = node_execute;

			LOG_INFO("Init for class_id: %llu", class_id);
		}
		else if (class_id == 10) {
			ExportEachBodyNodeExecute* node_execute = new ExportEachBodyNodeExecute();
			nodeExecuteMap[instance_id] = node_execute;

			LOG_INFO("Init for class_id: %llu", class_id);
		}
		else if (class_id == 10002) {
			StringNodeExecute* node_execute = new StringNodeExecute();
			nodeExecuteMap[instance_id] = node_execute;

			// TEMP
			node_execute->Run(input_params, output_params);

			LOG_INFO("Init for class_id: %llu", class_id);
		}
	}

	LOG_INFO("inputNodeParamsMap debug info start");
	for (auto it = inputNodeParamsMap.begin(); it != inputNodeParamsMap.end(); it++) {
		LOG_INFO("%llu, ptrs_size: %d", it->first, static_cast<int>(it->second->ptrs.size()));
	}
	LOG_INFO("inputNodeParamsMap debug info end");

	LOG_INFO("outputNodeParamsMap debug info start");
	for (auto it = outputNodeParamsMap.begin(); it != outputNodeParamsMap.end(); it++) {
		LOG_INFO("%llu, ptrs_size: %d", it->first, static_cast<int>(it->second->ptrs.size()));
	}
	LOG_INFO("outputNodeParamsMap debug info end");

	LOG_INFO("nodeExecuteMap debug info start");
	for (auto it = nodeExecuteMap.begin(); it != nodeExecuteMap.end(); it++) {
		LOG_INFO("%llu", it->first);
	}
	LOG_INFO("nodeExecuteMap debug info end");

	LOG_INFO("End");
}

void BPSystem::BPSystem::ParseClasses(Json::Value j)
{
	Json::Value node_classes_json = j["NodeClasses"];

	for (int i = 0; i < node_classes_json.size(); i++) {

		auto node_class_json = node_classes_json[i];

		NodeClass node_class(node_class_json);

		nodeClassMap[node_class.classId] = node_class;

		//LOG_INFO("node_class: %u ")
	}

	Json::Value pin_classes_json = j["PinClasses"];
	for (int i = 0; i < pin_classes_json.size(); i++) {

		auto pin_class_json = pin_classes_json[i];

		PinClass pin_class(pin_class_json);

		pinClassMap[pin_class.classId] = pin_class;
	}
}

void BPSystem::BPSystem::ParseInstances(Json::Value j)
{
	Json::Value node_instances_json = j["NodeInstances"];
	Json::Value pin_instances_json = j["PinInstances"];
	Json::Value links_json = j["Links"];

	for (int i = 0; i < node_instances_json.size(); i++) {
		auto node_instance_json = node_instances_json[i];
		NodeInstance node_instace(node_instance_json);

		nodeInstaceMap[node_instace.instanceId] = node_instace;
	}

	for (int i = 0; i < pin_instances_json.size(); i++) {
		auto pin_instance_json = pin_instances_json[i];
		//NodeInstance node_instace(node_instance_json);

		PinInstance pin_instance(pin_instance_json);

		pinInstaceMap[pin_instance.instanceId] = pin_instance;
	}

	for (int i = 0; i < links_json.size(); i++) {

		auto link_json = links_json[i];

		Link link(link_json);

		linkMap[link.linkId] = link;
	}

}

void BPSystem::BPSystem::PlayFrom(ull start_class_id)
{
	LOG_INFO("Start");

	std::queue<ull> q;

	// 1. 找所有和class_id一样的instance_id，放入队列

	for (auto it = nodeInstaceMap.begin(); it != nodeInstaceMap.end(); it++) {

		if (it->second.classId == start_class_id) {

			q.push(it->first); // instance_id

			LOG_INFO("q push: %llu %llu", it->first, start_class_id);

		}
	}

	auto find_vec_index = [](const std::vector<ull> vec, ull t) -> int {

		for (int i = 0; i < vec.size(); i++) {
			if (vec[i] == t) {
				return i;
			}
		}

		return static_cast<int>(vec.size());
	};


	while (!q.empty()) {

		ull front_instance_id = q.front();
		q.pop();

		NodeInstance node_instance = nodeInstaceMap[front_instance_id];
		NodeClass node_class = nodeClassMap[node_instance.classId];
		LOG_INFO("q pop: %llu %llu", front_instance_id, node_instance.classId);

		NodeParamsBase* input_params = inputNodeParamsMap[front_instance_id];
		NodeParamsBase* output_params = outputNodeParamsMap[front_instance_id];
		NodeExecuteBase* node_execute = nodeExecuteMap[front_instance_id];
		// 构造入参

		// 先读取入参pin instance，然后暴力寻找所有与这个pin相连的link以及输出的pin instance，然后获取那个pin的node instance id, 由此获得对应出参结构
		// TODO: 检查

		for (int i = 0; i < node_instance.inputPinsInstaceId.size(); i++) {
			ull pin_instance_id = node_instance.inputPinsInstaceId[i];

			// 遍历所有link去找

			std::vector<ull> out_pin_vec;

			for (auto it = linkMap.begin(); it != linkMap.end(); it++) {

				Link link = it->second;

				if (link.endPinInstanceId == pin_instance_id) {
					out_pin_vec.emplace_back(link.startPinInstanceId);
				}

			}

			if (!out_pin_vec.empty()) {
				// 取第一个
				ull out_pin = out_pin_vec.front();
				ull out_pin_node_instance_id = pinInstaceMap[out_pin].nodeInstanceId;

				// 取得对应参数结构
				NodeInstance out_node_instance = nodeInstaceMap[out_pin_node_instance_id];
				NodeParamsBase* out_node_params = outputNodeParamsMap[out_pin_node_instance_id];

				int param_index = find_vec_index(out_node_instance.outputPinsInstaceId, out_pin);

				void* param_ptr = out_node_params->ptrs[param_index];

				// 传指针引用
				input_params->ptrs[i] = param_ptr;
				
			}

		}

		// 构造出参

		// （先留空，目前感觉没啥能做的）

		// 执行
		if (node_execute) {
			node_execute->Run(input_params, output_params);
		}
		else {
			LOG_ERROR("node_execute is nullptr");
		}

		
		// 将返回值放回出参map：用指针的话已经自动做了

		// 将接下来要执行的节点放入队列中：
		// 先读取出参pins，暴力遍历所有link，然后将
		for (int i = 0; i < node_instance.outputPinsInstaceId.size(); i++) {
			ull pin_instance_id = node_instance.outputPinsInstaceId[i];

			PinInstance pin_instance = pinInstaceMap[pin_instance_id];

			if (pin_instance.classId == 2) {

				// 遍历所有links，找到与这个pin相连的下一个pin所属的node instance

				for (auto it = linkMap.begin(); it != linkMap.end(); it++) {

					Link link = it->second;

					if (link.startPinInstanceId == pin_instance_id) {
						PinInstance p2 = pinInstaceMap[link.endPinInstanceId];

						ull next_node_instance = p2.nodeInstanceId;
						q.push(next_node_instance);

						LOG_INFO("q push: %llu %llu", next_node_instance, nodeInstaceMap[next_node_instance].classId);

					}

				}

			}
		}



	}

	LOG_INFO("End");
}

BPSystem::BPSystem::BPSystem(std::string class_json_path, std::string instance_json_path)
{
	LOG_INFO("START");

	std::ifstream ifs(class_json_path);

	if (!ifs.is_open()) {
		LOG_ERROR("class_json_path open failed.");
		return;
	}

	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(ifs, root, false)) {
		LOG_ERROR("class_json_path parse failed.");
		return;
	}

	ParseClasses(root);

	ifs.close();
	ifs.open(instance_json_path);

	if (!ifs.is_open()) {
		LOG_ERROR("instance_json_path open failed.");
		return;
	}

	if (!reader.parse(ifs, root, false)) {
		LOG_ERROR("instance_json_path parse failed.");
		return;
	}

	ParseInstances(root);

	// [debug]
	LOG_INFO("nodeClassMap debug info start");
	for (auto it = nodeClassMap.begin(); it != nodeClassMap.end(); it++) {
		LOG_INFO("%llu %llu", it->first, it->second.classId);
	}
	LOG_INFO("nodeClassMap debug info end");


	LOG_INFO("pinClassMap debug info start");
	for (auto it = pinClassMap.begin(); it != pinClassMap.end(); it++) {
		LOG_INFO("%llu %llu", it->first, it->second.classId);
	}
	LOG_INFO("pinClassMap debug info end");

	LOG_INFO("nodeInstaceMap debug info start");
	for (auto it = nodeInstaceMap.begin(); it != nodeInstaceMap.end(); it++) {
		LOG_INFO("%llu %llu", it->first, it->second.classId);
	}
	LOG_INFO("nodeInstaceMap debug info end");

	LOG_INFO("pinInstaceMap debug info start");
	for (auto it = pinInstaceMap.begin(); it != pinInstaceMap.end(); it++) {
		LOG_INFO("%llu %llu", it->first, it->second.classId);
	}
	LOG_INFO("pinInstaceMap debug info end");

	LOG_INFO("Link debug info start");
	for (auto it = linkMap.begin(); it != linkMap.end(); it++) {
		LOG_INFO("%llu %llu", it->second.startPinInstanceId, it->second.endPinInstanceId);
	}

	LOG_INFO("Link debug info end");



	LOG_INFO("END");
}
