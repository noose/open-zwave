//-----------------------------------------------------------------------------
//
//	EnergyProduction.cpp
//
//	Implementation of the Z-Wave COMMAND_CLASS_ENERGY_PRODUCTION
//
//	Copyright (c) 2010 Mal Lansell <openzwave@lansell.org>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#include "CommandClasses.h"
#include "EnergyProduction.h"
#include "Defs.h"
#include "Msg.h"
#include "Node.h"
#include "Driver.h"
#include "Log.h"

#include "ValueDecimal.h"

using namespace OpenZWave;

enum EnergyProductionCmd
{
	EnergyProductionCmd_Get		= 0x02,
	EnergyProductionCmd_Report	= 0x03
};

enum 
{
	EnergyProductionIndex_Instant = 0,
	EnergyProductionIndex_Total,
	EnergyProductionIndex_Today,
	EnergyProductionIndex_Time
};

static char const* c_energyParameterNames[] = 
{
	"Instant energy production",
	"Total energy production",
	"Energy production today",
	"Total production time"
};

//-----------------------------------------------------------------------------
// <EnergyProduction::RequestState>												   
// Request current state from the device									   
//-----------------------------------------------------------------------------
bool EnergyProduction::RequestState
(
	uint32 const _requestFlags
)
{
	if( _requestFlags & RequestFlag_Dynamic )
	{
		// Request each of the production values
		RequestValue( _requestFlags, EnergyProductionIndex_Instant );
		RequestValue( _requestFlags, EnergyProductionIndex_Total );
		RequestValue( _requestFlags, EnergyProductionIndex_Today );
		RequestValue( _requestFlags, EnergyProductionIndex_Time );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// <EnergyProduction::RequestValue>												   
// Request current production from the device									   
//-----------------------------------------------------------------------------
void EnergyProduction::RequestValue
(
	uint32 const _requestFlags,
	uint8 const _valueEnum,		// one of EnergyProductionIndex enums
	uint8 const _dummy
)
{
	Log::Write( "Requesting the %s value from node %d", c_energyParameterNames[_valueEnum], GetNodeId() );
	Msg* msg = new Msg( "EnergyProductionCmd_Get", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
	msg->Append( GetNodeId() );
	msg->Append( 3 );
	msg->Append( GetCommandClassId() );
	msg->Append( EnergyProductionCmd_Get );
	msg->Append( _valueEnum );
	msg->Append( TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE );

	if( _requestFlags & RequestFlag_LowPriority )
	{
		msg->SetPriority( Msg::MsgPriority_Low );
	}
	GetDriver()->SendMsg( msg );
}

//-----------------------------------------------------------------------------
// <EnergyProduction::HandleMsg>
// Handle a message from the Z-Wave network
//-----------------------------------------------------------------------------
bool EnergyProduction::HandleMsg
(
	uint8 const* _data,
	uint32 const _length,
	uint32 const _instance	// = 1
)
{
	if (EnergyProductionCmd_Report == (EnergyProductionCmd)_data[0])
	{
		uint8 scale;
		string value = ExtractValue( &_data[2], &scale );

		Log::Write( "Received an Energy production report from node %d: %s = %s", GetNodeId(), c_energyParameterNames[_data[1]], value.c_str() );
		if( ValueDecimal* decimalValue = static_cast<ValueDecimal*>( GetValue( _instance, _data[1] ) ) )
		{
			decimalValue->OnValueChanged( value );
		}
		Node* node = GetNodeUnsafe();
		if( node != NULL && node->m_queryPending )
		{
			node->m_queryStageCompleted = true;
		}
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// <EnergyProduction::CreateVars>
// Create the values managed by this command class
//-----------------------------------------------------------------------------
void EnergyProduction::CreateVars
(
	uint8 const _instance
)
{
	if( Node* node = GetNodeUnsafe() )
	{
		node->CreateValueDecimal( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (uint8)EnergyProductionIndex_Instant, c_energyParameterNames[EnergyProductionIndex_Instant], "W", true, "0.0" );
		node->CreateValueDecimal( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (uint8)EnergyProductionIndex_Total, c_energyParameterNames[EnergyProductionIndex_Total], "kWh", true, "0.0" );
		node->CreateValueDecimal( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (uint8)EnergyProductionIndex_Today, c_energyParameterNames[EnergyProductionIndex_Today], "kWh", true, "0.0" );
		node->CreateValueDecimal( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (uint8)EnergyProductionIndex_Time, c_energyParameterNames[EnergyProductionIndex_Time], "", true, "0.0" );
	}
}



