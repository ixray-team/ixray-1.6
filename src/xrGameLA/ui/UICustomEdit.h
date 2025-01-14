
#pragma once

#include "UIStatic.h"

namespace text_editor
{
	class ENGINE_API line_edit_control;
	enum init_mode;
};

class CUICustomEdit : public CUIStatic
{
private:
	typedef			CUIStatic		inherited;

public:
					CUICustomEdit	();
	virtual			~CUICustomEdit	();

			void	Init			(u32 max_char_count, bool number_only_mode = false, bool read_mode = false, bool fn_mode = false, bool translate = true );
		
	virtual void	InitCustomEdit	(Fvector2 pos, Fvector2 size);
	virtual void	SendMessage		(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual bool	OnMouseAction			(float x, float y, EUIMessages mouse_action);
	virtual bool	OnKeyboardAction		(int dik, EUIMessages keyboard_action);
	virtual bool	OnKeyboardHold	(int dik);

	virtual void	OnFocusLost		();
	virtual void	Update			();
	virtual void	Draw			();
	virtual void	Show			(bool status);

			void	CaptureFocus	(bool bCapture);
			void	SetNextFocusCapturer(CUICustomEdit* next_capturer) { m_next_focus_capturer = next_capturer; };
	
			void	ClearText		();
	virtual	void	SetText			(LPCSTR str);
	virtual LPCSTR	GetText			()	const;
	virtual void	SetTextColor		(u32 color);
	virtual void	SetTextColorD		(u32 color);

	virtual void	Enable			(bool status);
			
			void	SetPasswordMode	(bool mode = true);
			void	SetDbClickMode	(bool mode = true)	{m_bFocusByDbClick = mode;}

	virtual void	Init_script				(float x, float y, float width, float height) { InitCustomEdit(Fvector2().set(x,y), Fvector2().set(width, height));} //SkyLoader: for scripts

protected:
			void				Register_callbacks();

			void  	nothing();
			void  	press_escape();
			void  	press_commit();
			void  	press_tab();

protected:
	typedef  fastdelegate::FastDelegate0<void>		Callback;

	enum								{ EDIT_BUF_SIZE = 256 };
	text_editor::line_edit_control*		m_editor_control;
	text_editor::line_edit_control&		ec();
	text_editor::line_edit_control const &	ec() const;
	
	u32		m_last_key_state_time;
	char	m_out_str[EDIT_BUF_SIZE];
	float	m_dx_cur;

	bool	m_bInputFocus;
	bool	m_force_update;
	bool	m_read_mode;
	bool	m_bFocusByDbClick;

	u32	m_textColor[2];

	CUICustomEdit*	m_next_focus_capturer;
};