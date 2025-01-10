#pragma once
#include "../state.h"

class CStateBurerShield : public CState
{
protected:
	using inherited = CState;

	CBurerBase* pBurerBase;

public:
						CStateBurerShield		(CBaseMonster* object);
						virtual ~CStateBurerShield() override;

	virtual	void		initialize				() override;
	virtual	void		execute					() override;
	virtual void		finalize				() override;
	virtual void		critical_finalize		() override;
	virtual void		remove_links			(CObject* object) override { inherited::remove_links(object); }

	virtual bool		check_start_conditions	() override;
	virtual bool		check_completion		() override;

private:
	TTime				m_last_shield_started;
	TTime				m_next_particle_allowed;
	float				m_shield_start_anim_length_sec;
	bool				m_started;
};

