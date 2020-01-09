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
#include <utDataflow/TriggerComponent.h>
#include <utDataflow/TriggerInPort.h>
#include <utDataflow/TriggerOutPort.h>
#include <utDataflow/ComponentFactory.h>
#include <utMeasurement/Measurement.h>

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


static log4cpp::Category& logger( log4cpp::Category::getInstance( "Example.Trigger_Component" ) );

namespace Ubitrack { namespace Example {

/**
 * Module used by ModuleComponent components, maintains a single main loop for all ModuleComponent components
 */
class TriggerComponent
	: public Dataflow::TriggerComponent
{
public:


	BasicComponent( const std::string& sName, boost::shared_ptr< Graph::UTQLSubgraph > subgraph )
      : Dataflow::Component( sName )
      , m_inPortPose( "Input", *this )
	  , m_outPortPose( "Output", *this )
	{
		pConfig->m_DataflowAttributes.getAttributeData("offset", m_offset);
		pConfig->m_DataflowAttributes.getAttributeData("speedup", m_speedup);
	}

	/** Method that computes the result. */
	void compute( Measurement::Timestamp t )
	{
		m_outPort.send( Measurement::Pose( t, *m_inPortPose.get() ) );
	}

private:
	/** offset if the event should be sent at some other time than its timestamp */
	int m_offset;

	/** speedup factor */
	double m_speedup;
	
	/** output port */
	Dataflow::TriggerInPort< Measurement::Pose > m_inPortPose;
	Dataflow::TriggerOutPort< Measurement::Pose > m_outPortPose;

	

};


} } // namespace Ubitrack::Example



namespace Ubitrack { namespace Components {

UBITRACK_REGISTER_COMPONENT( Dataflow::ComponentFactory* const cf ) {

	cf->registerComponent< Example::TriggerComponent > ( "TriggerComponent" );	
	
	
}


} } // namespace Ubitrack::Components
