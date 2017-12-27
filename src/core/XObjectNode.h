#pragma once
#ifndef __XOBJECT_NODE_H__
#define __XOBJECT_NODE_H__
namespace xgc
{
	class CORE_API XObjectNode : public XObject
	{
	public:
		///
		/// \brief �ڵ㹹�� 
		/// \date 11/13/2017
		/// \author xufeng04
		///
		XObjectNode( xObject hParent = INVALID_OBJECT_ID );

		///
		/// \brief �ڵ�����
		/// \date 11/13/2017
		/// \author xufeng04
		///
		~XObjectNode();

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
		xgc_void RemoveAll();

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
		xgc_void DeleteAll();

		///
		/// \brief ��ȡ�Ӷ�������
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xgc_size GetChildCount()const override;

		///
		/// \brief �����ӽڵ�
		/// \author albert.xu
		/// \date 8/3/2009
		/// \return true - ȷ�������ӽڵ�, false - �ӽڵ㱻���,��ӽڵ�ʧ��.
		///
		xObject Search( const std::function< xgc_bool( xObject ) > &Filter )const override;

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
		///
		virtual xgc_void OnInsertChild( XObject* pChild, xgc_lpvoid lpContext ) = 0;

		///
		/// \brief ɾ���ӽڵ�����
		/// \author albert.xu
		/// \date 8/3/2009
		///
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
		xgc::vector< xObject > mChildren;
	};
}

#endif // __XOBJECT_NODE_H__