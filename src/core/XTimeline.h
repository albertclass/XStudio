#ifndef __TIMELINE_H__
#define __TIMELINE_H__

///
/// \brief ���Ϣ
///
struct XActive
{
	/// ���ִ��ʱ��
	timespan relative;
	/// ��ĳ���ʱ��
	timespan duration;

	/// ���ʼʱִ��
	virtual xgc_bool onStart( datetime now ) = 0;

	/// �����
	virtual xgc_void onUpdate( timespan relative ) = 0;

	/// �����ʱִ��
	virtual xgc_void onClose() = 0;
};

///
/// \brief ʱ�����࣬���ڹ���ʱ�����ڵĻ
///
class XTimeline : public XObject
{
private:
	DECLARE_XCLASS();

	/// ��ʱ�����
	timer_h  mTimerHandle;

	/// ��һ������
	xgc_size mNext;

	/// ��ʼʱ��
	datetime mStart;
	/// ���¼��
	timespan mInterval;

	/// ��Ҫ���µĶ���
	xgc_vector< XActive* > mUpdates;

	/// ֡��Ϣ
	xgc_vector< XActive* > mActives;
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
	xgc_void AddActive( XActive *pActive );

	///
	/// \brief ����ʱ����
	/// \author albert.xu
	/// \date 2017/10/16
	///
	xgc_bool Start( datetime start );

protected:
	///
	/// \brief ����ʱ����״̬
	/// \author albert.xu
	/// \date 2017/10/17
	///
	xgc_void Update();
};
#endif // __TIMELINE_H__