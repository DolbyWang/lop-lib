//////////////////////////////////////////////////////////////////////
//  created:    2014/07/18
//  filename:   MWLib/redis/redisSystem.h
//  author:     League of Perfect
/// @brief
///
//////////////////////////////////////////////////////////////////////
#ifndef __MWLIB_REDIS_REDISSYSTEM_H__
#define __MWLIB_REDIS_REDISSYSTEM_H__

#include <google/protobuf/message_lite.h>
#include <SFLib/commonDefine/logFile.h>
#include <BCLib/utility/singleton.h>
//#include <SHLib/protobuf/message.pb.h>
#include <MWLib/redis/redlock.h>
#include <unordered_map>
//#ifdef _WIN32
//#include <hiredis/win/deps/hiredis/hiredis.h>
//#else
#include <hiredis.h>
//#endif

#define PROCESS_REPLY_ERROR     if(m_redisReply == NULL) \
								{ \
									this->disconnect(type);  \
									return false; \
								} \
								if(m_redisReply->type == REDIS_REPLY_ERROR) \
								{ \
									freeReplyObject(m_redisReply); \
									m_redisReply = NULL; \
									return false; \
								} \
								freeReplyObject(m_redisReply); \
								m_redisReply = NULL;	\
								return true;

#define PROCESS_REPLY_RETURN     if(m_redisReply == NULL) \
								{ \
									this->disconnect(type);  \
									return; \
								} \
								if(m_redisReply->type == REDIS_REPLY_ERROR) \
								{ \
									freeReplyObject(m_redisReply); \
									m_redisReply = NULL; \
									return; \
								} \
								freeReplyObject(m_redisReply); \
								m_redisReply = NULL;	\
								return;
#define DEFAULT_UNIQUEID 1000000

#define REDIS_LOCK(key, uniqueid, subkey)\
								CRedLock* pRedLock = MWLib::Redis::CRedisSystem::singleton().getRedLock();\
								CLock __my_lock_;\
								if (pRedLock != NULL)\
								{\
									while (1){\
										char key1[1024] = { 0 };\
										std::string fild = BCLib::Utility::CConvert::toStringA(uniqueid);\
										char subkey1[512] = { 0 };\
										snprintf(subkey1, 512, "%s:lock", subkey);\
										char key2[512] = { 0 };\
										snprintf(key2, 512, "%s:[%s]:", key, fild.c_str());\
										snprintf(key1, 1024, "%s%s",key2, subkey1);\
										bool _lock_flag_ = pRedLock->Lock(key1, 200, __my_lock_);\
										if(_lock_flag_)\
										{
#define REDIS_UNLOCK						pRedLock->Unlock(__my_lock_);\
											break;\
										}\
										else\
										{\
											BCLib::Utility::CThread::msleep(rand() % 3);\
										}\
									}\
								}
#define REDIS_LOCK_RETURN	if (pRedLock!=NULL)\
							{\
								pRedLock->Unlock(__my_lock_);\
							}\
							return;

namespace MWLib
{
namespace Redis
{
	enum EREDIS_CONTEXT_TYPE
	{
		E_REDIS_SERVERTYPE_BEGIN = 0,
		E_REDIS_SERVERTYPE_LOGIC,
		E_REDIS_SERVERTYPE_CACHE,
		E_REDIS_SERVERTYPE_FRIEND,
		E_REDIS_SERVERTYPE_END,
		
	};
	typedef struct Redis_Node
	{
		redisContext *m_redisContext;

		std::string m_host;
		int m_port;
		std::string m_passwd;
		Redis_Node() 
		{
			m_redisContext = NULL;
			m_host = "";
			m_port = 0;
			m_passwd = "";

		}
		~Redis_Node() 
		{
			if (m_redisContext != NULL)
			{
				redisFree(m_redisContext);
				m_redisContext = NULL;
			}			
		}
	}REDIS_NODE;
class CRedisSystem
{
	BCLIB_SINGLETON_DECLARE(CRedisSystem);

protected:
	CRedisSystem();
	virtual ~CRedisSystem();
public:

