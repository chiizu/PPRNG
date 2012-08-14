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


#import <Cocoa/Cocoa.h>


@interface LauncherController : NSWindowController
{
  // Gen 5 Common
  IBOutlet NSButton  *launchHashedSeedSearcher;
  IBOutlet NSButton  *launchStandardSeedInspector;
  IBOutlet NSButton  *launchWonderCardSeedSearcher;
  IBOutlet NSButton  *launchWonderCardSeedInspector;
  IBOutlet NSButton  *launchTIDSearcher;
  IBOutlet NSButton  *launchDSParameterSearcher;
  IBOutlet NSButton  *launchGen5ConfigurationEdit;
  
  // BW Only
  IBOutlet NSButton  *launchEggSeedSearcher;
  IBOutlet NSButton  *launchCGearSeedSearcher;
  IBOutlet NSButton  *launchCGearSeedInspector;
  
  // B2W2 Only
  IBOutlet NSButton  *launchDreamRadarSeedSearcher;
  IBOutlet NSButton  *launchDreamRadarSeedInspector;
  IBOutlet NSButton  *launchHiddenHollowSpawnSeedSearcher;
  IBOutlet NSButton  *launchB2W2ParameterSearcher;
  
  // Gen 4
  IBOutlet NSButton  *launchGen4SeedSearcher;
  IBOutlet NSButton  *launchGen4EggPIDSeedSearcher;
  IBOutlet NSButton  *launchGen4EggIVSeedSearcher;
  IBOutlet NSButton  *launchGen4SeedInspector;
  IBOutlet NSButton  *launchGen4ConfigurationEdit;
  
  NSDictionary    *buttonToClassMap;
}

- (IBAction)launch:(id)sender;

@end
