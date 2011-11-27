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


#import "Gen4ConfigurationEditController.h"
#include "BasicTypes.h"

using namespace pprng;

@implementation Gen4ConfigurationEditController

+ (NSMutableDictionary*)newConfig:(NSString*)configName
{
  return [NSMutableDictionary dictionaryWithObjectsAndKeys:
            configName, @"name",
            [NSNumber numberWithInt: -1], @"version",
            nil];
}

+ (void)initialize
{
  NSMutableDictionary  *gen4Config = [NSMutableDictionary dictionary];
  [gen4Config setObject:[NSArray arrayWithObject:[self newConfig:@"MyConfig"]]
              forKey: @"gen4Configurations"];
  [gen4Config setObject: [NSNumber numberWithInt: 0]
              forKey: @"gen4ConfigIndex"];
  
  [[NSUserDefaults standardUserDefaults] registerDefaults: gen4Config];
  
  [[NSUserDefaultsController sharedUserDefaultsController]
    setAppliesImmediately: NO];
}

- (NSString *)windowNibName
{
	return @"Gen4Configuration";
}

- (NSString*)uniqueConfigName
{
  return @"NewConfig";
}

- (BOOL)runModal
{
  NSUserDefaultsController  *sharedController =
    [NSUserDefaultsController sharedUserDefaultsController];
  
  NSInteger  result = [NSApp runModalForWindow: [self window]];
  
  if (result == 1)
  {
    NSInteger  selectedRow = [configurationTableView selectedRow];
    if (selectedRow >= 0)
    {
      [[NSUserDefaults standardUserDefaults]
        setObject: [NSNumber numberWithInteger: selectedRow]
        forKey: @"gen4ConfigIndex"];
    }
    
    [sharedController save: self];
  }
  else
  {
    [sharedController revert: self];
  }
  [[self window] close];
  
  return result == 1;
}

- (BOOL)runModalWithConfigIndex:(NSInteger)configIndex;
{
  [configurationTableView
      selectRowIndexes: [NSIndexSet indexSetWithIndex: configIndex]
      byExtendingSelection: NO];
  
  return [self runModal];
}

- (void)awakeFromNib
{
  // get current config
  NSInteger  currentConfigIndex =
    [[NSUserDefaults standardUserDefaults] integerForKey: @"gen4ConfigIndex"];
  
  [configurationTableView
    selectRowIndexes: [NSIndexSet indexSetWithIndex: currentConfigIndex]
    byExtendingSelection: NO];
  
  [versionPopUp setAutoenablesItems: NO];
}

- (IBAction)addConfig:(id)sender
{
  NSString  *configName = [self uniqueConfigName];
  
  NSMutableDictionary *config =
    [Gen4ConfigurationEditController newConfig: configName];
  
  [configurationArrayController addObject: config];
}

- (IBAction)removeConfig:(id)sender
{
  if ([[configurationArrayController arrangedObjects] count] > 1)
  {
    [configurationArrayController remove: self];
  }
}

- (IBAction)duplicateConfig:(id)sender
{
  NSMutableDictionary *dup =
    [NSMutableDictionary dictionaryWithDictionary:
      [[configurationArrayController selectedObjects] objectAtIndex: 0]];
  
  [configurationArrayController addObject: dup];
}

- (IBAction)changeVersion:(id)sender
{
}

- (IBAction)done:(id)sender
{
  [NSApp stopModalWithCode: 1];
}

- (IBAction)cancel:(id)sender
{
  [NSApp stopModalWithCode: 0];
}

- (BOOL)tableView:(NSTableView*)tableView
        shouldSelectTableColumn:(NSTableColumn*)column
{
  return NO;
}

@end
