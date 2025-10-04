/*********************************************************************
	wwBluetooth.cpp													  
	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/weaselLibrary
\*********************************************************************/

#include "pch.h"
#include "wwBluetooth.h"

#pragma region bluetoothConnection

bool wildWeasel::bluetoothConnection::bytesAvailableForRead()
{
	return (readBuffer.size() > 0);
}

#pragma endregion

/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma region bluetoothViaQt
#ifdef QT_VERSION

wildWeasel::bluetoothViaQt::bluetoothViaQt(QObject *parent) 
	:	bluetoothConnection(nullptr), parent(parent)
{
}

wildWeasel::bluetoothViaQt::~bluetoothViaQt()
{
	// client
    clientSocket = nullptr;
    if (discoveryAgent){
        discoveryAgent->stop();
    }
    delete discoveryAgent;
	delete clientSocket;	

	// server
	stopServer();
}

bool wildWeasel::bluetoothViaQt::setLocalAdapter(wstring& adapterName)
{
	// get adapter index
	if (adapterName == L"default bluetooth adapter") {
		adapterId = 0;
	} else {
		for (adapterId = 0; adapterId < localAdapters.count(); adapterId++) {
			wstring curAdapterName = localAdapters.at(adapterId).address().toString().toStdWString().c_str();
			if (adapterName == curAdapterName) {
				break;
			}
		}
		if (adapterId == localAdapters.count()) {
			adapterId = -1;
			return false;
		}
	}

	return true;
}

bool wildWeasel::bluetoothViaQt::startServer(const wstring& serviceName, const wstring& serviceDescription, const wstring& provider)
{
	// server already running?
	if ( rfcommServer) return false;
	if (adapterId < 0) return false;

	// set adapter
	const QBluetoothAddress& adapterAddress = localAdapters.isEmpty() ? QBluetoothAddress() : localAdapters.at(adapterId).address();
	QBluetoothLocalDevice adapter(adapterAddress);
	adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

	// create server
	rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, parent);
	QObject::connect(rfcommServer, &QBluetoothServer::newConnection, bind(&wildWeasel::bluetoothViaQt::clientConnected, this, rfcommServer));
	bool result = rfcommServer->listen(adapterAddress);
	if (!result) {
		qWarning() << "Cannot bind chat server to " << adapterAddress.toString();
		return false;
	}

	// make class id
	QBluetoothServiceInfo::Sequence profileSequence;
	QBluetoothServiceInfo::Sequence classId;
	classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
	classId << QVariant::fromValue(quint16(0x100));
	profileSequence.append(QVariant::fromValue(classId));
	serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, profileSequence);

	classId.clear();
	classId << QVariant::fromValue(QBluetoothUuid(serviceUuid));
	classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));

	serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

	// Service name, description and provider
	serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName,		serviceName.c_str());
	serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription, serviceDescription.c_str());
	serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider,	provider.c_str());

	// set Service UUID
	serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));

	// Service Discoverability
	QBluetoothServiceInfo::Sequence publicBrowse;
	publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
	serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

	// Protocol descriptor list
	QBluetoothServiceInfo::Sequence protocolDescriptorList;
	QBluetoothServiceInfo::Sequence protocol;
	protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
	protocolDescriptorList.append(QVariant::fromValue(protocol));
	protocol.clear();
	protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
			 << QVariant::fromValue(quint8(rfcommServer->serverPort()));
	protocolDescriptorList.append(QVariant::fromValue(protocol));
	serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);

	// Register service
	serviceInfo.registerService(adapterAddress);

	return true;
}

bool wildWeasel::bluetoothViaQt::stopServer()
{
    // Unregister service
    serviceInfo.unregisterService();

    // Close sockets
    qDeleteAll(clientSockets);

    // Close server
    delete rfcommServer;
    rfcommServer = nullptr;

	return true;
}

unsigned int wildWeasel::bluetoothViaQt::getNumberOfConnectedClients()
{
	return clientSockets.size();
}

bool wildWeasel::bluetoothViaQt::startSearchForRemoteDevices(function<void(list<deviceAndServiceName>&)> callbackOndiscoveryFinished)
{
	if (adapterId < 0) return false;
	
	const QBluetoothAddress localAdapter = localAdapters.isEmpty() ? QBluetoothAddress() : localAdapters.at(adapterId).address();

	this->callbackOndiscoveryFinished	= callbackOndiscoveryFinished;
	serviceDiscovyFinished				= false;
	discoveryAgent						= new QBluetoothServiceDiscoveryAgent(localAdapter);

	QObject::connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,	bind(&wildWeasel::bluetoothViaQt::serviceDiscovered, this, std::placeholders::_1));
	QObject::connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,			bind(&wildWeasel::bluetoothViaQt::discoveryFinished, this, discoveryAgent));
    QObject::connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled,			bind(&wildWeasel::bluetoothViaQt::discoveryFinished, this, discoveryAgent));

	if (discoveryAgent->isActive()) discoveryAgent->stop();
    discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

	return true;
}

bool wildWeasel::bluetoothViaQt::connectToRemoteDevice(deviceAndServiceName& remoteDnS)
{
	QBluetoothServiceInfo remoteService = m_discoveredServices.value(remoteDnS.deviceName + L" " + remoteDnS.serviceName);
	if (clientSocket) return false;

    // Connect to service
    clientSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    clientSocket->connectToService(remoteService);

    QObject::connect(clientSocket, &QBluetoothSocket::readyRead,											bind(&wildWeasel::bluetoothViaQt::readSocket				, this, clientSocket) );
    QObject::connect(clientSocket, &QBluetoothSocket::connected,											bind(&wildWeasel::bluetoothViaQt::connectedToServer			, this, clientSocket) );
    QObject::connect(clientSocket, &QBluetoothSocket::disconnected,											bind(&wildWeasel::bluetoothViaQt::disconnectedFromServer	, this, clientSocket) );
    QObject::connect(clientSocket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),	bind(&wildWeasel::bluetoothViaQt::onSocketErrorOccurred		, this, std::placeholders::_1) );
		
	return true;
}

