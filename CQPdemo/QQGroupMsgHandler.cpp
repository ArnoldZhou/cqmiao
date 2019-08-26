#include "stdafx.h"
#include "QQGroupMsgHandler.h"

#include <algorithm>
#include <execution>
#include <regex>
#include <array>
#include <chrono>

#include <asio.hpp>

#include "TSourceEngineQuery.h"
#include "HyDatabase.h"
#include "CQP.h"
#include "Encode.h"

using namespace std::chrono_literals;

constexpr std::tuple<const char *, const char *, const char *> serverlist[] = {
					{ "0F ��ȱ��Ԥ����", "", "" },
					{ "1F �������(��ʬ)", "111.67.194.144", "27015" },
					{ "2F ����Z����ʬ��", "cs.faithzone.cf", "27016" },
					{ "3F ��ȱ��Ԥ����", "", "" },
					{ "4F ��ҹ����(����)", "cn.cserfamily.cf", "1114" },
					{ "5F ��ȱ��Ԥ����", "", "" },
					{ "6F 100aa��èè", "cs-server.club", "27020" },
					{ "7F 100aa���ַɵ�", "cs-server.club", "27030" },
					{ "8F 10aa������", "cs-server.club", "27040" }
};

CQQGroupMsgHandler::~CQQGroupMsgHandler() = default;

void CQQGroupMsgHandler::HandleGroupMsg() noexcept
{
	try
	{
		m_sharedAccountDataByQQ = HyDatabase().QueryUserAccountDataByQQIDDeferred(fromQQ);
		std::for_each(std::execution::par, std::begin(CQQGroupMsgHandler::handlers), std::end(CQQGroupMsgHandler::handlers), [this](auto pmf) { (this->*pmf)(); });

	}
	catch (const std::exception &e)
	{
		CQ_sendGroupMsg(ac, fromGroup, e.what());
	}
	catch (...)
	{
		CQ_sendGroupMsg(ac, fromGroup, "δ֪�Ĵ���");
	}
}

void CQQGroupMsgHandler::ParseServerQueryMessage() noexcept
{
	try
	{
		if (ANSI_To_Unicode(msg).find(L"ip") == 0)
		{
			auto query_fn = [](auto sv) -> std::shared_future<TSourceEngineQuery::ServerInfoQueryResult> {
				auto [name, host, port] = sv;
				try
				{
					if (host && host[0])
					{
						if (!port || !port[0])
							port = "27015";
						return TSourceEngineQuery().GetServerInfoDataAsync(host, port);
					}
				}
				catch (...) {}
				return {};
			};

			constexpr auto servers_count = std::extent<decltype(serverlist)>::value;
			std::array<std::shared_future<TSourceEngineQuery::ServerInfoQueryResult>, servers_count> sf_list;
			std::transform(std::execution::par, std::begin(serverlist), std::end(serverlist), sf_list.begin(), query_fn);
			const auto abandon_time = std::chrono::system_clock::now() + 1s;
			std::ostringstream oss;
			for (size_t i = 0; i < servers_count; ++i)
			{
				auto &&[name, host, port] = serverlist[i];
				
				oss << name << "\t  - " << host << ":" << port << std::endl;

				if (sf_list[i].valid() && sf_list[i].wait_until(abandon_time) == std::future_status::ready)
				{
					auto result = sf_list[i].get();
					oss << "\t" << result.Map << " (" << result.PlayerCount << "/" << result.MaxPlayers << ")" << std::endl;
				}
			}
			CQ_sendGroupMsg(ac, fromGroup, ("[CQ:at,qq=" + std::to_string(fromQQ) + "] " + std::move(oss).str()).c_str());
		}
		else
		{

			std::string host;
			std::string port = "27015";

			if (msg.size() >= 2 && isdigit(msg[0]) && (msg[1] == 'f' || msg[1] == 'F'))
			{
				std::size_t n = msg[0] - '0';
				
				if (n < std::extent<decltype(serverlist)>::value)
				{
					std::tie(std::ignore, host, port) = serverlist[n];
				}
			}
			else
			{
				static std::regex r1(R"((^[0-9a-zA-Z]+[0-9a-zA-Z\.-]*\.[a-zA-Z]{2,4}):*(\d+)*)");
				static std::regex r2(R"((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))(:\d+)*)");

				std::optional<TSourceEngineQuery::ServerInfoQueryResult> data;

				if (std::smatch sm; std::regex_match(msg, sm, r1) && sm.size() > 2)
				{
					// host:port, host, port
					host = sm[1].str();
					if (sm[2].str().size())
						port = sm[2].str();
				}
				else if (std::smatch sm; std::regex_match(msg, sm, r2) && sm.size() > 2)
				{
					// ip:port, ...
					const auto adr = sm[0].str();
					auto iter = std::find(adr.begin(), adr.end(), ':');
					host.assign(adr.begin(), iter);
					if (iter != adr.end())
						port.assign(iter + 1, adr.end());
				}
			}
			

			if (!host.empty())
			{
				std::string msg2;
				if (fromQQ)
				{
					msg2 += "[CQ:at,qq=" + std::to_string(fromQQ) + "]";
				}
				msg2 += QueryServerInfo(host.c_str(), port.c_str());
				CQ_sendGroupMsg(ac, fromGroup, msg2.c_str());
			}
		}

		
	}
	catch (const std::exception &e)
	{
		CQ_sendGroupMsg(ac, fromGroup, e.what());
	}
	catch (...)
	{

	}
}

