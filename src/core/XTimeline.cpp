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

xgc_void XTimeline::AddActive( timespan start, XActive *pActive, timespan duration /*= 0*/ )
{
	Action act;
	act.relative = start;
	act.duration = duration;
	act.active = pActive;

	if( INVALID_TIMER_HANDLE == mTimerHandle )
	{
		// ֱ����ӣ�����ʱ����
		mActives.push_back( act );
	}
	else
	{
		// ����ʱ�����
		auto _Pred = []( const Action& act1, const Action& act2 ){
			return act1.relative + act1.duration < act2.relative + act2.duration;
		};

		auto it = std::upper_bound( mActives.begin(), mActives.end(), act, _Pred );
	
		if( it - mActives.begin() > (int)mNext )
			mActives.insert( it, act );
	}
}

///
/// \brief ����ʱ����
/// \author albert.xu
/// \date 2017/10/16
///

xgc_bool XTimeline::Start( datetime start )
{
	// �ȸ�����ʼʱ������
	std::sort( mActives.begin(), mActives.end(),
		[]( const Action& act_l, const Action& act_r ){
		return act_l.relative < act_r.relative;
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
	Event evt;
	evt.current = datetime::now();
	evt.relative = 0;
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
		Event evt;
		evt.current = datetime::now();
		evt.relative = datetime::now() - mStart;

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
	datetime now = datetime::now();

	// ���ݽ���ʱ�����򣬴˴�����δ���� relative �� duration �ı�����
	// WRAN ��ֵ�ı䣬���ܵ���һ������ȷ�Ķѳ��֡�
	auto _Pred = []( const Action& act1, const Action& act2 ){
		return act1.relative + act1.duration < act2.relative + act2.duration;
	};

	// ����Ƿ����¿�ʼ�Ļ
	while( mNext < mActives.size() && mStart + mActives[mNext].relative >= now )
	{
		auto &act = mActives[mNext];

		// ���ʼִ�У��ָ�������¶��С�
		mUpdates.push_back( act );
		// �����һ������������
		std::push_heap( mUpdates.begin(), mUpdates.end(), _Pred );
		// ֪ͨ���ʼ
		act.active->onStart( now );

		++mNext;
	}

	// ����Ƿ��н����Ļ
	while( mUpdates.size() )
	{
		auto &act = mUpdates[0];

		// ����Ƿ����
		if( now >= mStart + act.relative + act.duration )
			break; // �û�������ˣ���������

		// ��������������ѽ���
		std::pop_heap( mUpdates.begin(), mUpdates.end(), _Pred );

		// ֪ͨ��ر�
		act.active->onClose();

		// ɾ�����һ��Ԫ��
		mUpdates.pop_back();
	}

	// �����������ڸ����б��еĻ
	for( auto &act : mUpdates )
	{
		// ���»
		act.active->onUpdate( now - mStart );
	}

	// �ύ�¼�
	Event evt;
	evt.current = now;
	evt.relative = now - mStart;

	EmmitEvent( evt.cast, evt_tline_update );

	if( mUpdates.empty() && mNext == mActives.size() )
	{
		// �ύ�¼�
		EmmitEvent( evt.cast, evt_tline_finish );
	}
}
