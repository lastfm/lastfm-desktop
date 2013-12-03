/*
   Copyright 2012 Last.fm Ltd.
      - Primarily authored by Michael Coffey

   This file is part of the Last.fm Desktop Application Suite.

   lastfm-desktop is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   lastfm-desktop is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with lastfm-desktop.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#include "NamedPipeServer.h"

#include "common/c++/win/scrobSubPipeName.cpp"

#define CONNECTING_STATE 0
#define READING_STATE 1
#define WRITING_STATE 2
#define INSTANCES 8
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

typedef struct
{
   OVERLAPPED oOverlap;
   HANDLE hPipeInst;
   TCHAR chRequest[BUFSIZE];
   DWORD cbRead;
   TCHAR chReply[BUFSIZE];
   DWORD cbToWrite;
   DWORD dwState;
   BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES];

VOID DisconnectAndReconnect(DWORD i);
BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo);

NamedPipeServer::NamedPipeServer(QObject *parent)
    :QThread(parent)
{
}

void
NamedPipeServer::run()
{
   DWORD i, dwWait, cbRet, dwErr;
   BOOL fSuccess;
   QByteArray response;

   std::string s;
   DWORD r = scrobSubPipeName( &s );
   if (r != 0) throw std::runtime_error( formatWin32Error( r ) );
   QString const name = QString::fromStdString( s );

// The initial loop creates several instances of a named pipe
// along with an event object for each instance.  An
// overlapped ConnectNamedPipe operation is started for
// each instance.

   for (i = 0; i < INSTANCES; i++)
   {

   // Create an event object for this instance.

      hEvents[i] = CreateEvent(
         NULL,    // default security attribute
         TRUE,    // manual-reset event
         TRUE,    // initial state = signaled
         NULL);   // unnamed event object

      if (hEvents[i] == NULL)
      {
         printf("CreateEvent failed with %d.\n", GetLastError());
         return;
      }

      Pipe[i].oOverlap.hEvent = hEvents[i];

      Pipe[i].hPipeInst = CreateNamedPipe(
         (const wchar_t *)name.utf16(),            // pipe name
         PIPE_ACCESS_DUPLEX |     // read/write access
         FILE_FLAG_OVERLAPPED,    // overlapped mode
         PIPE_TYPE_MESSAGE |      // message-type pipe
         PIPE_READMODE_MESSAGE |  // message-read mode
         PIPE_WAIT,               // blocking mode
         INSTANCES,               // number of instances
         BUFSIZE*sizeof(TCHAR),   // output buffer size
         BUFSIZE*sizeof(TCHAR),   // input buffer size
         PIPE_TIMEOUT,            // client time-out
         NULL);                   // default security attributes

      if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
      {
         printf("CreateNamedPipe failed with %d.\n", GetLastError());
         return;
      }

   // Call the subroutine to connect to the new client

      Pipe[i].fPendingIO = ConnectToNewClient(
         Pipe[i].hPipeInst,
         &Pipe[i].oOverlap);

      Pipe[i].dwState = Pipe[i].fPendingIO ?
         CONNECTING_STATE : // still connecting
         READING_STATE;     // ready to read
   }

   while (1)
   {
   // Wait for the event object to be signaled, indicating
   // completion of an overlapped read, write, or
   // connect operation.

      dwWait = WaitForMultipleObjects(
         INSTANCES,    // number of event objects
         hEvents,      // array of event objects
         FALSE,        // does not wait for all
         INFINITE);    // waits indefinitely

   // dwWait shows which pipe completed the operation.

      i = dwWait - WAIT_OBJECT_0;  // determines which pipe
      if (i < 0 || i > (INSTANCES - 1))
      {
         printf("Index out of range.\n");
         return;
      }

   // Get the result if the operation was pending.

      if (Pipe[i].fPendingIO)
      {
         fSuccess = GetOverlappedResult(
            Pipe[i].hPipeInst, // handle to pipe
            &Pipe[i].oOverlap, // OVERLAPPED structure
            &cbRet,            // bytes transferred
            FALSE);            // do not wait

         switch (Pipe[i].dwState)
         {
         // Pending connect operation
            case CONNECTING_STATE:
               if (! fSuccess)
               {
                   printf("Error %d.\n", GetLastError());
                   return;
               }
               Pipe[i].dwState = READING_STATE;
               break;

         // Pending read operation
            case READING_STATE:
               if (! fSuccess || cbRet == 0)
               {
                  DisconnectAndReconnect(i);
                  continue;
               }
               Pipe[i].cbRead = cbRet;
               Pipe[i].dwState = WRITING_STATE;
               break;

         // Pending write operation
            case WRITING_STATE:
               if (! fSuccess || cbRet != Pipe[i].cbToWrite)
               {
                  DisconnectAndReconnect(i);
                  continue;
               }
               Pipe[i].dwState = READING_STATE;
               break;

            default:
            {
               printf("Invalid pipe state.\n");
               return;
            }
         }
      }

   // The pipe state determines which operation to do next.

      switch (Pipe[i].dwState)
      {
      // READING_STATE:
      // The pipe instance is connected to the client
      // and is ready to read a request from the client.

         case READING_STATE:
            fSuccess = ReadFile(
               Pipe[i].hPipeInst,
               Pipe[i].chRequest,
               BUFSIZE*sizeof(TCHAR),
               &Pipe[i].cbRead,
               &Pipe[i].oOverlap);

         // The read operation completed successfully.

            if (fSuccess && Pipe[i].cbRead != 0)
            {
               Pipe[i].fPendingIO = FALSE;
               Pipe[i].dwState = WRITING_STATE;
               continue;
            }

         // The read operation is still pending.

            dwErr = GetLastError();
            if (! fSuccess && (dwErr == ERROR_IO_PENDING))
            {
               Pipe[i].fPendingIO = TRUE;
               continue;
            }

         // An error occurred; disconnect from the client.

            DisconnectAndReconnect(i);
            break;

      // WRITING_STATE:
      // The request was successfully read from the client.
      // Get the reply data and write it to the client.

         case WRITING_STATE:
            response = emit lineReady( QString::fromUtf8( (char*)Pipe[i].chRequest, Pipe[i].cbRead ) ).toUtf8();
            StringCchCopy( Pipe[i].chReply, BUFSIZE, (LPCTSTR)response.data() );
            Pipe[i].cbToWrite = response.size();

            fSuccess = WriteFile(
               Pipe[i].hPipeInst,
               Pipe[i].chReply,
               Pipe[i].cbToWrite,
               &cbRet,
               &Pipe[i].oOverlap);

         // The write operation completed successfully.

            if (fSuccess && cbRet == Pipe[i].cbToWrite)
            {
               Pipe[i].fPendingIO = FALSE;
               Pipe[i].dwState = READING_STATE;
               continue;
            }

         // The write operation is still pending.

            dwErr = GetLastError();
            if (! fSuccess && (dwErr == ERROR_IO_PENDING))
            {
               Pipe[i].fPendingIO = TRUE;
               continue;
            }

         // An error occurred; disconnect from the client.

            DisconnectAndReconnect(i);
            break;

         default:
         {
            printf("Invalid pipe state.\n");
            return;
         }
      }
  }
}


// DisconnectAndReconnect(DWORD)
// This function is called when an error occurs or when the client
// closes its handle to the pipe. Disconnect from this client, then
// call ConnectNamedPipe to wait for another client to connect.

VOID
DisconnectAndReconnect(DWORD i)
{
// Disconnect the pipe instance.

   if (! DisconnectNamedPipe(Pipe[i].hPipeInst) )
   {
      printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
   }

// Call a subroutine to connect to the new client.

   Pipe[i].fPendingIO = ConnectToNewClient(
      Pipe[i].hPipeInst,
      &Pipe[i].oOverlap);

   Pipe[i].dwState = Pipe[i].fPendingIO ?
      CONNECTING_STATE : // still connecting
      READING_STATE;     // ready to read
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED)
// This function is called to start an overlapped connect operation.
// It returns TRUE if an operation is pending or FALSE if the
// connection has been completed.

BOOL
ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
   BOOL fConnected, fPendingIO = FALSE;

// Start an overlapped connection for this pipe instance.
   fConnected = ConnectNamedPipe(hPipe, lpo);

// Overlapped ConnectNamedPipe should return zero.
   if (fConnected)
   {
      printf("ConnectNamedPipe failed with %d.\n", GetLastError());
      return 0;
   }

   switch (GetLastError())
   {
   // The overlapped connection in progress.
      case ERROR_IO_PENDING:
         fPendingIO = TRUE;
         break;

   // Client is already connected, so signal an event.

      case ERROR_PIPE_CONNECTED:
         if (SetEvent(lpo->hEvent))
            break;

   // If an error occurs during the connect operation...
      default:
      {
         printf("ConnectNamedPipe failed with %d.\n", GetLastError());
         return 0;
      }
   }

   return fPendingIO;
}

