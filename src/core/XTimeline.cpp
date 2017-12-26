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

xgc_void XTimeline::AddActive( XActive * pActive )
{
	if( INVALID_TIMER_HANDLE == mTimerHandle )
	{
		// 直接添加，运行时排序
		mActives.push_back( pActive );
	}
	else
	{
		// 运行时新增活动
		auto _Pred = []( const XActive* pAct1, const XActive* pAct2 ){
			return pAct1->relative + pAct1->duration < pAct2->relative + pAct2->duration;
		};

		auto it = std::upper_bound( mActives.begin(), mActives.end(), pActive, _Pred );
	
		if( it - mActives.begin() > (int)mNext )
			mActives.insert( it, pActive );
	}
}

///
/// \brief 运行时间线
/// \author albert.xu
/// \date 2017/10/16
///

xgc_bool XTimeline::Start( datetime start )
{
	std::sort( mActives.begin(), mActives.end(),
		[]( const XActive *frame_l, const XActive *frame_r ){
		return frame_l->relative < frame_r->relative;
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
	XTimelineEvent evt;
	evt.current = datetime::now();
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
		XTimelineEvent evt;
		evt.current = datetime::now();
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
	XGC_ASSERT_RETURN( mNext >= mActives.size(), XGC_NONE );
	datetime now = datetime::now();

	// 根据结束时间排序，此处排序未考虑 relative 和 duration 改变的情况
	// WRAN 若值改变，可能导致一个不正确的堆出现。
	auto _Pred = []( const XActive* pAct1, const XActive* pAct2 ){
		return pAct1->relative + pAct1->duration < pAct2->relative + pAct2->duration;
	};

	// 检查是否有需要更新的活动
	while( mStart + mActives[mNext]->relative >= now )
	{
		auto pAct = mActives[mNext];

		// 活动开始执行，活动指针进入更新队列。
		mUpdates.push_back( pAct );
		// 将最后一个对象放入堆中
		std::push_heap( mUpdates.begin(), mUpdates.end(), _Pred );
		// 通知活动开始
		pAct->onStart( now );

		++mNext;
	}

	// 开始更新活动
	while( mUpdates.size() )
	{
		auto pAct = mUpdates[0];
		XGC_ASSERT( pAct );
		// 更新活动
		pAct->onUpdate( now - mStart );

		// 检查活动是否结束
		if( now < mStart + pAct->relative + pAct->duration )
			break; // 所有该结束的活动都结束了，则跳出。

		// 活动对象生命周期已结束
		std::pop_heap( mUpdates.begin(), mUpdates.end(), _Pred );
		// 删除最后一个元素
		mUpdates.pop_back();
		// 通知活动关闭
		pAct->onClose();
	}

	// 提交事件
	XTimelineEvent evt;
	evt.current = now;
	evt.relative = now - mStart;

	if( mUpdates.empty() && mNext == mActives.size() )
	{
		// 提交事件
		EmmitEvent( evt.cast, evt_tline_finish );
	}
	else
	{
		EmmitEvent( evt.cast, evt_tline_update );
	}
}
