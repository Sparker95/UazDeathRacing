class UDR_VehiclePositioningClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
/*!
	Entity for calculation of positions for vehicles at race start.
*/
class UDR_VehiclePositioning : GenericEntity
{
	[Attribute("{1DEFD52732318765}Prefabs/VehiclePositioning/VehiclePreview.et", UIWidgets.ResourceNamePicker)]
	protected ResourceName m_sVehiclePreviewPrefab;
	
	[Attribute("2", UIWidgets.EditBox)]
	protected int m_iNumRows;
	
	[Attribute("2", UIWidgets.EditBox)]
	protected int m_iNumColumns;
	
	[Attribute("5.0", UIWidgets.EditBox)]
	protected float m_fStepRows;
	
	[Attribute("5.0", UIWidgets.EditBox)]
	protected float m_fStepColumns;
	
	protected ref array<IEntity> m_aPreviews = {};
	
	// Assignment of players to spawn positions
	protected ref array<int> m_aAssignedPlayers = {};

	//------------------------------------------------------------------------------------------------
	// Returs position at given ID in world coordinates
	void GetPositionTransform(int id, out vector outTransform[])
	{
		if (id >= m_iNumColumns*m_iNumRows)
			Print(string.Format("UDR_VehiclePositioning: GetPosition: id %1 is more than predefined size", id), LogLevel.ERROR);
		
		int col = id % m_iNumColumns;
		int row = id / m_iNumColumns;
		
		vector localPos = GetPositionOffset(row, col);
		vector worldPos = CoordToParent(localPos);
		
		vector myTransform[4];
		GetWorldTransform(myTransform);
		
		outTransform[0] = myTransform[0]; // Aside - those three represent direction and are same as direction of this entity
		outTransform[1] = myTransform[1]; // Up
		outTransform[2] = myTransform[2]; // Dir
		outTransform[3] = worldPos;
	}
	
	
	//------------------------------------------------------------------------------------------------
	// Assignment of vehicle positions
	
	//------------------------------------------------------------------------------------------------
	int FindNextFreePosition()
	{
		int count = m_aAssignedPlayers.Count();
		for (int i = 0; i < count; i++)
		{
			if (m_aAssignedPlayers[i] == -1)
				return i;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	int FindAssignedPosition(int playerId)
	{
		int count = m_aAssignedPlayers.Count();
		for (int i = 0; i < count; i++)
		{
			if (m_aAssignedPlayers[i] == playerId)
				return i;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRandomPosition()
	{
		return Math.RandomInt(0, m_iNumRows*m_iNumColumns);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerAssignedToPosition(int positionId)
	{
		if (positionId < 0 || positionId >= m_aAssignedPlayers.Count())
			return -1;
		
		return m_aAssignedPlayers[positionId];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPositionCount()
	{
		return m_iNumRows * m_iNumColumns;
	}
	
	//------------------------------------------------------------------------------------------------
	// Assigns position to player
	void AssignPosition(int positionId, int playerId)
	{
		if (positionId < 0 || positionId >= m_aAssignedPlayers.Count())
			return;
		
		m_aAssignedPlayers[positionId] = playerId; // Assigned to this player
	}
	
	//------------------------------------------------------------------------------------------------
	void UnassignPosition(int positionId)
	{
		if (positionId < 0 || positionId >= m_aAssignedPlayers.Count())
			return;
		
		m_aAssignedPlayers[positionId] = -1; // Not assigned to anyone
	}
	
	//------------------------------------------------------------------------------------------------
	void UnassignPlayer(int playerId)
	{
		int count = m_aAssignedPlayers.Count();
		for (int i = 0; i < count; i++)
		{
			if (m_aAssignedPlayers[i] == playerId)
			{
				m_aAssignedPlayers[i] = -1;
				return;
			}
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void ClearAssignments()
	{
		for (int i = 0; i < m_aAssignedPlayers.Count(); i++)
			m_aAssignedPlayers[i] = -1;
	}
	
	
	
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
		{
			Resource res = Resource.Load(m_sVehiclePreviewPrefab);
			for (int iRow = 0; iRow < m_iNumRows; iRow++)
			{
				for (int iCol = 0; iCol < m_iNumColumns; iCol++)
				{
					EntitySpawnParams p = new EntitySpawnParams();
					p.TransformMode = ETransformMode.LOCAL;
					p.Transform[0] = Vector(1.0, 0.0, 0.0); // Aside
					p.Transform[1] = Vector(0.0, 1.0, 0.0); // Up
					p.Transform[2] = Vector(0.0, 0.0, 1.0); // Dir
					p.Transform[3] = GetPositionOffset(iRow, iCol); // Pos
					p.Parent = this;
					p.Scale = 1.0;
					
					IEntity previewEnt = GetGame().SpawnEntityPrefabLocal(res, params : p);
					
					m_aPreviews.Insert(previewEnt);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected vector GetPositionOffset(int row, int col)
	{
		float halfWidth = 0.5 * ((m_iNumColumns - 1) * m_fStepColumns);
		float x = col * m_fStepColumns - halfWidth;
		return Vector(x, 0, -row * m_fStepRows);
	}

	//------------------------------------------------------------------------------------------------
	void UDR_VehiclePositioning(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
		
		int nPositionsMax = m_iNumRows * m_iNumColumns;
		m_aAssignedPlayers.Resize(nPositionsMax);
		ClearAssignments();
	}

	//------------------------------------------------------------------------------------------------
	void ~UDR_VehiclePositioning()
	{
		// Clear previews
		foreach (IEntity preview : m_aPreviews)
		{
			SCR_Global.DeleteEntityAndChildren(preview);
		}
	}

}
