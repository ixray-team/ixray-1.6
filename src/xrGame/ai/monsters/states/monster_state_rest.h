#pragma once

#include "../state.h"
#include "../../../EntityCondition.h"

class	CStateMonsterRest : public CState {
protected:
	typedef CState		inherited;
	typedef CState*	state_ptr;

	u32					time_last_fun;	
	u32					time_idle_selected;

public:
						CStateMonsterRest		(CBaseMonster *obj);
	virtual				~CStateMonsterRest		();

	virtual	void		initialize				();
	virtual	void		execute					();
	virtual	void		finalize				();
	virtual	void		critical_finalize		();
	virtual void		remove_links			(CObject* object_) { inherited::remove_links(object_);}
};
