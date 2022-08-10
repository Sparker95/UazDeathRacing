modded class PauseMenuUI
{
	override override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		SCR_ButtonTextComponent comp;
		
		// Join Race
		comp = SCR_ButtonTextComponent.GetButtonText("JoinRace", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnJoinRaceButton);
		}
		
		// Join Spectators
		comp = SCR_ButtonTextComponent.GetButtonText("JoinSpectators", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnSpectateButton);
		}
	}
	
	protected void OnJoinRaceButton()
	{
		UDR_PlayerNetworkComponent comp = UDR_PlayerNetworkComponent.GetLocal();
		comp.Client_RequestJoinRace();
	}
	
	protected void OnSpectateButton()
	{
		UDR_PlayerNetworkComponent comp = UDR_PlayerNetworkComponent.GetLocal();
		comp.Client_RequestJoinSpectators();
	}
}