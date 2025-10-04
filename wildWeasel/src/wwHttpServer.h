/*********************************************************************\
	wwHttpServer.h
	Copyright (c) Thomas Weber. All rights reserved.
	Licensed under the MIT License.
	https://github.com/madweasel/zenTable
\*********************************************************************/
#pragma once

// https://docs.microsoft.com/en-us/windows/win32/http/http-server-sample-application

// header files
#include <windows.h>
#include <http.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <thread>
#include <sstream>
#include <functional>
#include <mutex>

// libraries
#pragma comment(lib, "httpapi.lib")

namespace wildWeasel
{
   	using namespace std;

	class httpServer
	{
	public:
										httpServer	            			()													    {};
										~httpServer	            			()													    {};

		virtual bool					startServer						    (const wstring& ipAddress, unsigned short port, const wstring& uri) {return false;};
		virtual bool					stopServer							()													    {return false;};

		function<void(wstring& peerName, vector<unsigned char>& readBuffer)>	messageReceivedCallback;

		virtual bool					bytesAvailableForRead				()													    {return false;};
		virtual bool					read								(vector<unsigned char>& bytes)						    {return false;};
		virtual bool					write								(vector<unsigned char>& bytes)						    {return false;};

	protected:
		vector<unsigned char>			readBuffer;
		wstring							ipAddress;
		unsigned short					port								= 0;
		wstring							uri;
	};

    class httpServerViaWinAPI : public httpServer
    {
    public:
    						            httpServerViaWinAPI 	            ();
    						            ~httpServerViaWinAPI 	            ();

		bool					        startServer						    (const wstring& ipAddress, unsigned short port, const wstring& uri) override;
		bool							stopServer							();

		bool					        bytesAvailableForRead				()                                                      override;
		bool					        read								(vector<unsigned char>& bytes)                          override;
		bool					        write								(vector<unsigned char>& bytes)                          override;

    private:
		static const unsigned int		MAX_ULONG_STR						= sizeof("4294967295");
		vector<wstring>					URIsToListenTo;
		vector<wstring>					URIsListening;
		HANDLE							hReqQueue							= NULL;
		thread							hThreadServer;
		std::condition_variable				threadConditionVariable;
		std::mutex							threadMutex;

        DWORD                           DoReceiveRequests                   (HANDLE hReqQueue);
        DWORD                           SendHttpResponse                    (IN HANDLE hReqQueue, IN PHTTP_REQUEST pRequest, IN USHORT StatusCode, IN char const* pReason, IN char const* pEntityString);
        DWORD                           SendHttpPostResponse                (IN HANDLE hReqQueue, IN PHTTP_REQUEST pRequest);

		static void						initializeHttpResponse              (HTTP_RESPONSE* resp, USHORT status, PCSTR reason);
		static void						addKnownHeader						(HTTP_RESPONSE  Response, HTTP_HEADER_ID HeaderId, PCSTR RawValue);

		int								threadProc							();
	};
} // namespace wildWeasel
