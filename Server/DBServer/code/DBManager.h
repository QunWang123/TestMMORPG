#ifndef  ____DBMANAGER_H
#define  ____DBMANAGER_H

#include <concurrent_queue.h>
#include "CommandData.h"
#include "DBConnetor.h"
#include "DBBuffer.h"
namespace db
{
	//���ݿ������
	class DBManager
	{
	public:
		DBManager();
		~DBManager();
	public:

		S_DBXML*  __GameDBXML;
		S_DBXML*  __AccountXML;

		 
		Concurrency::concurrent_queue<DBBuffer*>  __logicBuffs;//��ȫ�Ĳ�������
		ThreadSafePool  __poolBuffs; //buff���ջ����


		//mysql����
		std::vector<DBConnetor* > DBRead;//��Ҷ�ȡ���ݿ����
		std::vector<DBConnetor* > DBWrite;//���д���ݿ����

		DBConnetor* GetDBSource(int type);
		DBBuffer* PopPool();
		int     LoadDBXML(const char* filename);
		void    InitDB();
		void    update();
		void    PushToMainThread(DBBuffer * buffer);
		


	
		void	Thread_BeginAccount();
		void	Thread_UserRead(DBBuffer* buff);
		void	Thread_UserWrite(DBBuffer* buff);
		
		void	Read_200(DBBuffer* buff, DBConnetor* db);			// ѡ���ɫ������Ϸ
		void	Write_30(DBBuffer* _buff, DBConnetor* db);			// ���µ�¼ʱ��
		void    Write_UserLoginTime(DBBuffer* buff, DBConnetor* db);
		void    Write_UserRegister(DBBuffer* _buff, DBConnetor* db);
		void    Write_RoleData(DBBuffer* _buff, DBConnetor* db);
		void	Write_RoleBase(DBBuffer* _buff, DBConnetor* db);
		void	Write_100(DBBuffer* _buff, DBConnetor* db);
		void	Write_300(DBBuffer* _buff, DBConnetor* db);
		void	Write_400(DBBuffer* _buff, DBConnetor* db);

		// void	deserializeBinary(MySqlConnetor* mysql, app::S_USER_ROLE* role);
		
	};
	extern DBManager* __DBManager;
	extern std::string serializeString(app::S_ROLE_STAND_BAG* bag);
	extern std::string serializeString(app::S_ROLE_STAND_COMBAT* combat);
	// extern std::string serializeString(app::S_ROLE_STAND_WAREHOUSE* wh);
	extern void deserializeBinary(MySqlConnetor* mysql, app::S_USER_ROLE* role);
	extern void deserializeBinary_bag(std::string& value, app::S_ROLE_STAND_BAG* bag);
	// extern void deserializeBinary(std::string& value, app::S_ROLE_STAND_WAREHOUSE* wh);
}
#endif