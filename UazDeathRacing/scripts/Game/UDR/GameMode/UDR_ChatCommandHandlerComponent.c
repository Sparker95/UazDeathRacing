/*
Component for handling chat commands
*/

class UDR_ChatCommandHandlerComponentClass : ScriptComponentClass {}

class UDR_ChatCommandHandlerComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// todo find a better way to solve initialization of this on clients
		UDR_GameMode gm = GetGame().GetUdrGameMode();
		if (!gm)
			return;
		
		// Bail if we are initializing this for some other player
		if (gm.m_LocalChatCommandHandler)
			return;
		
		// This it to prevent multiple registration of chat commands for each player connected
		gm.m_LocalChatCommandHandler = this;
		
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		
		ChatCommandInvoker invokerHelp = mgr.GetCommandInvoker("help");
		invokerHelp.Insert(OnCommand_Help);
		
		ChatCommandInvoker invokerTracks = mgr.GetCommandInvoker("gettracks");
		invokerTracks.Insert(OnCommand_Tracks);
		
		ChatCommandInvoker invokerEndRace = mgr.GetCommandInvoker("endrace");
		invokerEndRace.Insert(OnCommand_EndRace);
		
		ChatCommandInvoker invokerSetTrack = mgr.GetCommandInvoker("settrack");
		invokerSetTrack.Insert(OnCommand_SetTrack);
	}
	
	//---------------------------------------------------------------------------------
	protected void OnCommand_Help(SCR_ChatPanel panel, string data)
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		
		array<string> msgs = {
			"Available commands:",
			"/help - Show this message",
			"/gettracks - List all race tracks",
			"/endrace - End current race",
			"/settrack id - Switch to race track with given id"
		};
		
		foreach (string msg : msgs)
			mgr.OnNewMessage(msg);
	}
	
	//---------------------------------------------------------------------------------
	protected void OnCommand_Tracks(SCR_ChatPanel panel, string data)
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		UDR_GameMode gm = GetGame().GetUdrGameMode();
		array<UDR_RaceTrackLogic> allRaceTracks = gm.GetAllRaceTracks();
		
		foreach (int i, UDR_RaceTrackLogic track : allRaceTracks)
		{
			string str = string.Format("%1 - %2", i, track.GetRaceTrackName());
			mgr.OnNewMessage(str);
		}
	}
	
	//---------------------------------------------------------------------------------
	protected void OnCommand_EndRace(SCR_ChatPanel panel, string data)
	{
		if (!IsAdmin())
		{
			ShowErrorAdminOnly();
			return;
		}
		
		Rpc(RpcAsk_EndRace);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_EndRace()
	{
		GetGame().GetUdrGameMode().Ask_AdminEndRace();
	}
	
	//---------------------------------------------------------------------------------
	protected void OnCommand_SetTrack(SCR_ChatPanel panel, string data)
	{
		if (!IsAdmin())
		{
			ShowErrorAdminOnly();
			return;
		}
		
		// Extract race track id
		int trackId = data.ToInt();
		
		Rpc(RpcAsk_SetTrack, trackId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetTrack(int trackId)
	{
		GetGame().GetUdrGameMode().Ask_AdminSwitchRaceTrack(trackId);
	}
	
	protected void ShowErrorAdminOnly()
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		mgr.OnNewMessage("Error: this command is only for admins");
	}
	
	protected bool IsAdmin()
	{
		PlayerController pc = PlayerController.Cast(GetOwner());
		return pc.HasRole(EPlayerRole.ADMINISTRATOR) || pc.HasRole(EPlayerRole.ADMINISTRATOR);
	}
}
