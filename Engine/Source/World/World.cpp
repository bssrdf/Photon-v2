#include "World/World.h"
#include "Core/Intersection.h"
#include "Common/primitive_type.h"
#include "Core/Ray.h"
#include "Actor/Actor.h"
#include "Core/CookedActor.h"
#include "World/Intersector/BruteForceIntersector.h"
#include "World/Intersector/Kdtree/KdtreeIntersector.h"
#include "World/LightSampler/UniformRandomLightSampler.h"
#include "World/Intersector/Bvh/ClassicBvhIntersector.h"

#include <limits>
#include <iostream>

namespace ph
{

World::World() : 
	//m_intersector(std::make_unique<BruteForceIntersector>()), 
	//m_intersector(std::make_unique<KdtreeIntersector>()), 
	m_intersector(std::make_unique<ClassicBvhIntersector>()), 
	m_lightSampler(std::make_unique<UniformRandomLightSampler>()), 
	m_scene()
{

}

void World::addActor(std::unique_ptr<Actor> actor)
{
	m_actors.push_back(std::move(actor));
}

void World::update(const real deltaS)
{
	std::cout << "updating world..." << std::endl;

	m_cookedActorStorage.clear();

	cookActors();

	std::cout << "world discretized into " << m_cookedActorStorage.numPrimitives() << " primitives" << std::endl;
	std::cout << "processing..." << std::endl;

	m_intersector->update(m_cookedActorStorage);
	m_lightSampler->update(m_cookedActorStorage);

	m_scene = Scene(m_intersector.get(), m_lightSampler.get());
}

void World::cookActors()
{
	for(const auto& actor : m_actors)
	{
		CookedActor cookedActor;
		actor->cook(&cookedActor);
		m_cookedActorStorage.add(std::move(cookedActor));
	}
}

const Scene& World::getScene() const
{
	return m_scene;
}

}// end namespace ph