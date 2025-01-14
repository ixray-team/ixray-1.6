#pragma once
#include "UIDialogWnd.h"
#include "UIWndCallback.h"

class CUIMessageBox;

class CUIMessageBoxEx : public CUIDialogWnd, public CUIWndCallback
{
public:
					CUIMessageBoxEx		();
	virtual			~CUIMessageBoxEx	();
			void	 SetText			(LPCSTR text);
			LPCSTR	GetText				();
	virtual void	InitMessageBox		(LPCSTR xml_template);
	virtual void	SendMessage			(CUIWindow* pWnd, s16 msg, void* pData = nullptr);

	LPCSTR			GetHost				();
	LPCSTR			GetPassword			();

	CUIWndCallback::void_function		func_on_ok;
	CUIWndCallback::void_function		func_on_no;
	void 	OnOKClicked			(CUIWindow*, void*);
	void 	OnNOClicked			(CUIWindow*, void*);

	virtual bool	OnKeyboardAction			(int dik, EUIMessages keyboard_action);
	virtual bool	NeedCenterCursor	()const	 {return false;}

    CUIMessageBox*						m_pMessageBox;
};