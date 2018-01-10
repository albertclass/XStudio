#ifndef __TIMELINE_H__
#define __TIMELINE_H__

///
/// \brief ���Ϣ
///
struct XActive
{
	/// ���ʼʱִ��
	virtual xgc_bool onStart( datetime now ) = 0;

	/// �����
	virtual xgc_void onUpdate( timespan relative ) = 0;

	/// �����ʱִ��
	virtual xgc_void onClose() = 0;

	/// �����ʱִ��
	virtual xgc_void onDestory() = 0;
};

///
/// \brief ʱ���ߴ����¼�
///
enum CORE_API eTimelineEvent
{
	evt_tline_start,
	evt_tline_update,
	evt_tline_cancel,
	evt_tline_finish,
	evt_tline_enable,
};

///
/// \brief ʱ�����࣬���ڹ���ʱ�����ڵĻ
///
class XTimeline : public XObject
{
	DECLARE_XCLASS();
public:
	///
	/// \brief ʱ�����¼�������
	///
	struct CORE_API Event
	{
		/// �̳���XObjectEvent
		XObjectEvent cast;
		/// ʱ���
		datetime current;
		/// ���ʱ��
		timespan relative;
	};

private:
	/// ��ʱ�����
	timer_h  mTimerHandle;

	/// ��һ������
	xgc_size mNext;

	/// ��ʼʱ��
	datetime mStart;
	/// ���¼��
	timespan mInterval;

	struct Action
	{
		/// ���ִ��ʱ��
		timespan relative;
		/// ��ĳ���ʱ��
		timespan duration;
		/// �����
		XActive *active;
	};

	/// ��Ҫ���µĶ���
	xgc::vector< Action > mUpdates;

	/// ֡��Ϣ
	xgc::vector< Action > mActives;
public:
	///
	/// \brief ʱ���߹���
	/// \author albert.xu
	/// \date 2017/10/16
	///
	XTimeline( timespan interval );

	///
	/// \brief ʱ��������
	/// \author albert.xu
	/// \date 2017/10/16
	///
	virtual ~XTimeline();

	///
	/// \brief ��ӹؼ�֡
	/// \author albert.xu
	/// \date 2017/10/16
	///
	xgc_void AddActive( timespan start, XActive *pActive, timespan duration = 0 );

	///
	/// \brief ����ʱ����
	/// \author albert.xu
	/// \date 2017/10/16
	///
	xgc_bool Start( datetime start );

	///
	/// \brief ֹͣʱ����
	/// \author albert.xu
	/// \date 2017/11/03
	///
	xgc_void Stop();
protected:
	///
	/// \brief ����ʱ����״̬
	/// \author albert.xu
	/// \date 2017/10/17
	///
	xgc_void Update();
};
#endif // __TIMELINE_H__