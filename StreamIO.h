/*
 * Copyright (C) 2017 Kai Niessen <kai.niessen@online.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   StreamIO.h
 * Author: Kai Niessen <kai.niessen@online.de>
 *
 * Created on April 11, 2017, 10:21 PM
 */

#ifndef STREAMIO_H
#define STREAMIO_H

#include <MediaIO.h>
#include <Url.h>
#include <HttpRequest.h>
#include <UrlProtocolRoster.h>
#include <Handler.h>
#include <AdapterIO.h>
#include <Looper.h>

#define MSG_META_CHANGE					'META'

class Station;
class StreamIO;

typedef void							(StreamIO::*DataFunc)(BUrlRequest* request, 
											const char* data,
											off_t position, ssize_t size,
											int next);

class DataFuncs {
public:
										DataFuncs() 
										  : fSize(10),
											fLast(-1)
										{ 
											  for (int i = 0; i < fSize; i++)
												  fFuncs[i] = NULL;
										};
	int									Add(DataFunc f) {
											fFuncs[++fLast] = f;
										}
	void								Add(DataFunc f, int before) {
											for (int i = fLast; i >= before; i--) 
												fFuncs[i+1] = fFuncs[i];
											fFuncs[before] = f;
											fLast++;
										};
	DataFunc							Remove(int index) {
											DataFunc result = fFuncs[index];
											for (int i = index; i < fLast; i++) 
												fFuncs[i] = fFuncs[i + 1];
											fFuncs[fLast--] = NULL;
										};
	DataFunc							Item(int index) {
											return fFuncs[index];
										};
	DataFunc							First() {
											return fFuncs[0];
										};
private:
	DataFunc							fFuncs[10];
	int									fSize;
	int									fLast;
};

using namespace BCodecKit;
class StreamIO : public BAdapterIO, protected BUrlProtocolListener {
public:
										StreamIO(Station* station, 
											BLooper* MetaListener = NULL);
	virtual								~StreamIO();
	virtual void						GetFlags(int32* flags) const;
	virtual	ssize_t						WriteAt(off_t position,
											const void* buffer, size_t size);
	virtual	ssize_t						ReadAt(off_t position, void* buffer,
											size_t size);
	virtual status_t					SetSize(off_t size);
	virtual status_t					Open();
	virtual bool						IsRunning() const;
	void								SetLimiter(size_t limit = 0);
	size_t								Buffered();
protected:
	virtual	status_t					SeekRequested(off_t position);
	virtual void						HeadersReceived(BUrlRequest* request, 
											const BUrlResult& result);
	virtual void						DataRedirectReceived(BUrlRequest* request, 
											const char* data,
											off_t position, ssize_t size, 
											int next);
	virtual void						DataWithMetaReceived(BUrlRequest* request, 
											const char* data,
											off_t position, ssize_t size, 
											int next);
	virtual void						DataUnsyncedReceived(BUrlRequest* request, 
											const char* data,
											off_t position, ssize_t size,
											int next);
	virtual void						DataSyncedReceived(BUrlRequest* request, 
											const char* data,
											off_t position, ssize_t size,
											int next);
	virtual void						DataReceived(BUrlRequest* request, 
											const char* data,
											off_t position, ssize_t size);
	virtual void						RequestCompleted(BUrlRequest* request, 
											bool success);
	void								UpdateSize();
	void								ProcessMeta();
private:
	DataFuncs							fDataFuncs;
	Station*							fStation;
	BInputAdapter*						fInputAdapter;
	BHttpRequest*						fReq;
	thread_id							fReqThread;
	off_t								fPosition;
	off_t								fTotalSize;
	bool								fIsMutable;
	size_t								fMetaInt;
	char								fMetaBuffer[512];
	size_t								fMetaSize;
	off_t								fUntilMetaStart;
	off_t								fUntilMetaEnd;
	bigtime_t							fReqStartTime;
	BLooper*							fMetaListener;
	BString								fStreamTitle;
	enum								FrameSync {
											none,
											first,
											done
	};
	FrameSync							fFrameSync;
	size_t								fLimit;
	size_t								fBuffered;
};

#endif /* STREAMIO_H */

