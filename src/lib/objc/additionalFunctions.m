//
//  iPhoneFunctions.h
//  YFrogLibrary
//
//  Created by Andrej Tsvigoon on 08.06.09.
//  Copyright 2009 BlitzClick. All rights reserved.
//

#import "additionalFunctions.h"


#if defined (MANAGE_NETWORK_ACTIVITY_INDICATOR_USING_YFROG_LIBRARY) && defined (TARGET_OS_IPHONE)
	static int NetworkActivityIndicatorCounter = 0;


	void increaseNetworkActivityIndicator(void)
	{
		NetworkActivityIndicatorCounter++;
		[UIApplication sharedApplication].networkActivityIndicatorVisible = NetworkActivityIndicatorCounter > 0;
	}

	void decreaseNetworkActivityIndicator(void)
	{
		NetworkActivityIndicatorCounter--;
		[UIApplication sharedApplication].networkActivityIndicatorVisible = NetworkActivityIndicatorCounter > 0;
	}
#endif
	
#if TARGET_OS_IPHONE
	// may cause a crash in non main thead
	UIImage* imageScaledToSize(UIImage* image, int maxDimension)
	{
		CGImageRef imgRef = image.CGImage;  
		
		CGFloat width = CGImageGetWidth(imgRef);  
		CGFloat height = CGImageGetHeight(imgRef);  
		
		CGAffineTransform transform = CGAffineTransformIdentity;
		CGRect bounds = CGRectMake(0, 0, width, height);  
		
		if(maxDimension > 0) //need scale
		{
			 if (width > maxDimension || height > maxDimension) 
			 {  
				 CGFloat ratio = width/height;  
				 if (ratio > 1)
				 {  
					 bounds.size.width = maxDimension;  
					 bounds.size.height = bounds.size.width / ratio;  
				 }  
				 else
				 {  
					 bounds.size.height = maxDimension;  
					 bounds.size.width = bounds.size.height * ratio;  
				 }  
			 }
		}
		CGFloat scaleRatio = bounds.size.width / width;
		CGSize imageSize = CGSizeMake(CGImageGetWidth(imgRef), CGImageGetHeight(imgRef));  
		CGFloat boundHeight;  
		
		UIImageOrientation orient = image.imageOrientation;  
		switch(orient) 
		{  
			case UIImageOrientationUp: //EXIF = 1  
				transform = CGAffineTransformIdentity;  
				break;  
				
			case UIImageOrientationUpMirrored: //EXIF = 2  
				transform = CGAffineTransformMakeTranslation(imageSize.width, 0.0);  
				transform = CGAffineTransformScale(transform, -1.0, 1.0);  
				break;  
				
			case UIImageOrientationDown: //EXIF = 3  
				transform = CGAffineTransformMakeTranslation(imageSize.width, imageSize.height);  
				transform = CGAffineTransformRotate(transform, M_PI);  
				break;  
				
			case UIImageOrientationDownMirrored: //EXIF = 4  
				transform = CGAffineTransformMakeTranslation(0.0, imageSize.height);  
				transform = CGAffineTransformScale(transform, 1.0, -1.0);  
				break;  
				
			case UIImageOrientationLeftMirrored: //EXIF = 5  
				boundHeight = bounds.size.height;  
				bounds.size.height = bounds.size.width;  
				bounds.size.width = boundHeight;  
				transform = CGAffineTransformMakeTranslation(imageSize.height, imageSize.width);  
				transform = CGAffineTransformScale(transform, -1.0, 1.0);  
				transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);  
				break;  
				
			case UIImageOrientationLeft: //EXIF = 6  
				boundHeight = bounds.size.height;  
				bounds.size.height = bounds.size.width;  
				bounds.size.width = boundHeight;  
				transform = CGAffineTransformMakeTranslation(0.0, imageSize.width);  
				transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);  
				break;  
				
			case UIImageOrientationRightMirrored: //EXIF = 7  
				boundHeight = bounds.size.height;  
				bounds.size.height = bounds.size.width;  
				bounds.size.width = boundHeight;  
				transform = CGAffineTransformMakeScale(-1.0, 1.0);  
				transform = CGAffineTransformRotate(transform, M_PI / 2.0);  
				break;  
				
			case UIImageOrientationRight: //EXIF = 8  
				boundHeight = bounds.size.height;  
				bounds.size.height = bounds.size.width;  
				bounds.size.width = boundHeight;  
				transform = CGAffineTransformMakeTranslation(imageSize.height, 0.0);  
				transform = CGAffineTransformRotate(transform, M_PI / 2.0);  
				break;  
				
			default:  
				[NSException raise:NSInternalInconsistencyException format:@"Invalid image orientation"];  
				
		}  
		
		UIGraphicsBeginImageContext(bounds.size);
		
		CGContextRef context = UIGraphicsGetCurrentContext();  
		
		if (orient == UIImageOrientationRight || orient == UIImageOrientationLeft)
		{
			CGContextScaleCTM(context, -scaleRatio, scaleRatio);
			CGContextTranslateCTM(context, -height, 0);  
		}
		else
		{  
			CGContextScaleCTM(context, scaleRatio, -scaleRatio);
			CGContextTranslateCTM(context, 0, -height);  
		}  
		
		CGContextConcatCTM(context, transform);  
		
		CGContextDrawImage(UIGraphicsGetCurrentContext(), CGRectMake(0, 0, width, height), imgRef);  
		UIImage *imageCopy = UIGraphicsGetImageFromCurrentImageContext();  
		UIGraphicsEndImageContext();  
		
		return imageCopy;
		
	}

#else

	NSSize calcDrawSizeForImageSize(NSSize imgSize, NSSize availableSize)
	{
		NSSize drawSize;
		if(imgSize.height == 0 || imgSize.width == 0)
			drawSize = availableSize;
		else if(imgSize.width <= availableSize.width && imgSize.height <= availableSize.height)
			drawSize = imgSize;
		else
		{
			float kAvailable = availableSize.height / availableSize.width;
			float kImage = imgSize.height / imgSize.width;
			if(kAvailable < kImage)
			{
				drawSize.height = ceil(availableSize.height);
				drawSize.width = ceil(drawSize.height / kImage);
			}
			else
			{
				drawSize.width = ceil(availableSize.width);
				drawSize.height = ceil(drawSize.width * kImage);
			}
		}
		return drawSize;
	}


	NSImage* imageScaledToSize(NSImage* source, int maxDimension)
	{
		NSSize size = calcDrawSizeForImageSize([source size], NSMakeSize(maxDimension, maxDimension));
		NSImage *newImage = [[[NSImage alloc] initWithSize:size] autorelease];
		[newImage lockFocus];
		[[NSColor clearColor] set];
		NSRect thumbRect = NSMakeRect(0.f, 0.f, size.width, size.height);
		[NSBezierPath fillRect:thumbRect];
		[[NSGraphicsContext currentContext] setImageInterpolation:NSImageInterpolationHigh];
		[source drawInRect: thumbRect fromRect:NSMakeRect(0.f, 0.f, [source size].width, [source size].height) operation:NSCompositeSourceOver fraction:1.0];
		[newImage unlockFocus];
		return newImage;
	}

#endif




