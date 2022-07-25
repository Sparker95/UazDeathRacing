modded class BaseCompartmentSlot : ExtBaseCompartmentSlot
{
	override void KillOccupant(IEntity instigator = null, bool eject = false, bool gettingIn = false, bool gettingOut = false)
	{
		IEntity controlledEntity = GetOccupant();
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return;
		
		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		CompartmentAccessComponent access = character.GetCompartmentAccessComponent();
		access.EjectOutOfVehicle();
		controller.GetInputContext().SetVehicleCompartment(null);

		// re enable damages so that player can get run over
		damageManager.EnableDamageHandling(true);
	}
}