	/**
	* ����:�Զ����Ƶ���ʽ����ptbuf
	* @param key ��ֵ����Ӧhash�� ������ʹ�÷���ϵͳ��
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param strPTbufName Ҫ�����ptbuf��
	* @param pPtbuf Ҫ�����ptbufָ��
	* @return true Or false
	*/
	bool savePTBuf(std::string &key, BCLib::uint64 uniqueid, std::string & strPTbufName, const ::google::protobuf::MessageLite *pPtbuf, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:�Զ����Ƶ���ʽ����ptbuf
	* @param key ��ֵ����Ӧhash�� ������ʹ�÷���ϵͳ��
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param strPTbufName Ҫ�����ptbuf��
	* @param pPtbuf ����������ptbufָ��
	* @param ptCacheSize ����������С��Ĭ��Ϊ (1024*16) redis ����������С
	* @return ���ش�redis��ȡ�Ķ��������ݳ��ȣ����ᳬ��(1024*16)
	*/
	BCLib::uint32 loadPTBuf(std::string &key, BCLib::uint64 uniqueid, std::string & strPTbufName, ::google::protobuf::MessageLite *pPtbuf, BCLib::uint32 ptCacheSize = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
public:
	void setInfo(std::string& host, int port, std::string& passwd, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	bool init();

	bool connect(std::string host, int port, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool connectWithTimeout(std::string host, int port, const struct timeval tv, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	void  disconnect(EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	void  disconnectAll();
	bool  exec(const char *cmd, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	bool setCommandTimeout(const struct timeval tv, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool checkStatus( EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	bool clear( EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);


	

	//String
	bool setString(const char *key, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool setString(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	std::string getString(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	std::string getString(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//Uint64
	bool setUint64(const char *key, BCLib::uint64 value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool setUint64(const char *key, BCLib::uint64 uniqueid, const char *subkey, BCLib::uint64 value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	BCLib::uint64 getUint64(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	BCLib::uint64 getUint64(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//Bin
	bool setBin(const char *key, const char *value, BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool setBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *value, BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	BCLib::uint32 getBin(const char *key,  char *value, BCLib::uint32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	BCLib::uint32 getBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, char *value, BCLib::uint32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	//ɾ��ָ��key����������ֵ
	bool delKey(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool delKey(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	//��������ʱ�� ��λs -1 ��������
	bool expireSecond(const char *key, BCLib::int32 second, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool expireSecond(const char *key, BCLib::uint64 uniqueid, const char *subkey, BCLib::int32 second, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//�Ƴ�����ʱ��
	bool persist(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	bool persist(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/*	KEYS�������������ƥ���key��O(N)Ч�ʽϵͣ���������
	KEYS * ƥ�����ݿ������� key ��
	KEYS h?llo ƥ�� hello �� hallo �� hxllo �ȡ�
	KEYS h*llo ƥ�� hllo �� heeeeello �ȡ�
	KEYS h[ae]llo ƥ�� hello �� hallo ������ƥ�� hillo ��
	��������� \ ����
	* @param key ��Ҫƥ���key���ַ���
	* @param keys ���ص�������ƥ���key
	*/
	bool keys(const char *key, std::vector<std::string> &values, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/*�������� key
	* @param matchKey * ƥ�����ݿ������� key ��
					KEYS h?llo ƥ�� hello �� hallo �� hxllo �ȡ�
					KEYS h*llo ƥ�� hllo �� heeeeello �ȡ�
					KEYS h[ae]llo ƥ�� hello �� hallo ������ƥ�� hillo ��
					��������� \ ����
	* @param values ���ص�keys
	* @param start ������ʼ�α�
	* @param count ���������������
	* @param type ��ʹ�õ�redis������
	* @return ���ر��α�������ʱ���α꣬Ϊ0��ʾ��ȫ������
	*/
	BCLib::uint64 scan(const char *matchKey, std::vector<std::string> &values, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	/*
	*map ��װ����������
	*/

	/**
	 * ����:���ض�field - value(��-ֵ)�����õ���ϣ��key��
	 * @param key ��ֵ����Ӧhash����
	 * @param field ��ϣ��key�е���
	 * @param value ��Ӧvalue
	 */
	bool hsetString(const char*key, const char*field, const char*value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:���ض�field - value(��-ֵ)�����õ���ϣ��key��
	* @param key ��ֵ����Ӧhash����
	* @param field ��ϣ��key�е���
	* @param value ��Ӧ��������ָ��value
	* @param len ������������
	*/
	bool hsetBin(const char *key, const char *field, const char *value, BCLib::uint32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:���ض�field - value(��-ֵ)�����õ���ϣ��key��
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param field ��ϣ��key�е��� �ַ�������
	* @param value ��Ӧvalue �ַ�����ַ
	*/
	bool hsetString(const char*key, BCLib::uint64 uniqueid, const char*subkey, const char*field, const char*value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:���ض�field - value(��-ֵ)�����õ���ϣ��key��
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param field ��ϣ��key�е��� �ַ�������
	* @param value ��Ӧvalue ��������ָ��
	* @param len value������������
	*/
	bool hsetBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *field, const char *value, BCLib::uint32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	 * ����:��ȡ��ϣ��key�ض�fieldֵ
	 * @param key ��ֵ����Ӧhash����
	 * @param field ��ϣ��key�е���
	 * @return  ��Ӧ��value
	 */
	std::string hgetString(const char*key, const char*field, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:���ض�field - value(��-ֵ)�����õ���ϣ��key��
	* @param field ��ϣ��key�е��� �ַ�������
	* @param value ���������Ľ���bufferָ��
	* @param len ����buffer�ĳ���
	* @return �����Ķ�����������
	*/
	BCLib::uint32 hgetBin(const char *key, const char *field, char *value, BCLib::uint32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:���ض�field - value(��-ֵ)�����õ���ϣ��key��
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param field ��ϣ��key�е��� �ַ�������
	* @return  ��Ӧ��value
	*/
	std::string hgetString(const char*key, BCLib::uint64 uniqueid, const char*subkey, const char*field, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:���ض�field - value(��-ֵ)�����õ���ϣ��key��
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param field ��ϣ��key�е��� �ַ�������
	* @param len ����buffer�ĳ���
	* @return �����Ķ�����������
	*/
	BCLib::uint32 hgetBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *field, char *value, BCLib::uint32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	 * ����:��ȡ��ϣ��key�ض�����ֵ
	 * @param key ��ֵ����Ӧhash����
	 * @param field_values ��ȡ����hash��field - value(��-ֵ)
	 */
	void hgetall(const char *key, std::map<std::string, std::string>& field_values, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:��ȡ��ϣ��key�ض�����ֵ	
	* @param key ��ֵ����Ӧhash����
	* @return const std::map<std::string, std::pair<char*, BCLib::uint32>*>* ��map<��ֵ��<����������������>>ָ��
	*/
	const std::map<std::string, std::pair<char*, BCLib::uint32>*>* hgetallBin(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:��ȡ��ϣ��key�ض�����ֵ
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param field_values ��ȡ����hash��field - value(��-ֵ)
	*/
	void hgetall(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::map<std::string, std::string>& field_values, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:��ȡ��ϣ��key�ض�����ֵ
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @return const std::map<std::string, std::pair<char*, BCLib::uint32>*>* ��map<��ֵ��<����������������>>ָ��
	*/
	const std::map<std::string, std::pair<char*, BCLib::uint32>*>* hgetallBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	 * ����:ͬʱ�����field - value(��-ֵ)�����õ���ϣ��key��
	 * @param key ��ֵ����Ӧhash����
	 * @param fields_value ���field value(�� ֵ)
	 * e.g.: hmset key_hash name "����" age 18 birthday "20010101"
	 */
	void hmset(const char *key, EREDIS_CONTEXT_TYPE type, std::string format, ...);

	/**
	* ����:ͬʱ�����field - value(��-ֵ)�����õ���ϣ��key��
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param fields_value ���field value(�� ֵ)
	* e.g.: hmset key_hash name "����" age 18 birthday "20010101"
	*/
	void hmset(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type, std::string format, ...);

	//void hmsetBin(const char *key, const char * filed, const char * value, BCLib::uint32 len);
	//void hmsetBin(const char*key, BCLib::uint64 uniqueid, const char*subkey, const char *filed, const char *value, BCLib::uint32 len);

	/**
	* ����:ɾ����ϣ��key�ض�field-value��ֵ��
	* @param key ��ֵ����Ӧhash����
	* @param field ��ϣ��key�е���
	* @return  true or false
	*/
	bool herase(const char *key, const char *field, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ����:ɾ����ϣ��key�ض�field-value��ֵ��
	* @param key ��ֵ����Ӧhash����
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param field ��ϣ��key�е���
	* @return  true or false
	*/
	bool herase(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *field, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param field_values ����key-value map
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ���ر�������ʱ�α�
	*/
	BCLib::uint64 hscan(const char *key, const char *matchKey, std::map<std::string, std::string>& field_values, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param field_values ����key-value map
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ���ر�������ʱ�α�
	*/
	BCLib::uint64 hscan(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *matchKey, std::map<std::string, std::string>& field_values, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param end ���ر��α����Ľ���ʱ���α� 0��ʾ��ȫ����
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ����key-value map
	*/
	const std::map<std::string, std::pair<char*, BCLib::uint32>*>* hscan(const char *key, const char *matchKey, BCLib::uint64 &end, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param uniqueid ʵ��ID û��ʵ��ID�Ŀ��Կ���ͳһʹ��1000000������
	* @param subkey ��������
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param end ���ر��α����Ľ���ʱ���α� 0��ʾ��ȫ����
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ����key-value map
	*/
	const std::map<std::string, std::pair<char*, BCLib::uint32>*>* hscan(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *matchKey, BCLib::uint64 &end, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);


	/*
	*list ��װ����������
	*/


	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listͷ��
	* @param key �б���
	* @param value ���ӵ��б��е�valueֵstring ����
	*/
	bool lpushString(const char *key, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listͷ��
	* @param key �б���
	* @param value ���ӵ��б��е�valueֵ��������ָ��
	* @param len ������������
	*/
	bool lpushBin(const char *key, const char *value, const BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listͷ��
	* @param key �б���
	* @param uniqueid key�����µ� �б���� 
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param value ���ӵ��б��е�valueֵstring ����
	*/
	bool lpushString(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listͷ��
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param value ���ӵ��б��е�valueֵ��������ָ��
	* @param len ������������
	*/
	bool lpushBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *value, const BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ȡ�б���ͷ��Ԫ��ֵ
	* @param key �б���
	* @return ���ض�ȡ��������value��string����
	*/
	std::string lreadString(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ȡ�б���ͷ��Ԫ��ֵ
	* @param key �б���
	* @param value ��ȡ��������value�����ն�����bufferָ��
	* @param len ����buffer��С
	* @return ��ȡ�Ķ�������������С
	*/
	BCLib::uint32 lreadBin(const char *key, char *value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ȡ�б���ͷ��Ԫ��ֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  
	* @param value ��ȡ��������value���ַ�������
	*/
	std::string lreadString(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡ�б���ͷ��Ԫ��ֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey
	* @param value �������������Ľ���buffer
	* @param len �������������Ľ���buffer����
	* @return ��ȡ�ķ��ض���������������
	*/
	BCLib::uint32 lreadBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, char *value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listβ��
	* @param key �б���
	* @param value ���ӵ��б��е�valueֵ�ַ�������
	*/
	bool rpushString(const char *key, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listβ��
	* @param key �б���
	* @param value ���ӵ��б��е�value������������
	* @param len ����������������
	*/
	bool rpushBin(const char *key, const char *value, const BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listβ��
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  
	* @param value ���ӵ��б��е�valueֵ�ַ�������
	*/
	bool rpushString(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ָ��ֵvalue���ӵ�listβ��
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey
	* @param value ���ӵ��б��е�value������������
	* @param len ����������������
	*/
	bool rpushBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *value, const BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ȡ��β����valueֵ
	* @param key �б���
	* @return ��ȡ��������value��string����
	*/
	std::string rreadString(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡ��β����valueֵ
	* @param key �б���
	* @param value ���������ݽ���bufferָ��
	* @param len ����buffer ����
	*/
	BCLib::uint32 rreadBin(const char *key, char *value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ȡ��β����valueֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  
	* @return ��ȡ��������value��string����
	*/
	std::string rreadString(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡ��β����valueֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey
	* @param value ������������bufferָ��
	* @param len ������������buffer�ĳ���
	* @return ���ض�ȡ�Ķ�����������
	*/
	BCLib::uint32 rreadBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, char *value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���ȡ�б���Ԫ�ظ���
	* @param key �б���
	* @return �����б�Ԫ�ظ��� ʧ�ܷ��� -1
	*/
	BCLib::int32 llen(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡ�б���Ԫ�ظ���
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey
	* @return �����б�Ԫ�ظ��� ʧ�ܷ��� -1
	*/
	BCLib::int32 llen(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б��е�һ��Ԫ�ز�����
	* @param key �б���
	* @param value ����Ԫ��
	* @return �ɹ���ʧ��
	*/
	std::string lpopString(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡ��β����valueֵ
	* @param key �б���
	* @param value ������������bufferָ��
	* @param len ������������buffer�ĳ���
	* @return ����pop�����Ķ�����������
	*/
	BCLib::uint32  lpopBin(const char *key, char*value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б��е�һ��Ԫ�ز�����
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param value ����Ԫ��
	* @return �ɹ���ʧ��
	*/
	std::string lpopString(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡ��β����valueֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey
	* @param value ������������bufferָ��
	* @param len ������������buffer�ĳ���
	* @return ����pop�����Ķ�����������
	*/
	BCLib::uint32 lpopBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, char*value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б������һ��Ԫ�ز�����
	* @param key �б���
	* @param value ����Ԫ��
	* @return �ɹ���ʧ��
	*/
	std::string rpopString(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б������һ��Ԫ�ز�����
	* @param key �б���
	* @param value ������������bufferָ��
	* @param len ������������buffer�ĳ���
	* @return �ɹ���ʧ��
	*/
	BCLib::uint32 rpopBin(const char *key, char*value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б��е�һ��Ԫ�ز�����
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param value ����Ԫ��
	* @return �ɹ���ʧ��
	*/
	std::string rpopString(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б��е�һ��Ԫ�ز�����
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param value ������������bufferָ��
	* @param len ������������buffer�ĳ���
	* @return �ɹ���ʧ��
	*/
	BCLib::uint32 rpopBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, char*value, const BCLib::int32 len = REDIS_READER_MAX_BUF, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ������б���indexλ���ϵ�Ԫ��ֵ
	* @param key �б���
	* @param index Ԫ������ Ҳ����ʹ�ø����±꣬�� -1 ��ʾ�б������һ��Ԫ�أ� -2 ��ʾ�б��ĵ����ڶ���Ԫ�أ��Դ����ơ�indexԽ�緵�ش���
	* @param value ����Ԫ��
	* @return �ɹ���ʧ��
	*/
	bool lsetString(const char *key, BCLib::int32 index, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ������б���indexλ���ϵ�Ԫ��ֵ
	* @param key �б���
	* @param index Ԫ������ Ҳ����ʹ�ø����±꣬�� -1 ��ʾ�б������һ��Ԫ�أ� -2 ��ʾ�б��ĵ����ڶ���Ԫ�أ��Դ����ơ�indexԽ�緵�ش���
	* @param value ���õĶ�������ָ��
	* @param len ����������С
	* @return �ɹ���ʧ��
	*/
	bool lsetBin(const char *key, BCLib::int32 index, char*value, BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ������б���indexλ���ϵ�Ԫ��ֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param index Ԫ������ Ҳ����ʹ�ø����±꣬�� -1 ��ʾ�б������һ��Ԫ�أ� -2 ��ʾ�б��ĵ����ڶ���Ԫ�أ��Դ����ơ�indexԽ�緵�ش���
	* @param value ����Ԫ��
	* @return �ɹ���ʧ��
	*/
	bool lsetString(const char *key, BCLib::uint64 uniqueid, const char *subkey, BCLib::int32 index, const char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ������б���indexλ���ϵ�Ԫ��ֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param index Ԫ������ Ҳ����ʹ�ø����±꣬�� -1 ��ʾ�б������һ��Ԫ�أ� -2 ��ʾ�б��ĵ����ڶ���Ԫ�أ��Դ����ơ�indexԽ�緵�ش���
	* @param value ���õĶ�������ָ��
	* @param len ����������С
	* @return �ɹ���ʧ��
	*/
	bool lsetBin(const char *key, BCLib::uint64 uniqueid, const char *subkey, BCLib::int32 index, char*value, BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���src�б������һ��Ԫ���Ƴ��������ӵ�tar�б���ͷ��
	* @param keyTar Ŀ���б���
	* @param keySrc Դ�б���
	* @return �ɹ���ʧ��
	*/
	bool rpoplpush(const char *keyTar, const char *keySrc, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���src�б������һ��Ԫ���Ƴ��������ӵ�tar�б���ͷ��
	* @param keyTar Ŀ���б���
	* @param uniqueidTar key�����µ� �б����
	* @param subkeyTar �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param keySrc Դ�б���
	* @param uniqueidSrc key�����µ� �б����
	* @param subkeySrc �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @return �ɹ���ʧ��
	*/
	bool rpoplpush(const char *keyTar, BCLib::uint64 uniqueidTar, const char *subkeyTar, const char *keySrc, BCLib::uint64 uniqueidSrc, const char *subkeySrc, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡredis��ָ��listȫ��ֵ
	* @param key �б���
	* @param mylist �����б�����
	* @return �ɹ���ʧ��
	*/
	bool lgetAllElement(const char *key, std::list<std::string>& mylist, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡredis��ָ��listȫ��ֵ
	* @param key �б���
	* @return const std::list<std::pair<char*, BCLib::uint32>*> ָ�룬std::pair<char*, BCLib::uint32> Ϊ<��������������>
	* ע�⣺���ָ���ȡ��Ҫ���������ݿ�����ȥ������һ�����������ݾͻᱻ�����д
	*/
	const std::list<std::pair<char*, BCLib::uint32>*> * lgetAllElement(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);


	/**
	* ���ܣ���ȡredis��ָ��listȫ��ֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param mylist �����б�����
	* @return �ɹ���ʧ��
	*/
	bool lgetAllElement(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::list<std::string>& mylist, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡredis��ָ��listȫ��ֵ
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @return const std::list<std::pair<char*, BCLib::uint32>*> ָ�룬std::pair<char*, BCLib::uint32> Ϊ<��������������> 
	* ע�⣺���ָ���ȡ��Ҫ���������ݿ�����ȥ������һ�����������ݾͻᱻ�����д
	*/
	const std::list<std::pair<char*, BCLib::uint32>*> *  lgetAllElement(const char *key, BCLib::uint64 uniqueid, const char*subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ��Ƴ��б�����valueֵ��ȵ�Ԫ��
	* @param key �б���
	* @param count �Ƴ�������count>0��ʾ�ӱ�ͷ���β������count < 0��ʾ��β���ͷ������count=0 ��ʾȫ���Ƴ�
	* @param value �ַ���
	* @return const std::list<std::pair<char*, BCLib::uint32>*> ָ�룬std::pair<char*, BCLib::uint32> Ϊ<��������������>
	* ע�⣺���ָ���ȡ��Ҫ���������ݿ�����ȥ������һ�����������ݾͻᱻ�����д
	*/
	bool lrem(const char *key, BCLib::int32 count, char*value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б�����valueֵ��ȵ�Ԫ��
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param count �Ƴ�������count>0��ʾ�ӱ�ͷ���β������count < 0��ʾ��β���ͷ������count=0 ��ʾȫ���Ƴ�
	* @param value �ַ���
	* @return const std::list<std::pair<char*, BCLib::uint32>*> ָ�룬std::pair<char*, BCLib::uint32> Ϊ<��������������>
	* ע�⣺���ָ���ȡ��Ҫ���������ݿ�����ȥ������һ�����������ݾͻᱻ�����д
	*/
	bool lrem(const char *key, BCLib::uint64 uniqueid, const char *subkey, BCLib::int32 count, char *value, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б�����valueֵ��ȵ�Ԫ��
	* @param key �б���
	* @param count �Ƴ�������count>0��ʾ�ӱ�ͷ���β������count < 0��ʾ��β���ͷ������count=0 ��ʾȫ���Ƴ�
	* @param value ����������ָ��
	* @param len ���������ݳ���
	* @return const std::list<std::pair<char*, BCLib::uint32>*> ָ�룬std::pair<char*, BCLib::uint32> Ϊ<��������������>
	* ע�⣺���ָ���ȡ��Ҫ���������ݿ�����ȥ������һ�����������ݾͻᱻ�����д
	*/
	bool lrem(const char *key, BCLib::int32 count, char *value, BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ��б�����valueֵ��ȵ�Ԫ��
	* @param key �б���
	* @param uniqueid key�����µ� �б����
	* @param subkey �б���µĶ������� ��key uniqueid ��ͬ���Ψһ���� key:[uniqueid]:subkey  ��ά�ṹ
	* @param count �Ƴ�������count>0��ʾ�ӱ�ͷ���β������count < 0��ʾ��β���ͷ������count=0 ��ʾȫ���Ƴ�
	* @param value ����������ָ��
	* @param len ���������ݳ���
	* @return const std::list<std::pair<char*, BCLib::uint32>*> ָ�룬std::pair<char*, BCLib::uint32> Ϊ<��������������>
	* ע�⣺���ָ���ȡ��Ҫ���������ݿ�����ȥ������һ�����������ݾͻᱻ�����д
	*/
	bool lrem(const char *key, BCLib::uint64 uniqueid, const char *subkey, BCLib::int32 count, char *value, BCLib::int32 len, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//ɾ�������б� ʹ�� delKey �ӿ�


	/*
	*set ���򼯺Ϸ�װ����������
	*/


	/**
	* ���ܣ�����һ��������Ա�����򼯺�
	* @param key ������
	* @param format ���ӵĳ�Ա�� format �����Ա ʹ�ÿո���� �� "member1 member2 member3"
	* @return �ɹ���ʧ��
	*/
	bool sadd(const char *key, EREDIS_CONTEXT_TYPE type, std::string members, ...);

	/**
	*���ܣ�����һ��������Ա�����򼯺�
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param format ���ӵĳ�Ա�� format �����Ա ʹ�ÿո���� �� "format format format"
	* @return �ɹ���ʧ��
	*/
	bool sadd(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE, std::string members, ...);

	/**
	* ���ܣ���ȡ�����ڳ�Ա����
	* @param key ������	
	* @return ��Ա����������ü��ϲ����ڷ���0
	*/
	BCLib::uint32 scard(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	*���ܣ���ȡ�����ڳ�Ա����
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @return ��Ա����������ü��ϲ����ڷ���0
	*/
	BCLib::uint32 scard(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��ж�member�Ƿ�Ϊ�ü��ϵĳ�Ա
	* @param key ������
	* @param memberΪ����Ա
	* @return true or false
	*/
	bool sismember(const char *key, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	*���ܣ��ж�member�Ƿ�Ϊ�ü��ϵĳ�Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param memberΪ����Ա
	* @return true or false
	*/
	bool sismember(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡset���г�Ա
	* @param key ������
	* @return true or false
	*/
	bool sgetAllMembers(const char *key,std::set<std::string> &mySet, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡset���г�Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @return true or false
	*/
	bool sgetAllMembers(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::set<std::string> &mySet, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�������һ�������Ա
	* @param key ������
	* @return true or false
	*/
	bool spop(const char *key, std::string &member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�������һ�������Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @return true or false
	*/
	bool spop(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::string &member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param members �����Ա�� �ո�ָ�
	* @return true or false
	*/
	bool sremove(const char *key, const std::string members, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param members �����Ա�� �ո�ָ�
	* @return true or false
	*/
	bool sremove(const char *key, BCLib::uint64 uniqueid, const char *subkey, const std::string members, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ָ����Ա member Ԫ�ش� source �����ƶ��� destination ����
	* @param disKey Ŀ�ļ�����
	* @param srcKey Դ������
	* @param member �ƶ��ĳ�Ա
	* @return �����ԱԪ�ر��ɹ��Ƴ�������true �����ԱԪ�ز���src���ϵĳ�Ա�򼯺ϲ����ڣ�����û���κβ����� destination ����ִ�У���ô���� false 
	*/
	bool smove(const char *disKey, const char *srcKey, const std::string member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param disKey ������
	* @param disUniqueid key�����µ��Ӽ��ͱ��
	* @param disSubkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param srcKey ������
	* @param srcUniqueid key�����µ��Ӽ��ͱ��
	* @param srcSubkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param member �ƶ��ĳ�Ա
	* @return �����ԱԪ�ر��ɹ��Ƴ�������true �����ԱԪ�ز���src���ϵĳ�Ա�򼯺ϲ����ڣ�����û���κβ����� destination ����ִ�У���ô���� false 
	*/
	bool smove(const char *disKey, BCLib::uint64 disUniqueid, const char *disSubkey, const char *srcKey, BCLib::uint64 srcUniqueid, const char *srcSubkey, const std::string member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���ȡ�������и������ϵĽ���,�����ڵļ��� key ����Ϊ�ռ�
	* @param keys ������ �����Ա ʹ�ÿո���� �� "key1 key2 key3" / "key:[uniqueid]:subkey1 key:[uniqueid]:subkey2 key:[uniqueid]:subkey3"  ��ά�ṹ
	* @return �ɹ���ʧ��
	*/
	bool sinter(std::set<std::string> &set, EREDIS_CONTEXT_TYPE type, std::string keys, ...);
	
	//ͬ��  by bhu
	bool sinter(std::set<std::string> &set, const std::vector<std::string> &keys, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);


	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param mySet ���ؼ���
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ���ر�������ʱ�α�
	*/
	BCLib::uint64 sscan(const char *key, const char *matchKey, std::set<std::string> &mySet, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param mySet ���ؼ���
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ���ر�������ʱ�α�
	*/
	BCLib::uint64 sscan(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *matchKey, std::set<std::string> &mySet, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/*
	*zset ���򼯺Ϸ�װ����������
	*/

	/**
	* ���ܣ����ӳ�Ա�����򼯺�
	* @param key ������
	* @param scores һ���������ϳ�Ա��Ӧ�ķ�ֵ
	* @param members һ���������ϳ�Ա
	* @return true or false
	*/
	bool zadd(const char *key, const std::vector<double> &scores, const std::vector<std::string> &members, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����ӳ�Ա�����򼯺�
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param scores һ���������ϳ�Ա��Ӧ�ķ�ֵ
	* @param members һ���������ϳ�Ա
	* @return true or false
	*/
	bool zadd(const char *key, BCLib::uint64 uniqueid, const char *subkey, const std::vector<double> &scores, const std::vector<std::string> &members, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//���ؽ����Ԫ�ظ���

	/**
	* ���ܣ����ؽ����Ԫ�ظ���
	* @param key ������
	* @return �����ڳ�Ա���� ���ϲ����ڷ���0
	*/
	BCLib::uint32  zcard(const char *key, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����ؽ����Ԫ�ظ���
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @return �����ڳ�Ա���� ���ϲ����ڷ���0
	*/
	BCLib::uint32  zcard(const char *key, BCLib::uint64 uniqueid, const char *subkey, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	

	//���ؽ����ָ��member������/���� score ��->�� ����/������0��ʼ
	/**
	* ���ܣ����ؼ�����ָ����Ա������ score ��->�� ������0��ʼ
	* @param key ������
	* @param member ��ѯ������ָ����Ա
	* @return ���ؼ�����ָ����Ա������,�����ڷ��� -1
	*/
	BCLib::int32 zrank(const char *key, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����ؽ����ָ����Ա������ score ��->�� ������0��ʼ
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param member ��ѯ������ָ����Ա
	* @return ���ؼ�����ָ����Ա������,�����ڷ��� -1
	*/
	BCLib::int32 zrank(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//���ؼ�����ָ����Ա������ score ��->�� ������0��ʼ

	/**
	* ���ܣ����ؼ�����ָ����Ա������ score ��->�� ������0��ʼ
	* @param key ������
	* @param member ��ѯ������ָ����Ա
	* @return ���ؼ�����ָ����Ա������,�����ڷ��� -1
	*/
	BCLib::int32 zrevRank(const char *key, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����ؽ����ָ����Ա������ score ��->�� ������0��ʼ
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param member ��ѯ������ָ����Ա
	* @return ���ؼ�����ָ����Ա������,�����ڷ��� -1
	*/
	BCLib::int32 zrevRank(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ��Ƴ�������һ����Ա������Ա
	* @param key ������
	* @param members �Ƴ�һ��������Ա��vector
	* @return �Ƴ��ɹ���ʧ��
	*/
	bool zrem(const char *key, std::vector<std::string> &members, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ��Ƴ�������һ����Ա������Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param members �Ƴ�һ��������Ա��vector
	* @return �Ƴ��ɹ���ʧ��
	*/
	bool zrem(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::vector<std::string> &members, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	//���ؼ�����ָ��������Χ�ڵ����г�Ա score ��->�� 0 ~-1 ȫ��ȡ��
	/**
	* ���ܣ�ȡ��������ָ��������Χ�ڵ����г�Ա(�������ɵ�->��) Ĭ��ȫ��ȡ�� 
	* @param key ������
	* @param members ��ȡ���ĳ�Ա-������vector
	* @param startIndex ָ���Ŀ�ʼ��ȡ����
	* @param stopIndex ָ���Ľ�����ȡ���� -1��ʾ������һ����-2��ʾ�����ڶ���,...
	* @return �ɹ���ʧ��
	*/
	bool zrange(const char *key, std::vector<std::pair<std::string, double>> &members, BCLib::int32 startIndex = 0, BCLib::int32 stopIndex = -1, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ�ȡ��������ָ��������Χ�ڵ����г�Ա(�������ɵ�->��) Ĭ��ȫ��ȡ��
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param members ��ȡ���ĳ�Ա-������vector
	* @param startIndex ָ���Ŀ�ʼ��ȡ����
	* @param stopIndex ָ���Ľ�����ȡ���� -1��ʾ������һ����-2��ʾ�����ڶ���,...
	* @return �ɹ���ʧ��
	*/
	bool zrange(const char *key, const BCLib::uint64 uniqueid, const char *subkey, std::vector<std::pair<std::string, double>> &members, BCLib::int32 startIndex = 0,  BCLib::int32 stopIndex = -1, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//���ؼ�����ָ��������Χ�ڵ����г�Ա score ��->�� 0 ~-1 ȫ��ȡ��
	/**
	* ���ܣ�ȡ��������ָ��������Χ�ڵ����г�Ա(�������ɸ�->��) Ĭ��ȫ��ȡ��
	* @param key ������
	* @param members ��ȡ����<��Ա-����>��vector
	* @param startIndex ָ���Ŀ�ʼ��ȡ����
	* @param stopIndex ָ���Ľ�����ȡ���� -1��ʾ������һ����-2��ʾ�����ڶ���,...
	* @return �ɹ���ʧ��
	*/
	bool zrevRange(const char *key, std::vector<std::pair<std::string, double>> &members, BCLib::int32 startIndex = 0, BCLib::int32 stopIndex = -1, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);// ZREVRANGE salary 0 - 1 WITHSCORES
	
	/**
	* ���ܣ�ȡ��������ָ��������Χ�ڵ����г�Ա(�������ɸ�->��) Ĭ��ȫ��ȡ��
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param members ��ȡ����<��Ա-����>��vector
	* @param startIndex ָ���Ŀ�ʼ��ȡ����
	* @param stopIndex ָ���Ľ�����ȡ���� -1��ʾ������һ����-2��ʾ�����ڶ���,...
	* @return �ɹ���ʧ��
	*/
	bool zrevRange(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::vector<std::pair<std::string, double>> &members, BCLib::int32 startIndex = 0, BCLib::int32 stopIndex = -1, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	//����ָ����Ա�ķ���ֵ

	/**
	* ���ܣ�ȡ��������ָ����Ա�ķ�ֵ
	* @param key ������
	* @param members ����ָ����Ա
	* @return ����ָ����Ա�ķ�ֵ û�иó�Ա����0
	*/
	double zscore(const char *key, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ�ȡ��������ָ����Ա�ķ�ֵ
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param members ����ָ����Ա
	* @return ����ָ����Ա�ķ�ֵ û�иó�Ա����0
	*/
	double zscore(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *member, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	//ZSCORE key member

	//ɾ��������ָ��������������г�Ա 0 ~-1 ɾ������

	/**
	* ���ܣ�ɾ��������ָ��������������г�Ա Ĭ��ɾ�����������г�Ա
	* @param key ������
	* @param startIndex �������俪ʼ����
	* @param stopIndex ��������������� -1��ʾ������һ����-2��ʾ�����ڶ���,...
	* @return �ɹ���ʧ�� 
	*/
	bool zremRangeByRank(const char *key, BCLib::int32 startIndex = 0,BCLib::int32 stopIndex = -1, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ�ɾ��������ָ��������������г�Ա Ĭ��ɾ�����������г�Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param startIndex �������俪ʼ����
	* @param stopIndex ��������������� -1��ʾ������һ����-2��ʾ�����ڶ���,...
	* @return �ɹ���ʧ��
	*/
	bool zremRangeByRank(const char *key, BCLib::uint64 uniqueid, const char *subkey, BCLib::int32 startIndex = 0, BCLib::int32 stopIndex = -1, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ���score����increment
	* @param key ������
	* @param member ���������ĳ�Ա
	* @param increment ����������
	* @return �ɹ���ʧ�� member ���ڼ����ڷ���ʧ��
	*/
	bool zincrby(const char *key, const char *member, double increment, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ���score����increment
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param member ���������ĳ�Ա
	* @param increment ����������
	* @return �ɹ���ʧ�� member ���ڼ����ڷ���ʧ��
	*/
	bool zincrby(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *member, double increment, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����򼯺���ָ����������ĳ�Ա����[min,max]
	* @param key ������
	* @param min ������������
	* @param max ������������
	* @return ���س�Ա���� key�����ڷ���-1
	*/
	BCLib::int32 zcount(const char *key, double min, double max, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����򼯺���ָ����������ĳ�Ա����[min,max]
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param min ������������
	* @param max ������������
	* @return ���س�Ա���� key�����ڷ���-1
	*/
	BCLib::int32 zcount(const char *key, BCLib::uint64 uniqueid, const char *subkey, double min, double max, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����򼯺���ָ����������ĳ�Ա�б� ���򼯳�Ա������ֵ����(��С����)��������
	* @param key ������
	* @param minStr ������������ ������:"5.0"��������:"(5.0" �� ������:"-inf"
	* @param maxStr ������������ ������:"9.0"��������:"(9.0" �� ������:"+inf"	
	* @return �ɹ���ʧ�� member ���ڼ����ڷ���ʧ��
	*/
	bool zrangeByScore(const char *key, std::vector<std::pair<std::string, double>> &members, const char *minStr, const char *maxStr, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ����򼯺���ָ����������ĳ�Ա�б� ���򼯳�Ա������ֵ����(��С����)��������
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param minStr ������������ ������:"5.0"��������:"(5.0" �� ������:"-inf"
	* @param maxStr ������������ ������:"9.0"��������:"(9.0" �� ������:"+inf"	
	* @return �ɹ���ʧ�� member ���ڼ����ڷ���ʧ��
	*/
	bool zrangeByScore(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::vector<std::pair<std::string, double>> &members, const char *minStr, const char *maxStr, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ�������ָ�����������ڵ����еĳ�Ա ���򼯳�Ա������ֵ�ݼ�(�Ӵ�С)�Ĵ�������
	* @param key ������
	* @param maxStr ������������ ������:"9.0"��������:"(9.0" �� ������:"+inf"
	* @param minStr ������������ ������:"5.0"��������:"(5.0" �� ������:"-inf"
	* @return �ɹ���ʧ�� member ���ڼ����ڷ���ʧ��
	*/
	bool zrevRangeByScore(const char *key, std::vector<std::pair<std::string, double>> &members, const char *maxStr, const char *minStr, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	/**
	* ���ܣ�������ָ�����������ڵ����еĳ�Ա ���򼯳�Ա������ֵ�ݼ�(�Ӵ�С)�Ĵ�������
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param maxStr ������������ ������:"9.0"��������:"(9.0" �� ������:"+inf"
	* @param minStr ������������ ������:"5.0"��������:"(5.0" �� ������:"-inf"
	* @return �ɹ���ʧ�� member ���ڼ����ڷ���ʧ��
	*/
	bool zrevRangeByScore(const char *key, BCLib::uint64 uniqueid, const char *subkey, std::vector<std::pair<std::string, double>> &members, const char *maxStr, const char *minStr, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param members ���ؼ���
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ���ر�������ʱ�α�
	*/
	BCLib::uint64 zscan(const char *key, const char *matchKey, std::vector<std::pair<std::string, double>> &members, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);
	
	/**
	* ���ܣ��Ƴ�һ����Ա������Ա
	* @param key ������
	* @param uniqueid key�����µ��Ӽ��ͱ��
	* @param subkey �Ӽ��ͱ���µĶ����Ӽ����� ��key uniqueid ��ͬ���Ψһ������ key:[uniqueid]:subkey  ��ά�ṹ
	* @param matchKey ƥ���ַ�����ƥ�����ͬscan
	* @param members ���ؼ���
	* @param start ������ʼ�α�
	* @param count ������������󷵻س�Ա��
	* @param type ��ʹ�õ�redis������
	* @return ���ر�������ʱ�α�
	*/
	BCLib::uint64 zscan(const char *key, BCLib::uint64 uniqueid, const char *subkey, const char *matchKey, std::vector<std::pair<std::string, double>> &members, BCLib::uint64 start = 0, BCLib::uint64 count = 50, EREDIS_CONTEXT_TYPE type = E_REDIS_SERVERTYPE_LOGIC);

	CRedLock *getRedLock() { return m_pRedLock; }
protected:
	void destoryTempList()
	{
		for (std::list<std::pair<char*, BCLib::uint32>*>::iterator iter = m_list.begin(); iter != m_list.end(); iter++)
		{
			if ((*iter) == NULL)
			{
				continue;
			}
			delete ((*iter)->first);
			(*iter)->first = NULL;
			(*iter)->second = 0;

			delete (*iter);
			(*iter) = NULL;
		}
		m_list.clear();
	}
	void destoryTempMap()
	{
		for (std::map<std::string, std::pair<char*, BCLib::uint32>*>::iterator iter = m_map.begin(); iter != m_map.end(); iter++)
		{
			std::pair<char*, BCLib::uint32>*p = iter->second;
			if (p == NULL)
			{
				continue;
			}
			//char *pChar = p->first;		
			delete (p->first);
			//pChar = NULL;
			p->first = NULL;
			p->second = 0;

			delete p;
			p = NULL;
			iter->second = NULL;
			
		}
		m_map.clear();
	}
	void destoryRedLock()
	{
		if (m_pRedLock != NULL)
		{
			delete m_pRedLock;
		}
		return;
	}
private:
	
	CRedLock  *m_pRedLock;

	redisContext *m_redisContext;
	redisReply *m_redisReply;

	std::unordered_map<BCLib::uint16, REDIS_NODE> m_redisContextMap;

	std::list<std::pair<char *, BCLib::uint32>*> m_list;

	std::map<std::string, std::pair<char *, BCLib::uint32>*> m_map;
};

}//Redis
}//MWLib

#endif//__MWLIB_REDIS_REDISSYSTEM_H__