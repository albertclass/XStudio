#include "XHeader.h"
#include "XTimeline.h"

IMPLEMENT_XCLASS_BEGIN( XTimeline, XObject )
IMPLEMENT_XCLASS_END();

XTimeline::XTimeline( timespan interval )
	: mTimerHandle( INVALID_OBJECT_ID )
	, mNext( 0 )
	, mInterval( interval )
{

}

XTimeline::~XTimeline()
{
	Stop();
}

///
/// \brief 添加关键帧
/// \author albert.xu
/// \date 2017/10/16
///

xgc_void XTimeline::AddActive( timespan start, XActive *pActive, timespan duration /*= 0*/ )
{
	Action act;
	act.relative = start;
	act.duration = duration;
	act.active = pActive;

	if( INVALID_TIMER_HANDLE == mTimerHandle )
	{
		// 直接添加，运行时排序
		mActives.push_back( act );
	}
	else
	{
		// 运行时新增活动
		auto _Pred = []( const Action& act1, const Action& act2 ){
			return act1.relative + act1.duration < act2.relative + act2.duration;
		};

		auto it = std::upper_bound( mActives.begin(), mActives.end(), act, _Pred );
	
		if( it - mActives.begin() > (int)mNext )
			mActives.insert( it, act );
	}
}

///
/// \brief 运行时间线
/// \author albert.xu
/// \date 2017/10/16
///

xgc_bool XTimeline::Start( datetime start )
{
	// 先根据起始时间排序
	std::sort( mActives.begin(), mActives.end(),
		[]( const Action& act_l, const Action& act_r ){
		return act_l.relative < act_r.relative;
	} );

	// 时间线开始时间
	mStart = start;

	// 插入定时器
	mTimerHandle = getTimer().insert(
		std::bind( &XTimeline::Update, this ),
		mStart,
		DURATION_FOREVER,
		timer_event::e_rept,
		mInterval.to_millisecnods() );

	// 提交事件
	Event evt;
	evt.current = datetime::now();
	evt.relative = 0;
	EmmitEvent( evt.cast, evt_tline_start );

	return true;
}

///
/// \brief 停止时间线
/// \author albert.xu
/// \date 2017/11/03
///

xgc_void XTimeline::Stop()
{
	if( INVALID_TIMER_HANDLE != mTimerHandle )
	{
		getTimer().remove( mTimerHandle );
		mTimerHandle = INVALID_TIMER_HANDLE;

		// 提交事件
		Event evt;
		evt.current = datetime::now();
		evt.relative = datetime::now() - mStart;

		EmmitEvent( evt.cast, evt_tline_cancel );
	}

	mUpdates.clear();
}

///
/// \brief 更新时间线状态
/// \author albert.xu
/// \date 2017/10/17
///

xgc_void XTimeline::Update()
{
	datetime now = datetime::now();

	// 根据结束时间排序，此处排序未考虑 relative 和 duration 改变的情况
	// WRAN 若值改变，可能导致一个不正确的堆出现。
	auto _Pred = []( const Action& act1, const Action& act2 ){
		return act1.relative + act1.duration < act2.relative + act2.duration;
	};

	// 检查是否有新开始的活动
	while( mNext < mActives.size() && mStart + mActives[mNext].relative >= now )
	{
		auto &act = mActives[mNext];

		// 活动开始执行，活动指针进入更新队列。
		mUpdates.push_back( act );
		// 将最后一个对象放入堆中
		std::push_heap( mUpdates.begin(), mUpdates.end(), _Pred );
		// 通知活动开始
		act.active->onStart( now );

		++mNext;
	}

	// 检查是否有结束的活动
	while( mUpdates.size() )
	{
		auto &act = mUpdates[0];

		// 检查活动是否结束
		if( now >= mStart + act.relative + act.duration )
			break; // 该活动都结束了，则跳出。

		// 活动对象生命周期已结束
		std::pop_heap( mUpdates.begin(), mUpdates.end(), _Pred );

		// 通知活动关闭
		act.active->onClose();

		// 删除最后一个元素
		mUpdates.pop_back();
	}

	// 更新所有仍在更新列表中的活动
	for( auto &act : mUpdates )
	{
		// 更新活动
		act.active->onUpdate( now - mStart );
	}

	// 提交事件
	Event evt;
	evt.current = now;
	evt.relative = now - mStart;

	EmmitEvent( evt.cast, evt_tline_update );

	if( mUpdates.empty() && mNext == mActives.size() )
	{
		// 提交事件
		EmmitEvent( evt.cast, evt_tline_finish );
	}
}
