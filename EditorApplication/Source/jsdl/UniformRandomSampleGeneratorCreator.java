// ========================================
// NOTE: THIS FILE CONTAINS GENERATED CODE 
//       DO NOT MODIFY                     
// ========================================
// last generated: 2018-12-27 18:43:35.071479 

package jsdl;

public class UniformRandomSampleGeneratorCreator extends SDLCoreCommand
{
	@Override
	public String getFullType()
	{
		return "sample-generator(uniform-random)";
	}

	public void setSampleAmount(SDLInteger data)
	{
		setInput("sample-amount", data);
	}

}

