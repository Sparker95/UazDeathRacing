/*
Pickup effect handles what happens when user picks it up.
Effects are embedded into UDP_Pickup entity as properties.
*/

[BaseContainerProps()]
class UDR_PickupEffectBase
{
	// Override those in inherited classes
	
	// Called on authority to apply the actual effect. 
	// Return true to cause the pickup to disappear and evenually respawn.
	// Return false to ignore the pickup from being picked up.
	bool Authority_ApplyEffect(IEntity ent);
	
	// Helper functions
	
	// Sends RPCs to play a UI sound for all occupants of a vehicle
	void SendUiEventToVehicle(IEntity vehicleEnt, string uiSoundEvent)
	{
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		
		BaseCompartmentManagerComponent compartmentMgr = BaseCompartmentManagerComponent.Cast(vehicleEnt.FindComponent(BaseCompartmentManagerComponent));
		array<BaseCompartmentSlot> compartments = {};
		compartmentMgr.GetCompartments(compartments);
		foreach (BaseCompartmentSlot slot : compartments)
		{
			IEntity occupant = slot.GetOccupant();
			if (!occupant)
				continue;
			
			int playerId = playerMgr.GetPlayerIdFromControlledEntity(occupant); // Returns 0 if not found
			if (playerId == 0)
				continue;
			
			PlayerController controller = playerMgr.GetPlayerController(playerId);
			if (!controller)
				continue;
			
			UDR_PlayerNetworkComponent udrNetwork = UDR_PlayerNetworkComponent.Cast(controller.FindComponent(UDR_PlayerNetworkComponent));
			udrNetwork.Authority_SendUiSoundEvent(uiSoundEvent);
		}
	}
}