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


#import "Gen5ConfigurationEditController.h"
#include "BasicTypes.h"

using namespace pprng;

@implementation Gen5ConfigurationEditController

+ (NSMutableDictionary*)newConfig:(NSString*)configName
{
  return [NSMutableDictionary dictionaryWithObjectsAndKeys:
            configName, @"name",
            [NSNumber numberWithInt: -1], @"version",
            [NSNumber numberWithInt: -1], @"dsType",
            nil];
}

+ (void)initialize
{
  NSMutableDictionary  *gen5Config = [NSMutableDictionary dictionary];
  [gen5Config setObject:[NSArray arrayWithObject:[self newConfig:@"MyConfig"]]
              forKey: @"gen5Configurations"];
  [gen5Config setObject: [NSNumber numberWithInt: 0]
              forKey: @"gen5ConfigIndex"];
  
  [[NSUserDefaults standardUserDefaults] registerDefaults: gen5Config];
  
  [[NSUserDefaultsController sharedUserDefaultsController]
    setAppliesImmediately: NO];
}

- (NSString *)windowNibName
{
	return @"Gen5Configuration";
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
        forKey: @"gen5ConfigIndex"];
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
  [[macAddressField0 formatter] setFormatWidth: 2];
  [[macAddressField1 formatter] setFormatWidth: 2];
  [[macAddressField2 formatter] setFormatWidth: 2];
  [[macAddressField3 formatter] setFormatWidth: 2];
  [[macAddressField4 formatter] setFormatWidth: 2];
  [[macAddressField5 formatter] setFormatWidth: 2];
  
  /* get current config */
  NSInteger  currentConfigIndex =
    [[NSUserDefaults standardUserDefaults] integerForKey: @"gen5ConfigIndex"];
  
  [configurationTableView
    selectRowIndexes: [NSIndexSet indexSetWithIndex: currentConfigIndex]
    byExtendingSelection: NO];
  
  [versionPopUp setAutoenablesItems: NO];
}

- (IBAction)addConfig:(id)sender
{
  NSString  *configName = [self uniqueConfigName];
  
  NSMutableDictionary *config =
    [Gen5ConfigurationEditController newConfig: configName];
  
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

- (IBAction)changeDSType:(id)sender
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
