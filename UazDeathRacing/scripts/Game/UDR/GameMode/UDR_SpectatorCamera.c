class UDR_SpectatorCameraClass : CameraBaseClass
{
}

class UDR_SpectatorCamera : CameraBase
{
	[Attribute("UDR_SpectatorCamera", UIWidgets.EditBox)]
	protected string m_sActionContext;
		
	protected static UDR_SpectatorCamera s_Instance;
	
	protected vector m_vYawPitchRoll = "0 -45 0";
	protected float m_fFollowDistance = 8.0;
	protected IEntity m_Target;
	protected vector m_vLastTargetPos;
	
	// Returns existing instance or creates a new one
	static UDR_SpectatorCamera GetInstance()
	{
		if (s_Instance)
			return s_Instance;
		
		ResourceName cameraResource = "{32D483EF0D31400D}Prefabs/GameMode/SpectatorCamera.et";
		Resource res = Resource.Load(cameraResource);
		IEntity cameraEnt = GetGame().SpawnEntityPrefabLocal(res);
		s_Instance = UDR_SpectatorCamera.Cast(cameraEnt);
		return s_Instance;
	}
	
	static void Destroy()
	{
		if (!s_Instance)
			return;
		
		SCR_Global.DeleteEntityAndChildren(s_Instance);
	}
	
	void SwitchToThisCamera()
	{
		GetGame().GetCameraManager().SetCamera(this);
	}
	
	void FollowEntity(IEntity ent)
	{
		if (!ent)
			return;
		
		m_Target = ent;
	}
	
	void UDR_SpectatorCamera(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(EntityEvent.INIT | EntityEvent.POSTFRAME);
		SetName("UDR_SpectatorCamera"); // Must keep this name, otherwise if it's not named, camera manager will reject this camera
	}
	
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		vector targetPos = m_vLastTargetPos;
		if (m_Target)
			targetPos = m_Target.GetOrigin();
		m_vLastTargetPos = targetPos;
		
		//------------------------------------
		// Handle inputs
		
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.ActivateContext("UDR_SpectatorContext", 0);
		float yawInput = inputMgr.GetActionValue("UDR_SpectatorCameraRotateYaw");
		float pitchInput = inputMgr.GetActionValue("UDR_SpectatorCameraRotatePitch");
		//float zoomInput = inputMgr.GetActionValue("ManualCameraZoom");
		
		float y = m_vYawPitchRoll[0];
		float p = m_vYawPitchRoll[1];
		
		y += timeSlice * yawInput;
		if (y >= 360.0)
			y -= 360.0;
		if (y < 0.0)
			y += 360.0;
		
		p -= timeSlice * pitchInput;
		if (p >= 90.0)
			p = 90.0;
		if (p <= 0.0)
			p = 0.0;
		
		m_vYawPitchRoll[0] = y;
		m_vYawPitchRoll[1] = p;

		//------------------------------------
		// Calculate camera transform
		
		// Coordinate system for the camera
		// It's facing outside of target! Remember to reverse vectors later.
		vector dir = m_vYawPitchRoll.AnglesToVector();
		vector upAngles = m_vYawPitchRoll;
		upAngles[1] = upAngles[1] + 90.0;
		vector up = upAngles.AnglesToVector();
		vector aside = dir * up;
		
		vector cameraPos = targetPos + m_fFollowDistance * dir;
		
		vector transform[4];
		transform[0] = -aside;
		transform[1] = up;
		transform[2] = -dir;
		transform[3] = cameraPos;
		
		SetTransform(transform);
		
		/*
		#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.UDR_SHOW_SPECTATOR_PANEL))
		{
			DbgUI.Begin("Spectator Camera");
			
			DbgUI.Text(string.Format("Yaw:   %1", y));
			DbgUI.Text(string.Format("Pitch: %1", p));
			
			
			DbgUI.End();
		}
		#endif
		*/
	}
}