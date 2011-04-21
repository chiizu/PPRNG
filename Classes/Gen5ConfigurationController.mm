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


#import "Gen5ConfigurationController.h"

#import "Gen5ConfigurationEditController.h"

using namespace pprng;

@implementation Gen5ConfigurationController

- (BOOL)verifyBasicParameters
{
/*
  return [macAddressParameterController isFilled] &&
         ((versionMenu == nil) || ([[versionMenu selectedItem] tag] != -1));*/
         return YES;
}

- (BOOL)verifySeedParameters
{
  return [self verifyBasicParameters] &&
         ([timer0LowField objectValue] != nil) &&
         ([timer0HighField objectValue] != nil) &&
         ([vcountLowField objectValue] != nil) &&
         ([vcountHighField objectValue] != nil) &&
         ([vframeLowField objectValue] != nil) &&
         ([vframeHighField objectValue] != nil);
}

- (BOOL)isFilled
{
  return ([macAddressField0 objectValue] != nil) &&
         ([macAddressField1 objectValue] != nil) &&
         ([macAddressField2 objectValue] != nil) &&
         ([macAddressField3 objectValue] != nil) &&
         ([macAddressField4 objectValue] != nil) &&
         ([macAddressField5 objectValue] != nil);
}

- (void)setCurrentConfiguration
{
  NSInteger  currentConfigIndex =
    [[NSUserDefaults standardUserDefaults] integerForKey: @"gen5ConfigIndex"];
  
  [configMenu selectItemAtIndex: currentConfigIndex];
  [configArrayController setSelectionIndex: [configMenu indexOfSelectedItem]];
}

- (void)awakeFromNib
{
  if (macAddressField0 != nil)
  {
    [[macAddressField0 formatter] setFormatWidth: 2];
    [[macAddressField1 formatter] setFormatWidth: 2];
    [[macAddressField2 formatter] setFormatWidth: 2];
    [[macAddressField3 formatter] setFormatWidth: 2];
    [[macAddressField4 formatter] setFormatWidth: 2];
    [[macAddressField5 formatter] setFormatWidth: 2];
  }
  
  [[tidField formatter] setFormatWidth: 5];
  [[tidField formatter] setPaddingCharacter: @"0"];
  [[sidField formatter] setFormatWidth: 5];
  [[sidField formatter] setPaddingCharacter: @"0"];
  
  [self setCurrentConfiguration];
}

- (IBAction)switchConfiguration:(id)sender
{
  NSInteger  idx = [sender indexOfSelectedItem];
  [configArrayController setSelectionIndex: idx];
  
  [[NSUserDefaults standardUserDefaults]
    setObject: [NSNumber numberWithInteger: idx]
    forKey: @"gen5ConfigIndex"];
  
  NSUserDefaultsController  *sharedController =
    [NSUserDefaultsController sharedUserDefaultsController];
  [sharedController save: self];
}

- (IBAction)editConfigurations:(id)sender
{
  Gen5ConfigurationEditController  *configController =
    [[Gen5ConfigurationEditController alloc] init];
  if ([configController
         runModalWithConfigIndex: [configMenu indexOfSelectedItem]])
  {
    [self setCurrentConfiguration];
  }
}

- (Game::Version)version
{
  NSDictionary  *config =
    [[configArrayController selectedObjects] objectAtIndex: 0];
  NSNumber      *verNum = [config objectForKey: @"version"];
  
  return static_cast<Game::Version>([verNum intValue]);
}

- (uint32_t)tid
{
  return [tidField intValue];
}

- (uint32_t)sid
{
  return [sidField intValue];
}

- (DS::Type)dsType
{
  NSDictionary  *config =
    [[configArrayController selectedObjects] objectAtIndex: 0];
  NSNumber      *dsTypeNum = [config objectForKey: @"dsType"];
  
  return static_cast<DS::Type>([dsTypeNum intValue]);
}

- (uint32_t)macAddressLow
{
  return [macAddressField0 intValue] |
         ([macAddressField1 intValue] << 8) |
         ([macAddressField2 intValue] << 16);
}

- (uint32_t)macAddressHigh
{
  return [macAddressField3 intValue] |
         ([macAddressField4 intValue] << 8) |
         ([macAddressField5 intValue] << 16);
}

- (uint32_t)timer0Low
{
  return [timer0LowField intValue];
}
- (uint32_t)timer0High
{
  return [timer0HighField intValue];
}

- (uint32_t)vcountLow
{
  return [vcountLowField intValue];
}
- (uint32_t)vcountHigh
{
  return [vcountHighField intValue];
}

- (uint32_t)vframeLow
{
  return [vframeLowField intValue];
}
- (uint32_t)vframeHigh
{
  return [vframeHighField intValue];
}

@end
