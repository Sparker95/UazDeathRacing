class UDR_BoostVehicleTriggerClass: ScriptedGameTriggerEntityClass
{
};

class UDR_BoostVehicleTrigger: ScriptedGameTriggerEntity
{
    override void OnActivate(IEntity ent)
	{
		PrintString("activateBoost");

        Vehicle vehicle = Vehicle.Cast(ent);
        VehicleBaseSimulation simulation = VehicleBaseSimulation.Cast(vehicle.FindComponent(VehicleBaseSimulation));
        VehicleWheeledSimulation vWheelSimu = VehicleWheeledSimulation.Cast(simulation);
        
        vWheelSimu.EngineSetPeakTorqueState(200);
        vWheelSimu.EngineSetPeakPowerState(250);
		
		GetGame().GetCallqueue().CallLater(RemoveBoost, 2000, false, vWheelSimu);
    }
	
	void RemoveBoost(VehicleWheeledSimulation vWheelSimu)
	{
		PrintString("RemoveBoost");
		
		vWheelSimu.EngineSetPeakTorqueState(80);
        vWheelSimu.EngineSetPeakPowerState(180);
	}
}