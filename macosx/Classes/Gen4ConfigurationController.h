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
#include "PPRNGTypes.h"

@interface Gen4ConfigurationController : NSObject
{
  IBOutlet id                  delegate;
  
  IBOutlet NSArrayController  *configArrayController;
  
  IBOutlet NSPopUpButton      *configMenu;
  
  IBOutlet NSTextField        *versionField;
  
  IBOutlet NSTextField        *tidField;
  IBOutlet NSTextField        *sidField;
}

- (IBAction)switchConfiguration:(id)sender;

- (IBAction)editConfigurations:(id)sender;

- (pprng::Game::Version)version;

- (uint32_t)tid;
- (uint32_t)sid;

@end