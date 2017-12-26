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
/// \brief ��ӹؼ�֡
/// \author albert.xu
/// \date 2017/10/16
///

xgc_void XTimeline::AddActive( XActive * pActive )
{
	if( INVALID_TIMER_HANDLE == mTimerHandle )
	{
		// ֱ����ӣ�����ʱ����
		mActives.push_back( pActive );
	}
	else
	{
		// ����ʱ�����
		auto _Pred = []( const XActive* pAct1, const XActive* pAct2 ){
			return pAct1->relative + pAct1->duration < pAct2->relative + pAct2->duration;
		};

		auto it = std::upper_bound( mActives.begin(), mActives.end(), pActive, _Pred );
	
		if( it - mActives.begin() > (int)mNext )
			mActives.insert( it, pActive );
	}
}

///
/// \brief ����ʱ����
/// \author albert.xu
/// \date 2017/10/16
///

xgc_bool XTimeline::Start( datetime start )
{
	std::sort( mActives.begin(), mActives.end(),
		[]( const XActive *frame_l, const XActive *frame_r ){
		return frame_l->relative < frame_r->relative;
	} );

	// ʱ���߿�ʼʱ��
	mStart = start;

	// ���붨ʱ��
	mTimerHandle = getTimer().insert(
		std::bind( &XTimeline::Update, this ),
		mStart,
		DURATION_FOREVER,
		timer_event::e_rept,
		mInterval.to_millisecnods() );

	// �ύ�¼�
	XTimelineEvent evt;
	evt.current = datetime::now();
	EmmitEvent( evt.cast, evt_tline_start );

	return true;
}

///
/// \brief ֹͣʱ����
/// \author albert.xu
/// \date 2017/11/03
///

xgc_void XTimeline::Stop()
{
	if( INVALID_TIMER_HANDLE != mTimerHandle )
	{
		getTimer().remove( mTimerHandle );
		mTimerHandle = INVALID_TIMER_HANDLE;

		// �ύ�¼�
		XTimelineEvent evt;
		evt.current = datetime::now();
		EmmitEvent( evt.cast, evt_tline_cancel );
	}

	mUpdates.clear();
}

///
/// \brief ����ʱ����״̬
/// \author albert.xu
/// \date 2017/10/17
///

xgc_void XTimeline::Update()
{
	XGC_ASSERT_RETURN( mNext >= mActives.size(), XGC_NONE );
	datetime now = datetime::now();

	// ���ݽ���ʱ�����򣬴˴�����δ���� relative �� duration �ı�����
	// WRAN ��ֵ�ı䣬���ܵ���һ������ȷ�Ķѳ��֡�
	auto _Pred = []( const XActive* pAct1, const XActive* pAct2 ){
		return pAct1->relative + pAct1->duration < pAct2->relative + pAct2->duration;
	};

	// ����Ƿ�����Ҫ���µĻ
	while( mStart + mActives[mNext]->relative >= now )
	{
		auto pAct = mActives[mNext];

		// ���ʼִ�У��ָ�������¶��С�
		mUpdates.push_back( pAct );
		// �����һ������������
		std::push_heap( mUpdates.begin(), mUpdates.end(), _Pred );
		// ֪ͨ���ʼ
		pAct->onStart( now );

		++mNext;
	}

	// ��ʼ���»
	while( mUpdates.size() )
	{
		auto pAct = mUpdates[0];
		XGC_ASSERT( pAct );
		// ���»
		pAct->onUpdate( now - mStart );

		// ����Ƿ����
		if( now < mStart + pAct->relative + pAct->duration )
			break; // ���иý����Ļ�������ˣ���������

		// ��������������ѽ���
		std::pop_heap( mUpdates.begin(), mUpdates.end(), _Pred );
		// ɾ�����һ��Ԫ��
		mUpdates.pop_back();
		// ֪ͨ��ر�
		pAct->onClose();
	}

	// �ύ�¼�
	XTimelineEvent evt;
	evt.current = now;
	evt.relative = now - mStart;

	if( mUpdates.empty() && mNext == mActives.size() )
	{
		// �ύ�¼�
		EmmitEvent( evt.cast, evt_tline_finish );
	}
	else
	{
		EmmitEvent( evt.cast, evt_tline_update );
	}
}
