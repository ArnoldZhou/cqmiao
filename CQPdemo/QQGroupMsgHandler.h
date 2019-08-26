#pragma once

#include <future>
#include <string>

#include "HyDatabase.h"

class CQQGroupMsgHandler
{
public:
	CQQGroupMsgHandler(int64_t group, int64_t qq, const char *anonymous, const char *pmsg)
		: fromGroup(group), fromQQ(qq), fromAnonymous(anonymous), msg(pmsg) {}
	~CQQGroupMsgHandler();

public:
	void HandleGroupMsg() noexcept;

protected:
	void ParseServerQueryMessage() noexcept;
	void ParseLoginCodeMessage() noexcept;
	void ParseItemQueryMessage() noexcept;
	void ParseSignInMessage() noexcept;

public:
	std::string QueryServerInfo(const char *host, const char *port) const;

protected:
	using handler_t = void (CQQGroupMsgHandler::*)() noexcept;
	static constexpr handler_t handlers[] = {
		&CQQGroupMsgHandler::ParseLoginCodeMessage,
		&CQQGroupMsgHandler::ParseServerQueryMessage,
		&CQQGroupMsgHandler::ParseItemQueryMessage,
		&CQQGroupMsgHandler::ParseSignInMessage

	};

private:
	const int64_t fromGroup;
	const int64_t fromQQ;
	const std::string fromAnonymous;
	const std::string msg;

	std::shared_future<HyUserAccountData> m_sharedAccountDataByQQ;
};

extern int ac;

