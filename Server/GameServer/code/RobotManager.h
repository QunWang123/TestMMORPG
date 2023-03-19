#ifndef ____ROBOTMANAGER
#define ____ROBOTMANAGER

#include "RobotData.h"
#include "IContainer.h"


namespace app
{
	class RobotManager
	{
	private:
		HashArray<S_ROBOT>* robots_;
	public:
		s32 count;
		s32 emptyPos;
		s32 checkPos;
		s32 checkCount;
	public:
		RobotManager();
		~RobotManager();

		void init();
		void update();
		void reset();
		void createRobot();

		s32	  createRobot_Copy(const u32 mapid, const s32 layer);
		void  clearRobot_Copy(const u32 mapid, const s32 layer);
	public:
		inline s32 getCount() { return robots_->length; };
		inline S_ROBOT* Robot(const u32 index)
		{
			if (index >= robots_->length) return nullptr;
			return robots_->Value(index);
		}

		S_ROBOT* findRobot(const u32 index);
		S_ROBOT* findRobot(const u32 index, const s32 layer);
		S_ROBOT* findRobot(S_RECT_BASE* edge, const u32 index, const s32 layer, const u32 mapid);
		S_ROBOT* findEmpty();
	};

	extern RobotManager* __RobotManager;
}


#endif // ____ROBOTMANAGER
