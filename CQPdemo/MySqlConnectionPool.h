#pragma once

#include <memory>
#include <string>
#include <mutex>

#include <mysql/jdbc.h>
#include "DatabaseConfig.h"

class MySqlConnection;

class MySqlConnectionUniqueAccessor : std::enable_shared_from_this<MySqlConnectionUniqueAccessor>
{
public:
	MySqlConnectionUniqueAccessor(std::shared_ptr<MySqlConnection> p);
	~MySqlConnectionUniqueAccessor();
	std::shared_ptr<sql::ResultSet> Query(const std::string &sql);
	int Update(const std::string &sql);

private:
	std::shared_ptr<MySqlConnection> connection;
};

class MySqlConnectionPool
{
public:
	MySqlConnectionPool(const DatabaseConfig &c = GetDatabaseConfig());
	~MySqlConnectionPool();

public:
	// ensures not nullptr
	std::shared_ptr<MySqlConnectionUniqueAccessor> acquire();

private:
	std::mutex m;
	std::vector<std::shared_ptr<MySqlConnection>> v;
	DatabaseConfig config;
};

