/*
Component to be attached to player controller which implements spectator mode.
*/

class UDR_SpectatorComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class UDR_SpectatorComponent : ScriptComponent
{
	protected bool m_bActive = false;
	protected UDR_PlayerNetworkEntity m_CurrentTarget;
	protected int m_iCurrentTargetId = 0; // Index in the array of targets. NOT a player ID!
	
	protected UDR_PlayerNetworkEntity m_MyNetworkEntity;
	
	//------------------------------------------------------------------------------------------------
	// Public interface
	
	//------------------------------------------------------------------------------------------------
	UDR_PlayerNetworkEntity GetCurrentTarget()
	{
		return m_CurrentTarget;
	}
	
	bool IsSpectatorActive()
	{
		return m_bActive;
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Private

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		InputManager im = GetGame().GetInputManager();
		
		im.AddActionListener("UDR_SpectatorNext", EActionTrigger.DOWN, OnActionNext);
		im.AddActionListener("UDR_SpectatorPrevious", EActionTrigger.DOWN, OnActionPrevious);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// We perform synchronization through our PlayerNetworkEntity
		if (!m_MyNetworkEntity)
		{
			int myPlayerId = GetGame().GetPlayerController().GetPlayerId();
			m_MyNetworkEntity = GetGame().GetUdrGameMode().GetPlayerNetworkSyncEntity(myPlayerId);
		}
		
		// It might be not loaded yet
		if (!m_MyNetworkEntity)
			return;
		
		if (m_bActive)
		{
			if (!m_MyNetworkEntity.m_bSpectating)
			{
				// Deactivate spectator
				DeactivateSpectator();
				m_bActive = false;
			}
			else
			{
				// Check that we are spectating a valid target, if not, switch to one
				bool findNewTarget = false;
				
				if (!m_CurrentTarget)
					findNewTarget = true;
				else
					findNewTarget = !IsValidSpectateTarget(m_CurrentTarget);
				
				if (findNewTarget)
					SpectateOffset(0);				
			}
		}
		else
		{
			if (m_MyNetworkEntity.m_bSpectating)
			{
				// Activate spectator mode
				ActivateSpectator();
				m_bActive = true;
			}
		}
	}

	
	
	//------------------------------------------------------------------------------------------------
	void ActivateSpectator()
	{
		// On next update the component will detect that we are not spectating anything
		// And it will switch to a valid target
		// For now just initialize the camera
		SpectateFallbackTarget();
	}
	
	//------------------------------------------------------------------------------------------------
	void DeactivateSpectator()
	{
		UDR_SpectatorCamera.Destroy();
		CameraManager cameraManager = GetGame().GetCameraManager();
		cameraManager.SetNextCamera();
	}
	
	//------------------------------------------------------------------------------------------------
	array<UDR_PlayerNetworkEntity> FindValidSpectateTargets()
	{
		array<UDR_PlayerNetworkEntity> players = GetGame().GetUdrGameMode().GetAllPlayerNetworkEntities();
		array<UDR_PlayerNetworkEntity> validSpectateTargets = {};
		foreach (UDR_PlayerNetworkEntity player : players)
		{
			if (IsValidSpectateTarget(player))
				validSpectateTargets.Insert(player);
		}
		
		return validSpectateTargets;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsValidSpectateTarget(notnull UDR_PlayerNetworkEntity player)
	{
		return !player.m_bSpectating && player.m_AssigedVehicleId.IsValid();
	}
	
	//------------------------------------------------------------------------------------------------
	void SpectatePlayer(notnull UDR_PlayerNetworkEntity target, int targetId)
	{
		RplComponent targetVehicleRpl = RplComponent.Cast(Replication.FindItem(target.m_AssigedVehicleId));
		if (!targetVehicleRpl)
			return;
		IEntity targetVehicle = targetVehicleRpl.GetEntity();
		if (!targetVehicle)
			return;
		
		UDR_SpectatorCamera camera = UDR_SpectatorCamera.GetInstance();
		camera.SwitchToThisCamera();
		camera.FollowEntity(targetVehicle);
		
		m_iCurrentTargetId = targetId;
		m_CurrentTarget = target;
	}
	
	//------------------------------------------------------------------------------------------------
	// Can be used when there is no valid target to spectate but we don't want to leave the camera hang in the ocean
	void SpectateFallbackTarget()
	{
		UDR_SpectatorCamera camera = UDR_SpectatorCamera.GetInstance();
		camera.SwitchToThisCamera();
		camera.FollowEntity(GetGame().GetUdrGameMode().GetFallbackSpectatorTarget());
		
		m_iCurrentTargetId = 0;
		m_CurrentTarget = null;
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		//owner.SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// Finds another valid target for spectating
	// Offset is 1, -1 or 0
	protected void SpectateOffset(int offset)
	{
		array<UDR_PlayerNetworkEntity> validTargets = FindValidSpectateTargets();
		
		if (validTargets.IsEmpty())
		{
			SpectateFallbackTarget();
			return;
		}
		
		int currentTargetId = -1;
		
		if (m_CurrentTarget)
			currentTargetId = validTargets.Find(m_CurrentTarget);
		else
			currentTargetId = m_iCurrentTargetId; // Hopefully it's a last valid target id, if target has disconnected
		
		if (currentTargetId == -1)
		{
			SpectatePlayer(validTargets[0], 0);
			return;
		}
		
		int count = validTargets.Count();
		currentTargetId = currentTargetId + offset;
		if (currentTargetId >= count)
			currentTargetId = currentTargetId % count;
		else if (currentTargetId < 0)
			currentTargetId = count - 1;
		
		SpectatePlayer(validTargets[currentTargetId], currentTargetId);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActionNext()
	{
		SpectateOffset(1);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActionPrevious()
	{
		SpectateOffset(-1);
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	void UDR_SpectatorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~UDR_SpectatorComponent()
	{
	}
	*/
}
