// ========================================
// NOTE: THIS FILE CONTAINS GENERATED CODE 
//       DO NOT MODIFY                     
// ========================================
// last generated: 2018-12-27 18:43:35.065479 

package jsdl;

public class PointLightSourceCreator extends SDLCreatorCommand
{
	@Override
	public String getFullType()
	{
		return "light-source(point)";
	}

	public void setLinearSrgb(SDLVector3 data)
	{
		setInput("linear-srgb", data);
	}

	public void setWatts(SDLReal data)
	{
		setInput("watts", data);
	}

}

