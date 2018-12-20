package util.minecraft;

import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Queue;

import jsdl.CuboidGeometryCreator;
import jsdl.MatteOpaqueMaterialCreator;
import jsdl.ModelActorCreator;
import jsdl.SDLCommand;
import jsdl.SDLGeometry;
import jsdl.SDLMaterial;
import jsdl.SDLVector3;
import util.Vector3f;
import util.Vector3i;

public class Terrain
{
	private Map<RegionCoord, RegionData> m_regions;
	
	public Terrain()
	{
		m_regions = new HashMap<>();
	}
	
	public RegionData getRegion(int regionX, int regionZ)
	{
		return m_regions.get(new RegionCoord(regionX, regionZ));
	}
	
	public void addRegion(RegionData region)
	{
		m_regions.put(region.getRegionCoord(), region);
	}
	
	public List<SectionUnit> getAllSections()
	{
		List<SectionUnit> sections = new ArrayList<>();
		for(RegionData region : m_regions.values())
		{
			for(int chunkZ = 0; chunkZ < RegionData.NUM_CHUNKS_Z; ++chunkZ)
			{
				for(int chunkX = 0; chunkX < RegionData.NUM_CHUNKS_X; ++chunkX)
				{
					ChunkData chunk = region.getChunk(chunkX, chunkZ);
					if(chunk == null)
					{
						continue;
					}
					
					for(int s = 0; s < ChunkData.NUM_SECTIONS; ++s)
					{
						SectionData section = chunk.getSection(s);
						if(section == null)
						{
							continue;
						}
						
						final int x = region.getX() + chunkX * ChunkData.SIZE_X;
						final int y = region.getY() + s * ChunkData.SIZE_Y;
						final int z = region.getZ() + chunkZ * ChunkData.SIZE_Z;
						sections.add(new SectionUnit(new Vector3i(x, y, z), section));
					}
				}
			}
		}
		return sections;
	}
	
	public List<SectionUnit> getReachableSections(Vector3f viewpoint)
	{
		// HACK
		int MAX_RADIUS = 128;
		
		Vector3f pv = toSectionCoord(viewpoint).toVector3f();
		System.err.println("pv: " + pv);
		
		class Flood implements Comparable<Flood>
		{
			Vector3i coord;
			EFacing  front;
			
			Flood(Vector3i coord, EFacing front)
			{
				this.coord = coord;
				this.front = front;
			}
			
			@Override
			public int compareTo(Flood other)
			{
				Vector3f p1 = coord.toVector3f();
				Vector3f p2 = other.coord.toVector3f();
				float value1 = pv.sub(p1).squareLength();
				float value2 = pv.sub(p2).squareLength();
				return value1 < value2 ? -1 : (value1 > value2 ? 1 : 0);
			}
		}
		
		Vector3i[] coordOffsets = new Vector3i[]{
			new Vector3i( 0,  0, -1),
			new Vector3i( 0,  0,  1),
			new Vector3i(-1,  0,  0),
			new Vector3i( 1,  0,  0),
			new Vector3i( 0, -1,  0),
			new Vector3i( 0,  1,  0)
		};
		
		SectionStateMap floodedArea = new SectionStateMap();
		Map<Vector3i, FaceReachability> sectionReachability = new HashMap<>();
		List<SectionUnit> reachableSections = new ArrayList<>();
		
		Map<Vector3i, SectionUnit> sectionMap = new HashMap<>();
		for(SectionUnit section : getAllSections())
		{
			int x = Math.floorDiv(section.getCoord().x, SectionData.SIZE_X);
			int y = Math.floorDiv(section.getCoord().y, SectionData.SIZE_Y);
			int z = Math.floorDiv(section.getCoord().z, SectionData.SIZE_Z);
			sectionMap.put(new Vector3i(x, y, z), section);
		}
		
		PriorityQueue<Flood> floodQueue = new PriorityQueue<>();
		
		{
			Vector3i rootCoord = toSectionCoord(viewpoint);
			for(EFacing front : EFacing.values())
			{
				floodQueue.add(new Flood(rootCoord, front));
			}
			floodedArea.setSection(rootCoord, true);
		}
		
		while(!floodQueue.isEmpty())
		{
			Flood flood = floodQueue.poll();
			Vector3i coord = flood.coord.add(coordOffsets[flood.front.getValue()]);
			if(coord.y < 0 || coord.y >= ChunkData.NUM_SECTIONS || 
			   floodedArea.getSection(coord))
			{
				continue;
			}
			
			System.err.println(coord);
			
			if(coord.toVector3f().sub(pv).length() > MAX_RADIUS)
			{
				break;
			}
			
			SectionUnit section = sectionMap.get(coord);
			
			if(section == null)
			{
				for(EFacing nextFront : EFacing.values())
				{
					floodQueue.add(new Flood(coord, nextFront));
				}
				floodedArea.setSection(coord, true);
			}
			else
			{
				FaceReachability reachability = sectionReachability.get(coord);
				if(reachability == null)
				{
					reachability = section.getData().determinReachability();
					sectionReachability.put(coord, reachability);
					reachableSections.add(section);
				}
				
				for(EFacing nextFront : EFacing.values())
				{
					if(flood.front != nextFront &&
					   reachability.isReachable(flood.front, nextFront))
					{
						floodQueue.add(new Flood(coord, nextFront));
						reachability.setReachability(flood.front, nextFront, false);
					}
				}
				
				System.err.println(reachability);
				if(reachability.isFullyUnreachable())
				{
					System.out.println("luuuauaa");
					floodedArea.setSection(coord, true);
				}
			}
		}// end while
		
		return reachableSections;
	}
	
	private static Vector3i toSectionCoord(Vector3f viewpoint)
	{
		float clampedY = Math.max(0.0f, Math.min(viewpoint.y, ChunkData.SIZE_Y));
		
		return new Vector3i(
			Math.floorDiv((int)Math.floor(viewpoint.x), SectionData.SIZE_X),
			Math.min((int)clampedY / SectionData.SIZE_Y, ChunkData.NUM_SECTIONS - 1),
			Math.floorDiv((int)Math.floor(viewpoint.z), SectionData.SIZE_Z));
	}
	
	// FIXME: this is wrong, use floor
//	private static RegionCoord toRegionCoord(Vector3f viewpoint)
//	{
//		int regionX = (int)viewpoint.x / RegionData.SIZE_X;
//		int regionZ = (int)viewpoint.z / RegionData.SIZE_Z;
//		
//		return new RegionCoord(regionX, regionZ);
//	}
}
