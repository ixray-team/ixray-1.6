#pragma once

#include "UI_IB_Static.h"
#include "UI_IB_FrameLineWnd.h"

class CUI3tButton;
class CUITrackButton;

class CUITrackBarVariable : public CUI_IB_FrameLineWnd
{
	friend class CUITrackButton;
public:
					CUITrackBarVariable				();
	// ControlledVariable
	virtual void	OnChangedValue();
	float			*f_controlledfloat;
	int				*i_controlledint;
	virtual	void	SetCurenntValue();

	virtual void	Draw					();
	virtual void	Update					();
	virtual bool	OnMouseAction			(float x, float y, EUIMessages mouse_action);
	// CUIWindow
			void	InitTrackBar			(Fvector2 pos, Fvector2 size);
	virtual void	Enable					(bool status);
			void	SetInvert				(bool v){m_b_invert=v;}
			bool	GetInvert				() const	{return m_b_invert;};
			void	SetStep					(float step);
			void	SetType					(bool b_float){m_b_is_float=b_float;};
			bool	GetCheck				();
			void	SetCheck				(bool b);
			int		GetIValue				(){return m_i_val;}
			float	GetFValue				(){return m_f_val;}
			void	SetOptIBounds			(int imin, int imax);
			void	SetOptFBounds			(float fmin, float fmax);
protected:
			void 	UpdatePos				();
			void 	UpdatePosRelativeToMouse();

    CUI3tButton*		m_pSlider;
	bool				m_b_invert;
	bool				m_b_is_float;
	bool				m_b_mouse_capturer;

	union{
		struct{
			float				m_f_val;
			float				m_f_max;
			float				m_f_min;
			float				m_f_step;
			float				m_f_opt_backup_value;
		};
		struct{
			int					m_i_val;
			int					m_i_max;
			int					m_i_min;
			int					m_i_step;
			int					m_i_opt_backup_value;
		};
	};
};