#pragma once

#include "UIDialogWnd.h"
#include "UIEditBox.h"
#include "../inventory_space.h"

class CUIDragDropListEx;
class CUIItemInfo;
class CUITextWnd;
class CUICharacterInfo;
class CUIPropertiesBox;
class CUI3tButton;
class CUICellItem;
class CInventoryBox;
class CInventoryOwner;
class CCar;

class CUICarBodyWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;
	bool					m_b_need_update;

	void ColorizeItem(CUICellItem* itm);

public:
							CUICarBodyWnd				();
	virtual					~CUICarBodyWnd				();

	virtual void			Init						();
	virtual bool			StopAnyMove					(){return true;}

	virtual void			SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);

	void					InitCustomInventory			(CInventoryOwner* pOurInv, CInventoryOwner* pOthersInv);
	void					InitInventoryBox			(CInventoryOwner* pOur, CInventoryBox* pInvBox);

	virtual void			Draw						();
	virtual void			Update						();
		
	virtual void			ShowDialog						(bool bDoHideIndicators);
	virtual void			HideDialog						();

	void					DisableAll					();
	void					EnableAll					();
	virtual bool			OnKeyboardAction					(int dik, EUIMessages keyboard_action);

	void					UpdateLists_delayed			();

protected:
	CInventoryOwner*		m_pOurObject;

	CInventoryOwner*		m_pOthersObject;
	CInventoryBox*			m_pInventoryBox;
	CInventory*				m_pInventory;

	CCar*					m_pCar;

	CUIDragDropListEx*		m_pUIOurBagList;
	CUIDragDropListEx*		m_pUIOthersBagList;

	CUIStatic*				m_pUIStaticTop;
	CUIStatic*				m_pUIStaticBottom;

	CUIFrameWindow*			m_pUIDescWnd;
	CUIStatic*				m_pUIStaticDesc;
	CUIItemInfo*			m_pUIItemInfo;

	CUIStatic*				m_pUIOurBagWnd;
	CUIStatic*				m_pUIOthersBagWnd;
	LPCSTR					m_othersBagWndPrefix;

	//���������� � ���������� 
	CUIStatic*				m_pUIOurIcon;
	CUIStatic*				m_pUIOthersIcon;
	CUICharacterInfo*		m_pUICharacterInfoLeft;
	CUICharacterInfo*		m_pUICharacterInfoRight;
	CUIPropertiesBox*		m_pUIPropertiesBox;
	CUI3tButton*			m_pUITakeAll;

	CUICellItem*			m_pCurrentCellItem;

	void					UpdateLists					();

	void					ActivatePropertiesBox		();
	void					EatItem						();

	bool					ToOurBag					();
	bool					ToOthersBag					();
	
	void					SetCurrentItem				(CUICellItem* itm);
	CUICellItem*			CurrentItem					();
	PIItem					CurrentIItem				();

	// ����� ���
	void					TakeAll						();


	bool		xr_stdcall	OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall	OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall	OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall	OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall	OnItemRButtonClick			(CUICellItem* itm);

	bool					TransferItem				(PIItem itm, CInventoryOwner* owner_from, CInventoryOwner* owner_to, bool b_check);
	void					BindDragDropListEvents		(CUIDragDropListEx* lst);



};