bool wildWeasel::bluetoothViaQt::getLocalAdapterList(list<wstring>& adapterNames)
{
	localAdapters = QBluetoothLocalDevice::allDevices();

	if (localAdapters.count() > 0) {
		for (int adapterId = 0; adapterId < localAdapters.count(); adapterId++) {
			adapterNames.push_back(localAdapters.at(adapterId).address().toString().toStdWString().c_str());
		}
	} else {
		adapterNames.push_back(L"default bluetooth adapter");
	}

	return true;
}

void wildWeasel::bluetoothViaQt::clientConnected(QBluetoothServer* rfcommServer)
{
    QBluetoothSocket *socket = rfcommServer->nextPendingConnection();
    if (!socket) return;
    QObject::connect(socket, &QBluetoothSocket::readyRead,		bind(&wildWeasel::bluetoothViaQt::readSocket				, this, socket));
    QObject::connect(socket, &QBluetoothSocket::disconnected,	bind(&wildWeasel::bluetoothViaQt::clientDisconnected		, this, socket));
    clientSockets.append(socket);
}

void wildWeasel::bluetoothViaQt::clientDisconnected(QBluetoothSocket *pClientSocket)
{
	// Backup solution if bind() does not pass a valid pointer
	//QBluetoothSocket *socket = nullptr;
	//for (auto& curSocket : clientSockets) {
	//	if (curSocket->state() == QAbstractSocket::ClosingState || curSocket->state() == QAbstractSocket::UnconnectedState) {
	//		socket = curSocket;
	//		break;
	//	}
	//}
	// QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(sender());
	QBluetoothSocket *socket = qobject_cast<QBluetoothSocket *>(pClientSocket);

    if (!socket) return;
    clientSockets.removeOne(socket);
    socket->deleteLater();
}

void wildWeasel::bluetoothViaQt::readSocket(QBluetoothSocket* pClientSocket)
{
	QBluetoothSocket *socket = nullptr;
	if (clientSocket) {
		socket = clientSocket;
	} else {
		// Backup solution if bind() does not pass a valid pointer
		/*for (auto& curSocket : clientSockets) {
			if (curSocket->bytesAvailable()) {
				socket = curSocket;
				break;
			}
		}*/
		// socket = qobject_cast<QBluetoothSocket *>(sender());
		socket = qobject_cast<QBluetoothSocket *>(pClientSocket);
	}
    if (!socket) return;

	while (socket->bytesAvailable()) {
        QByteArray line = socket->readAll();

		// write into buffer
		readBuffer.assign((unsigned char*) line.constData(), ((unsigned char*)line.constData() + line.size()));
		
		// use callback function
		if (messageReceivedCallback != nullptr) {
			wstring peerName = socket->peerName().toStdWString().c_str();
			messageReceivedCallback(peerName, readBuffer);
			readBuffer.clear();
		}
    }
}

void wildWeasel::bluetoothViaQt::serviceDiscovered(const QBluetoothServiceInfo& serviceInfo)
{
    QString remoteName;
    if (serviceInfo.device().name().isEmpty())
        remoteName = serviceInfo.device().address().toString();
    else
        remoteName = serviceInfo.device().name();

	wstring deviceName  = remoteName				.toStdWString().c_str();
	wstring serviceName	= serviceInfo.serviceName() .toStdWString().c_str();

    m_discoveredServices.insert(deviceName + L" " + serviceName, serviceInfo);

	deviceAndServiceName newDnS;
	newDnS.deviceName	= deviceName;
	newDnS.serviceName	= serviceName;
	discoveredServices.push_back(newDnS);
}

void wildWeasel::bluetoothViaQt::discoveryFinished(QBluetoothServiceDiscoveryAgent* discoveryAgent)
{
	serviceDiscovyFinished = true;
	callbackOndiscoveryFinished(discoveredServices);
}

void wildWeasel::bluetoothViaQt::connectedToServer(QBluetoothSocket* pClientSocket)
{
	connectionToServerEstablished = true;
}

void wildWeasel::bluetoothViaQt::disconnectedFromServer(QBluetoothSocket* pClientSocket)
{
	connectionToServerEstablished = false;
}

void wildWeasel::bluetoothViaQt::onSocketErrorOccurred(QBluetoothSocket::SocketError error)
{
	if (error == QBluetoothSocket::NoSocketError) return;
    QMetaEnum metaEnum = QMetaEnum::fromType<QBluetoothSocket::SocketError>();
    QString errorString = clientSocket->peerName() + QLatin1Char(' ') + metaEnum.valueToKey(error) + QLatin1String(" occurred");
}

bool wildWeasel::bluetoothViaQt::read(vector<unsigned char>& bytes)
{
	bytes = readBuffer;
	readBuffer.clear();
	return true;
}

bool wildWeasel::bluetoothViaQt::write(vector<unsigned char>& bytes)
{
	QByteArray text{(const char *) bytes.data(), (int) bytes.size()};

	if (clientSocket) {
		clientSocket->write(text);
	} else {
		for (QBluetoothSocket *socket : qAsConst(clientSockets)) {
			socket->write(text);
		}
	}
	
	return true;
}
#endif
#pragma endregion