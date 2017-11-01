#ifndef __TIMELINE_H__
#define __TIMELINE_H__

class XTimeline : public XObject
{
private:
	DECLARE_XCLASS();

	/// 帧信息
	struct XFrame
	{
		/// 定时器句柄
		timer_h  handle;
		/// 帧的执行时间
		timespan relative;
		/// 帧的持续时间
		timespan duration;
		/// 对象ID
		xgc_uint32 object;
		/// 执行内容
		xgc_string scripts;
	};

	/// 帧信息
	xgc_vector< XFrame* > mFrames;
public:
	XTimeline()
	{

	}

	~XTimeline()
	{

	}

	///
	/// \brief 添加关键帧
	/// \author albert.xu
	/// \date 2017/10/16
	///
	xgc_void AddFrame( xgc_real64 fSeconds, xObject hObject, xgc_string &&scripts )
	{
		XFrame* pFrame = XGC_NEW XFrame;
		pFrame->handle = INVALID_TIMER_HANDLE;
		pFrame->object = hObject;
		pFrame->relative = timespan::from_seconds( fSeconds );
		pFrame->duration = 0;
		pFrame->scripts = scripts;

		mFrames.push_back( pFrame );
	}

protected:
	///
	/// \brief 运行关键帧序列
	/// \author albert.xu
	/// \date 2017/10/16
	///
	virtual xgc_bool Start()
	{
		for( auto pFrame : mFrames )
		{
			pFrame->handle = getTimer().insert(
				std::bind( &XTimeline::OnFrame, this, pFrame ),
				datetime::relative_time( pFrame->relative ),
				pFrame->duration,
				"once"
			);
		}
	}

	///
	/// \brief 帧事件响应
	/// \author albert.xu
	/// \date 2017/10/17
	///
	virtual xgc_void OnFrame( XFrame* pFrame ) = 0;
};
#endif // __TIMELINE_H__