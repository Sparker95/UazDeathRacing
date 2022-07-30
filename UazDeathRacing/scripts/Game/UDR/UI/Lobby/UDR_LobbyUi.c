class UDR_LobbyUi : ScriptedWidgetComponent
{
	protected ref UDR_LobbyWidgets widgets = new UDR_LobbyWidgets();
	
	protected SCR_BaseGameMode m_GameMode;
	
	override void HandlerAttached(Widget w)
	{
		widgets.Init(w);
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (m_GameMode)
		{
			m_GameMode.GetOnPlayerRegistered().Insert(OnPlayerRegistered);
			m_GameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
		}
		
		widgets.m_JoinRaceButtonComponent.m_OnClicked.Insert(OnJoinRaceButton);
		widgets.m_SpectateButtonComponent.m_OnClicked.Insert(OnSpectateButton);
		
		CreatePlayerList();
		GetGame().GetCallqueue().CallLater(Update, 0, true);
	}
	
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(Update);
		
		if (m_GameMode)
		{
			m_GameMode.GetOnPlayerRegistered().Remove(OnPlayerRegistered);
			m_GameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnected);
		}
	}
	
	// Called each frame
	protected void Update()
	{
		
	}
	
	void CreatePlayerList()
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		pm.GetPlayers(playerIds);
		
		foreach (int id : playerIds)
		{
			CreatePlayerLine(id);
		}
	}
	
	void CreatePlayerLine(int playerId)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(UDR_PlayerLineWidgets.s_sLayout, widgets.m_PlayerList);
		UDR_PlayerLineComponent comp = UDR_PlayerLineComponent.Cast(w.FindHandler(UDR_PlayerLineComponent));
		comp.Init(playerId);
	}
	
	void DeletePlayerLine(int playerId)
	{
		Widget w = widgets.m_PlayerList.GetChildren();
		
		while (w)
		{
			UDR_PlayerLineComponent comp = UDR_PlayerLineComponent.Cast(w.FindHandler(UDR_PlayerLineComponent));
			if (comp.GetPlayerId() == playerId)
			{
				widgets.m_PlayerList.RemoveChild(w);
				break;
			}
			w = w.GetSibling();
		}
	}
	
	protected void OnPlayerRegistered(int playerId)
	{
		CreatePlayerLine(playerId);
	}
	
	protected void OnPlayerDisconnected(int playerId)
	{
		DeletePlayerLine(playerId);
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