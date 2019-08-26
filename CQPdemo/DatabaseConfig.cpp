#include "DatabaseConfig.h"

#include <fstream>
#include <nlohmann/json.hpp>


static DatabaseConfig ConstructFromJsonFile()
{
	std::ifstream i("hy-db.json");
	nlohmann::json j;
	i >> j;
	return { j["url"], j["user"], j["pass"], j["schema"] };
}

const DatabaseConfig & GetDatabaseConfig()
{
	static DatabaseConfig x = ConstructFromJsonFile();
	return x;
}