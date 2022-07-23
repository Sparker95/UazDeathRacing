class UDR_PlayerLineComponent : ScriptedWidgetComponent
{
	protected ref UDR_PlayerLineWidgets widgets = new UDR_PlayerLineWidgets();
	
	protected int m_iPlayerId = -1;
	
	void Init(int playerId)
	{
		m_iPlayerId = playerId;
		UpdateAllWidgets();
	}
	
	int GetPlayerId()
	{
		return m_iPlayerId;
	}
	
	override void HandlerAttached(Widget w)
	{
		widgets.Init(w);
		
		GetGame().GetCallqueue().CallLater(Update, 0, true);
	}
	
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(Update);
	}
	
	protected void Update()
	{
		UpdateAllWidgets();
	}
	
	protected void UpdateAllWidgets()
	{
		if (m_iPlayerId == -1)
			return;
		
		PlayerManager pm = GetGame().GetPlayerManager();

		// Player name
		string playerName = pm.GetPlayerName(m_iPlayerId);
		widgets.m_PlayerNameText.SetText(playerName);
		
		UDR_GameMode gm = UDR_GameMode.Cast(GetGame().GetGameMode());
		
		UDR_PlayerNetworkEntity networkSync = gm.GetPlayerNetworkSyncEntity(m_iPlayerId);
		
		if (!networkSync)
			return;
		
		/*
		PlayerController pc = pm.GetPlayerController(m_iPlayerId);
		if (!pc)
			return;
		
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.Cast(pc.FindComponent(UDR_PlayerNetworkComponent));
		if (!playerComp)
			return;
		*/
		
		// Position
		widgets.m_PositionText.SetText((networkSync.m_iPositionInRace + 1).ToString());
		
		// Lap
		widgets.m_CurrentLapText.SetText((networkSync.m_iLapCount + 1).ToString());
		
		// Best lap time
		widgets.m_BestLapText.SetText("");
	}
}