// UIMainIngameWnd.h:  окошки-информация в игре
// 
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UIProgressBar.h"
#include "UIGameLog.h"

#include "../alife_space.h"

#include "UICarPanel.h"
#include "../UIActorStateIcons.h"
#include "UIMotionIcon.h"
#include "../hudsound.h"

class					CUIPdaMsgListItem;
class					CLAItem;
class					CUIZoneMap;
class					CUIArtefactPanel;
class					CUIScrollView;
struct					GAME_NEWS_DATA;
class					CActor;
class					CWeapon;
class					CMissile;
class					CInventoryItem;

class CUIMainIngameWnd: public CUIWindow  
{
public:
	CUIMainIngameWnd();
	virtual ~CUIMainIngameWnd();

	virtual void Init();
	virtual void Draw();
	virtual void Update();

	bool OnKeyboardPress(int dik);

protected:
	
	CUIStatic			UIStaticHealth;
	//CUIStatic			UIStaticArmor;
	CUITextWnd			UIStaticQuickHelp;
	CUIProgressBar		UIHealthBar;
	CUIProgressBar		UIArmorBar;
	CUICarPanel			UICarPanel;
	CUIMotionIcon		UIMotionIcon;	
	CUIZoneMap*			UIZoneMap;

	// lost alpha start
	CUIStatic			UIStaticTime;
	CUIStatic			UIStaticDate;
	CUIStatic			UIStaticTorch;
	CUIProgressBar		UIFlashlightBar;
	CUIStatic			UIStaticTurret;
	CUIProgressBar		UITurretBar;
	CUIActorStateIcons	UIActorStateIcons;

	//иконка, показывающая количество активных PDA
	CUIStatic			UIPdaOnline;
	
	//изображение оружия
	CUIStatic			UIWeaponBack;
	CUIStatic			UIWeaponSignAmmo;
	CUIStatic			UIWeaponIcon;
	Frect				UIWeaponIcon_rect;
	CUIStatic			UIWeaponFiremode;
public:
	CUIStatic*			GetPDAOnline					() { return &UIPdaOnline; };
protected:

	CUIStatic			UIInvincibleIcon;
	CUIStatic			UIArtefactIcon;

	CUIScrollView*		m_UIIcons;
	CUIWindow*			m_pMPChatWnd;
	CUIWindow*			m_pMPLogWnd;
public:	
	CUIArtefactPanel*    m_artefactPanel;
	
public:
	
	// Енумы соответсвующие предупреждающим иконкам 
	enum EWarningIcons
	{
		ewiAll				= 0,
		ewiWeaponJammed,
		ewiRadiation,
		ewiWound,
		ewiStarvation,
		ewiThirst,
		ewiPsyHealth,
		ewiInvincible,
		ewiArtefact,
	};

	void				SetMPChatLog					(CUIWindow* pChat, CUIWindow* pLog);

	// Задаем цвет соответствующей иконке
	void				SetWarningIconColor				(EWarningIcons icon, const u32 cl);
	void				TurnOffWarningIcon				(EWarningIcons icon);

	// Пороги изменения цвета индикаторов, загружаемые из system.ltx
	typedef				xr_map<EWarningIcons, xr_vector<float> >	Thresholds;
	typedef				Thresholds::iterator						Thresholds_it;
	Thresholds			m_Thresholds;

	// Енум перечисления возможных мигающих иконок
	enum EFlashingIcons
	{
		efiPdaTask	= 0,
		efiMail,
		efiEncyclopedia
	};
	
	void				SetFlashIconState_				(EFlashingIcons type, bool enable);

	void				AnimateContacts					(bool b_snd);
	HUD_SOUND_ITEM			m_contactSnd;

	void				ReceiveNews						(GAME_NEWS_DATA* news);
	
protected:
	void				SetWarningIconColor				(CUIStatic* s, const u32 cl);
	void				InitFlashingIcons				(CUIXml* node);
	void				DestroyFlashingIcons			();
	void				UpdateFlashingIcons				();
	void				UpdateActiveItemInfo			();
	void				HandleBolt						();

	void				SetAmmoIcon						(const shared_str& sect_name);

	// first - иконка, second - анимация
	using FlashingIcons = xr_map<EFlashingIcons, CUIStatic*>;
	using FlashingIcons_it = FlashingIcons::iterator;
	FlashingIcons		m_FlashingIcons;

	//для текущего активного актера и оружия
	CActor*				m_pActor;	
	CWeapon*			m_pWeapon;
	CMissile*			m_pGrenade;
	CInventoryItem*		m_pItem;

	// Отображение подсказок при наведении прицела на объект
	void				RenderQuickInfos();

public:
	CUICarPanel&		CarPanel							(){return UICarPanel;};
	CUIMotionIcon&		MotionIcon							(){return UIMotionIcon;}
	void				OnConnected							();
	void				reset_ui							();
protected:
	CInventoryItem*		m_pPickUpItem;
	CUIStatic			UIPickUpItemIcon;

	float				m_iPickUpItemIconX;
	float				m_iPickUpItemIconY;
	float				m_iPickUpItemIconWidth;
	float				m_iPickUpItemIconHeight;

	void				UpdatePickUpItem();
public:
	void				SetPickUpItem	(CInventoryItem* PickUpItem);
#ifdef DEBUG_DRAW
	void				draw_adjust_mode					();
#endif //DEBUG_DRAW
};
