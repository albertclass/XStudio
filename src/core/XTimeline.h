#ifndef __TIMELINE_H__
#define __TIMELINE_H__

///
/// \brief 活动信息
///
struct XActive
{
	/// 活动的执行时间
	timespan relative;
	/// 活动的持续时间
	timespan duration;

	/// 活动开始时执行
	virtual xgc_bool onStart( datetime now ) = 0;

	/// 活动更新
	virtual xgc_void onUpdate( timespan relative ) = 0;

	/// 活动结束时执行
	virtual xgc_void onClose() = 0;
};

///
/// \brief 时间线类，用于管理时间线内的活动
///
class XTimeline : public XObject
{
private:
	DECLARE_XCLASS();

	/// 定时器句柄
	timer_h  mTimerHandle;

	/// 下一个检查点
	xgc_size mNext;

	/// 开始时间
	datetime mStart;
	/// 更新间隔
	timespan mInterval;

	/// 需要更新的对象
	xgc_vector< XActive* > mUpdates;

	/// 帧信息
	xgc_vector< XActive* > mActives;
public:
	///
	/// \brief 时间线构造
	/// \author albert.xu
	/// \date 2017/10/16
	///
	XTimeline( timespan interval );

	///
	/// \brief 时间线析构
	/// \author albert.xu
	/// \date 2017/10/16
	///
	virtual ~XTimeline();

	///
	/// \brief 添加关键帧
	/// \author albert.xu
	/// \date 2017/10/16
	///
	xgc_void AddActive( XActive *pActive );

	///
	/// \brief 运行时间线
	/// \author albert.xu
	/// \date 2017/10/16
	///
	xgc_bool Start( datetime start );

protected:
	///
	/// \brief 更新时间线状态
	/// \author albert.xu
	/// \date 2017/10/17
	///
	xgc_void Update();
};
#endif // __TIMELINE_H__