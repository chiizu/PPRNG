/*
  Copyright (C) 2011 chiizu
  chiizu.pprng@gmail.com
  
  This file is part of PPRNG.
  
  PPRNG is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  PPRNG is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with PPRNG.  If not, see <http://www.gnu.org/licenses/>.
*/


#import "AppDelegate.h"
#import "Gen5ConfigurationController.h"
#import "DSTypeTransformer.h"
#import "VersionTransformer.h"

@implementation AppDelegate

+ (void)initialize
{
  [NSValueTransformer setValueTransformer: [[DSTypeTransformer alloc] init]
                      forName: @"DSTypeTransformer"];
  [NSValueTransformer setValueTransformer: [[VersionTransformer alloc] init]
                      forName: @"VersionTransformer"];
}

// quit when all windows are closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
	return YES;
}

// start up
- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
	launcherController = [[LauncherController alloc] init];
	[launcherController showWindow:self];
}

@end
