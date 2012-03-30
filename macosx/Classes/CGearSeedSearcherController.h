/*
  Copyright (C) 2011-2012 chiizu
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
#import "VertResizeOnlyWindowController.h"
#import "SearcherController.h"
#import "IVParameterController.h"
#import "Gen5ConfigurationController.h"

@interface CGearSeedSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  IBOutlet IVParameterController        *ivParameterController;
  
  uint32_t  minFrame, maxFrame;
  uint32_t  minDelay, maxDelay;
  
  IBOutlet SearcherController           *searcherController;
}

@property uint32_t  minFrame, maxFrame;
@property uint32_t  minDelay, maxDelay;

- (void)inspectSeed:(id)sender;

@end
