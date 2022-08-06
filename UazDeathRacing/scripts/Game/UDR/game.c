modded class ArmaReforgerScripted
{
	override bool OnGameStart()
	{
		super.OnGameStart();
		
		UDR_DebugMenu.Init();
		
		return true;
	}
	
	override void OnUpdate(BaseWorld world, float timeslice)
	{
		super.OnUpdate(world, timeslice);
		
		GetInputManager().ActivateContext("TurretContext", 50);
		
		#ifdef WORKBENCH
		UDR_DebugMenu.UpdateMenus();
		#endif
	}
	
	UDR_GameMode GetUdrGameMode()
	{
		return UDR_GameMode.Cast(GetGameMode());
	}
}