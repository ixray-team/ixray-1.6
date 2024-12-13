#pragma once
class CParticlesObject;

class PARTICLES_API CParticlesAsync
{
public:
	static void Play();
	static void Wait();

	static bool Push(CParticlesObject* Obj);
	static void Pop(CParticlesObject* Obj);
	
	static void ForceUpdate(CParticlesObject* Obj);

private:
	void UpdateParticle(CParticlesObject* particle) const;
	static void Start();

public:
	CParticlesAsync();

private:
	xr_list<CParticlesObject*> Particles;
	volatile bool IsStarted = false;
};