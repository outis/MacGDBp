/*
 * MacGDBp
 * Copyright (c) 2007 - 2010, Blue Static <http://www.bluestatic.org>
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU 
 * General Public License as published by the Free Software Foundation; either version 2 of the 
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program; if not, 
 * write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#import <Cocoa/Cocoa.h>

@protocol DebuggerConnectionDelegate;
@class LoggingController;

@interface DebuggerConnection : NSObject
{
	// The port to connect on.
	NSUInteger port_;
	
	// If the connection to the debugger engine is currently active.
	BOOL connected_;

	// The raw CFSocket on which the two streams are based. Strong.
	CFSocketRef socket_;
	
	// The read stream that is scheduled on the main run loop. Weak.
	CFReadStreamRef readStream_;
	
	// The write stream. Weak.
	CFWriteStreamRef writeStream_;
	
	// An ever-increasing integer that gives each transaction a unique ID for the
	// debugging engine.
	NSUInteger transactionID;
	
	// The most recently received transaction ID.
	NSUInteger lastReadTransaction_;
	
	// The last transactionID written to the stream.
	NSUInteger lastWrittenTransaction_;
	
	// To prevent blocked writing, we enqueue all writes and then wait for the
	// write stream to tell us it's ready. We store the pending commands in this
	// array. We use this as a stack (FIFO), with index 0 being first.
	NSMutableArray* queuedWrites_;
	
	// We send queued writes in multiple places, sometimes off a run loop event.
	// Because of this, we need to ensure that only one client is dequeing and
	// sending at a time.
	NSRecursiveLock* writeQueueLock_;
	
	// Information about the current read loop. We append to |currentPacket_|
	// until |currentPacketSize_| has reached |packetSize_|.
	NSMutableString* currentPacket_;
	int packetSize_;
	int currentPacketIndex_;

	// The delegate.
	id <DebuggerConnectionDelegate> delegate_;
}

@property (readonly) NSUInteger port;
@property (readonly) BOOL connected;
@property (assign) id <DebuggerConnectionDelegate> delegate;

- (id)initWithPort:(NSUInteger)aPort;

- (void)connect;
- (void)close;
- (void)socketDidAccept;
- (void)socketDisconnected;
- (void)readStreamHasData;
- (void)send:(NSString*)command;
- (void)performSend:(NSString*)command;
- (void)errorEncountered:(NSString*)error;

- (void)handleResponse:(NSXMLDocument*)response;
- (void)handlePacket:(NSString*)packet;

- (NSNumber*)sendCommandWithFormat:(NSString*)format, ...;

- (void)sendQueuedWrites;

- (NSString*)escapedURIPath:(NSString*)path;
- (NSInteger)transactionIDFromResponse:(NSXMLDocument*)response;
- (NSInteger)transactionIDFromCommand:(NSString*)command;

@end

// Delegate ////////////////////////////////////////////////////////////////////

@protocol DebuggerConnectionDelegate <NSObject>

@optional

- (void)connectionDidAccept:(DebuggerConnection*)cx;
- (void)connectionDidCose:(DebuggerConnection*)cx;

- (void)handleInitialResponse:(NSXMLDocument*)response;

- (void)handleResponse:(NSXMLDocument*)response;

- (void)errorEncountered:(NSString*)error;

@end

