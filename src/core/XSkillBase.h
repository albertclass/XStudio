#pragma once
using namespace XGC;

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

namespace XGC
{
	//////////////////////////////////////////////////////////////////////////
	// 技能属性索引
	extern "C"
	{
		extern CORE_API xAttrIndex	attrSkillIndex;		  // 技能配置索引
		extern CORE_API xAttrIndex  attrSkillMainId;      // 技能主ID
		extern CORE_API xAttrIndex  attrSkillUseTime;     // 吟唱时长
		extern CORE_API xAttrIndex  attrSkillKeepTime;    // 持续时长
		extern CORE_API xAttrIndex  attrSkillCoolDown;    // 冷却时长
		extern CORE_API xAttrIndex  attrSkillRepeat;      // 重复次数 
		/*extern CORE_API xAttrIndex  attrSkillRepeatDelay; // 重复时间间隔*/

		extern CORE_API xAttrIndex  attrSkillCurrCoolTs;  // 当前冷却时间
		extern CORE_API xAttrIndex  attrSkillCurrRepeat;  // 当前重复次数（技能分段攻击使用）
	}
	enum EN_CASTING_STATUS
	{
		en_casting_init,   // 初始
		en_casting_in,     // 吟唱中
		en_casting_out,    // 吟唱结束
	};
	class XGameObject;
	class CORE_API XSkillBase : public XObject
	{
		DECLARE_XCLASS();
	public:
		XSkillBase();
		~XSkillBase( void );

		//---------------------------------------------------//
		// [12/29/2010 Albert]
		// Description:	开始使用技能 
		//---------------------------------------------------//
		xgc_bool CastSkill();

		//---------------------------------------------------//
		// [12/3/2010 Albert]
		// Description:	攻击目标
		// return : 技能释放需要使用的时间
		//---------------------------------------------------//
		xgc_void Attack( xObject hTarget );

		//---------------------------------------------------//
		// [12/6/2010 Albert]
		// Description:	设置技能释放状态 
		//---------------------------------------------------//
		xgc_void MarkCasting();

		//---------------------------------------------------//
		// [12/11/2010 Albert]
		// Description:	设置技能冷却状态 
		//---------------------------------------------------//
		xgc_void MarkCooldown();

		// ------------------------------------------------ //
		// [5/22/2014 jianglei.kinly]
		// 设置公共CD
		// ------------------------------------------------ //
		virtual xgc_void MarkPublicCD() = 0;

		//---------------------------------------------------//
		// [12/6/2010 Albert]
		// Description:	技能是否在吟唱 
		//---------------------------------------------------//
		xgc_bool IsCasting()const
		{
			return mCasting != INVALID_TIMER_HANDLE && mCastingStatus == en_casting_in;
		}
		//---------------------------------------------------//
		// [12/6/2010 Albert]
		// Description:	技能是否吟唱结束
		//---------------------------------------------------//
		xgc_bool IsCastingEnd()const
		{
			return mCasting == INVALID_TIMER_HANDLE && mCastingStatus == en_casting_out;
		}

		///
		/// 模糊的判断聚气是否结束 500ms
		/// [8/17/2014] create by jianglei.kinly
		///
		xgc_bool IsCastingEndFuzzy()const;

		// ------------------------------------------------ //
		// [3/12/2014 jianglei.kinly]
		// 判断是否是吟唱初始状态
		// ------------------------------------------------ //
		xgc_bool IsCastingInit()const
		{
			return mCastingStatus == en_casting_init;
		}

		//---------------------------------------------------//
		// [12/11/2010 Albert]
		// Description:	技能是否冷却中 
		//---------------------------------------------------//
		xgc_bool IsCooldown()const
		{
			return mCooldown != INVALID_TIMER_HANDLE;
		}

		///
		/// 技能是否冷却中 （模糊）300ms
		/// [8/17/2014] create by jianglei.kinly
		///
		xgc_bool IsCooldownFuzzy()const;

		/////
		/// [12/27/2010 Albert]
		/// Description:	技能是否结束
		/////
		xgc_bool IsOver()const
		{
			return mLifetime == INVALID_TIMER_HANDLE && mAttack == INVALID_TIMER_HANDLE;
		}

