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

#import "CoinFlipsTransformer.h"
#import "DSTypeTransformer.h"
#import "HGSSRoamerLocationsTransformer.h"
#import "ProfElmResponsesTransformer.h"
#import "VersionTransformer.h"

#import "VersionCheckURL.h"

@implementation AppDelegate

+ (void)initialize
{
  [NSValueTransformer setValueTransformer: [[CoinFlipsTransformer alloc] init]
                      forName: @"CoinFlipsTransformer"];
  [NSValueTransformer setValueTransformer: [[DSTypeTransformer alloc] init]
                      forName: @"DSTypeTransformer"];
  [NSValueTransformer
   setValueTransformer: [[HGSSRoamerLocationsTransformer alloc] init]
   forName: @"HGSSRoamerLocationsTransformer"];
  [NSValueTransformer
   setValueTransformer: [[ProfElmResponsesTransformer alloc] init]
   forName: @"ProfElmResponsesTransformer"];
  [NSValueTransformer setValueTransformer: [[VersionTransformer alloc] init]
                      forName: @"VersionTransformer"];
}

// quit when all windows are closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
  return YES;
}

// check for a newer version - should be called on background thread
- (void)checkVersion
{
  NSString  *currentVersion = [[[NSBundle mainBundle] infoDictionary]
                               objectForKey:(NSString*)kCFBundleVersionKey];
  
  NSString       *urlString = [NSString stringWithFormat: @"%@?version=%@",
                                 VERSION_CHECK_URL, currentVersion];
  
  NSURLRequest   *request =
    [NSURLRequest requestWithURL:[NSURL URLWithString: urlString]
                  cachePolicy: NSURLRequestReloadIgnoringLocalAndRemoteCacheData
                  timeoutInterval: 60.0];
  NSURLResponse  *response;
  NSError        *error;
  
  NSData  *result =
    [NSURLConnection sendSynchronousRequest:request
                     returningResponse:&response error:&error];
  if (result == nil)
  {
    NSLog(@"Unable to perform version check.  Error %@",
          [error localizedDescription]);
    return;
  }
  
  NSString  *latestVersion = [[NSString alloc] initWithBytes: [result bytes]
                              length: [result length]
                              encoding:NSISOLatin1StringEncoding];
  
  NSArray  *latestVersionElements =
    [latestVersion componentsSeparatedByString: @"."];
  NSArray  *currentVersionElements =
    [currentVersion componentsSeparatedByString: @"."];
  
  int   i = 0;
  BOOL  reportNewVersion = NO;
  while ((i < [latestVersionElements count]) &&
         (i < [currentVersionElements count]))
  {
    int  latest = [[latestVersionElements objectAtIndex: i] intValue];
    int  current = [[currentVersionElements objectAtIndex: i] intValue];
    
    if (current < latest)
    {
      reportNewVersion = YES;
      break;
    }
    ++i;
  }
  if (!reportNewVersion &&
      ([latestVersionElements count] > [currentVersionElements count]))
  {
    reportNewVersion = YES;
  }
  
  if (reportNewVersion)
  {
    [self performSelectorOnMainThread:@selector(notifyNewVersion:)
          withObject:latestVersion waitUntilDone:NO];
  }
}

- (void)notifyNewVersion:(NSString*)version
{
  NSAlert *alert = [[NSAlert alloc] init];
  
  [alert addButtonWithTitle:@"OK"];
  [alert setMessageText: [NSString stringWithFormat: @"Version %@ of PPRNG is now available", version]];
  [alert setInformativeText: [NSString stringWithFormat: @"Please update to get the latest features and bug fixes."]];
  [alert setAlertStyle:NSInformationalAlertStyle];
  
  [alert runModal];
}

// start up
- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
  launcherController = [[LauncherController alloc] init];
  [launcherController showWindow:self];
  [self performSelectorInBackground:@selector(checkVersion) withObject:nil];
}

@end
