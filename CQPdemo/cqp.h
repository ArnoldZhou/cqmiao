/*
* CoolQ SDK for VC++ 
* Api Version 9.6
* Written by Coxxs & Thanks for the help of orzFly
*/
#pragma once

#define CQAPIVER 9
#define CQAPIVERTEXT "9"

#ifndef CQAPI
#define CQAPI(RETN_TYPE) extern "C" __declspec(dllimport) RETN_TYPE __stdcall
#endif

#define CQEVENT(RETN_TYPE, FUNC_NAME, HEAD_SIZE) __pragma(comment(linker, "/EXPORT:" #FUNC_NAME "=_" #FUNC_NAME "@" #HEAD_SIZE ))\
 extern "C" RETN_TYPE __stdcall FUNC_NAME

typedef BOOL CQBOOL;

#define EVENT_IGNORE 0        //�¼�_����
#define EVENT_BLOCK 1         //�¼�_����

#define REQUEST_ALLOW 1       //����_ͨ��
#define REQUEST_DENY 2        //����_�ܾ�

#define REQUEST_GROUPADD 1    //����_Ⱥ���
#define REQUEST_GROUPINVITE 2 //����_Ⱥ����

#define CQLOG_DEBUG 0           //���� ��ɫ
#define CQLOG_INFO 10           //��Ϣ ��ɫ
#define CQLOG_INFOSUCCESS 11    //��Ϣ(�ɹ�) ��ɫ
#define CQLOG_INFORECV 12       //��Ϣ(����) ��ɫ
#define CQLOG_INFOSEND 13       //��Ϣ(����) ��ɫ
#define CQLOG_WARNING 20        //���� ��ɫ
#define CQLOG_ERROR 30          //���� ��ɫ
#define CQLOG_FATAL 40          //�������� ���

CQAPI(INT32) CQ_sendPrivateMsg(INT32 AuthCode, INT64 QQID, LPCSTR msg);
CQAPI(INT32) CQ_sendGroupMsg(INT32 AuthCode, INT64 groupid, LPCSTR msg);
CQAPI(INT32) CQ_sendDiscussMsg(INT32 AuthCode, INT64 discussid, LPCSTR msg);
CQAPI(INT32) CQ_sendLike(INT32 AuthCode, INT64 QQID);
CQAPI(INT32) CQ_setGroupKick(INT32 AuthCode, INT64 groupid, INT64 QQID, CQBOOL rejectaddrequest);
CQAPI(INT32) CQ_setGroupBan(INT32 AuthCode, INT64 groupid, INT64 QQID, INT64 duration);
CQAPI(INT32) CQ_setGroupAdmin(INT32 AuthCode, INT64 groupid, INT64 QQID, CQBOOL setadmin);
CQAPI(INT32) CQ_setGroupWholeBan(INT32 AuthCode, INT64 groupid, CQBOOL enableban);
CQAPI(INT32) CQ_setGroupAnonymousBan(INT32 AuthCode, INT64 groupid, LPCSTR anomymous, INT64 duration);
CQAPI(INT32) CQ_setGroupAnonymous(INT32 AuthCode, INT64 groupid, CQBOOL enableanomymous);
CQAPI(INT32) CQ_setGroupCard(INT32 AuthCode, INT64 groupid, INT64 QQID, LPCSTR newcard);
CQAPI(INT32) CQ_setGroupLeave(INT32 AuthCode, INT64 groupid, CQBOOL isdismiss);
CQAPI(INT32) CQ_setGroupSpecialTitle(INT32 AuthCode, INT64 groupid, INT64 QQID, LPCSTR newspecialtitle, INT64 duration);
CQAPI(INT32) CQ_setDiscussLeave(INT32 AuthCode, INT64 discussid);
CQAPI(INT32) CQ_setFriendAddRequest(INT32 AuthCode, LPCSTR responseflag, INT32 responseoperation, LPCSTR remark);
CQAPI(INT32) CQ_setGroupAddRequestV2(INT32 AuthCode, LPCSTR responseflag, INT32 requesttype, INT32 responseoperation, LPCSTR reason);
CQAPI(LPCSTR) CQ_getGroupMemberInfoV2(INT32 AuthCode, INT64 groupid, INT64 QQID, CQBOOL nocache);
CQAPI(LPCSTR) CQ_getStrangerInfo(INT32 AuthCode, INT64 QQID, CQBOOL nocache);
CQAPI(INT32) CQ_addLog(INT32 AuthCode, INT32 priority, LPCSTR category, LPCSTR content);
CQAPI(LPCSTR) CQ_getCookies(INT32 AuthCode);
CQAPI(INT32) CQ_getCsrfToken(INT32 AuthCode);
CQAPI(INT64) CQ_getLoginQQ(INT32 AuthCode);
CQAPI(LPCSTR) CQ_getLoginNick(INT32 AuthCode);
CQAPI(LPCSTR) CQ_getAppDirectory(INT32 AuthCode);
CQAPI(INT32) CQ_setFatal(INT32 AuthCode, LPCSTR errorinfo);
