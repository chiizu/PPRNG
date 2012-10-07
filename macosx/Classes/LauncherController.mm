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


#import "LauncherController.h"

// Gen 5 Common
#import "HashedSeedSearcherController.h"
#import "StandardSeedInspectorController.h"
#import "WonderCardSeedSearcherController.h"
#import "WonderCardSeedInspectorController.h"
#import "TrainerIDSearcherController.h"
#import "DSParameterSearcherController.h"
#import "Gen5ConfigurationEditController.h"

// BW Only
#import "EggSeedSearcherController.h"
#import "CGearSeedSearcherController.h"
#import "CGearSeedInspectorController.h"

// B2W2 Only
#import "DreamRadarSeedSearcherController.h"
#import "DreamRadarSeedInspectorController.h"
#import "HiddenHollowSpawnSeedSearcherController.h"
#import "B2W2ParameterSearcherController.h"

// Gen 4
#import "Gen4SeedSearcherController.h"
#import "Gen4EggPIDSeedSearcherController.h"
#import "Gen4EggIVSeedSearcherController.h"
#import "Gen4SeedInspectorController.h"
#import "Gen4TIDSearcherController.h"
#import "Gen4ConfigurationEditController.h"


@implementation LauncherController

- (NSString *)windowNibName
{
	return @"Launcher";
}

- (void)awakeFromNib
{
  NSString  *title = [[self window] title];
  NSString  *version = [[[NSBundle mainBundle] infoDictionary]
                        objectForKey:(NSString*)kCFBundleVersionKey];
  NSString  *newTitle = [NSString stringWithFormat: @"%@ %@", title, version];
  
  [[self window] setTitle: newTitle];
  
  buttonToClassMap =
    [NSDictionary dictionaryWithObjectsAndKeys:
      
      // Gen 5 Common
      [HashedSeedSearcherController class],
        [launchHashedSeedSearcher title],
      [StandardSeedInspectorController class],
        [launchStandardSeedInspector title],
      [WonderCardSeedSearcherController class],
        [launchWonderCardSeedSearcher title],
      [WonderCardSeedInspectorController class],
        [launchWonderCardSeedInspector title],
      [TrainerIDSearcherController class], [launchTIDSearcher title],
      [DSParameterSearcherController class], [launchDSParameterSearcher title],
      [Gen5ConfigurationEditController class],
        [launchGen5ConfigurationEdit title],
      
      // BW Only
      [EggSeedSearcherController class],
        [launchEggSeedSearcher title],
      [CGearSeedSearcherController class], [launchCGearSeedSearcher title],
      [CGearSeedInspectorController class], [launchCGearSeedInspector title],
      
      // B2W2 Only
      [DreamRadarSeedSearcherController class],
        [launchDreamRadarSeedSearcher title],
      [DreamRadarSeedInspectorController class],
        [launchDreamRadarSeedInspector title],
      [HiddenHollowSpawnSeedSearcherController class],
        [launchHiddenHollowSpawnSeedSearcher title],
      [B2W2ParameterSearcherController class],
        [launchB2W2ParameterSearcher title],
      
      // Gen 4
      [Gen4SeedSearcherController class], [launchGen4SeedSearcher title],
      [Gen4EggPIDSeedSearcherController class],
        [launchGen4EggPIDSeedSearcher title],
      [Gen4EggIVSeedSearcherController class],
        [launchGen4EggIVSeedSearcher title],
      [Gen4SeedInspectorController class], [launchGen4SeedInspector title],
      [Gen4TIDSearcherController class], [launchGen4TIDSearcher title],
      [Gen4ConfigurationEditController class],
        [launchGen4ConfigurationEdit title],
      nil];
}

- (IBAction)launch:(id)sender
{
  if (sender == launchGen5ConfigurationEdit)
  {
    Gen5ConfigurationEditController  *configController =
      [[Gen5ConfigurationEditController alloc] init];
    [configController runModal];
  }
  else if (sender == launchGen4ConfigurationEdit)
  {
    Gen4ConfigurationEditController  *configController =
      [[Gen4ConfigurationEditController alloc] init];
    [configController runModal];
  }
  else
  {
    Class  controllerClass = [buttonToClassMap objectForKey: [sender title]];
    
    if (controllerClass)
    {
      NSWindowController  *newWindowController = [[controllerClass alloc] init];
      
      if (newWindowController)
      {
        [newWindowController showWindow:self];
      };
    }
  }
}

@end
