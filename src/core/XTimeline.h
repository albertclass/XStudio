#ifndef __TIMELINE_H__
#define __TIMELINE_H__

class XTimeline : public XObject
{
private:
	DECLARE_XCLASS();

	/// ֡��Ϣ
	struct XFrame
	{
		/// ��ʱ�����
		timer_h  handle;
		/// ֡��ִ��ʱ��
		timespan relative;
		/// ֡�ĳ���ʱ��
		timespan duration;
		/// ����ID
		xgc_uint32 object;
		/// ִ������
		xgc_string scripts;
	};

	/// ֡��Ϣ
	xgc_vector< XFrame* > mFrames;
public:
	XTimeline()
	{

	}

	~XTimeline()
	{

	}

	///
	/// \brief ��ӹؼ�֡
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
	/// \brief ���йؼ�֡����
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
	/// \brief ֡�¼���Ӧ
	/// \author albert.xu
	/// \date 2017/10/17
	///
	virtual xgc_void OnFrame( XFrame* pFrame ) = 0;
};
#endif // __TIMELINE_H__