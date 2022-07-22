/*
This component implements race track logic:
- Reacting to events when a car activates a waypoint
- Tracking progress of cars: lap count, selection next waypoint
It must be attached to some entity in the world (not to car, not to player, not to player controller).
*/

class UDR_RaceTrackLogicRacerData : Managed
{
	int m_iNextWaypoint;
	int m_iPrevWaypoint;
	int m_iLapCount;
	float m_fLapProgress;	// Total distance traveled in current lap
	float m_fTotalProgress;	// Total distance travelled, including previous laps
}


class UDR_RaceTrackLogicComponentClass : ScriptComponentClass
{
}

class UDR_RaceTrackLogicComponent : ScriptComponent
{
	[Attribute("", UIWidgets.Auto, desc: "Names of UDR_Waypoint entities. There must be at least two of waypoints.")]
	protected ref array<string> m_aWaypointNames;
	
	// Waypoints
	protected ref array<UDR_Waypoint> m_aWaypoints = {};	// Entities
	protected ref array<vector> m_aWaypointPositions = {};	// Positiuon
	protected ref array<float> m_aWaypointDistances = {};	// Distance from that WP to next WP
	protected float m_fLapLength;
	
	protected ref map<IEntity, ref UDR_RaceTrackLogicRacerData> m_RacerData = new map<IEntity, ref UDR_RaceTrackLogicRacerData>;
	
	protected bool m_bInitSuccess = false;
	
	//----------------------------------------------------------------------------------------------
	// Registration of racers
	
	//----------------------------------------------------------------------------------------------
	// Registers a racer
	// racerEnt - entity which will be activating triggers (the vehicle entity)
	// racerData - the data object of this racer, can be passed in case of switching of vehicles, when the new vehicle inherits progress of the old one.
	void RegisterRacer(notnull IEntity racerEnt, UDR_RaceTrackLogicRacerData racerData = null)
	{
		_print(string.Format("RegisterRacer: %1", racerEnt));
		
		UDR_RaceTrackLogicRacerData oldRacerData = m_RacerData.Get(racerEnt);
		
		if (oldRacerData)
		{
			_print(string.Format("Racer is already registered: %1", racerEnt), LogLevel.ERROR);
			return;
		}
		
		if (racerData != null)
		{
			m_RacerData.Insert(racerEnt, racerData);
		}
		else
		{
			int firstWaypointId = 1;
			
			UDR_RaceTrackLogicRacerData newRacerData = new UDR_RaceTrackLogicRacerData();
			newRacerData.m_iNextWaypoint = firstWaypointId;
			int prevWpId = firstWaypointId - 1;
			if (prevWpId < 0)
				prevWpId = m_aWaypoints.Count() - 1;
			newRacerData.m_iPrevWaypoint = prevWpId;
			m_RacerData.Insert(racerEnt, newRacerData);
		}
		
	}
	
	//----------------------------------------------------------------------------------------------
	void UnregisterRacer(notnull IEntity racerEnt)
	{
		_print(string.Format("UnregisterRacer: %1", racerEnt));
		
		UDR_RaceTrackLogicRacerData racerData = m_RacerData.Get(racerEnt);
		
		if (!racerData)
		{
			_print(string.Format("Racer is not registerd: %1", racerEnt), LogLevel.ERROR);
			return;
		}
		
		m_RacerData.Remove(racerEnt);
	}
	
	//----------------------------------------------------------------------------------------------
	// Combines both register and unregister: unregisters old racer, registers new racer,
	// And makes the new racer inherit Racer Data of the old one.
	void SwitchRacer(IEntity oldRacerEnt, IEntity newRacerEnt)
	{
		UDR_RaceTrackLogicRacerData oldRacerData = m_RacerData.Get(oldRacerEnt);
		
		if (!oldRacerData)
		{
			_print("SwitchRacer: could not find the old racer entity!");
			return;
		}
		
		UnregisterRacer(oldRacerEnt);
		RegisterRacer(newRacerEnt, oldRacerData);
	}
	
