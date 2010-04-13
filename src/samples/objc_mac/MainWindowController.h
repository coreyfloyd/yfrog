//
//  MainWindowController.h
//  YFrogLibraryMacSample
//
//  Created by Andrej Tsvigoon on 12.06.09.
//  Copyright 2009 BlitzClick. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "yFrogImageUploader.h"
#import "yFrogImageDownoader.h"

@interface MainWindowController : NSWindowController <ImageUploaderDelegate, ImageDownoaderDelegate>
{
	IBOutlet NSImageView *imageForUploading;
	IBOutlet NSImageView *fullDownloadedImage;
	IBOutlet NSImageView *iPhoneDownloadedImage;
	IBOutlet NSImageView *thumbDownloadedImage;
	IBOutlet NSTextField *loginField;
	IBOutlet NSSecureTextField *passField;
   
	IBOutlet NSButton *start;
	IBOutlet NSButton *reset;


	ImageUploader *uploadConnection;
   
	ImageDownoader *downloadFullImageConnection;
	ImageDownoader *downloadIPhoneImageConnection;
	ImageDownoader *downloadThumbnailImageConnection;
}

- (IBAction)starUploadAndDownload:(id)sender;
- (IBAction)resetUploadAndDownload:(id)sender;
- (void)uploadedImage:(NSString*)yFrogURL sender:(ImageUploader*)sender;
- (void)receivedImage:(NSImage*)image sender:(ImageDownoader*)sender;

@property (nonatomic, retain) ImageUploader* uploadConnection;

@property (nonatomic, retain) ImageDownoader* downloadFullImageConnection;
@property (nonatomic, retain) ImageDownoader* downloadIPhoneImageConnection;
@property (nonatomic, retain) ImageDownoader* downloadThumbnailImageConnection;

@end
