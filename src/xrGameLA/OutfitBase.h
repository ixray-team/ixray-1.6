#pragma once

#include "inventory_item_object.h"
#include "hudsound.h"

struct SBoneProtections;

class COutfitBase: public CInventoryItemObject {
private:
    typedef	CInventoryItemObject inherited;
public:
									COutfitBase		(void);
	virtual							~COutfitBase		(void);

	virtual void					Load				(LPCSTR section);
	virtual void					save				(NET_Packet &output_packet);
	virtual void					load				(IReader &input_packet);

	//����������� ������ ����, ��� ������, ����� ������ ����� �� ���������
	virtual void					Hit					(float P, ALife::EHitType hit_type);

	//������������ �� ������� ����������� ���
	//��� ��������������� ���� �����������
	//���� �� ��������� ����� ������
	float							GetHitTypeProtection(ALife::EHitType hit_type, s16 element);
	float							GetDefHitTypeProtection(ALife::EHitType hit_type);

	////tatarinrafa: ������ �� �� ������� ����
	float							GetBoneArmor(s16 element);
	float							HitThroughArmor(float hit_power, s16 element, float ap, bool& add_wound, ALife::EHitType hit_type);
	
	float							GetArmorBody			();
	float							GetArmorHead			();

	//����������� �� ������� ����������� ������ ����
	//���� �� ��������� ����� ������
	float							GetPowerLoss		();

	float							GetHealthRestoreSpeed() { return m_fHealthRestoreSpeed; }
	float							GetRadiationRestoreSpeed() { return m_fRadiationRestoreSpeed; }
	float							GetSatietyRestoreSpeed() { return m_fSatietyRestoreSpeed; }
	float							GetPowerRestoreSpeed() { return m_fPowerRestoreSpeed; }
	float							GetBleedingRestoreSpeed() { return m_fBleedingRestoreSpeed; }

	virtual void					OnMoveToRuck		();
	virtual void					OnMoveToSlot		();

protected:
	HitImmunity::HitTypeSVec		m_HitTypeProtection;
	float							m_fPowerLoss;
	float							m_fHealthRestoreSpeed;
	float 							m_fRadiationRestoreSpeed;
	float 							m_fSatietyRestoreSpeed;
	float							m_fPowerRestoreSpeed;
	float							m_fBleedingRestoreSpeed;

	SBoneProtections*				m_boneProtection;
	
	// ��� ������ ��� ���� � ������, �� ������� �������� ������� ����� ��� ������ � UI
	shared_str						m_armorTestBoneBody;
	shared_str						m_armorTestBoneHead;

	float							GetArmorByBoneName		(const shared_str& boneName);

public:
	shared_str						m_BonesProtectionSect;

	virtual	BOOL					BonePassBullet			(int boneID);
	
			void			ReloadBonesProtection	();
			void			AddBonesProtection		(LPCSTR bones_section);
	virtual BOOL			net_Spawn			(CSE_Abstract* DC);

	// ������ ����������� ���������� ������ �� ������
	float							visorWetness_1_;
	float							visorWetness_2_;

	// ��� ������� ������� ��������� ������ ��� ��� ����� �� ����� ����� ������ ��� �� ����
	float							visorWetnessSpreadValue_1_;
	float							visorWetnessSpreadValue_2_;

	// �������� �� ������� ������, ����� �� ������, ����� ������
	bool							hasVisorEffects_;

protected:
	virtual bool			install_upgrade_impl( LPCSTR section, bool test );
};