	//----------------------------------------------------------------------------------------------
	bool GetRacerData(IEntity racerEnt, out float outTotalProgress, out int outLapCount, out int outNextWaypoint)
	{
		UDR_RaceTrackLogicRacerData racerData = m_RacerData.Get(racerEnt);
		
		if (!racerData)
			return false;
		
		outTotalProgress = racerData.m_fTotalProgress;
		outLapCount = racerData.m_iLapCount;
		outNextWaypoint = racerData.m_iNextWaypoint;
		return true;
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, true);
		SetEventMask(owner, EntityEvent.FIXEDFRAME | EntityEvent.INIT | EntityEvent.DIAG);
	}
	
	//----------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		int wpCount = m_aWaypointNames.Count();
		
		// Bail if there are not enough waypoints
		if (wpCount < 2)
		{
			_print("There must be at least 2 waypoints!", LogLevel.ERROR);
			return;
		}
		
		//-----------------------------------------------
		// If we are in editor, just check if all waypoint names are correct and return
		#ifdef WORKBENCH
		
		if (!GetGame().InPlayMode())
		{
			/*
			// Disabled this shit because it looks like it can't find entities when reloading a world and thus spams this sometimes
			WorldEditor worldEditor = Workbench.GetModule(WorldEditor); // Watch out, WorldEditor class doesn't exist not in Workbench
			WorldEditorAPI api = worldEditor.GetApi();
			
			foreach (string wpName : m_aWaypointNames)
			{
				IEntitySource wpEntSrc = api.FindEntityByName(wpName);
				if (!wpEntSrc)
					_print(string.Format("Could not find waypoint: %1", wpName), LogLevel.ERROR);
			}
			*/
			return;
		}
		#endif
		
		//-----------------------------------------------
		// Not in editor, but in actual game
		m_aWaypoints.Resize(wpCount);
		m_aWaypointPositions.Resize(wpCount);
		m_aWaypointDistances.Resize(wpCount);
		int nWaypointsNotFound = 0;
		foreach (int i, string wpName : m_aWaypointNames)
		{
			IEntity wpEnt = GetGame().FindEntity(wpName);
			if (!wpEnt)
			{
				nWaypointsNotFound++;
				_print(string.Format("Could not find waypoint: %1", wpName), LogLevel.ERROR);
				continue;
			}
			
			UDR_Waypoint wp = UDR_Waypoint.Cast(wpEnt);
			
			if (!wp)
			{
				nWaypointsNotFound++;
				_print(string.Format("Waypoint must be of UDR_Waypoint class: %1, %2", wpEnt, wpEnt.GetName()), LogLevel.ERROR);
				continue;
			}
			
			m_aWaypoints[i] = wp;
			m_aWaypointPositions[i] = wp.GetOrigin();
			
			wp.m_OnActivated.Insert(Callback_OnWaypointActivated);
			
		}
		
		if (nWaypointsNotFound == 0)
		{
			float trackLength = 0;
			for (int i = 0; i < wpCount; i++)
			{
				int nextWpId = (i + 1) % wpCount;
				
				vector thisWpPos = m_aWaypointPositions[i];
				vector nextWpPos = m_aWaypointPositions[nextWpId];
				float distance = vector.Distance(thisWpPos, nextWpPos);
				m_aWaypointDistances[i] = distance;
				trackLength += distance;
			}
			m_fLapLength = trackLength;
			
			m_bInitSuccess = true;
		}
		else
		{
			_print("Failed to initialize. Race track logic will not work!", LogLevel.ERROR);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	//override void EOnFixedFrame(IEntity owner, float timeSlice)
	//{
	//	
	//}
	
	//----------------------------------------------------------------------------------------------
	void UpdateAllRacers()
	{
		foreach (IEntity racerEnt, UDR_RaceTrackLogicRacerData racerData : m_RacerData)
		{
			if (!racerEnt)
				continue;
			
			UpdateRacer(racerEnt, racerData);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.UDR_SHOW_RACE_TRACK_LOGIC_PANEL))
		{
			DbgUI.Begin(string.Format("RaceTrackLogic %1", this));
			
			int i = 0;
			foreach (IEntity racerEnt, UDR_RaceTrackLogicRacerData racerData : m_RacerData)
			{
				if (!racerEnt)
					continue;
				
				DbgUI.Text(string.Format("%1 Waypoint ID: Next: %2, Prev: %3", i, racerData.m_iNextWaypoint, racerData.m_iPrevWaypoint));
				DbgUI.Text(string.Format("%1 Lap Count:      %2", i, racerData.m_iLapCount));
				DbgUI.Text(string.Format("%1 Lap Progress:   %2", i, racerData.m_fLapProgress));
				DbgUI.Text(string.Format("%1 Total Progress: %2", i, racerData.m_fTotalProgress));
				DbgUI.PlotLive(string.Format("%1 Total Progress", i), 400, 200, racerData.m_fTotalProgress, 100); 
				
				i++;
			}
			
			DbgUI.End();
		}
	}
	
	
	//----------------------------------------------------------------------------------------------
	void Callback_OnWaypointActivated(UDR_Waypoint wp, Vehicle veh)
	{
		if (!m_bInitSuccess)
			return;
		
		UDR_RaceTrackLogicRacerData racerData = m_RacerData.Get(veh);
		
		if (!racerData)
		{
			_print(string.Format("Waypoint activated by racer which is not registered: %1", veh), LogLevel.ERROR);
			return;
		}
		
		int wpId = m_aWaypoints.Find(wp);
		
		// Ignore if racer activated wrong waypoint
		if (wpId != racerData.m_iNextWaypoint)
		{
			_print(string.Format("Wrong waypoint activation order: racer %1, wp id: %2", veh, wpId), LogLevel.WARNING);
			return;
		}
		
		int nextWpId = (wpId + 1) % m_aWaypoints.Count();
		
		// If racer crossed the finish line, increase lap count
		if (wpId == 0)
		{
			racerData.m_iLapCount++;
		}
		
		racerData.m_iNextWaypoint = nextWpId;
		racerData.m_iPrevWaypoint = wpId;
	}
	
	//----------------------------------------------------------------------------------------------
	void UpdateRacer(notnull IEntity racerEnt, notnull UDR_RaceTrackLogicRacerData racerData)
	{
		// Route progress of current lap
		float lapProgress = 0;
		for (int i = 0; i < racerData.m_iPrevWaypoint; i++)
		{
			lapProgress += m_aWaypointDistances[i];
		}
		float distanceFromPrevWp = vector.Distance(racerEnt.GetOrigin(), m_aWaypointPositions[racerData.m_iPrevWaypoint]);
		lapProgress += distanceFromPrevWp;
		
		racerData.m_fLapProgress = lapProgress;
		racerData.m_fTotalProgress = racerData.m_iLapCount * m_fLapLength + lapProgress;
	}
	
	//----------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		Print(string.Format("UDR_RaceTrackLogicComponent %1: %2", GetOwner().GetName(), str), logLevel);
	}
}