		/////
		/// [12/6/2010 Albert]
		/// Description : 清除吟唱状态 
		/// bTriger     : 是否执行操作
		/////
		xgc_void ClearCasting( bool bTrigger = false );

		/////
		/// [12/6/2010 Albert]
		/// Description:	清除冷却状态 
		/////
		xgc_real32 ClearCooldown( bool bTrigger = false, bool bSendMsg = false );

		/////
		/// [12/6/2010 Albert]
		/// Description:	清除释放状态
		/////
		xgc_void ClearLifetime( xgc_bool isNormalEnd, bool bTrigger = false );

		/// ------------------------------------------------ //
		/// [1/24/2014 jianglei.kinly]
		/// 设定技能产生效果中的状态
		/// ------------------------------------------------ //
		xgc_void MarkAttack( timer_h handle );
		/////
		/// [12/6/2010 Albert]
		/// Description:	重置技能释放状态 
		/////
		xgc_void ResetCasting();

		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// 替换之前的Trigger SkillBegin
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillBegin() = 0;

		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// 替换之前的Trigger SkillAttackStep
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillAttackStep( xObject hTarget ) = 0;

		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// 替换之前的Trigger SkillCasting
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillCasting() = 0;
		
		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// 替换之前的Trigger SkillCooldown
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillCooldown( bool bSendMsg ) = 0;
		
		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// 替换之前的Trigger SkillOver
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillOver( xgc_bool isNormalEnd ) = 0;
		
		/// ------------------------------------------------ //
		/// [4/14/2014 jianglei.kinly]
		/// 替换之前的Trigger SkillAttack
		/// ------------------------------------------------ //
		virtual xgc_bool TriggerSkillAttack() = 0;

		///
		/// 聚气结束
		/// [7/4/2014] create by jianglei.kinly
		///
		virtual xgc_bool TriggerSkillEndCasting( xgc_bool isNormalEnd ) = 0;

		///
		/// 获得剩余冷却时间
		/// [7/9/2014] create by jianglei.kinly
		///
		xgc_uint32 GetLastCoolTs();

		///
		/// 获取当前repeat应该等待的时间
		/// [10/16/2014] create by jianglei.kinly
		///
		virtual xgc_real32 GetTriggerDelay( xgc_uint32 repeat ) = 0;
	protected:
		/// ------------------------------------------------ //
		/// [3/11/2014 jianglei.kinly]
		/// 结束聚气，置标志位，等待客户端发聚气结束的消息
		/// ------------------------------------------------ //
		xgc_void EndCasting( timer_h handle );

		/////
		/// [12/11/2010 Albert]
		/// Description:	重置技能冷却状态 
		/////
		xgc_void ResetCooldown( timer_h handle );

		/////
		/// [12/6/2010 Albert]
		/// Description:	重置技能释放状态 
		/////
		xgc_void ResetLifetime( timer_h handle );

	protected:
		/////
		/// 增加子节点前调用
		/// [8/3/2009 Albert]
		/// return :		true - 确认增加子节点, false - 子节点被否决,添加节点失败.
		/////
		virtual xgc_bool PreAddChild( XObject* pChild, xgc_lpcvoid lpContext ) { return true; }

		/////
		/// 加入子节点后调用
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnAddChild( XObject* pChild, xgc_lpcvoid lpContext ) {}

		/////
		/// [8/3/2009 Albert]
		/// Description:	增加子节点前调用
		/// return :		true - 确认移除子节点, false - 子节点被否决,移除子节点失败.
		//---------------------------------------------------//
		virtual xgc_bool PreRemoveChild( XObject* pChild, xgc_bool bRelease ) { return bRelease; }

		/////
		/// [8/3/2009 Albert]
		/// Description:	删除子节点后调用,此时对象尚未被删除
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild, xgc_bool bRelease ) {}

		///
		/// 销毁对象时调用 
		/// [6/9/2014] create by albert.xu
		///
		virtual xgc_void OnDestroy() {}
	public:
	private:
		timer_h mCasting;   ///< 技能吟唱
		timer_h mCooldown;  ///< 技能冷却
		timer_h mLifetime;  ///< 技能持续
		timer_h mAttack;    ///< 技能释放中（重复）,mLifetime结束的时候mAttack需要同时结束

		xgc_int8 mCastingStatus;  // 技能吟唱的状态
	public:
		Json::Value mJsonRoot;
	};
}
