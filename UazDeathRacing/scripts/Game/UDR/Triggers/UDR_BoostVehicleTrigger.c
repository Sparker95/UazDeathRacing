class UDR_BoostVehicleTriggerClass: ScriptedGameTriggerEntityClass
{
};

class UDR_BoostVehicleTrigger: ScriptedGameTriggerEntity
{
	static const string BOOST_PARTICLE_NAME = "{575C2DEE390847F1}Prefabs/Triggers/Assets/boostParticle.ptc";

	[Attribute("2000", UIWidgets.EditBox)]
	protected int m_iBoostDuration_ms;
	
	override void OnActivate(IEntity ent)
	{
		PrintString("activateBoost");

        Vehicle vehicle = Vehicle.Cast(ent);	
        VehicleBaseSimulation simulation = VehicleBaseSimulation.Cast(vehicle.FindComponent(VehicleBaseSimulation));
        VehicleWheeledSimulation vWheelSimu = VehicleWheeledSimulation.Cast(simulation);
        
        vWheelSimu.EngineSetPeakTorqueState(200);
        vWheelSimu.EngineSetPeakPowerState(250);
		
		array<SCR_ParticleEmitter> emittersList;
		emittersList = findEmitters(ent);
		foreach (SCR_ParticleEmitter emitter : emittersList)
			emitter.Play();
		GetGame().GetCallqueue().Remove(RemoveBoost); // Remove previous delayed calls to remove boost so that they don't conflict
		GetGame().GetCallqueue().CallLater(RemoveBoost, m_iBoostDuration_ms, false, vWheelSimu, emittersList);
    }
	
	// This must be static, otherwise when we remove boost from callqueue, we remove only boost
	// caused by this trigger, and not all another boost trigger
	static void RemoveBoost(VehicleWheeledSimulation vWheelSimu, array<SCR_ParticleEmitter> emittersList)
	{
		PrintString("RemoveBoost");
		
		if (!vWheelSimu)
			return;
		
		vWheelSimu.EngineSetPeakTorqueState(80);
        vWheelSimu.EngineSetPeakPowerState(180);
		foreach (SCR_ParticleEmitter emitter : emittersList)
			emitter.Stop();
	}
	
	array<SCR_ParticleEmitter> findEmitters(IEntity vehicle) 
	{
		array<SCR_ParticleEmitter> emitters = new array<SCR_ParticleEmitter>;
		IEntity child = vehicle.GetChildren();
		while (child)
		{
			SCR_ParticleEmitter childSlot = SCR_ParticleEmitter.Cast(child);
			if (childSlot)
				if (childSlot.GetPathToPTC() == UDR_BoostVehicleTrigger.BOOST_PARTICLE_NAME)
					emitters.Insert(childSlot);
			
			child = child.GetSibling();
		}
		
		return emitters;
	}
}