#include "HyDatabase.h"
#include "MySqlConnectionPool.h"

#include <mysql/jdbc.h>

#include <execution>

struct CHyDatabase::impl_t
{
public:
	MySqlConnectionPool pool;
};

static CHyDatabase g_DataBase;
CHyDatabase &HyDatabase()
{
	return g_DataBase;
}

CHyDatabase::CHyDatabase() : pimpl(std::make_shared<impl_t>()) {}

CHyDatabase::~CHyDatabase() = default;

static HyUserAccountData UserAccountDataFromSqlResult(const std::shared_ptr<sql::ResultSet> &res)
{
	if (!res->next())
		throw InvalidUserAccountDataException();
	return HyUserAccountData{
			res->getInt64("qqid"),
			res->getString("auth"),
			res->getInt("xscode"),
			res->getString("access"),
			res->getString("tag"),
			res->getString("type")
	};
}

HyUserAccountData CHyDatabase::QueryUserAccountDataByQQID(int64_t fromQQ)
{
	return UserAccountDataFromSqlResult(pimpl->pool.acquire()->Query("SELECT `qqid`, `auth`,`xscode`,`access`,`tag`,`type` FROM qqlogin WHERE `qqid` = '" + std::to_string(fromQQ) + "';"));
}

std::shared_future<HyUserAccountData> CHyDatabase::QueryUserAccountDataByQQIDAsync(int64_t fromQQ)
{
	return std::async([fromQQstr = std::to_string(fromQQ), conn = pimpl->pool.acquire()]() { return UserAccountDataFromSqlResult(conn->Query("SELECT `qqid`, `auth`,`xscode`,`access`,`tag`,`type` FROM qqlogin WHERE `qqid` = '" + fromQQstr + "';"));  });
}

std::shared_future<HyUserAccountData> CHyDatabase::QueryUserAccountDataByQQIDDeferred(int64_t fromQQ)
{
	return std::async(std::launch::deferred, [fromQQstr = std::to_string(fromQQ), conn = pimpl->pool.acquire()]() { return UserAccountDataFromSqlResult(conn->Query("SELECT `qqid`, `auth`,`xscode`,`access`,`tag`,`type` FROM qqlogin WHERE `qqid` = '" + fromQQstr + "';"));  });
}

HyUserAccountData CHyDatabase::QueryUserAccountDataByXSCodeAndNullQQID(int32_t xscode)
{
	return UserAccountDataFromSqlResult(pimpl->pool.acquire()->Query("SELECT `qqid`, `auth`,`xscode`,`access`,`tag`,`type` FROM qqlogin WHERE `xscode` = '" + std::to_string(xscode) + "' AND `qqid` IS NULL;"));
}

bool CHyDatabase::UpdateXSCodeByQQID(int64_t qqid, int32_t xscode)
{
	return pimpl->pool.acquire()->Update("UPDATE qqlogin SET `xscode`='" + std::to_string(xscode) + "' WHERE `qqid` = '" + std::to_string(qqid) + "';");
}

bool CHyDatabase::UpdateRegisteredQQIDByAuth(std::string auth, int64_t qqid)
{
	return pimpl->pool.acquire()->Update("UPDATE qqlogin SET `qqid`='" + std::to_string(qqid) + "' WHERE `auth` = '" + (auth) + "' AND `qqid` IS NULL;");
}

std::vector<HyUserOwnItemInfo> CHyDatabase::QueryUserOwnItemInfo(const std::string & auth)
{
	std::vector<HyUserOwnItemInfo> result;
	auto conn = pimpl->pool.acquire();
	auto res = conn->Query("SELECT itemown.`code`,`name`,`desc`,`quantifier`,`amount` FROM itemown, iteminfo WHERE auth = '" + auth + "' AND itemown.`code` = iteminfo.`code` AND amount > 0 LIMIT 50");

	while (res->next())
	{
		HyItemInfo item{
			res->getString(1), // code
			res->getString("name"),
			res->getString("desc"),
			res->getString("quantifier")
		};
		auto amount = res->getInt("amount");
		result.push_back({ item, amount });
	}
	return result;
}

