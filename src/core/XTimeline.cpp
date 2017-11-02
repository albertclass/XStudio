#include "XHeader.h"
#include "XTimeline.h"

XTimeline::XTimeline( timespan interval )
	: mTimerHandle( INVALID_OBJECT_ID )
	, mNext( 0 )
	, mInterval( interval )
{

}

XTimeline::~XTimeline()
{

}

///
/// \brief ��ӹؼ�֡
/// \author albert.xu
/// \date 2017/10/16
///

xgc_void XTimeline::AddActive( XActive * pActive )
{
	mActives.push_back( pActive );
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
	auto _Pred = [now]( const XActive* pAct1, const XActive* pAct2 ){
		return pAct1->relative + pAct1->duration < pAct2->relative + pAct2->duration;
	};

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
}
