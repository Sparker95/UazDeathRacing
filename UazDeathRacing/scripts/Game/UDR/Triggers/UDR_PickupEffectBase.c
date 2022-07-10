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
}