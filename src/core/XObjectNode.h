#pragma once
#ifndef __XOBJECT_NODE_H__
#define __XOBJECT_NODE_H__
namespace xgc
{
	class CORE_API XObjectNode : public XObject
	{
		DECLARE_XCLASS();
	protected:
		XObjectNode()
		{

		}

		~XObjectNode()
		{

		}

	public:
		///
		/// \brief �����ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xgc_bool Insert( XObject *pObject, xgc_lpvoid pContext );

		///
		/// \brief �Ƴ��ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xgc_bool Remove( XObject *pObject );

		///
		/// \brief �Ƴ��ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xgc_void RemoveAll( const XClassInfo *pClass = xgc_nullptr );

		///
		/// \brief ɾ���ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xgc_bool Delete( XObject *pObject );

		///
		/// \brief ɾ�������ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xgc_void DeleteAll( const XClassInfo *pClass = xgc_nullptr );

		///
		/// \brief �����ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xObject Search( const std::function< xgc_bool( xObject ) > &filter, const XClassInfo *pClass = xgc_nullptr )const;

		///
		/// \brief ��ȡ�Ӷ�������
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xgc_size GetChildrenCount( const XClassInfo *pClass = xgc_nullptr )const;

		///
		/// \brief ��ȡ���
		/// \author albert.xu
		/// \date 2017/11/01
		///
		xObject GetComposition( const XClassInfo &rClass, xgc_size nIndex = 0 )const
		{
			auto it = mChildren.find( &rClass );
			if( it == mChildren.end() )
				return INVALID_OBJECT_ID;

			if( it->second.size() <= nIndex )
				return INVALID_OBJECT_ID;

			return it->second[nIndex];
		}

		///
		/// \brief ��ȡ���
		/// \author albert.xu
		/// \date 2017/11/01
		///
		template< template< class, class > class Continer, class _Ax = xgc_allocator >
		typename Continer< xObject, _Ax > GetCompositions( const XClassInfo &rClass )const
		{
			Continer< xObject, _Ax > c;

			auto it = mChildren.find( &rClass );
			if( it != mChildren.end() )
			{
				std::copy( it->second.begin(), it->second.end(), std::back_inserter( c ) );
			}

			return c;
		}

	protected:
		/************************************************************************/
		/* ���õ�һЩ�¼���Ӧ�麯����
		/************************************************************************/
		///
		/// \brief �����ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		virtual xgc_bool InsertChild( xObject hObject );

		///
		/// \brief ɾ���ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		virtual xgc_bool RemoveChild( xObject hObject );

		///
		/// \brief �����ӽڵ�����
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		virtual xgc_void OnInsertChild( XObject* pChild, xgc_lpvoid lpContext ) = 0;

		/////
		/// ɾ���ӽڵ�����,��ʱ������δ��ɾ��
		/// [8/3/2009 Albert]
		/////
		virtual xgc_void OnRemoveChild( XObject* pChild ) = 0;

		///
		/// \brief ���ٶ���
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		virtual xgc_void OnDestroy() override;

	private:
		/// @var children
		xgc_unordered_map< xgc_lpcvoid, xgc_vector< xObject > > mChildren;
	};
}

#endif // __XOBJECT_NODE_H__