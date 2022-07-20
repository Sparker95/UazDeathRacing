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

	//------------------------------------------------------------------------------------------------
	// Returs position at given ID in world coordinates
	void GetPosition(int id, out vector outTransform[])
	{
		if (id >= m_iNumColumns*m_iNumRows)
			Print(string.Format("UDR_VehiclePositioning: GetPosition: id %1 is more than predefined size", id), LogLevel.ERROR);
		
		int col = id % m_iNumColumns;
		int row = id / m_iNumRows;
		
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
