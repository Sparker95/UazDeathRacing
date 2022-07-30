class UDR_HudSpectator : UDR_HudBase
{
	protected ref UDR_HudSpectatorWidgets widgets = new UDR_HudSpectatorWidgets();
	
	UDR_SpectatorComponent m_Spectator;
	
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		widgets.Init(GetRootWidget());
		GetRootWidget().SetOpacity(0);
		
		PlayerController pc = GetGame().GetPlayerController();
		m_Spectator = UDR_SpectatorComponent.Cast(pc.FindComponent(UDR_SpectatorComponent));
	}
	
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_Spectator.IsSpectatorActive())
		{
			Show(false);
			return;
		}
		
		Show(true);
		UDR_PlayerNetworkEntity target = m_Spectator.GetCurrentTarget();
		
		if (!target)
		{
			// Not spectating anyone
			widgets.m_PlayerNameText.SetText("There are no players to spectate!");
			widgets.m_PositionText.SetText("-");
			widgets.m_LapCountText.SetText("-");
		}
		else
		{
			// Spectating someone
			int targetPlayerId = target.m_iPlayerId;
			string playerName = GetGame().GetPlayerManager().GetPlayerName(targetPlayerId);
			widgets.m_PlayerNameText.SetText(playerName);
			
			UpdateRacePositionWidgets(target, widgets.m_PositionText, widgets.m_LapCountText);
		}
		
		UpdateNotificationWidget(widgets.m_NotificationText);
	}
}