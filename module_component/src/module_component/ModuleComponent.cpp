/*
 * Ubitrack - Library for Ubiquitous Tracking
 * Copyright 2006, Technische Universitaet Muenchen, and individual
 * contributors as indicated by the @authors tag. See the
 * copyright.txt in the distribution for a full listing of individual
 * contributors.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */

/**
 * @ingroup driver_components
 * @file
 * The player component for playback of recorded ue camera measurements
 * Based on standard ubitrack player component
 *
 * 
 */

// Ubitrack
#include <utUtil/OS.h>
#include <utUtil/CalibFile.h>
#include <utMeasurement/Measurement.h>
#include <utMeasurement/Timestamp.h>
#include <utDataflow/Module.h>
#include <utDataflow/PushSupplier.h>
#include <utDataflow/PullSupplier.h>

// std
#include <deque> // for image queue
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdint.h>
#include <mutex>
#include <condition_variable>


// Boost
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/shared_ptr.hpp>


static log4cpp::Category& logger( log4cpp::Category::getInstance( "Example.Module_Component" ) );

namespace Ubitrack { namespace Example {

// forward decls
class ModuleComponentComponentBase;


/**
 * Component key for ModuleComponentProducer/Consumer components.
 */

MAKE_DATAFLOWCONFIGURATIONATTRIBUTEKEY( ModuleComponentComponentKey, std::string, "file")

/**
 * Module used by ModuleComponent components, maintains a single main loop for all ModuleComponent components
 */
class ModuleComponentModule
	: public Dataflow::Module< Dataflow::SingleModuleKey, ModuleComponentComponentKey, ModuleComponentModule, ModuleComponentComponentBase >
{
public:
	/** simplifies our life afterwards */
	typedef Dataflow::Module< Dataflow::SingleModuleKey, ModuleComponentComponentKey, ModuleComponentModule, ModuleComponentComponentBase > BaseClass;

	ModuleComponentModule( const Dataflow::SingleModuleKey& key, boost::shared_ptr< Graph::UTQLSubgraph >, FactoryHelper* fh )
		: BaseClass( key, fh )
		, m_bStop( false )
	{
		LOG4CPP_INFO( logger, "created ModuleComponentModule" );
	}

	~ModuleComponentModule()
	{
		// stop main loop
		m_bStop = true;

		// wait for thread
		if ( m_pMainLoopThread )
			m_pMainLoopThread->join();

		LOG4CPP_INFO( logger, "destroyed ModuleComponentModule" );
	}

	void startThread()
	{
		LOG4CPP_DEBUG( logger, "starting thread" );

		// start mainloop
		if ( !m_pMainLoopThread )
			m_pMainLoopThread.reset( new boost::thread( boost::bind( &ModuleComponentModule::mainloop, this ) ) );
	}

protected:

	/** the main loop thread */
	boost::shared_ptr< boost::thread > m_pMainLoopThread;

	/** stop the main loop? */
	bool m_bStop;

	/** method that runs the main loop */
	void mainloop();

	/** create new components */
	boost::shared_ptr< ModuleComponentComponentBase > createComponent( const std::string& type, const std::string& name,
		boost::shared_ptr< Graph::UTQLSubgraph > pConfig, const ComponentKey& key, ModuleComponentModule* pModule );
};


/**
 * Base class for all ModuleComponent components.
 */
class ModuleComponentComponentBase
	: public ModuleComponentModule::Component
{
public:
	ModuleComponentComponentBase( const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph >, const ModuleComponentComponentKey& key,
		ModuleComponentModule* module )
		: ModuleComponentModule::Component( name, key, module )
	{}

	virtual ~ModuleComponentComponentBase()
	{}

	
	/** send the next event with the given offset */
	virtual void sendNext( Measurement::Timestamp ts )
	{ assert( false ); }
	

	virtual void start()
	{
		// for some reason, the default startModule mechanism does not work here...
		ModuleComponentModule::Component::start();
		getModule().startThread();
	}
};



class ModuleComponentComponentImage
	: public ModuleComponentComponentBase
{
public:
	/** loads the file */
	ModuleComponentComponentImage( const std::string& name, boost::shared_ptr< Graph::UTQLSubgraph > pConfig, const ModuleComponentComponentKey& key,
		ModuleComponentModule* module )
		: ModuleComponentComponentBase( name, pConfig, key, module )
		, m_tsFile( "" )
		, m_offset( 0 )
		, m_speedup( 1.0 )
		, m_outPortPosePull( "OutputPull", *this, boost::bind(&ModuleComponentComponentImage::getPose, this, _1) )
		, m_outPortPosePush( "OutputPush", *this )
	{
		

		pConfig->m_DataflowAttributes.getAttributeData("offset", m_offset);
		pConfig->m_DataflowAttributes.getAttributeData("speedup", m_speedup);
		
	}
	
    


	/** send the next event */
	void sendNext(Measurement::Timestamp ts )
	{
		m_outPortPosePush.send(Measurement::PoseMeasurement(ts, Math::Pose(Math::Quaternion(0,0,0,1), Math::Vector3d(0,0,0))));
	}

    Measurement::Pose getPose(Measurement::Timestamp t)
    {
	   	
		return Measurement::Pose(t, Math::Pose(Math::Quaternion(0,0,0,1), Math::Vector3d(0,0,0)));
	}

	

	
	
protected:
		
	
	/** offset if the event should be sent at some other time than its timestamp */
	int m_offset;

	/** speedup factor */
	double m_speedup;
	
	/** output port */
	Dataflow::PullSupplier< Measurement::Pose > m_outPortPosePull;
	Dataflow::PushSupplier< Measurement::Pose > m_outPortPosePush;

	
};



void ModuleComponentModule::mainloop()
{
	
	// main loop
	while ( !m_bStop)
	{
		Util::sleep(100);

		now = Measurement::now();
		
		// iterate all components
		ComponentList l( getAllComponents() );
		int count = 0;
		for ( ComponentList::iterator it = l.begin(); it != l.end(); it++ )
		{
			(*it)->sendNext(now);
		}
		
	}
}


// has to be here, after all class declarations
boost::shared_ptr< ModuleComponentComponentBase > ModuleComponentModule::createComponent( const std::string& type, const std::string& name,
	boost::shared_ptr< Graph::UTQLSubgraph > pConfig, const ModuleComponentModule::ComponentKey& key, ModuleComponentModule* pModule )
{
	if ( type == "ModuleComponentImage" )
		return boost::shared_ptr< ModuleComponentComponentBase >( new ModuleComponentComponentImage( name, pConfig, key, pModule ) );


	UBITRACK_THROW( "Class " + type + " not supported by ModuleComponent module" );
}


} } // namespace Ubitrack::Drivers


UBITRACK_REGISTER_COMPONENT( Ubitrack::Dataflow::ComponentFactory* const cf ) {
	// create list of supported types
	std::vector< std::string > ModuleComponentComponents;

	ModuleComponentComponents.push_back( "ModuleComponent" );

	cf->registerModule< Ubitrack::Drivers::ModuleComponentModule > ( ModuleComponentComponents );
}
