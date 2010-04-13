//
//  iPhoneFunctions.h
//  YFrogLibrary
//
//  Created by Andrej Tsvigoon on 08.06.09.
//  Copyright 2009 BlitzClick. All rights reserved.
//


#if TARGET_OS_IPHONE
	// may cause a crash in non main thead
	UIImage* imageScaledToSize(UIImage* image, int maxDimension);
#else
	NSImage* imageScaledToSize(NSImage* source, int maxDimension);
#endif


#if defined (MANAGE_NETWORK_ACTIVITY_INDICATOR_USING_YFROG_LIBRARY) && defined (TARGET_OS_IPHONE)

	void increaseNetworkActivityIndicator(void);
	void decreaseNetworkActivityIndicator(void);
	
	#define INCREASE_NETWORK_ACTIVITY_INDICATOR		increaseNetworkActivityIndicator()
	#define DECREASE_NETWORK_ACTIVITY_INDICATOR		decreaseNetworkActivityIndicator()

#else

	#define INCREASE_NETWORK_ACTIVITY_INDICATOR
	#define DECREASE_NETWORK_ACTIVITY_INDICATOR

#endif


