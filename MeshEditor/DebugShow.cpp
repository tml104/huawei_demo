#include "StdAfx.h"
#include "DebugShow.h"

std::map<std::string, SPAposition> DebugShow::Singleton::debug_points_map;

void DebugShow::Init()
{
}

void DebugShow::Clear()
{
	DebugShow::Singleton::debug_points_map.clear();
}

void DebugShow::AddPoint(const std::string & name, const SPAposition & pos)
{
	DebugShow::Singleton::debug_points_map[name] = pos;
}

void DebugShow::AddPoint(const int num_as_name, const SPAposition & pos)
{
	const std::string name = std::to_string(static_cast<long long>(num_as_name));

	AddPoint(name, pos);
}
