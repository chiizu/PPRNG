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


#import "ChatotPitchTransformer.h"

#include "PPRNGTypes.h"
#include "Utilities.h"

using namespace pprng;

@implementation ChatotPitchTransformer

+ (Class)transformedValueClass
{
  return [NSString class];
}

+ (BOOL)allowsReverseTransformation
{
  return NO;
}

- (id)transformedValue:(id)value
{
  if (value == nil)
    return nil;
  
  if (![value respondsToSelector: @selector(unsignedIntValue)])
  {
    [NSException raise: NSInternalInconsistencyException
                 format: @"Value (%@) does not respond to -unsignedIntValue.",
                         [value class]];
  }
  
  uint32_t  chatotPitch = [value unsignedIntValue] / 82;
  NSString  *formatString = nil;
  if (chatotPitch < 20)
    formatString = @"Lo (%.2d)";
  else if (chatotPitch < 40)
    formatString = @"LM (%.2d)";
  else if (chatotPitch < 60)
    formatString = @"Mi (%.2d)";
  else if (chatotPitch < 80)
    formatString = @"MH (%.2d)";
  else
    formatString = @"Hi (%.2d)";
  
  return [NSString stringWithFormat: formatString, chatotPitch];
}

@end
