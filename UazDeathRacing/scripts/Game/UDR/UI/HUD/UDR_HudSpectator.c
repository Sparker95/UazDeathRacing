class UDR_HudSpectator : SCR_InfoDisplay
{
	protected ref UDR_HudSpectatorWidgets widgets = new UDR_HudSpectatorWidgets();
	
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		widgets.Init(GetRootWidget());
		GetRootWidget().SetOpacity(0);
	}
	
	
}