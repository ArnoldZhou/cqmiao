#pragma once

#include <string>

struct DatabaseConfig
{
	std::string url;
	std::string user;
	std::string pass;
	std::string schema;
};

const DatabaseConfig &GetDatabaseConfig();
