/*
  Copyright (C) 2012 chiizu
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
#import "Gen4ConfigurationController.h"

@interface Gen4EggIVSeedSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen4ConfigurationController  *gen4ConfigController;
  IBOutlet IVParameterController        *ivParameterController;
  IBOutlet SearcherController           *searcherController;
  
  int       mode;
  
  uint32_t  minDelay, maxDelay;
  uint32_t  minFrame, maxFrame;
  
  NSNumber  *aHP, *aAT, *aDF, *aSA, *aSD, *aSP;
  NSNumber  *bHP, *bAT, *bDF, *bSA, *bSD, *bSP;
}

@property int       mode;
@property uint32_t  minDelay, maxDelay;
@property uint32_t  minFrame, maxFrame;

@property (copy)
NSNumber  *aHP, *aAT, *aDF, *aSA, *aSD, *aSP;

@property (copy)
NSNumber  *bHP, *bAT, *bDF, *bSA, *bSD, *bSP;

@end
