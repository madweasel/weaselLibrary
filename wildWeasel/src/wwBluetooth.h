/*********************************************************************
	wwBluetooth.cpp													  
	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#pragma once

#include <list>
#include <vector>
#include <string>
#include <functional>
#ifdef QT_VERSION
#include <QtWidgets/qapplication.h>
#include <QtBluetooth/qbluetoothhostinfo.h>
#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothsocket.h>
#include <QtBluetooth/qbluetoothserver.h>
#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothservicediscoveryagent.h>
#include <QtCore/qmetaobject.h>
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

static const QLatin1String		serviceUuid							("00001101-0000-1000-8000-00805f9b34fb");
#endif 

namespace wildWeasel
{
	using namespace std;

	class bluetoothConnection
	{
	public:
		class deviceAndServiceName
		{
		public:
			wstring						deviceName;
			wstring						serviceName;
		};

										bluetoothConnection					(void* parent)										{};
										~bluetoothConnection				()													{};

		// general
		virtual bool					setLocalAdapter						(wstring& adapterName)								{return false;};
		virtual bool					getLocalAdapterList					(list<wstring>& adapterNames)						{return false;};

		// server
		virtual bool					startServer							(const wstring& serviceName, const wstring& serviceDescription, const wstring& provider)								{return false;};
		virtual bool					stopServer							()													{return false;};
		virtual unsigned int			getNumberOfConnectedClients			()													{return 0; };
		//function<void(wstring& clientName)>									aClientHasConnected;
		//function<void(wstring& clientName)>									aClientHasDisconected;

		// client
		virtual bool					startSearchForRemoteDevices			(function<void(list<deviceAndServiceName>&)> callbackOndiscoveryFinished)	{return false;};		
		virtual bool					connectToRemoteDevice				(deviceAndServiceName& remoteDnS)					{return false;};

		// communication
		function<void(wstring& peerName, vector<unsigned char>& readBuffer)>	messageReceivedCallback;
		bool							bytesAvailableForRead				();
		virtual bool					read								(vector<unsigned char>& bytes)						{return false;};
		virtual bool					write								(vector<unsigned char>& bytes)						{return false;};

	protected:
		vector<unsigned char>			readBuffer;
		int								adapterId							= -1;
		list<deviceAndServiceName>		discoveredServices;
		function<void(list<deviceAndServiceName>&)>							callbackOndiscoveryFinished;
	};

#ifdef QT_VERSION
	class bluetoothViaQt : public bluetoothConnection
	{
	friend class QObject;

	private:
		// general
		QList<QBluetoothHostInfo>				localAdapters;
		QObject *								parent;

		// server
		QList<QBluetoothSocket *>				clientSockets;
		QBluetoothServiceInfo					serviceInfo;
		QBluetoothServer *						rfcommServer						= nullptr;

		// client
		bool									connectionToServerEstablished		= false;
		bool									serviceDiscovyFinished				= false;
		QBluetoothSocket *						clientSocket						= nullptr;
		QBluetoothServiceDiscoveryAgent *		discoveryAgent						= nullptr;
		QMap<wstring, QBluetoothServiceInfo>	m_discoveredServices;				// key: deviceName + L" " + serviceName; value: qtService

		// communication
		void							readSocket									(QBluetoothSocket* pClientSocket);
		
		// server
		void							clientConnected								(QBluetoothServer* rfcommServer);
		void							clientDisconnected							(QBluetoothSocket *pClientSocket);

		// client
		void							serviceDiscovered							(const QBluetoothServiceInfo &serviceInfo);
		void							discoveryFinished							(QBluetoothServiceDiscoveryAgent* discoveryAgent);
		void							connectedToServer							(QBluetoothSocket* pClientSocket);
		void							disconnectedFromServer						(QBluetoothSocket* pClientSocket);
		void							onSocketErrorOccurred						(QBluetoothSocket::SocketError error);

	public:
										bluetoothViaQt								(QObject *parent);
										~bluetoothViaQt								();

		bool							getLocalAdapterList							(list<wstring>& adapterNames)						override;
		bool							setLocalAdapter								(wstring& adapterName)								override;
		bool							startServer									(const wstring& serviceName, const wstring& serviceDescription, const wstring& provider)	override;
		bool							stopServer									()													override;
		unsigned int					getNumberOfConnectedClients					()													override;
		bool							startSearchForRemoteDevices					(function<void(list<deviceAndServiceName>&)> callbackOndiscoveryFinished)	override;
		bool							connectToRemoteDevice						(deviceAndServiceName& remoteDnS)					override;

		bool							read										(vector<unsigned char>& bytes)						override;
		bool							write										(vector<unsigned char>& bytes)						override;
	};
#endif

} // namespace wildWeasel