void CQQGroupMsgHandler::ParseLoginCodeMessage() noexcept
{
	try
	{
		if (msg.size() == 6)
		{
			if (int32_t xscode = std::stoi(msg); xscode >= 100000 && xscode <= 999999) // �����׳� invalid_argument
			{
				try
				{
					auto dataval = m_sharedAccountDataByQQ.get(); // �������쳣
					HyDatabase().UpdateXSCodeByQQID(fromQQ, xscode);
					CQ_sendGroupMsg(ac, fromGroup, ("[CQ:at,qq=" + std::to_string(fromQQ) + "] �ɹ���¼��¼��Ϣ����ʹ�� ��" + UTF8_To_GBK(dataval.tag) + "��" + UTF8_To_GBK(dataval.auth) + " �˺ŵ�¼��������").c_str());
				}
				catch (const InvalidUserAccountDataException &) {
					try
					{
						auto dataval = HyDatabase().QueryUserAccountDataByXSCodeAndNullQQID(xscode);
						HyDatabase().UpdateRegisteredQQIDByAuth(dataval.auth, fromQQ);
						CQ_sendGroupMsg(ac, fromGroup, ("[CQ:at,qq=" + std::to_string(fromQQ) + "] ע��ɹ�����ʹ�� ��" + UTF8_To_GBK(dataval.tag) + "��" + UTF8_To_GBK(dataval.auth) + " �˺ŵ�¼��������").c_str());
					}
					catch (...)
					{
						throw std::invalid_argument("�����ע��ID");
					}
				}
			}
		}
	}
	catch (const std::invalid_argument & e)
	{
		// 6λ���ִ���ʧ��
		return;
	}
	catch (const std::exception &e)
	{
		CQ_sendGroupMsg(ac, fromGroup, e.what());
	}
	catch (...)
	{

	}
}

void CQQGroupMsgHandler::ParseItemQueryMessage() noexcept
{
	try
	{
		if (ANSI_To_Unicode(msg).find(L"����") != 0)
		{
			return;
		}
		
		auto dataval = m_sharedAccountDataByQQ.get(); // �������쳣

		auto infolist = HyDatabase().QueryUserOwnItemInfo(dataval.auth);

		std::ostringstream oss;
		oss << "[CQ:at,qq=" << fromQQ << "] ";
		if (infolist.empty())
		{
			oss << "��Ĳֿ�����û���κε���" << std::endl;
		}
		else
		{
			oss << " �ֿ��б�" << std::endl;
			for (auto &&info : infolist)
			{
				oss << UTF8_To_GBK(info.item.name) << "\t" << info.amount << UTF8_To_GBK(info.item.quantifier) << std::endl;
			}
		}
			
		CQ_sendGroupMsg(ac, fromGroup, std::move(oss).str().c_str());

	}
	catch (const std::exception &e)
	{
		CQ_sendGroupMsg(ac, fromGroup, e.what());
	}
	catch (...)
	{

	}
}

