#include "stdafx.h"
#include "ParticlesAsyncManager.h"
#include "ParticlesObject.h"

#include "../Include/xrRender/ParticleCustom.h"
#include "../Include/xrRender/RenderVisual.h"

static CParticlesAsync Instance;
//static xr_task_group ParticleObjectTasks;

void CParticlesAsync::Play()
{
	if (!psDeviceFlags.test(mtParticles))
	{
		Instance.Start();
	}
}

void CParticlesAsync::Start()
{
	Instance.IsStarted = true;

	{
		PROF_EVENT("Particle Update");
		for (CParticlesObject* particle : Instance.Particles)
		{
			if (particle->m_bDead)
				continue;

			Instance.UpdateParticle(particle);
		}
	}

	PROF_EVENT("Particle Shedule");
	for (CParticlesObject* particle : Instance.Particles)
	{
		particle->Update(Device.dwTimeDelta);
	}

	Instance.IsStarted = false;
}

void CParticlesAsync::Wait()
{
	if (psDeviceFlags.test(mtParticles))
	{
		PROF_EVENT("Particles Wait");
		while (Instance.IsStarted)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(0));
		}
		//ParticleObjectTasks.wait();
	}
}

bool CParticlesAsync::Push(CParticlesObject* Obj)
{
	Instance.Particles.push_back(Obj);

	return Instance.IsStarted;
}

void CParticlesAsync::Pop(CParticlesObject* Obj)
{
	Wait();

	Instance.Particles.remove(Obj);
}

void CParticlesAsync::ForceUpdate(CParticlesObject* Obj)
{
	if (!Instance.IsStarted)
		return;

	Instance.UpdateParticle(Obj);
}

CParticlesAsync::CParticlesAsync()
{
	if (!DevicePtr || Device.IsEditorMode())
		return;

	Device.ParticleWorkerCallback = Start;
}

void CParticlesAsync::UpdateParticle(CParticlesObject* particle) const
{
	u32 dt = Device.dwTimeGlobal - particle->dwLastTime;
	IParticleCustom* V = smart_cast<IParticleCustom*>(particle->renderable.visual);

	if (V == nullptr)
		return;

	PROF_EVENT("Particle OnFrame");
	V->OnFrame(dt);

	particle->dwLastTime = Device.dwTimeGlobal;
}