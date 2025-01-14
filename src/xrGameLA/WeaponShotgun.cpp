#include "stdafx.h"
#include "weaponshotgun.h"
#include "entity.h"
#include "ParticlesObject.h"
#include "xr_level_controller.h"
#include "inventory.h"
#include "level.h"
#include "actor.h"

CWeaponShotgun::CWeaponShotgun() : CWeaponCustomPistol("TOZ34")
{
	m_eSoundClose			= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
	m_eSoundAddCartridge	= ESoundTypes(SOUND_TYPE_WEAPON_SHOOTING);
}

CWeaponShotgun::~CWeaponShotgun()
{
}

void CWeaponShotgun::net_Destroy()
{
	inherited::net_Destroy();
}

void CWeaponShotgun::Load	(LPCSTR section)
{
	inherited::Load		(section);

	if(pSettings->line_exist(section, "tri_state_reload")){
		m_bTriStateReload = !!pSettings->r_bool(section, "tri_state_reload");
	};
	if(m_bTriStateReload){
		m_sounds.LoadSound(section, "snd_open_weapon", "sndOpen", false, m_eSoundOpen);

		m_sounds.LoadSound(section, "snd_add_cartridge", "sndAddCartridge", false, m_eSoundAddCartridge);

		m_sounds.LoadSound(section, "snd_close_weapon", "sndClose", false, m_eSoundClose);
	};

}

void CWeaponShotgun::switch2_Fire	()
{
	inherited::switch2_Fire	();
	// bWorking = false;
}


bool CWeaponShotgun::Action			(u16 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;

	if(	m_bTriStateReload && GetState()==eReload &&
		cmd==kWPN_FIRE && flags&CMD_START &&
		m_sub_state==eSubstateReloadInProcess		)//���������� ������������
	{
		AddCartridge(1);
		m_sub_state = eSubstateReloadEnd;
		return true;
	}
	return false;
}

void CWeaponShotgun::OnAnimationEnd(u32 state) 
{
	if(!m_bTriStateReload || state != eReload)
		return inherited::OnAnimationEnd(state);

	switch(m_sub_state){
		case eSubstateReloadBegin:{
			m_sub_state = eSubstateReloadInProcess;
			SwitchState(eReload);
		}break;

		case eSubstateReloadInProcess:{
			if( 0 != AddCartridge(1) ){
				m_sub_state = eSubstateReloadEnd;
			}
			SwitchState(eReload);
		}break;

		case eSubstateReloadEnd:{
			m_sub_state = eSubstateReloadBegin;
			SwitchState(eIdle);
		}break;
		
	};
}

void CWeaponShotgun::Reload() 
{
	if(m_bTriStateReload){
		TriStateReload();
	}else
		inherited::Reload();
}

void CWeaponShotgun::TriStateReload()
{
	if (m_magazine.size() == (u32)maxMagazineSize_ || !HaveCartridgeInInventory(1))return;
	CWeapon::Reload		();
	m_sub_state			= eSubstateReloadBegin;
	SwitchState			(eReload);
}

void CWeaponShotgun::OnStateSwitch	(u32 S)
{
	if(!m_bTriStateReload || S != eReload){
		inherited::OnStateSwitch(S);
		return;
	}

	CWeapon::OnStateSwitch(S);

	if (m_magazine.size() == (u32)maxMagazineSize_ || !HaveCartridgeInInventory(1)){
			switch2_EndReload		();
			m_sub_state = eSubstateReloadEnd;
			return;
	};

	switch (m_sub_state)
	{
	case eSubstateReloadBegin:
		if( HaveCartridgeInInventory(1) )
			switch2_StartReload	();
		break;
	case eSubstateReloadInProcess:
			if( HaveCartridgeInInventory(1) )
				switch2_AddCartgidge	();
		break;
	case eSubstateReloadEnd:
			switch2_EndReload		();
		break;
	};
}

void CWeaponShotgun::switch2_StartReload()
{
	PlaySound			("sndOpen",get_LastFP());
	PlayAnimOpenWeapon	();
	SetPending			(TRUE);
}

void CWeaponShotgun::switch2_AddCartgidge	()
{
	PlaySound	("sndAddCartridge",get_LastFP());
	PlayAnimAddOneCartridgeWeapon();
	SetPending			(TRUE);
}

void CWeaponShotgun::switch2_EndReload	()
{
	SetPending			(FALSE);
	PlaySound			("sndClose",get_LastFP());
	PlayAnimCloseWeapon	();
}

void CWeaponShotgun::PlayAnimOpenWeapon()
{
	VERIFY(GetState()==eReload);
	PlayHUDMotion("anim_open",FALSE,this,GetState());
}
void CWeaponShotgun::PlayAnimAddOneCartridgeWeapon()
{
	VERIFY(GetState()==eReload);
	PlayHUDMotion("anim_add_cartridge",FALSE,this,GetState());
}
void CWeaponShotgun::PlayAnimCloseWeapon()
{
	VERIFY(GetState()==eReload);

	PlayHUDMotion("anim_close",FALSE,this,GetState());
}

