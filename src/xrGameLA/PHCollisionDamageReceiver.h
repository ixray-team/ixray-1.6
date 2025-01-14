#pragma once

class CPhysicsShellHolder;
struct dContact;
struct SGameMtl;

class CPHCollisionDamageReceiver
{
typedef std::pair<u16,float> SControledBone;

using DAMAGE_CONTROLED_BONES_V = xr_vector<SControledBone>;
using DAMAGE_BONES_I = DAMAGE_CONTROLED_BONES_V::iterator;

struct SFind{u16 id;SFind(u16 _id){id=_id;};bool operator () (const SControledBone& cb){return cb.first==id;}};
DAMAGE_CONTROLED_BONES_V m_controled_bones;
float m_hit_threshold = 5.f;

public:
			void						SetCollisionHitThreshold	(float v) { m_hit_threshold = v; }
			float						CollisionHitThreshold		() { return m_hit_threshold; }

protected:
	virtual CPhysicsShellHolder*		PPhysicsShellHolder			()																		=0;
			void						Init						()																		;
			void						Hit							(u16 source_id,u16 bone_id,float power,const Fvector &dir,Fvector &pos)	;
			void						Clear						()																		;
private:
			void						BoneInsert					(u16 id,float k)														;

	IC		DAMAGE_BONES_I				FindBone					(u16 id)
	{
		return std::find_if(m_controled_bones.begin(),m_controled_bones.end(),SFind(id));
	}
	static	void 						CollisionCallback			(bool& do_colide,bool bo1,dContact& c,SGameMtl* material_1,SGameMtl* material_2)	;
};
