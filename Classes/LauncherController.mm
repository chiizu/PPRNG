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

#import "HashedSeedSearcherController.h"
#import "HashedSeedInspectorController.h"
#import "EggSeedSearcherController.h"
#import "WonderCardSeedSearcherController.h"
#import "WonderCardSeedInspectorController.h"
#import "CGearSeedSearcherController.h"
#import "CGearSeedInspectorController.h"
#import "TrainerIDSearcherController.h"
#import "DSParameterSearcherController.h"
#import "Gen5ConfigurationEditController.h"
#import "Gen4SeedSearcherController.h"


@implementation LauncherController

- (NSString *)windowNibName
{
	return @"Launcher";
}

- (void)awakeFromNib
{
  buttonToClassMap =
    [NSDictionary dictionaryWithObjectsAndKeys:
      [HashedSeedSearcherController class],
        [launchHashedSeedSearcher title],
      [HashedSeedInspectorController class],
        [launchHashedSeedInspector title],
      [EggSeedSearcherController class],
        [launchEggSeedSearcher title],
      [WonderCardSeedSearcherController class],
        [launchWonderCardSeedSearcher title],
      [WonderCardSeedInspectorController class],
        [launchWonderCardSeedInspector title],
      [CGearSeedSearcherController class], [launchCGearSeedSearcher title],
      [CGearSeedInspectorController class], [launchCGearSeedInspector title],
      [TrainerIDSearcherController class], [launchTIDSearcher title],
      [DSParameterSearcherController class], [launchDSParameterSearcher title],
      [Gen5ConfigurationEditController class],
        [launchGen5ConfigurationEdit title],
      [Gen4SeedSearcherController class], [launchGen4SeedSearcher title],
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
