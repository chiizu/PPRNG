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


#import "Gen4ConfigurationController.h"

#import "Gen4ConfigurationEditController.h"

using namespace pprng;

@implementation Gen4ConfigurationController

- (void)setCurrentConfiguration
{
  NSInteger  currentConfigIndex =
    [[NSUserDefaults standardUserDefaults] integerForKey: @"gen4ConfigIndex"];
  
  [configMenu selectItemAtIndex: currentConfigIndex];
  [configArrayController setSelectionIndex: [configMenu indexOfSelectedItem]];
}

- (void)awakeFromNib
{
  [self setCurrentConfiguration];
}

- (IBAction)switchConfiguration:(id)sender
{
  NSInteger  idx = [sender indexOfSelectedItem];
  [configArrayController setSelectionIndex: idx];
  
  [[NSUserDefaults standardUserDefaults]
    setObject: [NSNumber numberWithInteger: idx]
    forKey: @"gen4ConfigIndex"];
  
  NSUserDefaultsController  *sharedController =
    [NSUserDefaultsController sharedUserDefaultsController];
  [sharedController save: self];
}

- (IBAction)editConfigurations:(id)sender
{
  Gen4ConfigurationEditController  *configController =
    [[Gen4ConfigurationEditController alloc] init];
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

@end