bool CWeaponShotgun::HaveCartridgeInInventory(u8 cnt)
{
	if (unlimited_ammo())	return true;

	m_pAmmo = NULL;
	if(m_pCurrentInventory) 
	{
		//���������� ����� � ��������� ������� �������� ���� 
		m_pAmmo = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[m_ammoType]));
		
		if(!m_pAmmo )
 		{
			for(u32 i = 0; i < m_ammoTypes.size(); ++i) 
 			{
				//��������� ������� ���� ���������� �����
				m_pAmmo = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny(*m_ammoTypes[i]));
				if(m_pAmmo) 
				{ 
					m_ammoType = i; 
					break; 
				}
 			}
 		}
 	}
	return (m_pAmmo!=NULL)&&(m_pAmmo->m_boxCurr>=cnt) ;
}

u8 CWeaponShotgun::AddCartridge		(u8 cnt)
{
	if(IsMisfire())	bMisfire = false;

	if ( m_set_next_ammoType_on_reload != u8(-1) )
	{
		m_ammoType						= m_set_next_ammoType_on_reload;
		m_set_next_ammoType_on_reload	= u8(-1);
	}

	if( !HaveCartridgeInInventory(1) )
		return 0;

	m_pAmmo = smart_cast<CWeaponAmmo*>(m_pCurrentInventory->GetAny( m_ammoTypes[m_ammoType].c_str() ));
	VERIFY((u32)iAmmoElapsed == m_magazine.size());


	if (m_DefaultCartridge.m_LocalAmmoType != m_ammoType)
		m_DefaultCartridge.Load(m_ammoTypes[m_ammoType].c_str(), m_ammoType);

	CCartridge l_cartridge = m_DefaultCartridge;
	while(cnt)
	{
		if (!unlimited_ammo())
		{
			if (!m_pAmmo->Get(l_cartridge)) break;
		}
		--cnt;
		++iAmmoElapsed;
		l_cartridge.m_LocalAmmoType = m_ammoType;
		m_magazine.push_back(l_cartridge);
//		m_fCurrentCartirdgeDisp = l_cartridge.m_kDisp;
	}

	VERIFY((u32)iAmmoElapsed == m_magazine.size());

	//�������� ������� ��������, ���� ��� ������
	if(m_pAmmo && !m_pAmmo->m_boxCurr && OnServer()) 
		m_pAmmo->SetDropManual(TRUE);

	return cnt;
}

BOOL CWeaponShotgun::net_Spawn(CSE_Abstract* server_entity)
{
	BOOL spawn_res = inherited::net_Spawn(server_entity);

	CSE_ALifeItemWeaponShotGun* const shot_gun = smart_cast<CSE_ALifeItemWeaponShotGun*>(server_entity);
	R_ASSERT(shot_gun);

	// load data exported to server in ::net_Export
	if (shot_gun->m_AmmoIDs.size() > 0) //override CWeapon code for spawning magagine (shotguns can have deverced ammo in magazine)
	{
		m_magazine.clear();

		for (u8 i = 0; i < shot_gun->m_AmmoIDs.size(); i++)
		{
			u8 ammo_type = shot_gun->m_AmmoIDs[i];

			if (ammo_type >= m_ammoTypes.size())
			{
				Msg("!m_ammoType index %u is out of ammo indexes (should be less than %d); Weapon section is [%s]. Probably default ammo upgrade is the reason", ammo_type, m_ammoTypes.size(), cNameSect_str());
				ammo_type = 0;
			}

			if (m_DefaultCartridge.m_LocalAmmoType != ammo_type) // if cartridge type loaded in CWeapon is not same or if shotgun has deverced ammo types in magazine - load new
				m_DefaultCartridge.Load(m_ammoTypes[ammo_type].c_str(), ammo_type);

			CCartridge l_cartridge = m_DefaultCartridge;

			l_cartridge.m_LocalAmmoType = ammo_type;
			m_magazine.push_back(l_cartridge);
		}

		R_ASSERT((u32)iAmmoElapsed == m_magazine.size());
	}

	return spawn_res;
}

void CWeaponShotgun::net_Export	(NET_Packet& P)
{
	inherited::net_Export(P);	
	P.w_u8(u8(m_magazine.size()));	
	for (u32 i=0; i<m_magazine.size(); i++)
	{
		CCartridge& l_cartridge = *(m_magazine.begin()+i);
		P.w_u8(l_cartridge.m_LocalAmmoType);
	}
}

void CWeaponShotgun::net_Import	(NET_Packet& P)
{
	inherited::net_Import(P);	

	Msg("CWeaponShotgun::net_Import not active in SP");
}