void CQQGroupMsgHandler::ParseSignInMessage() noexcept
{
	try
	{
		if (ANSI_To_Unicode(msg).find(L"ǩ��") != 0)
		{
			return;
		}
		auto user = m_sharedAccountDataByQQ.get(); // �������쳣

		auto res = HyDatabase().DoUserDailySign(user);

		std::ostringstream oss;
		oss << "[CQ:at,qq=" << fromQQ << "] ";
		if (res.first == HyUserSignResultType::failure_already_signed)
		{
			oss << "������Ѿ�ǩ������" << std::endl;
		}
		else if(res.second.has_value())
		{
			HyUserSignResult resval = res.second.value();
			oss << "ǩ���ɹ������ڽ���� " << resval.iRank << " ��ǩ����";
			if(resval.vecItems.empty())
			{
				oss << "��ϲ��û�л���κν�����" << std::endl
					<< "TIPS�����Ѿ�����ǩ�� " << resval.iContinuouslyKeepDays << " �죬ÿ������ǩ�����Ի�ø��ཱ������";
			}
			else if (resval.vecItems.size() == 1)
			{
				auto &&reward = resval.vecItems.front();
				oss << "��ϲ���ý�����" << UTF8_To_GBK(reward.item.name) << " " << reward.add_amount << " " << UTF8_To_GBK(reward.item.quantifier)
					<< "(���� " << reward.cur_amount << " " << UTF8_To_GBK(reward.item.quantifier) << ")��"
					<< "TIPS�����Ѿ�����ǩ�� " << resval.iContinuouslyKeepDays << " �죬ÿ������ǩ�����Ի�ø��ཱ������";
			}
			else
			{
				oss << "�Ѿ�����ǩ�� " << resval.iContinuouslyKeepDays << " �졣";
				if (user.access.find('o') != std::string::npos)
					oss << "����������" << UTF8_To_GBK(user.tag) << "��" << resval.iMultiply << " ��ǩ������Ȩ�ޣ����������õĽ�����";
				else
					oss << "���������õ� " << resval.iMultiply << " ��ǩ��������";

				for (auto &reward : resval.vecItems)
				{
					oss << std::endl
						<< UTF8_To_GBK(reward.item.name) << " " << reward.add_amount << " " << UTF8_To_GBK(reward.item.quantifier)
						<< "(���� " << reward.cur_amount << " " << UTF8_To_GBK(reward.item.quantifier) << ")";
				}
			}
		}

		CQ_sendGroupMsg(ac, fromGroup, std::move(oss).str().c_str());
	}
	catch (const std::exception &e)
	{
		CQ_sendGroupMsg(ac, fromGroup, e.what());
	}
	catch (...)
	{

	}
}

std::string CQQGroupMsgHandler::QueryServerInfo(const char * host, const char * port) const noexcept(false)
{
	TSourceEngineQuery tseq;
	auto finfo = tseq.GetServerInfoDataAsync(host, port);
	auto fplayer = tseq.GetPlayerListDataAsync(host, port);

	if (finfo.wait_for(500ms) != std::future_status::timeout)
	{
		auto result = finfo.get(); // try
		std::ostringstream oss;
		oss << result.ServerName  << std::endl;
		oss << "\t" << result.Map << " (" << result.PlayerCount << "/" << result.MaxPlayers << ") - " << result.FromAddress << ":" << result.FromPort << std::endl;

		std::string myReply = oss.str();

		if (finfo.wait_for(1000ms) == std::future_status::ready)
		{
			auto playerlist = std::get<1>(fplayer.get().Results);
			for (const auto &player : playerlist)
			{
				myReply += player.Name;
				myReply += " [";
				myReply += std::to_string(player.Score) + "��";
				//int duration = static_cast<int>(player.Duration);
				//myReply += std::to_string(duration / 60) + ":" + std::to_string(duration % 60);
				myReply += "] / ";
			}
		}

		return myReply;
	}
	return "������δ��Ӧ��";
}
