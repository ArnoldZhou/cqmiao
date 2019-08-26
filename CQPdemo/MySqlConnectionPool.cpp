#include "MySqlConnectionPool.h"
#include "DatabaseConfig.h"

#include <mutex>

class MySqlConnection : std::enable_shared_from_this<MySqlConnection>
{
public:
	MySqlConnection(const DatabaseConfig &config) :
		driver(get_driver_instance()),
		config(config)
	{
		sql::ConnectOptionsMap connection_properties;
		connection_properties["hostName"] = config.url;
		connection_properties["userName"] = config.user;
		connection_properties["password"] = config.pass;
		connection_properties["OPT_RECONNECT"] = true;
		connection_properties["OPT_CHARSET_NAME"] = "utf8";

		con.reset(driver->connect(std::move(connection_properties)));
		con->setSchema(config.schema);
		stmt.reset(con->createStatement());
	}

public:
	std::mutex connectMutex;
	DatabaseConfig config;
	sql::Driver *driver;
	std::unique_ptr<sql::Connection> con;
	std::unique_ptr<sql::Statement> stmt;

public:
	std::weak_ptr<MySqlConnectionUniqueAccessor> accessor;
};

MySqlConnectionPool::MySqlConnectionPool(const DatabaseConfig & c) : config(c)
{

}

MySqlConnectionPool::~MySqlConnectionPool() = default;

std::shared_ptr<MySqlConnectionUniqueAccessor> MySqlConnectionPool::acquire()
{
	std::lock_guard l(m); // 先加锁

	std::shared_ptr<MySqlConnectionUniqueAccessor> ret = nullptr;
	while (ret == nullptr)
	{
		if (auto iter = std::find_if(v.cbegin(), v.cend(), [](const std::shared_ptr<MySqlConnection> &p) { return p->accessor.lock() == nullptr; }); iter != v.cend())
		{
			// 有可用连接，设置后返回。
			ret = std::make_shared<MySqlConnectionUniqueAccessor>(*iter);
			(*iter)->accessor = ret;
			//break;
		}
		else
		{
			v.push_back(std::make_shared<MySqlConnection>(config));
			//continue;
		}
	}
	return ret;
}

MySqlConnectionUniqueAccessor::MySqlConnectionUniqueAccessor(std::shared_ptr<MySqlConnection> p) : connection(std::move(p))
{

}

MySqlConnectionUniqueAccessor::~MySqlConnectionUniqueAccessor() = default;

std::shared_ptr<sql::ResultSet> MySqlConnectionUniqueAccessor::Query(const std::string & sql)
{
	return std::shared_ptr<sql::ResultSet>(connection->stmt->executeQuery(sql));
}

int MySqlConnectionUniqueAccessor::Update(const std::string & sql)
{
	return connection->stmt->executeUpdate(sql);
}