std::pair<HyUserSignResultType, std::optional<HyUserSignResult>> CHyDatabase::DoUserDailySign(const HyUserAccountData &user)
{
	int rank = 1;
	int rewardmultiply = 1;
	int signcount = 0;

	// 判断是否重复签到
	auto conn = pimpl->pool.acquire();
	{
		auto res = conn->Query("SELECT TO_DAYS(NOW()) - TO_DAYS(`signdate`) AS signdelta, `signcount` FROM event WHERE auth ='" + user.auth + "';");
		if (res->next())
		{
			auto signdelta = res->getInt64(1);
			
			if (!res->isNull(1) && signdelta == 0 )
			{
				// 已经签到过
				return { HyUserSignResultType::failure_already_signed , std::nullopt };
			}

			if (signdelta == 1)
				signcount = res->getInt64(2);
			else
				signcount = 0;

			conn->Update("UPDATE event SET `signdate`=NOW(), `signcount`='" + std::to_string(signcount) + "' WHERE `auth`='" + user.auth + "';");


		}
		else
		{
			conn->Update("INSERT INTO event(auth) VALUES('" + user.auth + "');");
		}
	}
		

	// 计算签到名次
	{
		auto res = conn->Query("SELECT `auth` FROM event WHERE TO_DAYS(`signdate`) = TO_DAYS(NOW());");
		rank = res->rowsCount();
	}

	if (rank == 1)
		rewardmultiply *= 3;
	else if (rank == 2)
		rewardmultiply *= 5;
	else if (rank == 3)
		rewardmultiply *= 0;
	else if (rank == 4)
		rewardmultiply *= 7;
	else if (rank == 9)
		rewardmultiply *= 0;
	else if (rank == 10)
		rewardmultiply *= 2;
	
	if (user.access.find('p') != std::string::npos)
		rewardmultiply *= 100;
	else if (user.access.find('o') != std::string::npos)
		rewardmultiply *= 3;

	++signcount;

	// 填充签到奖励表

	auto f = [conn = pimpl->pool.acquire(), &user, signcount]() -> HyUserSignGetItemInfo {
		// 随机选择签到奖励
		auto res = conn->Query("SELECT "
			"amx.itemaward.`code` AS icode, "
			"amx.iteminfo.`name` AS iname, "
			"amx.iteminfo.`desc` AS idesc, "
			"amx.iteminfo.`quantifier` AS iquantifier, "
			"amx.itemaward.`amount` AS iamount "
			"FROM amx.itemaward, amx.iteminfo "
			"WHERE amx.itemaward.`code` = amx.iteminfo.`code` AND '" + std::to_string(signcount) + "' BETWEEN `minfrags` AND `maxfrags` ORDER BY RAND() ASC LIMIT 1");
		if (!res->next())
			return {};

		HyItemInfo item{
				res->getString("icode"),
				res->getString("iname"),
				res->getString("idesc"),
				res->getString("iquantifier"),
		};
		auto add_amount = res->getInt("iamount");

		// 查询已有数量
		auto cur_amount = 0;
		{
			auto res2 = conn->Query("SELECT `amount` FROM itemown WHERE `auth` ='" + user.auth + "' AND `code` = '" + item.code + "'");
			if (res2->next())
			{
				cur_amount = res2->getInt("amount");
			}
			else
			{
				// 找不到记录就先插入
				cur_amount = 0;
				conn->Update("INSERT INTO itemown(auth, code, amount) VALUES('" + user.auth + "', '" + item.code + "', '0');");
			}
		}
		cur_amount += add_amount;
		
		// 返回
		return HyUserSignGetItemInfo{
			std::move(item),
			add_amount,
			cur_amount
		};
	};
	std::vector<HyUserSignGetItemInfo> vecItems(rewardmultiply);
	std::generate(std::execution::par, vecItems.begin(), vecItems.end(), f);

	// 设置新奖励
	auto f2 = [conn = pimpl->pool.acquire(), &user](const HyUserSignGetItemInfo &info) {
		conn->Update("UPDATE itemown SET `amount`=`amount`+'" + std::to_string(info.add_amount) + "' WHERE `auth` ='" + user.auth + "' AND `code` = '" + info.item.code + "'");
	};
	std::for_each(std::execution::par, vecItems.begin(), vecItems.end(), f2);

	return { HyUserSignResultType::success, HyUserSignResult{ rank, signcount, rewardmultiply, std::move(vecItems)} };
}
