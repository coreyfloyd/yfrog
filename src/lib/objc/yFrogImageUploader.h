// Copyright (c) 2009 Imageshack Corp.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#import <Foundation/Foundation.h>
#import "YFrogConnectionProtocol.h"

@class ImageUploader;

@protocol ImageUploaderDelegate<NSObject>

- (void)uploadedImage:(NSString*)yFrogURL sender:(ImageUploader*)sender;

@end

typedef enum _TargetBlog
{
	kNoBlog,
	kTwitter
} TargetBlog;

@interface ImageUploader : NSObject <YFrogConnectionProtocol>
{
	NSMutableData*	result;
	id <ImageUploaderDelegate> delegate;
	id userData;
	
	NSURLConnection *connection;
	
	NSMutableString* contentXMLProperty;
	NSString*		newURL;
	NSString*		login;
	NSString*		password;
	BOOL			canceled;
	BOOL			scaleIfNeed;
	
	BOOL			useLocations;
	float			latitude;
	float			longitude;
	
	float			imageScalingSize;

	NSString*		contentType;
	NSString*		messageForBlog;
	TargetBlog		blog;
}

- (void)uploadJPEGData:(NSData*)imageJPEGData delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data;
- (void)uploadJPEGData:(NSData*)imageJPEGData twitterUpdate:(NSString*)twitterText delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data;
- (void)uploadMP4Data:(NSData*)movieData delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data;
- (void)uploadMP4Data:(NSData*)movieData twitterUpdate:(NSString*)twitterText delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data;
#if TARGET_OS_IPHONE
- (void)uploadImage:(UIImage*)image delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data; // call uploadJPEGData:delegate:userData:
- (void)uploadImage:(UIImage*)image twitterUpdate:(NSString*)twitterText delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data; // call uploadJPEGData:delegate:userData:
#else
- (void)uploadImage:(NSImage*)image delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data; // call uploadJPEGData:delegate:userData:
- (void)uploadImage:(NSImage*)image twitterUpdate:(NSString*)twitterText delegate:(id <ImageUploaderDelegate>)dlgt userData:(id)data; // call uploadJPEGData:delegate:userData:
#endif
- (void)cancel;
- (BOOL)canceled;


@property (nonatomic, retain) NSURLConnection *connection;
@property (nonatomic, retain) NSMutableString* contentXMLProperty;
@property (nonatomic, retain) NSString* newURL;
@property (nonatomic, retain) NSString* login;
@property (nonatomic, retain) NSString* password;
@property (nonatomic, retain) id userData;
@property (nonatomic, retain) id <ImageUploaderDelegate> delegate;
@property (readwrite, assign) BOOL scaleIfNeed;
@property (readwrite, assign) float imageScalingSize;
@property (nonatomic, retain) NSString* contentType;
@property (nonatomic, retain) NSString* messageForBlog;
@property (readwrite, assign) TargetBlog blog;

@end
