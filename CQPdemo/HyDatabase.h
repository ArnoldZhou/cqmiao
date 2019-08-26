#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <optional>
#include <vector>
#include <future>

struct HyUserAccountData
{
	int64_t qqid;
	std::string auth;
	int32_t xscode;
	std::string access;
	std::string tag;
	std::string type;
};

struct HyItemInfo
{
	std::string code;
	std::string name;
	std::string desc;
	std::string quantifier;
};

struct HyUserOwnItemInfo
{
	HyItemInfo item;
	int32_t amount;
};

struct HyUserSignGetItemInfo
{
	HyItemInfo item;
	int32_t add_amount;
	int32_t cur_amount;
};

enum class HyUserSignResultType
{
	success,
	failure_already_signed,
};

struct HyUserSignResult
{
	int iRank;
	int iContinuouslyKeepDays;
	int iMultiply;
	std::vector<HyUserSignGetItemInfo> vecItems;
};

class InvalidUserAccountDataException : std::invalid_argument {
public:
	InvalidUserAccountDataException() : std::invalid_argument("InvalidUserAccountDataException : 此账号未注册。") {}

	const char *what() const override
	{
		return "此账号未注册。";
	}
};

class CHyDatabase
{
public:
	CHyDatabase();
	~CHyDatabase();

public:
	// 登录用
	HyUserAccountData QueryUserAccountDataByQQID(int64_t qqid); // 可能抛出InvalidUserAccountDataException
	std::shared_future<HyUserAccountData> QueryUserAccountDataByQQIDAsync(int64_t qqid); // 可能抛出InvalidUserAccountDataException
	std::shared_future<HyUserAccountData> QueryUserAccountDataByQQIDDeferred(int64_t qqid); // 可能抛出InvalidUserAccountDataException
	bool UpdateXSCodeByQQID(int64_t qqid, int32_t xscode);

	// 注册用
	HyUserAccountData QueryUserAccountDataByXSCodeAndNullQQID(int32_t xscode); // 可能抛出InvalidUserAccountDataException
	bool UpdateRegisteredQQIDByAuth(std::string auth, int64_t qqid);

	// 查道具用
	std::vector<HyUserOwnItemInfo> QueryUserOwnItemInfo(const std::string &auth);
	std::vector<HyUserOwnItemInfo> QueryUserOwnItemInfo(const HyUserAccountData &user) { return QueryUserOwnItemInfo(user.auth);  }

	// 签到用
	std::pair<HyUserSignResultType, std::optional<HyUserSignResult>> DoUserDailySign(const HyUserAccountData &user);

private:
	struct impl_t;
	std::shared_ptr<impl_t> pimpl;
};

CHyDatabase &HyDatabase();